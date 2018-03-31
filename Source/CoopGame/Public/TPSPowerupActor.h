// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TPSPowerupActor.generated.h"

UCLASS()
class COOPGAME_API ATPSPowerupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATPSPowerupActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	float PowerupInterval;

	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	int32 TotalNrOfTicks;

	int32 TicksProcessed;

	FTimerHandle TimerHandle_PowerupTicks;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnActivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnExpired();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnPowerupTicked();

	UFUNCTION()
	void OnTickPowerup();

	void ActivatePowerUp();

	
	
};
