// Fill out your copyright notice in the Description page of Project Settings.

#include "TPSPowerupActor.h"
#include "TimerManager.h"


// Sets default values
ATPSPowerupActor::ATPSPowerupActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PowerupInterval = 0.0f;
	TotalNrOfTicks = 0;

}

// Called when the game starts or when spawned
void ATPSPowerupActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATPSPowerupActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void ATPSPowerupActor::OnTickPowerup()
{
	OnPowerupTicked();

	if (TicksProcessed >= TotalNrOfTicks)
	{
		OnExpired();
		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTicks);
	}

	TicksProcessed++;

}


void ATPSPowerupActor::ActivatePowerUp()
{
	OnActivated();
	if (PowerupInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_PowerupTicks, this, &ATPSPowerupActor::OnTickPowerup, PowerupInterval, true, 0.0f);
	}
	else
	{
		OnTickPowerup();
	}

}

