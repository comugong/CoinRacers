#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CXGameMode.generated.h"

UCLASS(MinimalAPI)
class ACXGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACXGameMode();

	virtual void BeginPlay() override;

protected:
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

private:
	bool IsPlayerStartAvailable(const APlayerStart* PlayerStart) const;

	void DecreaseWaitingTime();

	void DecreasePlayingTime();

	void FinishGame();

protected:
	UPROPERTY(EditDefaultsOnly)
	FVector VehicleSpawnCheckHalfExtent = FVector(250.0, 120.0, 80.0);

	UPROPERTY(EditDefaultsOnly)
	FVector VehicleSpawnCheckOffset = FVector(0.0, 0.0, 80.0);

	bool bHasFirstPlayerJoined = false;

	FTimerHandle CountdownTimerHandle;
};
