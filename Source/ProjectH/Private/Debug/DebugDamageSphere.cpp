// Fill out your copyright notice in the Description page of Project Settings.
#include "Debug/DebugDamageSphere.h"

#include "Components/SphereComponent.h"

#include "Kismet/GameplayStatics.h"

#include "DamageTypes/DamageType_Base.h"

ADebugDamageSphere::ADebugDamageSphere()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

    //auto Root = GetRootComponent();

	Damage = 1.f;
	Radius = 50.f;

    Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
    //Collision->SetupAttachment(Root);
    Collision->InitSphereRadius(Radius);

	Collision->bHiddenInGame = false;
}

void ADebugDamageSphere::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		Collision->OnComponentBeginOverlap.AddDynamic(this, &ADebugDamageSphere::Server_OnOverlapBegin);
	}
}

void ADebugDamageSphere::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		Collision->OnComponentBeginOverlap.RemoveDynamic(this, &ADebugDamageSphere::Server_OnOverlapBegin);
	}

	Super::EndPlay(EndPlayReason);
}

void ADebugDamageSphere::OnConstruction(const FTransform& Transform)
{
    Collision->SetSphereRadius(Radius);
}

void ADebugDamageSphere::Server_OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	UGameplayStatics::ApplyDamage(OtherActor, Damage, nullptr, this, DamageType);
}
