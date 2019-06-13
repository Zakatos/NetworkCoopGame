// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerup.h"

// Sets default values
ASPowerup::ASPowerup()
{
	PowerUpInterval = 0.0f;
	TotalNrOfTicks = 0;
}

// Called when the game starts or when spawned
void ASPowerup::BeginPlay()
{
	Super::BeginPlay();
}

void ASPowerup::OnTickPowerup()
{
	TicksProcessed++;

	OnPowerupTicked();

	if(TicksProcessed >= TotalNrOfTicks)
	{
		OnExpired();

		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);
	}
}

void ASPowerup::ActivatePowerup()
{
	if(PowerUpInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick,this,&ASPowerup::OnTickPowerup,PowerUpInterval,true,0.0f);
	}
	else
	{
		OnTickPowerup();
	}
	
	
}

