// Fill out your copyright notice in the Description page of Project Settings.


#include "SProjectileWeapon.h"
#include "Components/SkeletalMeshComponent.h"



void ASProjectileWeapon::Fire() 
{
	AActor* myOwner = GetOwner();
	if(myOwner && ProjectileClass)
	{
		FVector eyeLocation;
		FRotator eyeRotation;
		myOwner->GetActorEyesViewPoint(eyeLocation,eyeRotation);

		FVector muzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		FActorSpawnParameters spawnParams;

		spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		GetWorld()->SpawnActor<AActor>(ProjectileClass,muzzleLocation,eyeRotation,spawnParams);


	}
}
