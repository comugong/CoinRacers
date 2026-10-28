#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CXUI.generated.h"

class UChatUI;

/**
 * Simple Vehicle HUD class.
 * Displays speed, gear, remaining time, and game result.
 */
UCLASS(abstract)
class CX_API UCXUI : public UUserWidget
{
	GENERATED_BODY()

#pragma region ChatUI
public:
	void OnNewChatMessage(const FString& InSender, const FString& InMessage);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UChatUI> UI_Chat;
#pragma endregion

protected:
	/** Controls the display of speed in Km/h or MPH */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Vehicle)
	bool bIsMPH = false;

public:
	/** Called to update the speed display */
	void UpdateSpeed(float NewSpeed);

	/** Called to update the gear display */
	void UpdateGear(int32 NewGear);

	/** Called to update the remaining time display */
	void UpdateRemainingTime(int32 RemainingTime);

	/** Called to update the game state display */
	void UpdateGameStatus(const FString& Status);

	/** Called to display the local player's game result */
	void ShowGameResult(bool bIsWinner);

protected:
	/** Implemented in Blueprint to display the new speed */
	UFUNCTION(BlueprintImplementableEvent, Category = Vehicle)
	void OnSpeedUpdate(float NewSpeed);

	/** Implemented in Blueprint to display the new gear */
	UFUNCTION(BlueprintImplementableEvent, Category = Vehicle)
	void OnGearUpdate(int32 NewGear);

	/** Implemented in Blueprint to display the remaining time */
	UFUNCTION(BlueprintImplementableEvent, Category = Game)
	void OnRemainingTimeUpdate(const FText& TimeText);

	/** Implemented in Blueprint to display the game state */
	UFUNCTION(BlueprintImplementableEvent, Category = Game)
	void OnGameStatusUpdate(const FText& StatusText);

	/** Implemented in Blueprint to display the game result */
	UFUNCTION(BlueprintImplementableEvent, Category = Game)
	void OnGameResultUpdate(bool bIsWinner);
};
