// Fill out your copyright notice in the Description page of Project Settings.
#include "Pickups/Pickup_Base.h"

#include "Net/UnrealNetwork.h"

#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"

#include "Components/ContainerComponent.h"

#include "PlayerCharacter.h"

APickup_Base::APickup_Base()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
    SetReplicateMovement(false);

    SetCanBeDamaged(false);

    ItemScale = FVector(1.f);
    ItemOffset = FVector(0.f);
    ItemRotation = FRotator(0.f);

    auto Root = GetRootComponent();

    CapsuleCollider = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collider"));
    SetRootComponent(CapsuleCollider);
    CapsuleCollider->InitCapsuleSize(25.f, 50.f);
    CapsuleCollider->SetGenerateOverlapEvents(true);

    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> StaticMeshAsset(TEXT("StaticMesh'/Engine/VREditor/UI/SM_Radial_Disk.SM_Radial_Disk'"));
    if (StaticMeshAsset.Succeeded())
    {
        StaticMesh->SetStaticMesh(StaticMeshAsset.Object);
    }
    StaticMesh->SetupAttachment(CapsuleCollider);
    StaticMesh->SetRelativeScale3D(FVector(5.f));

    DataTable = nullptr;
    static ConstructorHelpers::FObjectFinder<UDataTable> DataTableObject(TEXT("DataTable'/Game/ProjectH/Core/DataTables/DT_Items.DT_Items'"));
    if (DataTableObject.Succeeded())
    {
        DataTable = DataTableObject.Object;
    }
}

void APickup_Base::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APickup_Base, ItemName);
    DOREPLIFETIME(APickup_Base, ActiveItem);
}

void APickup_Base::OnConstruction(const FTransform& Transform)
{
    if (Mesh)
    {
        StaticMesh->SetStaticMesh(Mesh);
    }

    if (ActiveItem)
    {
        ActiveItem->Destroy();
    }
    FItemData* Data = DataTable->FindRow<FItemData>(ItemName, TEXT(""));
    if (Data)
    {
        FActorSpawnParameters SpawnParams = {};
        SpawnParams.Owner = this;

        FTransform SpawnTransform;
        SpawnTransform.SetLocation(GetActorLocation());

        ActiveItem = GetWorld()->SpawnActorDeferred<AItem_Base>(Data->Class, SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
        if (ActiveItem)
        {
            ActiveItem->Name = ItemName;
            ActiveItem->FinishSpawning(SpawnTransform);

            ActiveItem->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName(""));
            ActiveItem->SetActorRelativeScale3D(ItemScale);
            ActiveItem->SetActorRelativeLocation(ItemOffset);
            ActiveItem->SetActorRelativeRotation(ItemRotation);
        }
    }
}

void APickup_Base::BeginPlay()
{
    if (HasAuthority())
    {
        CapsuleCollider->OnComponentBeginOverlap.AddDynamic(this, &APickup_Base::OnOverlapBegin);
    }
}

void APickup_Base::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (HasAuthority())
    {
        CapsuleCollider->OnComponentBeginOverlap.RemoveDynamic(this, &APickup_Base::OnOverlapBegin);
    }
}

void APickup_Base::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!ActiveItem) return;

    UContainerComponent* Container = Cast<UContainerComponent>(OtherActor->GetComponentByClass(UContainerComponent::StaticClass()));
    if (!Container) return;

    if (Container->AddItems(ItemName))
    {
        Multicast_OnItemPickUp(ActiveItem);
    }
}

void APickup_Base::Multicast_OnItemPickUp_Implementation(AActor* Item)
{
    Item->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    Item->SetActorLocation({ 0.f, 0.f, -10000.f });
    Item->Destroy();
    Item->MarkAsGarbage();
    ActiveItem = nullptr;
}

void APickup_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (ActiveItem)
    {
        ItemRotation.Yaw += 5.f * DeltaTime;
        if (ItemRotation.Yaw > 360.f)
        {
            ItemRotation.Yaw -= 360.f;
        }
        ActiveItem->SetActorRelativeRotation(ItemRotation);
    }
}

