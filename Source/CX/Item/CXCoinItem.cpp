#include "CXCoinItem.h"
#include "Player/CXPawn.h"
#include "Player/CXPlayerState.h"

ACXCoinItem::ACXCoinItem()
{
	PickupEffect = nullptr;
	PickupSound = nullptr;
}

bool ACXCoinItem::ApplyItemEffect(ACXPawn* CXPawn)
{
	if (!IsValid(CXPawn))
	{
		return false;
	}

	ACXPlayerState* CXPlayerState = CXPawn->GetPlayerState<ACXPlayerState>();
	if (!IsValid(CXPlayerState))
	{
		return false;
	}

	CXPlayerState->SetCoins(10);

	return true;
}