#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CXBaseItem.generated.h"

class USceneComponent;
class UPrimitiveComponent;
class UStaticMeshComponent;
class USphereComponent;
class UParticleSystem;
class USoundBase;
class ACXPawn;

UCLASS(Abstract)
class CX_API ACXBaseItem : public AActor
{
	GENERATED_BODY()

public:
	ACXBaseItem();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnSphereBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastMakeEffects();

	virtual bool ApplyItemEffect(ACXPawn* CXPawn);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UStaticMeshComponent> ItemMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<USphereComponent> SphereCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Effect")
	TObjectPtr<UParticleSystem> PickupEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Effect")
	TObjectPtr<USoundBase> PickupSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Movement")
	float BobbingAmplitude = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Movement")
	float BobbingSpeed = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Movement")
	float RotationSpeed = 90.0f;

private:
	FVector InitialLocation;
	FRotator InitialRotation;
	float RunningTime = 0.0f;
};