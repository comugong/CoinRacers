#include "CXBaseItem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Player/CXPawn.h"
#include "TimerManager.h"

ACXBaseItem::ACXBaseItem()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	MinNetUpdateFrequency = 1.0f;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(SceneRoot);

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SphereCollision->SetupAttachment(SceneRoot);
	SphereCollision->SetSphereRadius(100.0f);
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollision->SetCollisionObjectType(ECC_WorldDynamic);
	SphereCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereCollision->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Overlap);
	SphereCollision->SetGenerateOverlapEvents(true);

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(SceneRoot);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACXBaseItem::BeginPlay()
{
	Super::BeginPlay();

	InitialLocation = GetActorLocation();
	InitialRotation = GetActorRotation();

	if (HasAuthority())
	{
		SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &ACXBaseItem::OnSphereBeginOverlap);
	}
}

void ACXBaseItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RunningTime += DeltaTime;
	const float ZOffset = FMath::Sin(RunningTime * BobbingSpeed) * BobbingAmplitude;

	FVector NewLocation = InitialLocation;
	NewLocation.Z += ZOffset;
	SetActorLocation(NewLocation);

	FRotator NewRotation = InitialRotation;
	NewRotation.Yaw += RunningTime * RotationSpeed;
	SetActorRotation(NewRotation);
}

void ACXBaseItem::OnSphereBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	ACXPawn* CXPawn = Cast<ACXPawn>(OtherActor);
	if (!CXPawn)
	{
		return;
	}

	if (!ApplyItemEffect(CXPawn))
	{
		return;
	}

	MulticastMakeEffects();
	Destroy();
}

bool ACXBaseItem::ApplyItemEffect(ACXPawn* CXPawn)
{
	return false;
}

void ACXBaseItem::MulticastMakeEffects_Implementation()
{
	if (IsValid(PickupEffect))
	{
		UParticleSystemComponent* EffectComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PickupEffect, GetActorLocation(), GetActorRotation());

		if (IsValid(EffectComponent))
		{
			EffectComponent->bAutoDestroy = true;

			TWeakObjectPtr<UParticleSystemComponent> WeakEffectComponent = EffectComponent;

			FTimerHandle EffectTimerHandle;

			GetWorld()->GetTimerManager().SetTimer(
				EffectTimerHandle,
				[WeakEffectComponent]()
				{
					if (WeakEffectComponent.IsValid())
					{
						WeakEffectComponent->DestroyComponent();
					}
				},
				1.5f,
				false);
		}
	}

	if (IsValid(PickupSound))
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			PickupSound,
			GetActorLocation());
	}
}