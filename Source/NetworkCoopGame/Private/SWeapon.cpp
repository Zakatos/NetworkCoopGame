// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "NetworkCoopGame.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"


static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVarDebugWeaponDrawing
(TEXT("COOP.DebugWeapons"),
	DebugWeaponDrawing,
TEXT("Draw debug line for Weapons"),ECVF_Cheat);


// Sets default values
ASWeapon::ASWeapon()
{

    MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";

	TracerTargetName = "Target";

	BaseDamage = 20.0f;

	RateOfFire = 600;

	BulletSpread = 2.0f;

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

 void ASWeapon::BeginPlay()
 {
	 Super::BeginPlay();

	 TimeBetweenShots = 60/RateOfFire;
 }


 void ASWeapon::OnRep_HitScanTrace()
 {
	 //Play cosmetic FX
	 PlayFireEffect(HitScanTrace.TraceTo);

	 PlayImpactEffects(HitScanTrace.SurfaceType,HitScanTrace.TraceTo);

 }

 void ASWeapon::Fire()
{

	 if(Role < ROLE_Authority)
	 {
		ServerFire();
	 }


	AActor* myOwner = GetOwner();
	if(myOwner)
	{
		FVector eyeLocation;
		FRotator eyeRotation;

		myOwner->GetActorEyesViewPoint(eyeLocation,eyeRotation);

		FVector shotDirection = eyeRotation.Vector();
		//Bullet spread
		float halfRad = FMath::DegreesToRadians(BulletSpread);
		shotDirection = FMath::VRandCone(shotDirection,halfRad,halfRad);
		
		FVector traceEnd = eyeLocation + (shotDirection * 1000);

		FCollisionQueryParams queryParams;
		queryParams.AddIgnoredActor(myOwner);
		queryParams.AddIgnoredActor(this);
		queryParams.bTraceComplex = true;
		queryParams.bReturnPhysicalMaterial = true;

		//Particle "Target" Parameter
		FVector tracerEndPoint = traceEnd;

		EPhysicalSurface surfaceType = SurfaceType_Default;

		//Trace the world from pawn eyes to crosshair location
		FHitResult hit;

		if( GetWorld()->LineTraceSingleByChannel(hit,eyeLocation,traceEnd,COLLISION_WEAPON,queryParams))
		{
			//Blocking hit process damage

			AActor* hitActor = hit.GetActor();
			surfaceType = UPhysicalMaterial::DetermineSurfaceType(hit.PhysMaterial.Get());

			float actualDamage = BaseDamage;
			if(surfaceType == SURFACE_FLESHVUNERABLE)
			{
				actualDamage *= 4.0f;
			}
			UGameplayStatics::ApplyPointDamage(hitActor,actualDamage,shotDirection,hit,myOwner->GetInstigatorController(),myOwner,DamageType);

			PlayImpactEffects(surfaceType,hit.ImpactPoint);

			tracerEndPoint = hit.ImpactPoint;
			
		}

		if(DebugWeaponDrawing > 0)
		{
			DrawDebugLine(GetWorld(),eyeLocation,traceEnd,FColor::White,false,1.0f,0,1.0f);
		}
		
		PlayFireEffect(tracerEndPoint);

		if(Role==ROLE_Authority)
		{
			HitScanTrace.TraceTo = tracerEndPoint;
			HitScanTrace.SurfaceType = surfaceType;
		}

		LastFireTime = GetWorld()->TimeSeconds;
	}

	
}

 void ASWeapon::ServerFire_Implementation()
 {
	 Fire();
 }

 bool ASWeapon::ServerFire_Validate()
 {
	 return true;
 }


 void ASWeapon::StartFire()
 {
	 float firstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds,0.0f);
	 
	 GetWorldTimerManager().SetTimer(TimeHandle_TimeBetweenShoots,this,&ASWeapon::Fire,TimeBetweenShots,true,firstDelay);
 }

 void ASWeapon::StopFire()
 {
	 GetWorldTimerManager().ClearTimer(TimeHandle_TimeBetweenShoots);

 }

 void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASWeapon,HitScanTrace,COND_SkipOwner);

}


 void ASWeapon::PlayFireEffect(FVector TraceEnd)
 {
	 if (MuzzleEffect)
	 {
		 UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	 }

	 if (TracerEffect)
	 {
		 FVector muzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		 UParticleSystemComponent* tracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, muzzleLocation);

		 if (tracerComp)
		 {
			 tracerComp->SetVectorParameter(TracerTargetName, TraceEnd);

		 }
	 }

	 APawn* myOwner = Cast<APawn>(GetOwner());

	 if(myOwner)
	 {
		 APlayerController* pc = Cast<APlayerController>(myOwner->GetController());
		 if(pc)
		 {
			 pc->ClientPlayCameraShake(FireCameraShake);
		 }
	 }

 }

 void ASWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType,FVector ImpactPoint)
 {
	 UParticleSystem* selectedEffect = nullptr;

	 switch (SurfaceType)
	 {
	 case SURFACE_FLESHDEFAULT:
	 case SURFACE_FLESHVUNERABLE:
		 selectedEffect = FleshImpactEffect;
		 break;
	 default:
		 selectedEffect = DefaultImpactEffect;
		 break;
	 }

	 if (selectedEffect)
	 {
		 FVector muzzlelocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		 FVector shotDirection = ImpactPoint - muzzlelocation;
		 shotDirection.Normalize();

		 UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), selectedEffect, ImpactPoint, shotDirection.Rotation());
	 }
 }

