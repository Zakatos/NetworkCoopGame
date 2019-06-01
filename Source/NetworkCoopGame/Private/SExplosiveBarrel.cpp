// Fill out your copyright notice in the Description page of Project Settings.


#include "SExplosiveBarrel.h"
#include "Components/SHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "SCharacter.h"
#include "EngineUtils.h"


// Sets default values
ASExplosiveBarrel::ASExplosiveBarrel()
{
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASExplosiveBarrel::OnHealthChanged);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetSimulatePhysics(true);

	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	RootComponent = MeshComp;

	ExplosionRadius = 250;
	BaseDamage = 50;

	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialComp"));
	RadialForceComp->SetupAttachment(MeshComp);
	RadialForceComp->Radius = ExplosionRadius;
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->bAutoActivate = false; //Prevent Component from ticking
	RadialForceComp->bIgnoreOwningActor = true; //ignore self

	ExplosionImpulse = 400;
}

// Called when the game starts or when spawned
void ASExplosiveBarrel::BeginPlay()
{
	Super::BeginPlay();

}

void ASExplosiveBarrel::Explode(TArray<AActor*> ignoredActors)
{
	UGameplayStatics::ApplyRadialDamage(GetWorld(),BaseDamage,GetActorLocation(),ExplosionRadius,BarrelDamageType,ignoredActors,this,nullptr,false, ECollisionChannel::ECC_Visibility);
}

void ASExplosiveBarrel::OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (bExploded)
	{
		return;
	}

	if (Health <= 0.0f && !bExploded)
	{
		//Die
		bExploded = true;

		FVector BoostIntensity = FVector::UpVector * ExplosionImpulse;
		MeshComp->AddImpulse(BoostIntensity, NAME_None, true);

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorTransform());

		MeshComp->SetMaterial(0, ExplodedMaterial);


		RadialForceComp->FireImpulse();
		TArray<AActor*> ignoredActors;
		for (TActorIterator<ASCharacter> aItr(GetWorld()); aItr; ++aItr)
		{
			float distance = GetDistanceTo(*aItr);

			if (distance <= 250)
			{
				Explode(ignoredActors);
			}
		}
	}
}



