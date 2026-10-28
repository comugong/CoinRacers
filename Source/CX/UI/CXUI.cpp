#include "CXUI.h"
#include "ChatUI.h"

void UCXUI::OnNewChatMessage(
	const FString& InSender,
	const FString& InMessage)
{
	if (IsValid(UI_Chat))
	{
		UI_Chat->OnNewChatMesssage(InSender, InMessage);
	}
}

void UCXUI::UpdateSpeed(float NewSpeed)
{
	// format the speed to KPH or MPH
	float FormattedSpeed =
		FMath::Abs(NewSpeed) * (bIsMPH ? 0.022f : 0.036f);

	// call the Blueprint handler
	OnSpeedUpdate(FormattedSpeed);
}

void UCXUI::UpdateGear(int32 NewGear)
{
	// call the Blueprint handler
	OnGearUpdate(NewGear);
}

void UCXUI::UpdateRemainingTime(int32 RemainingTime)
{
	const FText TimeText = FText::FromString(
		FString::Printf(TEXT("%ds"), RemainingTime)
	);

	OnRemainingTimeUpdate(TimeText);
}

void UCXUI::UpdateGameStatus(const FString& Status)
{
	OnGameStatusUpdate(FText::FromString(Status));
}

void UCXUI::ShowGameResult(bool bIsWinner)
{
	OnGameResultUpdate(bIsWinner);
}
