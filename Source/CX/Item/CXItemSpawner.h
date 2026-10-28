#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CXItemSpawner.generated.h"

UCLASS()
class CX_API ACXItemSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACXItemSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void SpawnItem();

	UFUNCTION()
	void OnItemDestroyed(AActor* DestroyedActor);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Spawn)
	TSubclassOf<AActor> SpawnItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Spawn)
	float SpawnInterval = 5.f;

	UPROPERTY()
	TObjectPtr<AActor> SpawnedItem;

	FTimerHandle SpawnTimerHandle;
};
