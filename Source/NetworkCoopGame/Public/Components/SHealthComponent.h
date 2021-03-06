// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "SHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthCHangedSignature,USHealthComponent*,HealthComp,float,Health,float,HealthDelta,const class UDamageType*,  DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);


UCLASS( ClassGroup=(COOP), meta=(BlueprintSpawnableComponent) )
class NETWORKCOOPGAME_API USHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USHealthComponent();
	//Todo add replicated if team switching is to be implemented
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Health Component")
	uint8 TeamNum;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	bool bIsDead;

	UPROPERTY(ReplicatedUsing=OnRep_Health,BlueprintReadOnly,Category = "Health Component")
	float Health;

	UFUNCTION()
	void OnRep_Health(float oldhealth);

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Health Component")
	float DefaultHealth;

	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float  Damage, const class UDamageType*  DamageType, class AController* InstigatedBy, AActor* DamageCauser);

public:	

	float GetHealth() const;


	UPROPERTY(BlueprintAssignable,Category = "Events")
	FOnHealthCHangedSignature OnHealthChanged;
	UFUNCTION(BlueprintCallable,Category = "HealthComponent")
	void Heal(float healamount);
	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "HealthComponent")
	static bool IsFriendly(AActor* ActorA,AActor* ActorB);
};
