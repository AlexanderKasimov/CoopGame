// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExplosiveBarrel.generated.h"

class UTPSHealthComponent;
class UMaterial;
class URadialForceComponent;

UCLASS()
class COOPGAME_API AExplosiveBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AExplosiveBarrel();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
		UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
		UTPSHealthComponent* HealthComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
		UMaterial* ExplodedMat;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
		UParticleSystem* ExplosionFX;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
		float UpImpulseForce;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
		float RadialImpulseStrength;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
		float RadialImpulseRadius;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	TEnumAsByte<ERadialImpulseFalloff> FalloffType;

	UFUNCTION()
		void OnHealthChanged(UTPSHealthComponent* HealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
		URadialForceComponent* RadialForceComponent;

	void Explode();

	UPROPERTY(ReplicatedUsing = OnRep_bExploded, BlueprintReadOnly, Category = "Explosion")
	bool bExploded;

	UFUNCTION()
	void OnRep_bExploded();

	void RemoveRadialForceComponent();

	FTimerHandle TimerHandle_RemoveRadialForceComponent;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
