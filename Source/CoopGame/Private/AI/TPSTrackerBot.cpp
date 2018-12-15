// Fill out your copyright notice in the Description page of Project Settings.

#include "TPSTrackerBot.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "AI/Navigation/NavigationSystem.h"
#include "AI/Navigation/NavigationPath.h"
#include "DrawDebugHelpers.h"
#include "TPSHealthComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SphereComponent.h"
#include "TimerManager.h"
#include "TPSCharacter.h"
#include "Sound/SoundCue.h"


static int32 DebugTrackerBotDrawing = 0;

FAutoConsoleVariableRef CVARDebugTrackerBotDrawing(TEXT("COOP.DebugTrackerBot"), DebugTrackerBotDrawing, TEXT("Draw Debug Lines for TrackerBot"), ECVF_Cheat);


// Sets default values
ATPSTrackerBot::ATPSTrackerBot()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
	MeshComponent->SetCanEverAffectNavigation(false);
	MeshComponent->SetSimulatePhysics(true);

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetupAttachment(RootComponent);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);	

	HealthComponent = CreateDefaultSubobject<UTPSHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->OnHealthChanged.AddDynamic(this, &ATPSTrackerBot::HandleTakeDamage);

	MovementForce = 1000;
	bUseVelocityChange = true;
	RequiredDistanceToTarget = 100;
	
	ExplosionDamage = 60;
	ExplosionRadius = 350;
	SelfDamageInterval = 0.25f;

	PowerLevel = 0;
	MaxPowerLevel = 4;
	PowerLevelRadius = 300;

	SphereComponent->SetSphereRadius(ExplosionRadius);
}

// Called when the game starts or when spawned
void ATPSTrackerBot::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority)
	{
		NextPathPoint = GetNextPathPoint();
	}	
}


void ATPSTrackerBot::HandleTakeDamage(UTPSHealthComponent * HealthComp, float Health, float HealthDelta, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
	if (MaterialInstance == nullptr)
	{
		MaterialInstance = MeshComponent->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComponent->GetMaterial(0));
	}

	if (MaterialInstance)
	{
		MaterialInstance->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}

	if (Health <= 0.0f)
	{
		SelfDestruct();
	}

	//UE_LOG(LogTemp, Log, TEXT("Health %s of %s //HealthDelta = %s"), *FString::SanitizeFloat(Health), *GetName(), *FString::SanitizeFloat(HealthDelta));

}


FVector ATPSTrackerBot::GetNextPathPoint()
{

	AActor* BestTarget = nullptr;
	float NearestTargetDistance = MAX_FLT;

	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		APawn* TestPawn = It->Get();
		if (TestPawn == nullptr || UTPSHealthComponent::IsFriendly(TestPawn, this))
		{
			continue;
		}
		UTPSHealthComponent* TestHealthComponent = Cast<UTPSHealthComponent>(TestPawn->GetComponentByClass(UTPSHealthComponent::StaticClass()));

		if (TestHealthComponent && TestHealthComponent->GetHealth() > 0.0f)
		{
			float DistanceToTestPawn = (TestPawn->GetActorLocation() - GetActorLocation()).Size();
			if (DistanceToTestPawn < NearestTargetDistance)
			{
				BestTarget = TestPawn;
				NearestTargetDistance = DistanceToTestPawn;
			}		
		}
	}
	
	if (BestTarget)
	{
		UNavigationPath* NavPath = UNavigationSystem::FindPathToActorSynchronously(this, GetActorLocation(), BestTarget);

		GetWorldTimerManager().ClearTimer(TimerHandle_RefreshPath);
		GetWorldTimerManager().SetTimer(TimerHandle_RefreshPath, this, &ATPSTrackerBot::RefreshPath, 5.0f, false);

		if (NavPath)
		{
			if (NavPath->PathPoints.Num() > 1)
			{
				return NavPath->PathPoints[1];
			}
		}
	}



	return GetActorLocation();
}


void ATPSTrackerBot::SelfDestruct()
{
	if (bExploded)
	{
		return;
	}

	bExploded = true;

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());

	UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());

	MeshComponent->SetVisibility(false, true);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (Role == ROLE_Authority)
	{
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		float FinalDamage = ExplosionDamage;
		if (PowerLevel > 1)
		{
			 FinalDamage *= PowerLevel;
		}

		UGameplayStatics::ApplyRadialDamage(this, FinalDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController(), true);

		if (DebugTrackerBotDrawing)
		{
			DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 2.0f, 0, 1.0f);
		}
		

		SetLifeSpan(2.0f);
	}

}


void ATPSTrackerBot::NotifyActorBeginOverlap(AActor * OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (!bStartedSelfDestruct && !bExploded)
	{
		ATPSCharacter* PlayerPawn = Cast<ATPSCharacter>(OtherActor);

		if (PlayerPawn && !UTPSHealthComponent::IsFriendly(OtherActor, this))
		{
			// Possible errors but non critical
			bStartedSelfDestruct = true;

			if (Role == ROLE_Authority)
			{
				GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ATPSTrackerBot::DamageSelf, SelfDamageInterval, true, 0.0f);
			}

			UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);
		}
	}

}


void ATPSTrackerBot::RefreshPath()
{
	NextPathPoint = GetNextPathPoint();
}

void ATPSTrackerBot::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this, 20, GetInstigatorController(), this, nullptr);
}

// Called every frame
void ATPSTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Role == ROLE_Authority && !bExploded)
	{
		//Moving

		float Distance = (GetActorLocation() - NextPathPoint).Size();

		if (Distance <= RequiredDistanceToTarget)
		{
			NextPathPoint = GetNextPathPoint();

			//DrawDebugString(GetWorld(), GetActorLocation(), "Target Reached");
		}
		else
		{
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			ForceDirection.Normalize();
			ForceDirection *= MovementForce;

			MeshComponent->AddForce(ForceDirection, NAME_None, bUseVelocityChange);

			if (DebugTrackerBotDrawing)
			{
				DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32, FColor::Yellow, false, 0.0f, 0, 1.0f);
			}

		}
		if (DebugTrackerBotDrawing)
		{
			DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Yellow, false, 0.0f, 0, 2.0f);
		}	

		//Increase PowerLevel

		uint8 TempPowerLevel = 0;

		for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
		{
			ATPSTrackerBot* TestPawn = Cast<ATPSTrackerBot>(It->Get());

			if (TestPawn == nullptr || TestPawn == this)
			{
				//UE_LOG(LogTemp, Log, TEXT("Cast fail"));
				continue;				
			}

			float DistanceToTestPawn = (TestPawn->GetActorLocation() - GetActorLocation()).Size();

			if (DistanceToTestPawn > PowerLevelRadius)
			{
				//UE_LOG(LogTemp, Log, TEXT("Distance fail"));
				continue;
			}		

			UTPSHealthComponent* HealthComponent = Cast<UTPSHealthComponent>(TestPawn->GetComponentByClass(UTPSHealthComponent::StaticClass()));

			if (HealthComponent && HealthComponent->GetHealth() > 0.0f)
			{
				TempPowerLevel++;
			}
		}
		PowerLevel = FMath::Clamp(TempPowerLevel,(uint8)0, MaxPowerLevel);
		//UE_LOG(LogTemp, Log, TEXT("PowerLevel = %d "), PowerLevel);

		if (MaterialInstance == nullptr)
		{
			MaterialInstance = MeshComponent->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComponent->GetMaterial(0));
		}

		float MaterialAlpha = PowerLevel / (float)MaxPowerLevel;

		if (MaterialInstance)
		{			
			MaterialInstance->SetScalarParameterValue("PowerLevelAlpha", MaterialAlpha);
		}
	}

}



