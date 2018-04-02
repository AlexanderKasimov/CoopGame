// Fill out your copyright notice in the Description page of Project Settings.

#include "TPSGameMode.h"
#include "TimerManager.h"
#include "TPSHealthComponent.h"
#include "Engine/World.h"
#include "TPSGameState.h"
#include "TPSPlayerState.h"



ATPSGameMode::ATPSGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f;

	
	GameStateClass = ATPSGameState::StaticClass();
	PlayerStateClass = ATPSPlayerState::StaticClass();

	TimeBetweenWaves = 2.0f;
}


void ATPSGameMode::StartPlay()
{	
	Super::StartPlay();

	PrepareForNextWave();
}


void ATPSGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckWaveState();

	CheckAnyPlayerAlive();
}


void ATPSGameMode::SetWaveState(EWaveState NewState)
{
	ATPSGameState* GS = GetGameState<ATPSGameState>();
	if (ensureAlways(GS))
	{
		GS->SetWaveState(NewState);
	}
}

void ATPSGameMode::RestartDeadPlayers()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn() == nullptr)
		{
			RestartPlayer(PC);
		}
	}


}


void ATPSGameMode::StartWave()
{
	WaveCount++;
	NrOfBotsToSpawn = 2 * WaveCount;

	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ATPSGameMode::SpawnBotTimerElapsed, 1.0f, true, 0.0f);

	SetWaveState(EWaveState::WaveInProgress);
}


void ATPSGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);
	SetWaveState(EWaveState::WaitingToComplete);
}


void ATPSGameMode::PrepareForNextWave()
{
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ATPSGameMode::StartWave, TimeBetweenWaves, false);

	SetWaveState(EWaveState::WaitingToStart);

	RestartDeadPlayers();
}


void ATPSGameMode::SpawnBotTimerElapsed()
{
	SpawnNewBot();

	NrOfBotsToSpawn--;

	if (NrOfBotsToSpawn <= 0)
	{
		EndWave();
	}

}


void ATPSGameMode::CheckWaveState()
{
	bool bIsPreparingForWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);

	if (NrOfBotsToSpawn > 0 || bIsPreparingForWave)
	{
		return;
	}

	bool bIsAnyBotAlive = false;

	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		APawn* TestPawn = It->Get();
		if (TestPawn == nullptr || TestPawn->IsPlayerControlled())
		{
			continue;
		}
		UTPSHealthComponent* HealthComponent = Cast<UTPSHealthComponent>(TestPawn->GetComponentByClass(UTPSHealthComponent::StaticClass()));

		if (HealthComponent && HealthComponent->GetHealth() > 0.0f)
		{
			bIsAnyBotAlive = true;
			break;
		}
	}

	if (!bIsAnyBotAlive)
	{
		PrepareForNextWave();
		SetWaveState(EWaveState::WaveComplete);
	}

}

void ATPSGameMode::CheckAnyPlayerAlive()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn())
		{
			APawn* PlayerPawn = PC->GetPawn();

			UTPSHealthComponent* HealthComponent = Cast<UTPSHealthComponent>(PlayerPawn->GetComponentByClass(UTPSHealthComponent::StaticClass()));
			if (ensure(HealthComponent) && HealthComponent->GetHealth() > 0.0f)
			{
				return;
			}
		}
	}

	GameOver();

}


void ATPSGameMode::GameOver()
{
	EndWave();

	SetWaveState(EWaveState::GameOver);

	UE_LOG(LogTemp, Log, TEXT("GAME OVER! Players Died!"));
}
