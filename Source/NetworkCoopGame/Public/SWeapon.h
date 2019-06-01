// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "SWeapon.generated.h"


class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;
class UCameraShake;

//Contains single information of a single hitscan weapon
USTRUCT()
struct FHitScanTrace
{
	GENERATED_BODY()
public:

	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> SurfaceType;

	UPROPERTY()
	FVector_NetQuantize TraceTo;

};


UCLASS()
class NETWORKCOOPGAME_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	

	// Sets default values for this actor's properties
	ASWeapon();

protected:

	virtual void BeginPlay() override;

	void PlayFireEffect(FVector TraceEnd);

	void PlayImpactEffects(EPhysicalSurface SurfaceType,FVector ImpactPoint);

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "Components")
	USkeletalMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category = "Weapon")
	FName MuzzleSocketName;

	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category = "Weapon")
	FName TracerTargetName;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Weapon")
	UParticleSystem* MuzzleEffect;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Weapon")
	UParticleSystem* DefaultImpactEffect;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Weapon")
	UParticleSystem* FleshImpactEffect;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Weapon")
	UParticleSystem* TracerEffect;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Weapon")
	TSubclassOf<UCameraShake> FireCameraShake;
	
	UPROPERTY(EditDefaultsOnly,Category = "Weapon")
	float BaseDamage;

	FTimerHandle TimeHandle_TimeBetweenShoots;

	float LastFireTime;

	//RPM bullets per minute firing
	UPROPERTY(EditDefaultsOnly,Category = "Weapon")
	float RateOfFire;

	//Derived from rate of fire
	float TimeBetweenShots;

	UPROPERTY(ReplicatedUsing=OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;

	UFUNCTION()
	void OnRep_HitScanTrace();
public:	

	UFUNCTION(BlueprintCallable,Category = "Weapon")
	virtual void Fire(); //Move to protected?

	UFUNCTION(Server,Reliable,WithValidation)
	void ServerFire();

	void StartFire();

	void StopFire();

};
