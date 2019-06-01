// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SExplosiveBarrel.generated.h"

class USHealthComponent;
class UParticleSystem;
class UStaticMeshComponent;
class URadialForceComponent;
class UDamageType;


UCLASS()
class NETWORKCOOPGAME_API ASExplosiveBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASExplosiveBarrel();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere,Category = "Components")
	USHealthComponent* HealthComp;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "FX")
	UParticleSystem* ExplosionEffect;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "FX")
	float ExplosionImpulse;

	UPROPERTY(VisibleAnywhere,Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere,Category = "Components")
	URadialForceComponent* RadialForceComp;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Barrel")
	TSubclassOf<UDamageType> BarrelDamageType;

	UFUNCTION()
	void OnHealthChanged(USHealthComponent* OwningHealthComp,float Health,float HealthDelta,const class UDamageType*  DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	void Explode(TArray<AActor*> ignoredActors);

	bool bExploded;

	UPROPERTY(EditDefaultsOnly,Category = "Barrel")
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly,Category = "Barrel")
	float ExplosionRadius;

	UPROPERTY(EditDefaultsOnly,Category = "Components")
	UMaterialInterface* ExplodedMaterial;



public:	

};
