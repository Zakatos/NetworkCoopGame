// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPowerup.generated.h"

UCLASS()
class NETWORKCOOPGAME_API ASPowerup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASPowerup();

protected:

	/*Time between powerup ticks*/
	UPROPERTY(EditDefaultsOnly,Category = "PowerUps")
	float PowerUpInterval;
	/*Total times we apply the powerup effect*/
	UPROPERTY(EditDefaultsOnly,Category = "PowerUps")
	int32 TotalNrOfTicks;

	//Total Number of ticks applied
	int32 TicksProcessed;

	FTimerHandle TimerHandle_PowerupTick;
	UFUNCTION()
	void OnTickPowerup();

    //Keeps state of powerup
	UPROPERTY(ReplicatedUsing=OnRep_PowerupActive)
	bool bIsPowerupActive;
	
	UFUNCTION()
	void OnRep_PowerupActive();

	UFUNCTION(BlueprintImplementableEvent,Category = "PowerUps")
	void OnPowerupStateChanged(bool bnewisactive);

public:	

	void ActivatePowerup(AActor* activatefor);

	UFUNCTION(BlueprintImplementableEvent,Category = "PowerUps")
	void OnActivated(AActor* activatefor);
	
	UFUNCTION(BlueprintImplementableEvent,Category = "PowerUps")
	void OnExpired();

	UFUNCTION(BlueprintImplementableEvent,Category = "PowerUps")
	void OnPowerupTicked();
};
