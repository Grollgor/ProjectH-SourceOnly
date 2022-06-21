// Fill out your copyright notice in the Description page of Project Settings.
#include "Items/Item_Base.h"

#include "Components/StaticMeshComponent.h"

AItem_Base::AItem_Base()
{
	PrimaryActorTick.bCanEverTick = false;

    bReplicates = true;
    SetReplicateMovement(true);

    Owner = nullptr;
    Amount = 1;
    bInCombat = false;

    DataTable = nullptr;
    static ConstructorHelpers::FObjectFinder<UDataTable> DataTableObject(TEXT("DataTable'/Game/ProjectH/Core/DataTables/DT_Items.DT_Items'"));
    if (DataTableObject.Succeeded())
    {
        DataTable = DataTableObject.Object;
    }

    AnimationTable = nullptr;
    static ConstructorHelpers::FObjectFinder<UDataTable> AnimationTableObject(TEXT("DataTable'/Game/ProjectH/Core/DataTables/DT_ItemAnims.DT_ItemAnims'"));
    if (AnimationTableObject.Succeeded())
    {
        AnimationTable = AnimationTableObject.Object;
    }

    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
    SetRootComponent(StaticMesh);
    StaticMesh->SetGenerateOverlapEvents(false);
    StaticMesh->SetCollisionProfileName("NoCollision");
    StaticMesh->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
}

void AItem_Base::OnConstruction(const FTransform& Transform)
{
    if (!DataTable) return;

    FItemData* Data = DataTable->FindRow<FItemData>(Name, TEXT(""), true);
    if (Data)
    {
        if (Data->StaticMesh)
        {
            StaticMesh->SetStaticMesh(Data->StaticMesh);
            if (Data->Material)
            {
                StaticMesh->SetMaterial(0, Data->Material);
            }
        }

        FItemAnimations* AnimPtr = AnimationTable->FindRow<FItemAnimations>(Data->AnimationSet, TEXT(""));
        if (AnimPtr)
        {
            Anims = *AnimPtr;
        }
    }
}

void AItem_Base::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AItem_Base, Name);
    DOREPLIFETIME(AItem_Base, Amount);

    DOREPLIFETIME(AItem_Base, bInCombat);
}

void AItem_Base::Multicast_EnablePhysics_Implementation()
{
    StaticMesh->SetCollisionProfileName(FName("IgnoreOnlyPawn"));
    StaticMesh->SetSimulatePhysics(true);
    StaticMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void AItem_Base::Multicast_SetVisuals_Implementation(FName ItemName)
{
    FItemData* Data = DataTable->FindRow<FItemData>(ItemName, TEXT(""), true);
    if (Data)
    {
        if (Data->StaticMesh)
        {
            StaticMesh->SetStaticMesh(Data->StaticMesh);
            if (Data->Material)
            {
                StaticMesh->SetMaterial(0, Data->Material);
            }
        }

        FItemAnimations* AnimPtr = AnimationTable->FindRow<FItemAnimations>(Data->AnimationSet, TEXT(""));
        if (AnimPtr)
        {
            Anims = *AnimPtr;
        }
    }
}

void AItem_Base::Multicast_SetMesh_Implementation(USkeletalMeshComponent* Mesh)
{
    OwningMesh = Mesh;
}

void AItem_Base::Server_SetCombatState_Implementation()
{
    bInCombat = !bInCombat;
    Multicast_SetCombatState(bInCombat);
}

void AItem_Base::Multicast_SetCombatState_Implementation(bool bIsInCombat)
{
    UAnimMontage* Montage;
    if (bIsInCombat)
    {
        Montage = Anims.IdleToIdleUsableMontage;
    }
    else
    {
        Montage = Anims.IdleUsableToIdleMontage;
    }

    UAnimInstance* AnimInstance = OwningMesh->GetAnimInstance();
    if (AnimInstance)
    {
        AnimInstance->Montage_Play(Montage);
    }
}

void AItem_Base::OnEquip()
{
    Server_OnEquip();
}

void AItem_Base::Server_OnEquip_Implementation()
{
    Server_OnEquipDelegate.Broadcast();

    Client_OnEquip();
    Multicast_OnEquip();
}

void AItem_Base::Client_OnEquip_Implementation()
{
    Client_OnEquipDelegate.Broadcast();
}

void AItem_Base::Multicast_OnEquip_Implementation()
{
    Multicast_OnEquipDelegate.Broadcast();
}

void AItem_Base::OnUnequip()
{
    Server_OnUnequip();
}

void AItem_Base::Server_OnUnequip_Implementation()
{
    Server_OnUnequipDelegate.Broadcast();

    Client_OnUnequip();
    Multicast_OnUnequip();
}

void AItem_Base::Client_OnUnequip_Implementation()
{
    Client_OnUnequipDelegate.Broadcast();
}

void AItem_Base::Multicast_OnUnequip_Implementation()
{
    Multicast_OnUnequipDelegate.Broadcast();
}

void AItem_Base::OnUse()
{
    Server_OnUse();
}

void AItem_Base::Server_OnUse_Implementation()
{
    Server_OnUseDelegate.Broadcast();

    Client_OnUse();
    Multicast_OnUse();
}

void AItem_Base::Client_OnUse_Implementation()
{
    Client_OnUseDelegate.Broadcast();
}

void AItem_Base::Multicast_OnUse_Implementation()
{
    Multicast_OnUseDelegate.Broadcast();
}
