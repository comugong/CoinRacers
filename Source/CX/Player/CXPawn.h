#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "CXPawn.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class UChaosWheeledVehicleMovementComponent;
class UParticleSystem;
class ACXMineItem;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateVehicle, Log, All);

/**
 *  Vehicle Pawn class
 *  Handles common functionality for all vehicle types,
 *  including input handling and camera management.
 *  
 *  Specific vehicle configurations are handled in subclasses.
 */
UCLASS(abstract)
class ACXPawn : public AWheeledVehiclePawn
{
	GENERATED_BODY()

	/** Spring Arm for the front camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* FrontSpringArm;

	/** Front Camera component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FrontCamera;

	/** Spring Arm for the back camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* BackSpringArm;

	/** Back Camera component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* BackCamera;

	/** Cast pointer to the Chaos Vehicle movement component */
	TObjectPtr<UChaosWheeledVehicleMovementComponent> ChaosVehicleMovement;

protected:
	/** Steering Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* SteeringAction;

	/** Throttle Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* ThrottleAction;

	/** Brake Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* BrakeAction;

	/** Handbrake Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* HandbrakeAction;

	/** Look Around Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* LookAroundAction;

	/** Toggle Camera Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* ToggleCameraAction;

	/** Reset Vehicle Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* ResetVehicleAction;

	/** Attack Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* AttackAction;

	/** Place Mine Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* PlaceMineAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mine")
	TSubclassOf<ACXMineItem> MineClass;

	/** Keeps track of which camera is active */
	bool bFrontCameraActive = false;

	/** Particle System for the attack effect */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Effects)
	UParticleSystem* HitParticle;

	/** Vehicle's base maximum speed in centimeters per second */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement)
	float BaseMaxSpeed = 1500.0f;

	/** Additional maximum speed granted by item buffs */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	float BonusMaxSpeed = 0.0f;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	float SpeedPenalty = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement)
	float SpeedPenaltyRecoveryRate = 100.0f;

	/** Whether the throttle is disabled by a mine */
	UPROPERTY(ReplicatedUsing = OnRep_ThrottleDisabled, VisibleAnywhere, BlueprintReadOnly, Category = "Mine")
	bool bThrottleDisabled = false;

	FTimerHandle ThrottleRecoveryTimer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bGameStateHandbrakeLocked = true;

public:
	ACXPawn();

	// Begin Pawn interface

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	// End Pawn interface

	// Begin Actor interface

	virtual void Tick(float Delta) override;

	// End Actor interface

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastHitEffect(const FVector& Start, const FVector& End, bool bHitVehicle);

	/** Applies an additional maximum speed bonus to this vehicle */
	void SetBonusMaxSpeed(float Amount);

	/** Returns the vehicle's current maximum speed */
	float GetMaxSpeed() const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Disables throttle input temporarily */
	void ApplyThrottleFailure();

protected:
	/** Handles steering input */
	void Steering(const FInputActionValue& Value);

	/** Handles throttle input */
	void Throttle(const FInputActionValue& Value);

	/** Handles brake input */
	void Brake(const FInputActionValue& Value);

	/** Handles brake start/stop inputs */
	void StartBrake(const FInputActionValue& Value);
	void StopBrake(const FInputActionValue& Value);

	/** Handles handbrake start/stop inputs */
	void StartHandbrake(const FInputActionValue& Value);
	void StopHandbrake(const FInputActionValue& Value);

	/** Handles look around input */
	void LookAround(const FInputActionValue& Value);

	/** Handles toggle camera input */
	void ToggleCamera(const FInputActionValue& Value);

	/** Handles reset vehicle input */
	void ResetVehicle(const FInputActionValue& Value);

	/** Called when the brake lights are turned on or off */
	UFUNCTION(BlueprintImplementableEvent, Category="Vehicle")
	void BrakeLights(bool bBraking);

	/** Handles attack input */
	void Attack(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable)
	void ServerAttack();

	/** Handles mine placement input */
	void PlaceMine(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable)
	void ServerPlaceMine();

	/** Checks whether the hit bone belongs to a wheel */
	bool IsWheelBone(FName BoneName) const;

	/** Called when throttle-disabled state is replicated */
	UFUNCTION()
	void OnRep_ThrottleDisabled();

	/** Restores throttle input after the failure duration */
	void RecoverThrottle();

	bool IsGamePlaying() const;

public:
	/** Returns the front spring arm subobject */
	FORCEINLINE USpringArmComponent* GetFrontSpringArm() const { return FrontSpringArm; }
	/** Returns the front camera subobject */
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FrontCamera; }
	/** Returns the back spring arm subobject */
	FORCEINLINE USpringArmComponent* GetBackSpringArm() const { return BackSpringArm; }
	/** Returns the back camera subobject */
	FORCEINLINE UCameraComponent* GetBackCamera() const { return BackCamera; }
	/** Returns the cast Chaos Vehicle Movement subobject */
	FORCEINLINE const TObjectPtr<UChaosWheeledVehicleMovementComponent>& GetChaosVehicleMovement() const { return ChaosVehicleMovement; }
};
