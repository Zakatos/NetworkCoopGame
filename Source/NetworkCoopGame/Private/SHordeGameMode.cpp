// Fill out your copyright notice in the Description page of Project Settings.


#include "SHordeGameMode.h"
#include "TimerManager.h"
#include "SGameState.h"
#include "SPlayerState.h"
#include "Components/SHealthComponent.h"



ASHordeGameMode::ASHordeGameMode()
{
	TimeBetweenWaves = 2.0f;

	GameStateClass = ASHordeGameMode::StaticClass();
	PlayerStateClass = ASPlayerState::StaticClass();

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f;
}

void ASHordeGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CheckWaveState();

	CheckAnyPlayerAlive();
}


void ASHordeGameMode::SpawnTimerElapsed()
{
	SpawnNewBot();

	NrOfBotsToSpawn--;

	if(NrOfBotsToSpawn <= 0)
	{
		EndWave();
	}
}

void ASHordeGameMode::StartWave()
{
	WaveCount++;

	NrOfBotsToSpawn = 2 * WaveCount;

	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner,this,&ASHordeGameMode::SpawnTimerElapsed,1.0f,true,0.0f);

	SetWaveState(EWaveState::WaveInProgress);
}

void ASHordeGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);

	SetWaveState(EWaveState::WaitingToComplete);

}

void ASHordeGameMode::PrepareForNextWave()
{	
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart,this,&ASHordeGameMode::StartWave,TimeBetweenWaves,false);

	SetWaveState(EWaveState::WaitingToStart);

	RestartDeadPlayers();
}

void ASHordeGameMode::CheckWaveState()
{
	bool bIsPreparingForWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);

	
	if(NrOfBotsToSpawn > 0 || bIsPreparingForWave)
	{
		return;
	}


	bool bIsAnyBotAlive = false;

	for(FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; It++)
	{
		APawn* TestPawn = It->Get();
		if(TestPawn == nullptr || TestPawn->IsPlayerControlled())
		{
			continue;
		}

		USHealthComponent* HealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));

		if(HealthComp && HealthComp->GetHealth() > 0.0f)
		{
			bIsAnyBotAlive = true;
			break;
		}
	}

	if(!bIsAnyBotAlive)
	{
		SetWaveState(EWaveState::WaveComplete);

		PrepareForNextWave();
	}

}

void ASHordeGameMode::CheckAnyPlayerAlive()
{
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator();It;++It)
	{
		APlayerController* PC = It->Get();
		if(PC && PC->GetPawn())
		{
			APawn* MyPawn = PC->GetPawn();
			USHealthComponent* HealthComp =  Cast<USHealthComponent>(MyPawn->GetComponentByClass(USHealthComponent::StaticClass()));
			if(ensure(HealthComp) && HealthComp->GetHealth() > 0.0f)
			{
				// A player is still alive
				return;
			}
		}
	}

	//No Player Alive
	GameOver();
}

void ASHordeGameMode::GameOver()
{
	EndWave();

	//TODO finish the match, presenta game over

	SetWaveState(EWaveState::GameOver);

	UE_LOG(LogTemp,Log,TEXT("GAME OVER!"));
}

void ASHordeGameMode::SetWaveState(EWaveState NewState)
{
	ASGameState* GS = GetGameState<ASGameState>();

	if(ensureAlways(GS))
	{
		GS->SetWaveState(NewState);
	}

}

void ASHordeGameMode::RestartDeadPlayers()
{
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator();It;++It)
	{
		APlayerController* PC = It->Get();
		if(PC && PC->GetPawn() == nullptr)
		{
			RestartPlayer(PC);
		}

	}
}


void ASHordeGameMode::StartPlay()
{
	Super::StartPlay();

	PrepareForNextWave();
}


