// Fill out your copyright notice in the Description page of Project Settings.

#include "TPSPickupActor.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "TPSPowerupActor.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "TPSCharacter.h"

// Sets default values
ATPSPickupActor::ATPSPickupActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetSphereRadius(75.0f);
	RootComponent = SphereComponent;

	DecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComponent"));
	DecalComponent->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	DecalComponent->DecalSize = FVector(65, 75, 75);
	DecalComponent->SetupAttachment(RootComponent);

	CooldownRate = 5.0f;
}

// Called when the game starts or when spawned
void ATPSPickupActor::BeginPlay()
{
	Super::BeginPlay();
	
	Respawn();
}


void ATPSPickupActor::Respawn()
{
	if (PowerUpClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("PowerUpClass is nullptr in %s. Please update your BP"), *GetName());
		return;
	}
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	PowerUpInstance = GetWorld()->SpawnActor<ATPSPowerupActor>(PowerUpClass, GetTransform(), SpawnParams);

}

// Called every frame
void ATPSPickupActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATPSPickupActor::NotifyActorBeginOverlap(AActor * OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	
	ATPSCharacter* PlayerPawn = Cast<ATPSCharacter>(OtherActor);
	if (PlayerPawn == nullptr)
	{
		return;
	}

	if (PowerUpInstance)
	{
		PowerUpInstance->ActivatePowerUp();
		PowerUpInstance = nullptr;

		GetWorldTimerManager().SetTimer(TimerHandle_RespawnTimer, this, &ATPSPickupActor::Respawn, CooldownRate);
	}
}

