#pragma once

#include "CoreMinimal.h"
#include "Item/CXBaseItem.h"
#include "CXSpeedBuffItem.generated.h"

UCLASS()
class CX_API ACXSpeedBuffItem : public ACXBaseItem
{
	GENERATED_BODY()
	
public:
	ACXSpeedBuffItem();

protected:
	virtual bool ApplyItemEffect(ACXPawn* CXPawn) override;
};
