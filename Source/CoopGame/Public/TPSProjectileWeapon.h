// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TPSBaseWeapon.h"
#include "TPSProjectileWeapon.generated.h"

/**
 * 
 */

class ATPSBaseProjectile;

UCLASS()
class COOPGAME_API ATPSProjectileWeapon : public ATPSBaseWeapon
{
	GENERATED_BODY()
	
public:

	ATPSProjectileWeapon();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
		virtual void Fire() override;


protected:
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<ATPSBaseProjectile> ProjectileClass;
	

};
