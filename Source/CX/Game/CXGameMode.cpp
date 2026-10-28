#include "CXGameMode.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "Subsystem/ChatSubsystem.h"
#include "Player/CXPlayerState.h"
#include "Player/CXPlayerController.h"
#include "CXGameState.h"
#include "TimerManager.h"

ACXGameMode::ACXGameMode()
{
	PlayerControllerClass = ACXPlayerController::StaticClass();
	PlayerStateClass = ACXPlayerState::StaticClass();
	GameStateClass = ACXGameState::StaticClass();
}

void ACXGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (ACXGameState* CXGS = GetGameState<ACXGameState>())
	{
		CXGS->SetCurrentGameState(EGameState::Waiting);
		CXGS->SetRemainingTime(10);
	}
}

AActor* ACXGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<APlayerStart*> AvailablePlayerStarts;

	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* PlayerStart = *It;

		if (IsPlayerStartAvailable(PlayerStart))
		{
			AvailablePlayerStarts.Add(PlayerStart);
		}
	}

	checkf(AvailablePlayerStarts.IsEmpty() == false, TEXT("There is no available Vehicle PlayerStart."));

	const int32 RandomIndex = FMath::RandHelper(AvailablePlayerStarts.Num());
	return Cast<AActor>(AvailablePlayerStarts[RandomIndex]);
}

void ACXGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (IsValid(NewPlayer) == false)
	{
		return;
	}

	if (!bHasFirstPlayerJoined)
	{
		bHasFirstPlayerJoined = true;

		GetWorldTimerManager().SetTimer(
			CountdownTimerHandle,
			this,
			&ACXGameMode::DecreaseWaitingTime,
			1.0f,
			true
		);
	}
}

bool ACXGameMode::IsPlayerStartAvailable(const APlayerStart* PlayerStart) const
{
	if (IsValid(PlayerStart) == false || IsValid(GetWorld()) == false)
	{
		return false;
	}

	const FTransform PlayerStartTransform = PlayerStart->GetActorTransform();
	const FVector CheckLocation = PlayerStartTransform.TransformPosition(VehicleSpawnCheckOffset);
	const FQuat CheckRotation = PlayerStartTransform.GetRotation();
	const FCollisionShape CheckShape = FCollisionShape::MakeBox(VehicleSpawnCheckHalfExtent);
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Vehicle);

	const FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(VehiclePlayerStartCheck), false);

	const bool bVehicleExists =
		GetWorld()->OverlapAnyTestByObjectType(
			CheckLocation,
			CheckRotation,
			ObjectQueryParams,
			CheckShape,
			QueryParams
		);

	return !bVehicleExists;
}

void ACXGameMode::DecreaseWaitingTime()
{
	ACXGameState* CXGS = GetGameState<ACXGameState>();
	if (!IsValid(CXGS))
	{
		GetWorldTimerManager().ClearTimer(CountdownTimerHandle);

		return;
	}

	const int32 RemainingWaitingTime = CXGS->GetRemainingTime() - 1;
	CXGS->SetRemainingTime(RemainingWaitingTime);

	if (RemainingWaitingTime <= 0)
	{
		GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
		CXGS->SetCurrentGameState(EGameState::Playing);
		CXGS->SetRemainingTime(30);

		GetWorldTimerManager().SetTimer(
			CountdownTimerHandle,
			this,
			&ACXGameMode::DecreasePlayingTime,
			1.0f,
			true
		);

		return;
	}
}

void ACXGameMode::DecreasePlayingTime()
{
	ACXGameState* CXGS = GetGameState<ACXGameState>();
	if (!IsValid(CXGS))
	{
		GetWorldTimerManager().ClearTimer(CountdownTimerHandle);

		return;
	}

	const int32 RemainingPlayingTime = CXGS->GetRemainingTime() - 1;
	CXGS->SetRemainingTime(RemainingPlayingTime);

	if (RemainingPlayingTime <= 0)
	{
		GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
		CXGS->SetCurrentGameState(EGameState::Finished);
		FinishGame();

		return;
	}
}

void ACXGameMode::FinishGame()
{
	ACXGameState* CXGS = GetGameState<ACXGameState>();
	TArray<TObjectPtr<ACXPlayerState>> Winners;
	int32 MostCoins = -1;

	for (APlayerState* PS : CXGS->PlayerArray)
	{
		if (ACXPlayerState* CXPS = Cast<ACXPlayerState>(PS))
		{
			int32 PlayerCoins = CXPS->GetCoins();
			if (PlayerCoins > MostCoins)
			{
				MostCoins = PlayerCoins;
				Winners.Empty();
				Winners.Add(CXPS);
			}
			else if (PlayerCoins == MostCoins)
			{
				Winners.Add(CXPS);
			}
		}
	}

	CXGS->SetWinners(Winners);
}
