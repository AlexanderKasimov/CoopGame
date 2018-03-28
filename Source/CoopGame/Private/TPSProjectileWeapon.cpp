// Fill out your copyright notice in the Description page of Project Settings.

#include "TPSProjectileWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "TPSBaseProjectile.h"
#include "Kismet/GameplayStatics.h"





ATPSProjectileWeapon::ATPSProjectileWeapon()
{

}


void ATPSProjectileWeapon::Fire()
{
	if (ProjectileClass)
	{
		FVector MuzzleLocation = MeshComponent->GetSocketLocation(MuzzleSocketName);
		FRotator MuzzleRotation = MeshComponent->GetSocketRotation(MuzzleSocketName);
		AActor* Owner = GetOwner();
		if (Owner)
		{
			FVector temp;
			FRotator EyeRotation;
			Owner->GetActorEyesViewPoint(temp, EyeRotation);
			MuzzleRotation = EyeRotation;
		}

		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ActorSpawnParams.Instigator = Cast<APawn>(this->GetOwner());

		GetWorld()->SpawnActor<ATPSBaseProjectile>(ProjectileClass, MuzzleLocation, MuzzleRotation, ActorSpawnParams);

		if (MuzzleEffect)
		{
			UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComponent, MuzzleSocketName);
		}
	}

}
