#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CXPlayerState.generated.h"

UCLASS()
class CX_API ACXPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	int32 GetCoins() const { return Coins; }
	float GetBonusMaxSpeed() const { return BonusMaxSpeed; }
	float GetMaxSpeed(float BaseMaxSpeed) const { return BaseMaxSpeed + BonusMaxSpeed; }
	
	void SetCoins(int32 Amount);
	void SetBonusMaxSpeed(float Amount);

	UFUNCTION(Server, Reliable)
	void ServerSetCoins(int32 Amount);

	UFUNCTION(Server, Reliable)
	void ServerSetBonusMaxSpeed(float Amount);

	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	FString GetPlayerInfo() const;

protected:
	UPROPERTY(Replicated)
	int32 Coins = 0;

	UPROPERTY(Replicated)
	float BonusMaxSpeed = 0.f;
};
