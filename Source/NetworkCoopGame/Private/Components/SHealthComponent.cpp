// Fill out your copyright notice in the Description page of Project Settings.


#include "SHealthComponent.h"
#include "GameFramework/Actor.h"
#include "..\..\Public\Components\SHealthComponent.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	DefaultHealth = 100;

	SetIsReplicated(true);
}


// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	//Only hook if we are server
	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* myOwner = GetOwner();
		if (myOwner)
		{
			myOwner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::HandleTakeAnyDamage);
		}
	}

	Health = DefaultHealth;
}

void USHealthComponent::OnRep_Health(float oldhealth)
{
	float damage = Health - oldhealth;

	OnHealthChanged.Broadcast(this,Health,damage,nullptr,nullptr,nullptr);
}

void USHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float  Damage, const class UDamageType*  DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if(Damage <= 0)
	{
		return;
	}

	//Update health clamped
	Health = FMath::Clamp(Health-Damage,0.0f,DefaultHealth);

	UE_LOG(LogTemp,Log,TEXT("Health Changed: %s"),*FString::SanitizeFloat(Health));

	OnHealthChanged.Broadcast(this,Health,Damage,DamageType,InstigatedBy,DamageCauser);
}

void USHealthComponent::Heal(float healamount)
{
	if(healamount <=0.0f || Health <= 0.0f)
	{
		return;
	}

	Health = FMath::Clamp(Health + healamount,0.0f,DefaultHealth);

	UE_LOG(LogTemp,Log,TEXT("Health Changed: %s (+%s)"),*FString::SanitizeFloat(Health),*FString::SanitizeFloat(healamount));

	OnHealthChanged.Broadcast(this,Health,-healamount,nullptr,nullptr,nullptr);

}

 void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USHealthComponent,Health);

}

