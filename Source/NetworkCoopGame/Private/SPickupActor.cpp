// Fill out your copyright notice in the Description page of Project Settings.


#include "SPickupActor.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "SPowerup.h"
#include "TimerManager.h"
#include "SCharacter.h"

// Sets default values
ASPickupActor::ASPickupActor()
{
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Comp"));
	SphereComp->SetSphereRadius(75.0f);
	RootComponent = SphereComp;

	DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("Decal Comp"));
	DecalComp->SetRelativeRotation(FRotator(90.0f,0.0f,0.0f));
	DecalComp->DecalSize = FVector(64.0f,75.0f,75.0f);
	DecalComp->SetupAttachment(RootComponent);

	CoolDownDuration = 10.0f;

	SetReplicates(true);
}

// Called when the game starts or when spawned
void ASPickupActor::BeginPlay()
{
	Super::BeginPlay();

	if(Role == ROLE_Authority)
	{
		Respawn();
	}
	
}

void ASPickupActor::Respawn()
{
	if(PowerUpClass == nullptr)
	{
		UE_LOG(LogTemp,Warning,TEXT("Powerup class is nullptr in %s. Please update your blueprint"),*GetName());
		return;
	}
	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


	PowerupInstance = GetWorld()->SpawnActor<ASPowerup>(PowerUpClass,GetTransform(),spawnParams);
}


void ASPickupActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	
	if (ASCharacter* characterActor = Cast<ASCharacter>(OtherActor))
	{
		if (Role == ROLE_Authority && PowerupInstance)
		{
			PowerupInstance->ActivatePowerup(OtherActor);
				PowerupInstance = nullptr;

				//Set timer to respawn
				GetWorldTimerManager().SetTimer(TimerHandle_RespawnTimer, this, &ASPickupActor::Respawn, CoolDownDuration);
		}

	}

}


