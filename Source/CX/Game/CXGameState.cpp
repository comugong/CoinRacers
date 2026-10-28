#include "CXGameState.h"
#include "Net/UnrealNetwork.h"
#include "Player/CXPlayerState.h"

void ACXGameState::SetCurrentGameState(EGameState NewState)
{
	if (!HasAuthority())
	{
		return;
	}

	if (CurrentGameState == NewState)
	{
		return;
	}

	CurrentGameState = NewState;

	OnGameStateChanged.Broadcast(CurrentGameState);

	ForceNetUpdate();
}

void ACXGameState::SetRemainingTime(int32 NewRemainingTime)
{
	if (!HasAuthority())
	{
		return;
	}

	const int32 ClampedTime = FMath::Max(0, NewRemainingTime);

	if (RemainingTime == ClampedTime)
	{
		return;
	}

	RemainingTime = ClampedTime;

	OnRemainingTimeChanged.Broadcast(RemainingTime);

	ForceNetUpdate();
}

void ACXGameState::SetWinners(
	TArray<TObjectPtr<ACXPlayerState>> NewWinners)
{
	if (!HasAuthority())
	{
		return;
	}

	Winners = MoveTemp(NewWinners);

	OnWinnersChanged.Broadcast();

	ForceNetUpdate();
}

void ACXGameState::OnRep_CurrentGameState()
{
	OnGameStateChanged.Broadcast(CurrentGameState);
}

void ACXGameState::OnRep_RemainingTime()
{
	OnRemainingTimeChanged.Broadcast(RemainingTime);
}

void ACXGameState::OnRep_Winners()
{
	OnWinnersChanged.Broadcast();
}

void ACXGameState::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACXGameState, CurrentGameState);
	DOREPLIFETIME(ACXGameState, RemainingTime);
	DOREPLIFETIME(ACXGameState, Winners);
}
