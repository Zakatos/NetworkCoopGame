// Fill out your copyright notice in the Description page of Project Settings.


#include "STrackerBot.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "AI/NavigationSystemBase.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "DrawDebugHelpers.h"
#include "Containers/Array.h"
#include "Components/SphereComponent.h"
#include "SCharacter.h"
#include "Components/SHealthComponent.h"
#include "Sound/SoundCue.h"


static int32 DebugTrackerBotDrawing = 0;
FAutoConsoleVariableRef CVarDebugTrackerBotDrawing
(TEXT("COOP.DebugTrackerBot"),
	DebugTrackerBotDrawing,
TEXT("Draw debug line for TrackerBot"),ECVF_Cheat);




// Sets default values
ASTrackerBot::ASTrackerBot()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASTrackerBot::HandleTakeDamage);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(200);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComp->SetupAttachment(RootComponent);

	
	bUseVelocityChange = false;
	MovementForce = 1000;
	RequiredDistanceToTarget = 100;

	ExplosionDamage = 50;
	ExplosionRadius = 350;

	SelfDamageInterval = 0.25f;

	AllyDetectionRadius = 600.0f;

}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();


	if (Role == ROLE_Authority)
	{
		//Find initial move to
		NextPathPoint = GetNextPathPoint();
		FTimerHandle TimerHandle_CheckPower;
		GetWorldTimerManager().SetTimer(TimerHandle_CheckPower,this,&ASTrackerBot::OnCheckNearbyAllies,1.0f,true);
	}

}


void ASTrackerBot::HandleTakeDamage(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	//Explode on Death
	if (MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	if (MatInst)
	{
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}

	if (Health <= 0.0f)
	{
		SelfDestruct();
	}
}

void ASTrackerBot::RefreshPath()
{
	NextPathPoint = GetNextPathPoint();
}

FVector ASTrackerBot::GetNextPathPoint()
{

	AActor* bestTarget = nullptr;
	float nearestTargetDistance = FLT_MAX;

	for(FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; It++)
	{
		APawn* TestPawn = It->Get();
		if(TestPawn == nullptr || USHealthComponent::IsFriendly(TestPawn,this))
		{
			continue;
		}

		USHealthComponent* testPawnHealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));

		if(testPawnHealthComp && testPawnHealthComp->GetHealth() > 0.0f)
		{
			float distance = (TestPawn->GetActorLocation() - GetActorLocation()).Size();

			if(distance < nearestTargetDistance)
			{
				bestTarget = TestPawn;
				nearestTargetDistance = distance;
			}
		}


	}
	if (bestTarget)
	{
		UNavigationPath* navPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), bestTarget);
		
		GetWorldTimerManager().ClearTimer(TimerHandle_RefreshPath);

		GetWorldTimerManager().SetTimer(TimerHandle_RefreshPath,this,&ASTrackerBot::RefreshPath,5.0f,false);

		if (navPath && navPath->PathPoints.Num() > 1)
		{
			//return next point to the path
			return navPath->PathPoints[1];
		}
	}

	//Failed to find path
	return GetActorLocation();
}

void ASTrackerBot::SelfDestruct()
{
	if (bExploded)
	{
		return;
	}

	bExploded = true;

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());

	UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());

	MeshComp->SetVisibility(false,true);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	
	if (Role == ROLE_Authority)
	{
		TArray<AActor*> ignoredActors;
		ignoredActors.Add(this);

		float finalDamage = ExplosionDamage + (ExplosionDamage * PowerLevel);

		//Apply Damage
		UGameplayStatics::ApplyRadialDamage(this, finalDamage, GetActorLocation(), ExplosionRadius, nullptr, ignoredActors, this, GetInstigatorController(), true);

		if(DebugTrackerBotDrawing)
		{
			DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 2.0f, 0.0f, 1.0f);
		}
		

		//Destroy it after a while to be able to notify the clients in time
		SetLifeSpan(2.0f);
	}
}

void ASTrackerBot::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this, 20, GetInstigatorController(), this, nullptr);
}

void ASTrackerBot::OnCheckNearbyAllies()
{

	FCollisionShape collShape;
	//Set collision shape
	collShape.SetSphere(AllyDetectionRadius);
	//Only Find Pawns
	FCollisionObjectQueryParams queryParams;
	queryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	queryParams.AddObjectTypesToQuery(ECC_Pawn);

	TArray<FOverlapResult> overlaps;
	GetWorld()->OverlapMultiByObjectType(overlaps,GetActorLocation(),FQuat::Identity,queryParams,collShape);
	if (DebugTrackerBotDrawing)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), AllyDetectionRadius, 12, FColor::White, false, 1.0f);
	}
	int32 nrOfBots = 0;
	for (FOverlapResult result : overlaps)
	{
		ASTrackerBot* bot = Cast<ASTrackerBot>(result.GetActor());
		if(bot && bot != this)
		{
			nrOfBots++;
			//UE_LOG(LogTemp,Log,TEXT(" Changed: %s"),*FString::SanitizeFloat(nrOfBots));
		}
	}

	const int32 maxPowerLevel = 4;

	PowerLevel = FMath::Clamp(nrOfBots,0,maxPowerLevel);
	
	if(MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0,MeshComp->GetMaterial(0));
	}
	if(MatInst)
	{

		float alpha = PowerLevel / (float)maxPowerLevel;
		
		MatInst->SetScalarParameterValue("PowerLevelAlpha",alpha);

	}
	if (DebugTrackerBotDrawing)
	{
		DrawDebugString(GetWorld(), FVector(0, 0, 0), FString::FromInt(PowerLevel), this, FColor::White, 1.0f, true);
	}
}

// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (Role == ROLE_Authority && !bExploded)
	{
		float distanceToTarget = (GetActorLocation() - NextPathPoint).Size();

		if (distanceToTarget <= RequiredDistanceToTarget)
		{
			NextPathPoint = GetNextPathPoint();
			if (DebugTrackerBotDrawing)
			{
				DrawDebugString(GetWorld(), GetActorLocation(), "Target Reached");
			}
		}
		else
		{
			FVector forceDirection = NextPathPoint - GetActorLocation();
			forceDirection.Normalize();

			forceDirection *= MovementForce;

			MeshComp->AddForce(forceDirection, NAME_None, bUseVelocityChange);
			if(DebugTrackerBotDrawing)
			{
				DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + forceDirection, 32, FColor::Yellow, false, 0.0f, 0, 1.0f);
			}
		}
		if (DebugTrackerBotDrawing)
		{
			DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Yellow, false, 0.0f, 1.0f);
		}
	}
}

void ASTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{

	Super::NotifyActorBeginOverlap(OtherActor);

	if (!bStartedSelfDestruction && !bExploded)
	{
		ASCharacter* playerPawn = Cast<ASCharacter>(OtherActor);

		if (playerPawn && !USHealthComponent::IsFriendly(OtherActor,this))
		{
			if (Role == ROLE_Authority)
			{
				//Start SelfDestruction Sequence
				GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ASTrackerBot::DamageSelf, SelfDamageInterval, true, 0.0f);
			}
			bStartedSelfDestruction = true;

			UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);
		}
	}
}


