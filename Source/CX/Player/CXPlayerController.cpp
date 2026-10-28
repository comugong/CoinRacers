#include "CXPlayerController.h"
#include "CXPawn.h"
#include "EnhancedInputSubsystems.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Game/CXGameState.h"
#include "GameFramework/PlayerState.h"
#include "Subsystem/ChatSubsystem.h"
#include "CXPlayerState.h"
#include "UI/CXUI.h"
#include "EngineUtils.h"

void ACXPlayerController::ServerRequestChatLog_Implementation()
{
	if (UChatSubsystem* ChatSubsystem = GetWorld()->GetSubsystem<UChatSubsystem>())
	{
		const TArray<FChatData>& ChatLog = ChatSubsystem->GetChatLog();
		ClientReceiveChatLog(ChatLog);
	}
}

void ACXPlayerController::ClientReceiveChatLog_Implementation(const TArray<FChatData>& ChatLog)
{
	if (!IsValid(VehicleUI))
	{
		return;
	}

	for (const FChatData& ChatData : ChatLog)
	{
		VehicleUI->OnNewChatMessage(ChatData.Sender, ChatData.Message);
	}
}

void ACXPlayerController::ServerChatMessage_Implementation(const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("ServerChatMessage: %s"), *Message);

	ACXPlayerState* CXPS = Cast<ACXPlayerState>(PlayerState);
	if (!CXPS)
	{
		return;
	}

	const FString Sender = CXPS->GetPlayerName();
	if (UChatSubsystem* ChatSubsystem = GetWorld()->GetSubsystem<UChatSubsystem>())
	{
		ChatSubsystem->AddChatMessage(Sender, Message);
	}

	for (TActorIterator<ACXPlayerController> It(GetWorld()); It; ++It)
	{
		if (ACXPlayerController* CXPC = *It)
		{
			CXPC->ClientChatMessage(Sender, Message);
		}
	}
}

void ACXPlayerController::ClientChatMessage_Implementation(const FString& Sender, const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("ClientChatMessage: %s: %s"), *Sender, *Message);

	if (VehicleUI)
	{
		VehicleUI->OnNewChatMessage(Sender, Message);
	}
}

void ACXPlayerController::BindGameStateEvents()
{
	ACXGameState* CXGS = GetWorld()->GetGameState<ACXGameState>();

	if (!IsValid(CXGS))
	{
		UE_LOG(LogTemp, Warning, TEXT("CXGameState is not valid."));
		return;
	}

	// 남은 시간 변경 이벤트 구독
	CXGS->OnRemainingTimeChanged.AddUObject(
		this,
		&ACXPlayerController::HandleRemainingTimeChanged
	);

	// 게임 상태 변경 이벤트 구독
	CXGS->OnGameStateChanged.AddUObject(
		this,
		&ACXPlayerController::HandleGameStateChanged
	);

	// 승자 목록 변경 이벤트 구독
	CXGS->OnWinnersChanged.AddUObject(
		this,
		&ACXPlayerController::HandleWinnersChanged
	);

	// 이벤트를 구독하기 전에 설정된 값도 UI에 반영
	HandleRemainingTimeChanged(CXGS->GetRemainingTime());

	HandleGameStateChanged(CXGS->GetCurrentGameState());
}

void ACXPlayerController::HandleRemainingTimeChanged(int32 RemainingTime)
{
	if (!IsValid(VehicleUI))
	{
		return;
	}

	VehicleUI->UpdateRemainingTime(RemainingTime);
}

void ACXPlayerController::HandleGameStateChanged(EGameState NewState)
{
	if (!IsValid(VehicleUI))
	{
		return;
	}

	switch (NewState)
	{
	case EGameState::Waiting:
		VehicleUI->UpdateGameStatus(TEXT("Waiting"));
		break;

	case EGameState::Playing:
		VehicleUI->UpdateGameStatus(TEXT("Playing"));
		break;

	case EGameState::Finished:
		VehicleUI->UpdateGameStatus(TEXT("Finished"));

		// 게임 종료 시 승패 확인
		HandleWinnersChanged();
		break;
	}
}

void ACXPlayerController::HandleWinnersChanged()
{
	if (!IsLocalController() || !IsValid(VehicleUI))
	{
		return;
	}

	ACXGameState* CXGS = GetWorld()->GetGameState<ACXGameState>();

	if (!IsValid(CXGS))
	{
		return;
	}

	// 게임이 끝난 경우에만 결과 표시
	if (CXGS->GetCurrentGameState() != EGameState::Finished)
	{
		return;
	}

	ACXPlayerState* LocalPlayerState = GetPlayerState<ACXPlayerState>();

	if (!IsValid(LocalPlayerState))
	{
		return;
	}

	const auto& Winners = CXGS->GetWinners();

	const bool bIsWinner = Winners.Contains(LocalPlayerState);

	VehicleUI->ShowGameResult(bIsWinner);
}

void ACXPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	VehicleUI = CreateWidget<UCXUI>(this, VehicleUIClass);

	if (!IsValid(VehicleUI))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create VehicleUI."));
		return;
	}

	VehicleUI->AddToViewport();

	// 게임 상태 이벤트 연결
	BindGameStateEvents();

	// 기존 채팅 기능 유지
	ServerRequestChatLog();
}

void ACXPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// get the enhanced input subsystem
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// add the mapping context so we get controls
		Subsystem->AddMappingContext(InputMappingContext, 0);

		// optionally add the steering wheel context
		if (bUseSteeringWheelControls && SteeringWheelInputMappingContext)
		{
			Subsystem->AddMappingContext(SteeringWheelInputMappingContext, 1);
		}
	}
}

void ACXPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, VehiclePawn);
}

void ACXPlayerController::Tick(float Delta)
{
	Super::Tick(Delta);

	if (IsValid(VehiclePawn) && IsValid(VehicleUI))
	{
		VehicleUI->UpdateSpeed(VehiclePawn->GetChaosVehicleMovement()->GetForwardSpeed());
		VehicleUI->UpdateGear(VehiclePawn->GetChaosVehicleMovement()->GetCurrentGear());
	}
}

void ACXPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// get a pointer to the controlled pawn
	VehiclePawn = CastChecked<ACXPawn>(InPawn);
}
