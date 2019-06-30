// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SHordeGameMode.generated.h"


enum class EWaveState : uint8;

/**
 * 
 */
UCLASS()
class NETWORKCOOPGAME_API ASHordeGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	FTimerHandle TimerHandle_BotSpawner;

	FTimerHandle TimerHandle_NextWaveStart;

	//Bots to spawn in current wave
	int32 NrOfBotsToSpawn;

	int32 WaveCount;

	UPROPERTY(EditDefaultsOnly,Category = "GameMode")
	float TimeBetweenWaves;

protected:

	//Hook for BP to spawn a single bot
	UFUNCTION(BlueprintImplementableEvent,Category = "GameMode")
	void SpawnNewBot();

	void SpawnTimerElapsed();

	void StartWave();

	void EndWave();

	//Set timer for next wave
	void PrepareForNextWave();

	void CheckWaveState();

	void CheckAnyPlayerAlive();

	void GameOver();

	void SetWaveState(EWaveState NewState);

public:

	ASHordeGameMode();

	virtual void StartPlay() override;

	virtual void Tick(float DeltaSeconds) override;
	
};
