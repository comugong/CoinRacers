#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CXGameState.generated.h"

class ACXPlayerState;

UENUM(BlueprintType)
enum class EGameState : uint8
{
	Waiting,
	Playing,
	Finished
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnRemainingTimeChanged, int32);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnGameStateChanged, EGameState);
DECLARE_MULTICAST_DELEGATE(FOnWinnersChanged);

UCLASS()
class CX_API ACXGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	EGameState GetCurrentGameState() const { return CurrentGameState; }
	int32 GetRemainingTime() const { return RemainingTime; }

	const TArray<TObjectPtr<ACXPlayerState>>& GetWinners() const
	{
		return Winners;
	}

	void SetCurrentGameState(EGameState NewState);
	void SetRemainingTime(int32 NewRemainingTime);
	void SetWinners(TArray<TObjectPtr<ACXPlayerState>> NewWinners);

	FOnRemainingTimeChanged OnRemainingTimeChanged;
	FOnGameStateChanged OnGameStateChanged;
	FOnWinnersChanged OnWinnersChanged;

protected:
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_CurrentGameState();

	UFUNCTION()
	void OnRep_RemainingTime();

	UFUNCTION()
	void OnRep_Winners();

protected:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentGameState)
	EGameState CurrentGameState = EGameState::Waiting;

	UPROPERTY(ReplicatedUsing = OnRep_RemainingTime)
	int32 RemainingTime = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Winners)
	TArray<TObjectPtr<ACXPlayerState>> Winners;
};
