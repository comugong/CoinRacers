#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ChatSubsystem.generated.h"

USTRUCT()
struct FChatData
{
	GENERATED_BODY()

	UPROPERTY()
	FString Sender;
	UPROPERTY()
	FString Message;

	FChatData() = default;
	FChatData(const FString& InSender, const FString& InMessage)
		: Sender(InSender), Message(InMessage)
	{
	}
};

UCLASS()
class CX_API UChatSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void AddChatMessage(const FString& InSender, const FString& InMessage);

	const TArray<FChatData>& GetChatLog() const { return ChatLog; }

protected:
	TArray<FChatData> ChatLog;
	
	static inline int32 MaxChatLog = 10;
};
