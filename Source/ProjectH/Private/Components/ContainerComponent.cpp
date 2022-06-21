// Fill out your copyright notice in the Description page of Project Settings.
#include "Components/ContainerComponent.h"

#include "Net/UnrealNetwork.h"

#include "Items/Item_Base.h"

#include "PlayerCharacter.h"

DEFINE_LOG_CATEGORY(PHContainer);

UContainerComponent::UContainerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

    SetIsReplicatedByDefault(true);

    DataTable = nullptr;
    static ConstructorHelpers::FObjectFinder<UDataTable> DataTableObject(TEXT("DataTable'/Game/ProjectH/Core/DataTables/DT_Items.DT_Items'"));
    if (DataTableObject.Succeeded())
    {
        DataTable = DataTableObject.Object;
    }
}

void UContainerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UContainerComponent, Items);
}

void UContainerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UContainerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UContainerComponent::AddItems(FName ItemName, int32 Amount)
{
    // NOTE(philipp): Make sure we only ever add an item if we are the server/hosting client
    check(GetOwner()->HasAuthority());
    check(Amount > 0);

    FItemData* Data = DataTable->FindRow<FItemData>(ItemName, TEXT(""));
    check(Data);

    for (FContainerItem& Item : Items)
    {
        if (Item.ItemName == ItemName)
        {
            if (Data->MaxAmount != -1 && (Item.Amount + Amount > Data->MaxAmount))
            {
                // TODO(philipp): I probably should return here how many items I couldn't consume, instead of doing nothing
                return false;
            }
            Item.Amount += Amount;
            
            OnItemPickedUp(DataTable, ItemName, Amount);
            return true;
        }
    }
        
    FContainerItem NewItem;
    NewItem.ItemName = ItemName;
    NewItem.Amount = Amount;
    NewItem.bIsEquipped = false;
    Items.Add(NewItem);
    OnNewItemPickedUp(DataTable, ItemName, Amount);
    return true;
}

void UContainerComponent::RemoveItems(FName ItemName, int32 Index, int32 Amount)
{
    check(GetOwner()->HasAuthority());
    check(Amount > 0);
    check(ItemName == Items[Index].ItemName);
    
    Items[Index].Amount -= Amount;
    check(Items[Index].Amount >= 0);
    if (Items[Index].Amount == 0)
    {
        Items.RemoveAt(Index, 1, true);
    }
}

int32 UContainerComponent::QueryItem(FName ItemName) const
{
    for (int32 Index = 0; Index < Items.Num(); Index++)
    {
        if (Items[Index].ItemName == ItemName)
        {
            return Index;
        }
    }
    return -1;
}

void UContainerComponent::OnNewItemPickedUp(const UDataTable* InDataTable, FName ItemName, int32 Amount)
{
    Server_OnNewItemPickedUp(InDataTable, ItemName, Amount);
}

void UContainerComponent::Server_OnNewItemPickedUp_Implementation(const UDataTable* InDataTable, FName ItemName, int32 Amount)
{
    UE_LOG(PHContainer, Log, TEXT("New item picked up on server"));
    Server_OnNewItemPickedUpDelegate.Broadcast(DataTable, ItemName, Amount);

    Client_OnNewItemPickedUp(InDataTable, ItemName, Amount);
    Multicast_OnNewItemPickedUp(InDataTable, ItemName, Amount);
}

void UContainerComponent::Client_OnNewItemPickedUp_Implementation(const UDataTable* InDataTable, FName ItemName, int32 Amount)
{
    UE_LOG(PHContainer, Log, TEXT("New item picked up broadcasted to client"));
    Client_OnNewItemPickedUpDelegate.Broadcast(InDataTable, ItemName, Amount);
}

void UContainerComponent::Multicast_OnNewItemPickedUp_Implementation(const UDataTable* InDataTable, FName ItemName, int32 Amount)
{
    UE_LOG(PHContainer, Log, TEXT("New item picked up broadcasted to multicast"));
    Multicast_OnNewItemPickedUpDelegate.Broadcast(InDataTable, ItemName, Amount);
}

void UContainerComponent::OnItemPickedUp(const UDataTable* InDataTable, FName ItemName, int32 Amount)
{
    Server_OnItemPickedUp(InDataTable, ItemName, Amount);
}

void UContainerComponent::Server_OnItemPickedUp_Implementation(const UDataTable* InDataTable, FName ItemName, int32 Amount)
{
    UE_LOG(PHContainer, Log, TEXT("Item picked up on server"));
    Server_OnItemPickedUpDelegate.Broadcast(InDataTable, ItemName, Amount);

    Client_OnItemPickedUp(InDataTable, ItemName, Amount);
    Multicast_OnItemPickedUp(InDataTable, ItemName, Amount);
}

void UContainerComponent::Client_OnItemPickedUp_Implementation(const UDataTable* InDataTable, FName ItemName, int32 Amount)
{
    UE_LOG(PHContainer, Log, TEXT("Item picked up broadcasted to client"));
    Client_OnItemPickedUpDelegate.Broadcast(InDataTable, ItemName, Amount);
}

void UContainerComponent::Multicast_OnItemPickedUp_Implementation(const UDataTable* InDataTable, FName ItemName, int32 Amount)
{
    UE_LOG(PHContainer, Log, TEXT("Item picked up broadcasted to multicast"));
    Multicast_OnItemPickedUpDelegate.Broadcast(InDataTable, ItemName, Amount);
}

void UContainerComponent::DropItems(FName ItemName, int32 Index, int32 Amount)
{
    Server_DropItems(ItemName, Index, Amount);
}

void UContainerComponent::Server_DropItems_Implementation(FName ItemName, int32 Index, int32 Amount)
{
    check(ItemName == Items[Index].ItemName);

    // NOTE(philipp): Weapons are currently limited to 1 per type so I don't need to check if there is another one of that item
    if (Items[Index].bIsEquipped)
    {
        UnequipItem();
    }
    RemoveItems(ItemName, Index, Amount);

    FVector Direction = GetOwner()->GetActorRotation().Vector();
    FVector SpawnLocation = GetOwner()->GetActorLocation() + Direction * 50.f;

    FTransform SpawnTransform;
    SpawnTransform.SetLocation(SpawnLocation);

    FItemData* Data = DataTable->FindRow<FItemData>(ItemName, TEXT(""));
    check(Data);

    AItem_Base* NewItem = GetWorld()->SpawnActorDeferred<AItem_Base>(Data->Class, SpawnTransform);
    NewItem->Name = ItemName;
    NewItem->FinishSpawning(SpawnTransform);
    NewItem->Multicast_SetVisuals(ItemName);

    NewItem->Amount = Amount;
    NewItem->Multicast_EnablePhysics();
}

void UContainerComponent::EquipItem(FName ItemName, int32 Index)
{
    Server_EquipItem(ItemName, Index);
}

void UContainerComponent::Server_EquipItem_Implementation(FName ItemName, int32 Index)
{
    APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwner());
    if (!Player) return;

    if (Player->EquippedItem)
    {
        Server_UnequipItem();
    }

    FItemData* Data = DataTable->FindRow<FItemData>(ItemName, TEXT(""));
    check(Data);

    Items[Index].bIsEquipped = true;

    ACharacter* Character = Cast<ACharacter>(GetOwner());
    AItem_Base* NewItem = SpawnItemIntoWorld(GetWorld(), Data, FTransform(), ItemName, Character->GetMesh());

    NewItem->AttachToComponent(Player->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("weapon_r"));
    Player->EquippedItem = NewItem;
    NewItem->OnEquip();
}

void UContainerComponent::UnequipItem()
{
    Server_UnequipItem();
}

void UContainerComponent::Server_UnequipItem_Implementation()
{
    APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwner());
    if (!Player && !Player->EquippedItem) return;

    Player->EquippedItem->OnUnequip();
    Multicast_UnequipItem(Player->EquippedItem);

    for (FContainerItem& OldItem : Items)
    {
        if (OldItem.bIsEquipped)
        {
            OldItem.bIsEquipped = false;
        }
    }
    Player->EquippedItem = nullptr;
}

void UContainerComponent::Multicast_UnequipItem_Implementation(AItem_Base* Item)
{
    Item->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    Item->SetActorLocation({ 0.f, 0.f, -10000.f });
    Item->Destroy();
    Item->MarkAsGarbage();
}

void UContainerComponent::UseItem(FName ItemName, int32 Index, int32 Amount)
{
    Server_UseItem(ItemName, Index, Amount);
}

void UContainerComponent::Server_UseItem_Implementation(FName ItemName, int32 Index, int32 Amount)
{
    check(ItemName == Items[Index].ItemName);

    FItemData* Data = DataTable->FindRow<FItemData>(ItemName, TEXT(""));
    check(Data);
    FTransform NewTransform = {};
    NewTransform.SetLocation({ 0.f, 0.f, -1000.f });
    
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    AItem_Base* NewItem = SpawnItemIntoWorld(GetWorld(), Data, NewTransform, ItemName, Character->GetMesh());

    RemoveItems(ItemName, Index, Amount);
    NewItem->OwningActor = GetOwner();
    NewItem->OnUse();
    NewItem->Destroy();
    NewItem->MarkAsGarbage();
}

AItem_Base* SpawnItemIntoWorld(UWorld* World, FItemData* Data, FTransform Transform, FName ItemName, USkeletalMeshComponent* OwningMesh)
{
    AItem_Base* NewItem = World->SpawnActorDeferred<AItem_Base>(Data->Class, Transform);
    NewItem->Name = ItemName;
    NewItem->FinishSpawning(Transform);
    NewItem->Multicast_SetVisuals(ItemName);
    NewItem->Multicast_SetMesh(OwningMesh);
    return NewItem;
}
