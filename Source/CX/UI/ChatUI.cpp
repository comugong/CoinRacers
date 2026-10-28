#include "ChatUI.h"
#include "Player/CXPlayerController.h"

void UChatUI::OnNewChatMesssage(const FString& InSender, const FString& InMessage)
{
	FString NewMessage = FString::Printf(TEXT("%s: %s"), *InSender, *InMessage);
	FString OldMessage = MultiLineEditableTextBox_ChatLog->GetText().ToString();
	OldMessage.Append(TEXT("\n"));
	FText Message = FText::FromString(OldMessage + NewMessage);

	MultiLineEditableTextBox_ChatLog->SetText(Message);
}

void UChatUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (EditableTextBox_ChatInput->OnTextCommitted.IsAlreadyBound(this, &UChatUI::OnChatInputTextCommitted) == false)
	{
		EditableTextBox_ChatInput->OnTextCommitted.AddDynamic(this, &UChatUI::OnChatInputTextCommitted);
	}
}

void UChatUI::NativeDestruct()
{
	if (EditableTextBox_ChatInput->OnTextCommitted.IsAlreadyBound(this, &UChatUI::OnChatInputTextCommitted) == true)
	{
		EditableTextBox_ChatInput->OnTextCommitted.RemoveDynamic(this, &UChatUI::OnChatInputTextCommitted);
	}

	Super::NativeDestruct();
}

void UChatUI::OnChatInputTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		if (!Text.IsEmpty())
		{
			FString Message = Text.ToString();
			EditableTextBox_ChatInput->SetText(FText::GetEmpty());

			if (APlayerController* PC = GetOwningPlayer())
			{
				if (ACXPlayerController* CXPC = Cast<ACXPlayerController>(PC))
				{
					CXPC->ServerChatMessage(Message);
				}
			}
		}
	}
}
