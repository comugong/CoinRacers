#include "CXPlayerState.h"
#include "CXPawn.h"
#include "Net/UnrealNetwork.h"

void ACXPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ACXPlayerState, Coins, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ACXPlayerState, BonusMaxSpeed, COND_OwnerOnly);
}

void ACXPlayerState::SetCoins(int32 Amount)
{
	Coins += Amount;
	UE_LOG(LogTemp, Log, TEXT("SetCoins: Player %s now has %d coins."), *GetPlayerName(), Coins);
}

void ACXPlayerState::SetBonusMaxSpeed(float Amount)
{
	float RealAmount = Amount * 30.0f;
	BonusMaxSpeed += RealAmount;
	BonusMaxSpeed = FMath::Max(0.0f, BonusMaxSpeed);

	if (ACXPawn* CXPawn = Cast<ACXPawn>(GetPawn()))
	{
		CXPawn->SetBonusMaxSpeed(RealAmount);
	}

	UE_LOG(LogTemp, Log, TEXT("SetBonusMaxSpeed: Player %s now has %.2f bonus max speed."), *GetPlayerName(), BonusMaxSpeed);
}

void ACXPlayerState::ServerSetCoins_Implementation(int32 Amount)
{
	if (HasAuthority() == false)
	{
		return;
	}

	SetCoins(Amount);
}

void ACXPlayerState::ServerSetBonusMaxSpeed_Implementation(float Amount)
{
	if (HasAuthority() == false)
	{
		return;
	}

	SetBonusMaxSpeed(Amount);
}

FString ACXPlayerState::GetPlayerInfo() const
{
	return FString::Printf(TEXT("Player: %s, Coins: %d, BonusMaxSpeed: %.2f"), *GetPlayerName(), Coins, BonusMaxSpeed);
}
