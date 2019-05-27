// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"


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
}



 void ASWeapon::Fire()
{

	AActor* myOwner = GetOwner();
	if(myOwner)
	{
		FVector eyeLocation;
		FRotator eyeRotation;

		myOwner->GetActorEyesViewPoint(eyeLocation,eyeRotation);

		FVector shotDirection = eyeRotation.Vector();

		
		FVector traceEnd = eyeLocation + (shotDirection * 1000);

		FCollisionQueryParams queryParams;
		queryParams.AddIgnoredActor(myOwner);
		queryParams.AddIgnoredActor(this);
		queryParams.bTraceComplex = true;

		//Particle "Target" Parameter
		FVector tracerEndPoint = traceEnd;

		//Trace the world from pawn eyes to crosshair location
		FHitResult hit;

		if( GetWorld()->LineTraceSingleByChannel(hit,eyeLocation,traceEnd,ECC_Visibility,queryParams))
		{
			//Blocking hit process damage

			AActor* hitActor = hit.GetActor();

			UGameplayStatics::ApplyPointDamage(hitActor,20.0f,shotDirection,hit,myOwner->GetInstigatorController(),this,DamageType);
			if(ImpactEffect)		
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),ImpactEffect,hit.ImpactPoint,hit.ImpactNormal.Rotation());
			}

			tracerEndPoint = hit.ImpactPoint;
		}

		if(DebugWeaponDrawing > 0)
		{
			DrawDebugLine(GetWorld(),eyeLocation,traceEnd,FColor::White,false,1.0f,0,1.0f);
		}
		
		PlayFireEffect(tracerEndPoint);
	}

	
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

