#include "CXPawn.h"
#include "CXWheelFront.h"
#include "CXWheelRear.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "CXPlayerState.h"
#include "Engine/DamageEvents.h"
#include "Net/UnrealNetwork.h"
#include "Item/CXMineItem.h"
#include "Game/CXGameState.h"

#define LOCTEXT_NAMESPACE "VehiclePawn"

DEFINE_LOG_CATEGORY(LogTemplateVehicle);

ACXPawn::ACXPawn()
{
	// construct the front camera boom
	FrontSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Front Spring Arm"));
	FrontSpringArm->SetupAttachment(GetMesh());
	FrontSpringArm->TargetArmLength = 0.0f;
	FrontSpringArm->bDoCollisionTest = false;
	FrontSpringArm->bEnableCameraRotationLag = true;
	FrontSpringArm->CameraRotationLagSpeed = 15.0f;
	FrontSpringArm->SetRelativeLocation(FVector(30.0f, 0.0f, 120.0f));

	FrontCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Front Camera"));
	FrontCamera->SetupAttachment(FrontSpringArm);
	FrontCamera->bAutoActivate = false;

	// construct the back camera boom
	BackSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Back Spring Arm"));
	BackSpringArm->SetupAttachment(GetMesh());
	BackSpringArm->TargetArmLength = 650.0f;
	BackSpringArm->SocketOffset.Z = 150.0f;
	BackSpringArm->bDoCollisionTest = false;
	BackSpringArm->bInheritPitch = false;
	BackSpringArm->bInheritRoll = false;
	BackSpringArm->bEnableCameraRotationLag = true;
	BackSpringArm->CameraRotationLagSpeed = 2.0f;
	BackSpringArm->CameraLagMaxDistance = 50.0f;

	BackCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Back Camera"));
	BackCamera->SetupAttachment(BackSpringArm);

	// Configure the car mesh
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(FName("Vehicle"));

	// get the Chaos Wheeled movement component
	ChaosVehicleMovement = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());

	bReplicates = true;
}

void ACXPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// steering 
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &ACXPawn::Steering);
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Completed, this, &ACXPawn::Steering);

		// throttle 
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &ACXPawn::Throttle);
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &ACXPawn::Throttle);

		// break 
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &ACXPawn::Brake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Started, this, &ACXPawn::StartBrake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Completed, this, &ACXPawn::StopBrake);

		// handbrake 
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &ACXPawn::StartHandbrake);
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &ACXPawn::StopHandbrake);

		// look around 
		EnhancedInputComponent->BindAction(LookAroundAction, ETriggerEvent::Triggered, this, &ACXPawn::LookAround);

		// toggle camera 
		EnhancedInputComponent->BindAction(ToggleCameraAction, ETriggerEvent::Triggered, this, &ACXPawn::ToggleCamera);

		// reset the vehicle 
		EnhancedInputComponent->BindAction(ResetVehicleAction, ETriggerEvent::Triggered, this, &ACXPawn::ResetVehicle);

		// attack
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ACXPawn::Attack);

		// place mine
		EnhancedInputComponent->BindAction(PlaceMineAction, ETriggerEvent::Started, this, &ACXPawn::PlaceMine);
	}
	else
	{
		UE_LOG(LogTemplateVehicle, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ACXPawn::Tick(float Delta)
{
	Super::Tick(Delta);

	// Add some angular damping if the vehicle is in midair.
	bool bMovingOnGround = ChaosVehicleMovement->IsMovingOnGround();
	GetMesh()->SetAngularDamping(bMovingOnGround ? 0.0f : 3.0f);

	// Recover speed penalty over time on the server.
	if (HasAuthority() && SpeedPenalty > 0.0f)
	{
		SpeedPenalty = FMath::Max(0.0f, SpeedPenalty - SpeedPenaltyRecoveryRate * Delta);
	}

	// Limit vehicle speed using the base speed and collected bonus speed.
	const FVector CurrentVelocity = GetMesh()->GetPhysicsLinearVelocity();
	const float CurrentSpeed = CurrentVelocity.Size();
	const float MaxSpeed = GetMaxSpeed();

	if (CurrentSpeed > MaxSpeed)
	{
		GetMesh()->SetPhysicsLinearVelocity(CurrentVelocity.GetSafeNormal() * MaxSpeed);
	}

	// Realign the camera yaw to face front.
	float CameraYaw = BackSpringArm->GetRelativeRotation().Yaw;
	CameraYaw = FMath::FInterpTo(CameraYaw, 0.0f, Delta, 1.0f);

	BackSpringArm->SetRelativeRotation(FRotator(0.0f, CameraYaw, 0.0f));
}

float ACXPawn::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	ACXPawn* Attacker = Cast<ACXPawn>(DamageCauser);

	if (HasAuthority() == false || DamageCauser == this || ActualDamage < 0.f || Attacker == nullptr)
	{
		return 0.f;
	}

	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const FPointDamageEvent& PointDamageEvent = static_cast<const FPointDamageEvent&>(DamageEvent);
		const FName HitBoneName = PointDamageEvent.HitInfo.BoneName;

		if (IsWheelBone(HitBoneName))
		{
			UE_LOG(LogTemplateVehicle, Warning, TEXT("Wheel hit! Bone: %s"), *HitBoneName.ToString());
			SpeedPenalty = FMath::Clamp(SpeedPenalty + 300.0f, 0.0f, 1000.0f);
		}
	}

	ACXPlayerState* AttackerPlayerState = Cast<ACXPlayerState>(Attacker->GetPlayerState());
	ACXPlayerState* VictimPlayerState = Cast<ACXPlayerState>(GetPlayerState());

	if (AttackerPlayerState == nullptr || VictimPlayerState == nullptr)
	{
		return ActualDamage;
	}

	if (VictimPlayerState->GetCoins() >= 5)
	{
		UE_LOG(LogTemplateVehicle, Error, TEXT("Player %s got 5 coins from Player %s"), *AttackerPlayerState->GetPlayerName(), *VictimPlayerState->GetPlayerName());
		VictimPlayerState->ServerSetCoins(-5);
		AttackerPlayerState->ServerSetCoins(5);
	}
	else
	{
		UE_LOG(LogTemplateVehicle, Error, TEXT("Player %s does not have enough coins to give (needs 5)"), *VictimPlayerState->GetPlayerName());
	}

	return ActualDamage;
}

void ACXPawn::MulticastHitEffect_Implementation(const FVector& Start, const FVector& End, bool bHitVehicle)
{
	if (HasAuthority())
	{
		return;
	}

	DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 2.0f, 0, 5.0f);

	if (bHitVehicle)
	{
		// Play hit effect on all clients
		if (HitParticle)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle, End);
		}
	}
}

void ACXPawn::Steering(const FInputActionValue& Value)
{
	if (!IsGamePlaying())
	{
		ChaosVehicleMovement->SetSteeringInput(0.0f);
		return;
	}

	// get the input magnitude for steering
	float SteeringValue = Value.Get<float>();

	// add the input
	ChaosVehicleMovement->SetSteeringInput(SteeringValue);
}

void ACXPawn::Throttle(const FInputActionValue& Value)
{
	// get the input magnitude for the throttle
	const float ThrottleValue = Value.Get<float>();

	if (!IsGamePlaying())
	{
		ChaosVehicleMovement->SetThrottleInput(0.0f);
		return;
	}

	if (bThrottleDisabled)
	{
		ChaosVehicleMovement->SetThrottleInput(0.0f);

		return;
	}

	// add the input
	ChaosVehicleMovement->SetThrottleInput(ThrottleValue);
}

void ACXPawn::Brake(const FInputActionValue& Value)
{
	if (!IsGamePlaying())
	{
		ChaosVehicleMovement->SetBrakeInput(0.0f);
		return;
	}

	const float BrakeValue = Value.Get<float>();
	ChaosVehicleMovement->SetBrakeInput(BrakeValue);
}

void ACXPawn::StartBrake(const FInputActionValue& Value)
{
	// call the Blueprint hook for the break lights
	BrakeLights(true);
}

void ACXPawn::StopBrake(const FInputActionValue& Value)
{
	// call the Blueprint hook for the break lights
	BrakeLights(false);

	// reset brake input to zero
	ChaosVehicleMovement->SetBrakeInput(0.0f);
}

void ACXPawn::StartHandbrake(const FInputActionValue& Value)
{
	// add the input
	ChaosVehicleMovement->SetHandbrakeInput(true);

	// call the Blueprint hook for the break lights
	BrakeLights(true);
}

void ACXPawn::StopHandbrake(const FInputActionValue& Value)
{
	if (bGameStateHandbrakeLocked || !IsGamePlaying())
	{
		ChaosVehicleMovement->SetHandbrakeInput(true);
		BrakeLights(true);
		return;
	}

	ChaosVehicleMovement->SetHandbrakeInput(false);
	BrakeLights(false);
}

void ACXPawn::LookAround(const FInputActionValue& Value)
{
	// get the flat angle value for the input 
	float LookValue = Value.Get<float>();

	// add the input
	BackSpringArm->AddLocalRotation(FRotator(0.0f, LookValue, 0.0f));
}

void ACXPawn::ToggleCamera(const FInputActionValue& Value)
{
	// toggle the active camera flag
	bFrontCameraActive = !bFrontCameraActive;

	FrontCamera->SetActive(bFrontCameraActive);
	BackCamera->SetActive(!bFrontCameraActive);
}

void ACXPawn::ResetVehicle(const FInputActionValue& Value)
{
	if (!IsGamePlaying())
	{
		return;
	}

	// reset to a location slightly above our current one
	FVector ResetLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);

	// reset to our yaw. Ignore pitch and roll
	FRotator ResetRotation = GetActorRotation();
	ResetRotation.Pitch = 0.0f;
	ResetRotation.Roll = 0.0f;
	
	// teleport the actor to the reset spot and reset physics
	SetActorTransform(FTransform(ResetRotation, ResetLocation, FVector::OneVector), false, nullptr, ETeleportType::TeleportPhysics);

	GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);

	UE_LOG(LogTemplateVehicle, Error, TEXT("Reset Vehicle"));
}

void ACXPawn::Attack(const FInputActionValue& Value)
{
	if (!IsGamePlaying())
	{
		return;
	}

	ServerAttack();
}

void ACXPawn::PlaceMine(const FInputActionValue& Value)
{
	if (!IsGamePlaying())
	{
		return;
	}

	ServerPlaceMine();
}

void ACXPawn::ServerPlaceMine_Implementation()
{
	if (!IsGamePlaying())
	{
		return;
	}

	UE_LOG(LogTemplateVehicle, Warning, TEXT("PlaceMine"));

	if (!HasAuthority() || !MineClass)
	{
		return;
	}

	const FVector SpawnLocation = GetActorLocation() - GetActorForwardVector() * 275.0f;
	const FRotator SpawnRotation = GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ACXMineItem* SpawnedMine = GetWorld()->SpawnActor<ACXMineItem>(MineClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (SpawnedMine)
	{
		UE_LOG(LogTemplateVehicle, Warning, TEXT("Mine spawned by %s"), *GetName());
	}

}

void ACXPawn::ServerAttack_Implementation()
{
	if (!IsGamePlaying())
	{
		return;
	}

	UE_LOG(LogTemplateVehicle, Error, TEXT("Attack"));

	FVector Start = GetActorLocation();
	FVector End = Start + GetActorForwardVector() * 1000.0f;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FHitResult HitResult;
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);
	bool bHitVehicle = false;

	if (bHit)
	{
		End = HitResult.Location;
		UE_LOG(LogTemplateVehicle, Error, TEXT("Hit: %s"), *HitResult.GetActor()->GetName());

		AActor* HitActor = HitResult.GetActor();
		if (ACXPawn* CXPawn = Cast<ACXPawn>(HitActor))
		{
			const FVector ShotDirection = (HitResult.ImpactPoint - Start).GetSafeNormal();

			UGameplayStatics::ApplyPointDamage(
				CXPawn,
				1.f,
				ShotDirection,
				HitResult,
				GetController(),
				this,
				UDamageType::StaticClass()
			);

			bHitVehicle = true;
		}
	}
	else
	{
		UE_LOG(LogTemplateVehicle, Error, TEXT("Miss"));
	}

	MulticastHitEffect(Start, End, bHitVehicle);
}

bool ACXPawn::IsWheelBone(FName BoneName) const
{
	static const TSet<FName> WheelBoneNames =
	{
		FName(TEXT("Phys_Wheel_BL")),
		FName(TEXT("Phys_Wheel_BR")),
		FName(TEXT("Phys_Wheel_FL")),
		FName(TEXT("Phys_Wheel_FR"))
	};

	return WheelBoneNames.Contains(BoneName);
}

void ACXPawn::OnRep_ThrottleDisabled()
{
	if (bThrottleDisabled)
	{
		ChaosVehicleMovement->SetThrottleInput(0.0f);
	}
}

void ACXPawn::RecoverThrottle()
{
	if (!HasAuthority())
	{
		return;
	}

	bThrottleDisabled = false;

	UE_LOG(LogTemplateVehicle, Warning, TEXT("Throttle recovered: %s"), *GetName());
}

bool ACXPawn::IsGamePlaying() const
{
	const ACXGameState* CXGameState = GetWorld()->GetGameState<ACXGameState>();

	return CXGameState && CXGameState->GetCurrentGameState() == EGameState::Playing;
}

void ACXPawn::SetBonusMaxSpeed(float Amount)
{
	BonusMaxSpeed += Amount;
	BonusMaxSpeed = FMath::Max(0.0f, BonusMaxSpeed);
}

float ACXPawn::GetMaxSpeed() const
{
	return FMath::Max(0.0f, BaseMaxSpeed + BonusMaxSpeed - SpeedPenalty);
}

void ACXPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACXPawn, SpeedPenalty);
	DOREPLIFETIME(ACXPawn, bThrottleDisabled);
}

void ACXPawn::ApplyThrottleFailure()
{
	if (!HasAuthority())
	{
		return;
	}

	bThrottleDisabled = true;

	// Immediately stop applying throttle.
	ChaosVehicleMovement->SetThrottleInput(0.0f);

	// Reset the recovery timer if another mine triggers.
	GetWorldTimerManager().ClearTimer(ThrottleRecoveryTimer);

	GetWorldTimerManager().SetTimer(
		ThrottleRecoveryTimer,
		this,
		&ACXPawn::RecoverThrottle,
		2.0f,
		false
	);

	// Notify the owning client immediately.
	OnRep_ThrottleDisabled();
}

#undef LOCTEXT_NAMESPACE