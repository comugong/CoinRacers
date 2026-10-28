#pragma once

#include "CoreMinimal.h"
#include "Item/CXBaseItem.h"
#include "CXCoinItem.generated.h"

UCLASS()
class CX_API ACXCoinItem : public ACXBaseItem
{
	GENERATED_BODY()

public:
	ACXCoinItem();

protected:
	virtual bool ApplyItemEffect(ACXPawn* CXPawn) override;
};