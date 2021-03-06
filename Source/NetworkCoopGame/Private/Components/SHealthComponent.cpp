// Fill out your copyright notice in the Description page of Project Settings.


#include "SHealthComponent.h"
#include "GameFramework/Actor.h"
#include "..\..\Public\Components\SHealthComponent.h"
#include "SHordeGameMode.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	DefaultHealth = 100;

	TeamNum = 255;

	bIsDead = false;

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
	if(Damage <= 0.0f || bIsDead)
	{
		return;
	}

	if (DamageCauser != DamagedActor && IsFriendly(DamagedActor,DamageCauser))
	{
		return;
	}

	//Update health clamped
	Health = FMath::Clamp(Health-Damage,0.0f,DefaultHealth);

	UE_LOG(LogTemp,Log,TEXT("Health Changed: %s"),*FString::SanitizeFloat(Health));

	bIsDead = Health <= 0.0f;

	OnHealthChanged.Broadcast(this,Health,Damage,DamageType,InstigatedBy,DamageCauser);

	if (Health <= 0.0f)
	{
		ASHordeGameMode* GM = Cast<ASHordeGameMode>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			GM->OnActorKilled.Broadcast(GetOwner(), DamageCauser,InstigatedBy);
		}
	}
}

float USHealthComponent::GetHealth() const
{
	return Health;
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

bool USHealthComponent::IsFriendly(AActor* ActorA,AActor* ActorB)
{

	if(ActorA == nullptr|| ActorB == nullptr)
	{
		//Assume friendly
		return true;
	}

	USHealthComponent* healthCompA = Cast<USHealthComponent>(ActorA->GetComponentByClass(USHealthComponent::StaticClass()));

	USHealthComponent* healthCompB = Cast<USHealthComponent>(ActorB->GetComponentByClass(USHealthComponent::StaticClass()));

	if(healthCompA == nullptr || healthCompB == nullptr)
	{
		//Assume friendly
		return true;
	}


	return healthCompA->TeamNum == healthCompB->TeamNum;
}

 void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USHealthComponent,Health);

}

