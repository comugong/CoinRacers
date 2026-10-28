#include "CXSpeedBuffItem.h"
#include "Player/CXPawn.h"
#include "Player/CXPlayerState.h"

ACXSpeedBuffItem::ACXSpeedBuffItem()
{
	PickupEffect = nullptr;
	PickupSound = nullptr;
}

bool ACXSpeedBuffItem::ApplyItemEffect(ACXPawn* CXPawn)
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

	CXPlayerState->SetBonusMaxSpeed(1.f);

	return true;
}
