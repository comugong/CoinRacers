#include "Item/CXItemSpawner.h"

// Sets default values
ACXItemSpawner::ACXItemSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ACXItemSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	if (!HasAuthority())
	{
		return;
	}

	SpawnItem();
}

void ACXItemSpawner::SpawnItem()
{
	if (IsValid(SpawnItemClass) == false)
	{
		return;
	}

	SpawnedItem = GetWorld()->SpawnActorDeferred<AActor>(SpawnItemClass, GetActorTransform(), this);
	if (IsValid(SpawnedItem) == false)
	{
		return;
	}

	SpawnedItem->OnDestroyed.AddDynamic(this, &ACXItemSpawner::OnItemDestroyed);
	SpawnedItem->FinishSpawning(GetActorTransform());
}

void ACXItemSpawner::OnItemDestroyed(AActor* DestroyedActor)
{
	if (HasAuthority() == false)
	{
		return;
	}
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ACXItemSpawner::SpawnItem, SpawnInterval, false);
}
