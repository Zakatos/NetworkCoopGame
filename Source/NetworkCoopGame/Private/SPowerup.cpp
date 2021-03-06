// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerup.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASPowerup::ASPowerup()
{
	PowerUpInterval = 0.0f;
	TotalNrOfTicks = 0;

	bIsPowerupActive = false;

	SetReplicates(true);

	
}

void ASPowerup::OnTickPowerup()
{
	TicksProcessed++;

	OnPowerupTicked();

	if(TicksProcessed >= TotalNrOfTicks)
	{
		OnExpired();

		bIsPowerupActive = false;
		OnRep_PowerupActive();

		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);
	}
}

void ASPowerup::OnRep_PowerupActive()
{
	OnPowerupStateChanged(bIsPowerupActive);
}

void ASPowerup::ActivatePowerup(AActor* activatefor)
{
	OnActivated(activatefor);

	bIsPowerupActive = true;
	OnRep_PowerupActive();

	if(PowerUpInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick,this,&ASPowerup::OnTickPowerup,PowerUpInterval,true);
	}
	else
	{
		OnTickPowerup();
	}
	
}

void ASPowerup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASPowerup,bIsPowerupActive);
}

