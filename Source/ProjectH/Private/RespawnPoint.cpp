// Fill out your copyright notice in the Description page of Project Settings.

#include "RespawnPoint.h"

#include "Components/SphereComponent.h"
#include "Components/ArrowComponent.h"

#include "Components/CapsuleComponent.h"

#include "PlayerCharacter.h"

// Sets default values
ARespawnPoint::ARespawnPoint()
{
 	// Respawn points should only ever react to events, so I don't need ticks for them
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// NOTE(philipp): This actor need to replicate or RPCs won't be called correctly
	bReplicates = true;
	SetReplicateMovement(false);

	SetCanBeDamaged(false);

	auto Root = GetRootComponent();

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->SetupAttachment(Root);
	Collision->InitSphereRadius(500.f);

	Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
    Arrow->SetupAttachment(Collision);
	Arrow->SetRelativeScale3D({ 6.25f, 6.25f, 6.25f });

    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    StaticMesh->SetupAttachment(Collision);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> StaticMeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Cube1.Cube1'"));
    if (StaticMeshAsset.Object)
    {
        StaticMesh->SetStaticMesh(StaticMeshAsset.Object);
    }
	StaticMesh->SetCollisionProfileName(TEXT("NoCollision"));
}

void ARespawnPoint::Server_OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (IsValid(Player))
	{
		FVector Offset(0.f);
		Offset.Z = Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

		// TODO(philipp): For multiplayer it probable isn't a good idea to spawn all players on the same location
		//                so select a random point in the area maybe? Or have slots that get cycled through
		Player->SetRespawnPoint(GetActorLocation(), GetActorRotation());
	}
}

void ARespawnPoint::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		Collision->OnComponentBeginOverlap.AddDynamic(this, &ARespawnPoint::Server_OnOverlapBegin);
	}
}

void ARespawnPoint::EndPlay(EEndPlayReason::Type EndReason)
{
	if (HasAuthority())
	{
		Collision->OnComponentBeginOverlap.RemoveDynamic(this, &ARespawnPoint::Server_OnOverlapBegin);
	}
}
