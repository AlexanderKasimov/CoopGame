// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TPSPickupActor.generated.h"

class USphereComponent;
class UDecalComponent;
class ATPSPowerupActor;

UCLASS()
class COOPGAME_API ATPSPickupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATPSPickupActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Componets")
	USphereComponent* SphereComponent;

	UPROPERTY(VisibleAnywhere, Category = "Componets")
	UDecalComponent* DecalComponent;

	UPROPERTY(EditAnywhere, Category = "Pickup Actor")
	TSubclassOf<ATPSPowerupActor> PowerUpClass;

	ATPSPowerupActor* PowerUpInstance;

	UFUNCTION()
	void Respawn();

	UPROPERTY(EditAnywhere, Category = "Pickup Actor")
	float CooldownRate;

	FTimerHandle TimerHandle_RespawnTimer;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	
};
