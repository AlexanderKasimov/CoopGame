// Fill out your copyright notice in the Description page of Project Settings.

#include "TPSBaseWeapon.h"
//#include "Engine/World.h"  //for Intellisense
#include "DrawDebugHelpers.h"
#include "Components/SkeletalMeshComponent.h"  
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CoopGame.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "TPSCharacter.h"

static int32 DebugWeaponDrawing = 0;

FAutoConsoleVariableRef CVARDebugWeaponDrawing(TEXT("COOP.DebugWeapons"), DebugWeaponDrawing, TEXT("Draw Debug Lines for Weapons"), ECVF_Cheat);



// Sets default values
ATPSBaseWeapon::ATPSBaseWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));	
	RootComponent = MeshComponent;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "BeamEnd";

	BaseDamage = 20.0f;
	BulletSpread = 2.0f;
	RateOfFire = 600;

	TotalAmmo = 360;
	CurrentAmmo = TotalAmmo;
	MagazineSize = 60;
	CurrentAmmoInMagazine = MagazineSize;
	ReloadTime = 2.3f;

	SetReplicates(true);
	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}


// Called when the game starts or when spawned
void ATPSBaseWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	TimeBetweenShots = 60 / RateOfFire;

	LastFireTime = -TimeBetweenShots;

	
}

bool ATPSBaseWeapon::CanReload() const
{
	bool bMagazineNotFull = CurrentAmmoInMagazine < MagazineSize;
	bool bHasAmmo = CurrentAmmo > 0;
	return (bMagazineNotFull && bHasAmmo && !OwningPlayer->GetReloading());

}

void ATPSBaseWeapon::Fire()
{	
	if (CurrentAmmoInMagazine == 0)
	{
		StopFire();
		StartReload();
		return;
	}

	if (Role < ROLE_Authority)
	{
		ServerFire();
	}	

	CurrentAmmoInMagazine--;

	AActor* Owner = GetOwner();
	if (Owner)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		Owner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
		FVector ShotDirection = EyeRotation.Vector();
		float HalfRad = FMath::DegreesToRadians(BulletSpread);
		ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);
		FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

		EPhysicalSurface SurfaceType = SurfaceType_Default;

		FCollisionQueryParams TraceParams;
		TraceParams.AddIgnoredActor(Owner);
		TraceParams.AddIgnoredActor(this);
		TraceParams.bTraceComplex = true;
		TraceParams.bReturnPhysicalMaterial = true;

		FHitResult Hit;
		//Tracer Particle "Target" Parameter
		FVector TracerEndPoint = TraceEnd;

		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, TraceParams))
		{
			//Processing Hit			
			AActor* HitActor = Hit.GetActor();	

			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			float ActualDamage = BaseDamage;

			if (SurfaceType == SURFACE_FLESHVULNERABLE)
			{
				ActualDamage *= 4.0f;
			}

			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, Owner->GetInstigatorController(), Owner, DamageType);			

			PlayImpactEffects(SurfaceType, Hit.ImpactPoint);
			TracerEndPoint = Hit.ImpactPoint;
		}

		if (DebugWeaponDrawing > 0)
		{
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);
		}
		if (Role == ROLE_Authority)
		{
			HitScanTrace.SurfaceType = SurfaceType;
			HitScanTrace.TraceTo = TracerEndPoint;
		}

		PlayFireEffects(TracerEndPoint);

		LastFireTime = GetWorld()->TimeSeconds;

	}
	if (CurrentAmmoInMagazine == 0)
	{
		StopFire();
		StartReload();
		return;
	}
}

//void ATPSBaseWeapon::ServerCheckAmmo()
//{
//}


void ATPSBaseWeapon::OnRep_HitScanTrace()
{
	PlayFireEffects(HitScanTrace.TraceTo);
	
	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}


void ATPSBaseWeapon::ServerFire_Implementation()
{
	Fire();
}


bool ATPSBaseWeapon::ServerFire_Validate()
{
	return true;
}


void ATPSBaseWeapon::StartFire()
{
	if (CurrentAmmoInMagazine > 0 && !OwningPlayer->GetReloading())
	{
		float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);

		GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ATPSBaseWeapon::Fire, TimeBetweenShots, true, FirstDelay);
	}
	else if (CanReload())
	{
		StartReload();
	}

}


void ATPSBaseWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}


void ATPSBaseWeapon::StartReload()
{
	if (CanReload())
	{
		GetWorldTimerManager().SetTimer(TimerHandle_Reload, this, &ATPSBaseWeapon::Reload, ReloadTime, false, ReloadTime);
		OwningPlayer->SetReloading(true);
	}
}


void ATPSBaseWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	UParticleSystem* SelectedEffect = nullptr;

	switch (SurfaceType)
	{
	case SURFACE_FLESHDEFAULT:
		SelectedEffect = FleshImpactEffect;
		break;
	case SURFACE_FLESHVULNERABLE:
		SelectedEffect = FleshImpactEffect;
		break;
	default:
		SelectedEffect = DefaultImpactEffect;
		break;
	}

	if (SelectedEffect)
	{
		FVector MuzzleLocation = MeshComponent->GetSocketLocation(MuzzleSocketName);
		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
	}
}


void ATPSBaseWeapon::PlayFireEffects(FVector TracerEndPoint)
{
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComponent, MuzzleSocketName);
	}

	if (TracerEffect)
	{
		FVector MuzzleLocation = MeshComponent->GetSocketLocation(MuzzleSocketName);
		UParticleSystemComponent* TracerComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (TracerComponent)
		{
			TracerComponent->SetVectorParameter(TracerTargetName, TracerEndPoint);
		}
	}

	APawn* Owner = Cast<APawn>(GetOwner());
	if (Owner)
	{
		APlayerController* PC = Cast<APlayerController>(Owner->GetController());
		if (PC)
		{
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}

}

void ATPSBaseWeapon::Reload()
{
	float ReloadingAmount = FMath::Min(CurrentAmmo, MagazineSize - CurrentAmmoInMagazine);
	CurrentAmmo -= ReloadingAmount;
	CurrentAmmoInMagazine += ReloadingAmount;
	OwningPlayer->SetReloading(false);
}

// Called every frame
void ATPSBaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void ATPSBaseWeapon::SetOwningPlayer(ATPSCharacter * NewOwningPlayer)
{
	OwningPlayer = NewOwningPlayer;

}


void ATPSBaseWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ATPSBaseWeapon, HitScanTrace, COND_SkipOwner);
}

