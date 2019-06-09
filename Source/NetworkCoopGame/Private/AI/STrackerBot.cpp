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
	HealthComp->OnHealthChanged.AddDynamic(this,&ASTrackerBot::HandleTakeDamage);
	
	bUseVelocityChange = false;
	MovementForce = 1000;
	RequiredDistanceToTarget = 100;
}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();
	
	//Find initial move to
	NextPathPoint = GetNextPathPoint();	
}


void ASTrackerBot::HandleTakeDamage(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	//Explode on Death

	//Todo Pulse the material on hit
	if(MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0,MeshComp->GetMaterial(0));
	}

	if(MatInst)
	{
		MatInst->SetScalarParameterValue("LastTimeDamageTaken",GetWorld()->TimeSeconds);
	}

	UE_LOG(LogTemp,Log,TEXT("Health %s of %s"),*FString::SanitizeFloat(Health),*GetName());
}

FVector ASTrackerBot::GetNextPathPoint()
{
	//Temp Hack to find player location 
	ACharacter* playerPawn = UGameplayStatics::GetPlayerCharacter(this,0);
	
	
	UNavigationPath* navPath = UNavigationSystemV1::FindPathToActorSynchronously(this,GetActorLocation(),playerPawn);
	
	if(!navPath)
	{
		return FVector::ZeroVector;
	}

	if(navPath->PathPoints.Num() > 1)
	{
		//return next point to the path
		return navPath->PathPoints[1];
	}
	//Failed to find path
	return GetActorLocation();
}

// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float distanceToTarget = (GetActorLocation() - NextPathPoint).Size();

	if(distanceToTarget <= RequiredDistanceToTarget)
	{
		NextPathPoint = GetNextPathPoint();	

		DrawDebugString(GetWorld(), GetActorLocation(), "Target Reached");
	}
	else
	{
		FVector forceDirection = NextPathPoint - GetActorLocation();
		forceDirection.Normalize();

		forceDirection *= MovementForce;

		MeshComp->AddForce(forceDirection,NAME_None,bUseVelocityChange);

		DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + forceDirection, 32, FColor::Yellow, false, 0.0f, 0, 1.0f);
	}

	DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Yellow, false, 0.0f, 1.0f);
}


