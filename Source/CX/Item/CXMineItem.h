#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CXMineItem.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class USphereComponent;
class UPrimitiveComponent;
class AActor;
class UParticleSystem;
class USoundBase;

UCLASS()
class CX_API ACXMineItem : public AActor
{
    GENERATED_BODY()

public:
    ACXMineItem();

protected:
    UFUNCTION()
    void OnMineOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

    UFUNCTION(NetMulticast, Reliable)
    void MulticastExplode();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mine")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mine")
    TObjectPtr<UStaticMeshComponent> MineMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mine")
    TObjectPtr<USphereComponent> TriggerSphere;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mine|Effects")
    TObjectPtr<UParticleSystem> ExplosionParticle;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mine|Effects")
    TObjectPtr<USoundBase> ExplosionSound;
};
