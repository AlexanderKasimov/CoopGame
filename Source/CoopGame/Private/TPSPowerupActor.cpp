// Fill out your copyright notice in the Description page of Project Settings.

#include "TPSPowerupActor.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"


// Sets default values
ATPSPowerupActor::ATPSPowerupActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PowerupInterval = 0.0f;
	TotalNrOfTicks = 0;

	bIsPowerupActive = false;

	SetReplicates(true);

}

// Called when the game starts or when spawned
void ATPSPowerupActor::BeginPlay()
{
	Super::BeginPlay();
	
}


void ATPSPowerupActor::OnRep_PowerupActive()
{
	OnPowerupStateChanged(bIsPowerupActive);

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

		bIsPowerupActive = false;
		OnRep_PowerupActive();

		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTicks);
	}

	TicksProcessed++;

}


void ATPSPowerupActor::ActivatePowerUp(AActor* OverlappedActor)
{
	OnActivated(OverlappedActor);
	
	bIsPowerupActive = true;
	OnRep_PowerupActive();

	if (PowerupInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_PowerupTicks, this, &ATPSPowerupActor::OnTickPowerup, PowerupInterval, true, 0.0f);
	}
	else
	{
		OnTickPowerup();
	}

}

void ATPSPowerupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATPSPowerupActor, bIsPowerupActive);
}
