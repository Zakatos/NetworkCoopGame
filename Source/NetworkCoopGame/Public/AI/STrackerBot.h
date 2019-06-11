// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STrackerBot.generated.h"

class UStaticMeshComponent;
class USHealthComponent;
class UParticleSystem;
class USphereComponent;
class USoundCue;
class UMaterialInstanceDynamic;

UCLASS()
class NETWORKCOOPGAME_API ASTrackerBot : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASTrackerBot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly,Category = "Components")
	UStaticMeshComponent* MeshComp;
	UPROPERTY(VisibleDefaultsOnly,Category = "Components")
	USHealthComponent* HealthComp;
	UPROPERTY(EditDefaultsOnly,Category = "Tracker Bot")
	UParticleSystem* ExplosionEffect;
	UPROPERTY(EditDefaultsOnly,Category = "Tracker Bot")
	USphereComponent* SphereComp;

	UFUNCTION()
	void HandleTakeDamage(USHealthComponent* OwningHealthComp,float Health,float HealthDelta,const class UDamageType*  DamageType, 
		class AController* InstigatedBy, AActor* DamageCauser);

	FVector GetNextPathPoint();

	//Next Path in Navigation
	FVector NextPathPoint;
	UPROPERTY(EditDefaultsOnly,Category = "Tracker Bot")
	float MovementForce;
	UPROPERTY(EditDefaultsOnly,Category = "Tracker Bot")
	bool bUseVelocityChange;
	UPROPERTY(EditDefaultsOnly,Category = "Tracker Bot")
	float RequiredDistanceToTarget;

	//Dynamic Material to Pulse on damage
	UMaterialInstanceDynamic* MatInst;

	void SelfDestruct();

	bool bExploded;

	bool bStartedSelfDestruction;
	UPROPERTY(EditDefaultsOnly,Category = "Tracker Bot")
	float ExplosionRadius;
	UPROPERTY(EditDefaultsOnly,Category = "Tracker Bot")
	float ExplosionDamage;
	UPROPERTY(EditDefaultsOnly,Category = "Tracker Bot")
	float SelfDamageInterval;

	int32 PowerLevel;
	UPROPERTY(EditDefaultsOnly,Category = "Tracker Bot")
	float AllyDetectionRadius;

	FTimerHandle TimerHandle_SelfDamage;


	void DamageSelf();

	void OnCheckNearbyAllies();

	UPROPERTY(EditDefaultsOnly,Category = "Tracker Bot")
	USoundCue* SelfDestructSound;
	UPROPERTY(EditDefaultsOnly,Category = "Tracker Bot")
	USoundCue* ExplodeSound;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

};
