#include "CXMineItem.h"
#include "Player/CXPawn.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

ACXMineItem::ACXMineItem()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    MineMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MineMesh"));
    MineMesh->SetupAttachment(SceneRoot);

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetupAttachment(SceneRoot);
    TriggerSphere->SetSphereRadius(150.0f);
    TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerSphere->SetCollisionObjectType(ECC_WorldDynamic);
    TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerSphere->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Overlap);

    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ACXMineItem::OnMineOverlap);
}

void ACXMineItem::OnMineOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority() || !OtherActor)
    {
        return;
    }

    // Ignore the vehicle that placed this mine.
    if (OtherActor == GetOwner())
    {
        return;
    }

    ACXPawn* HitVehicle = Cast<ACXPawn>(OtherActor);

    if (!HitVehicle)
    {
        return;
    }

    // Disable the vehicle's throttle for 2 seconds.
    HitVehicle->ApplyThrottleFailure();

    UE_LOG(LogTemplateVehicle, Warning, TEXT("Mine triggered by %s"), *HitVehicle->GetName());

    MulticastExplode();
    TriggerSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SetLifeSpan(0.1f);
}

void ACXMineItem::MulticastExplode_Implementation()
{
    if (ExplosionParticle)
    {
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            ExplosionParticle,
            GetActorTransform()
        );
    }

    if (ExplosionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            ExplosionSound,
            GetActorLocation()
        );
    }
}
