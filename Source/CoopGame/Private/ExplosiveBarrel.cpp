// Fill out your copyright notice in the Description page of Project Settings.

#include "ExplosiveBarrel.h"
#include "Materials/Material.h"
#include "Components/StaticMeshComponent.h"
#include "TPSHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AExplosiveBarrel::AExplosiveBarrel()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
	MeshComponent->SetCanEverAffectNavigation(false);
	MeshComponent->SetSimulatePhysics(true);

	HealthComponent = CreateDefaultSubobject<UTPSHealthComponent>(TEXT("HealthComponent"));

	RadialForceComponent = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComponent"));
	RadialForceComponent->SetupAttachment(RootComponent);

	bExploded = false;

	UpImpulseForce = 1000.0f;
	RadialImpulseStrength = 1000000.0f;
	RadialImpulseRadius = 500.0f;

	SetReplicates(true);
	SetReplicateMovement(true);
	
}

// Called when the game starts or when spawned
void AExplosiveBarrel::BeginPlay()
{
	Super::BeginPlay();


	HealthComponent->OnHealthChanged.AddDynamic(this, &AExplosiveBarrel::OnHealthChanged);
	RadialForceComponent->SetActive(false, true);

	RadialForceComponent->Radius = RadialImpulseRadius;
	RadialForceComponent->ImpulseStrength = RadialImpulseStrength;
	RadialForceComponent->ForceStrength = RadialImpulseStrength;
	RadialForceComponent->bIgnoreOwningActor = true;
}


void AExplosiveBarrel::OnHealthChanged(UTPSHealthComponent * HealthComp, float Health, float HealthDelta, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
	if (Health <= 0 && !bExploded)
	{
		bExploded = true;
		OnRep_bExploded();
		Explode();
	}

}


void AExplosiveBarrel::Explode()
{
	UE_LOG(LogTemp, Log, TEXT("Explode"));
	FVector Impulse = GetActorUpVector() * UpImpulseForce;
	MeshComponent->AddImpulse(Impulse, NAME_None, false);
	RadialForceComponent->SetActive(true, true);
	//URadialForceComponent* RadialForceComponent = NewObject<URadialForceComponent>(this);
	////RadialForceComponent->RegisterComponent();
	//RadialForceComponent->SetupAttachment(RootComponent);
	//RadialForceComponent->bIgnoreOwningActor = true;
	//RadialForceComponent->ImpulseStrength = RadialImpulseStrength;
	//RadialForceComponent->ForceStrength = RadialImpulseStrength;
	//RadialForceComponent->AddObjectTypeToAffect(EObjectTypeQuery::ObjectTypeQuery4);
	//RadialForceComponent->Falloff = FalloffType;
	GetWorldTimerManager().SetTimer(TimerHandle_RemoveRadialForceComponent, this, &AExplosiveBarrel::RemoveRadialForceComponent, 3.0f, false);

}

void AExplosiveBarrel::OnRep_bExploded()
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionFX, GetActorLocation());
	MeshComponent->SetMaterial(0, ExplodedMat);

}

void AExplosiveBarrel::RemoveRadialForceComponent()
{
	RadialForceComponent->SetActive(false, true);
	/*URadialForceComponent* RadialForceComponent = Cast<URadialForceComponent>(GetComponentByClass(URadialForceComponent::StaticClass()));
	RadialForceComponent->DestroyComponent();*/


}

// Called every frame
void AExplosiveBarrel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AExplosiveBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AExplosiveBarrel, bExploded);
}