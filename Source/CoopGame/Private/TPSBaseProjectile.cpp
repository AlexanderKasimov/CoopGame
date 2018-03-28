// Fill out your copyright notice in the Description page of Project Settings.

#include "TPSBaseProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"



// Sets default values
ATPSBaseProjectile::ATPSBaseProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	RootComponent = MeshComponent;

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	
	ProjectileMovementComponent->UpdatedComponent = MeshComponent;
	ProjectileMovementComponent->InitialSpeed = 1000.0f;
	ProjectileMovementComponent->MaxSpeed = 1000.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bShouldBounce = true;

	InitialLifeSpan = 3.0f;

	ExplosionRadius = 100.0f;

}

// Called when the game starts or when spawned
void ATPSBaseProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	GetWorldTimerManager().SetTimer(TimerHandle_Explode, this, &ATPSBaseProjectile::Explode, 1.0f);

}

void ATPSBaseProjectile::Explode()
{
	TArray<AActor*> IgnoredActors;
	//IgnoredActors.Add(this);
	//UGameplayStatics::GetPlayerController(this, 0)
	UGameplayStatics::ApplyRadialDamage(GetWorld(),  20.0f, GetActorLocation(), ExplosionRadius, DamageType, IgnoredActors, this,nullptr , false, ECollisionChannel::ECC_Visibility);
	if (ExplosionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	}

	static const auto CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("COOP.DebugWeapons"));
	int32 Value = CVar->GetInt();
	UE_LOG(LogTemp, Log, TEXT("CVAR = %d"), Value);
	if (Value > 0)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Yellow, false, 2.0f, 0, 1.0f);	
	}

	Destroy();

}

// Called every frame
void ATPSBaseProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

