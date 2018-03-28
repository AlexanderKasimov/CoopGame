// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TPSCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class ATPSBaseWeapon;
class UTPSHealthComponent;


UCLASS()
class COOPGAME_API ATPSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATPSCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);

	void MoveRight(float Value);

	void BeginCrouch();

	void EndCrouch();
	
	void BeginZoom();

	void EndZoom();

	void StartFire();

	void StopFire();

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<ATPSBaseWeapon> StarterWeaponClass;
	
	UPROPERTY(Replicated)
	ATPSBaseWeapon* CurrentWeapon;

	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
	FName WeaponAttachementSocketName;
		
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComponent;	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UTPSHealthComponent* HealthComponent;

	UPROPERTY(Replicated,BlueprintReadOnly, Category = "Player")
	bool bDead;

	UFUNCTION()
	void OnHealthChanged(UTPSHealthComponent* HealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	bool bWantsToZoom;

	//Set at begin play
	float DefaultFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float ZoomedFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin=0.1, ClampMax=100))
	float ZoomInterpSpeed;

	UFUNCTION(BlueprintImplementableEvent)
	void RestartLevel();


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetPawnViewLocation() const override;
	
};
