// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"

#include "Components/ActorComponent.h"

#include "Items/Item_Base.h"

#include "ContainerComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(PHContainer, Log, All);

USTRUCT(BlueprintType)
struct PROJECTH_API FContainerItem
{
    GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Amount;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    uint8 bIsEquipped : 1;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnNewItemPickedUp, const UDataTable*, InDataTable, FName, ItemName, int32, Amount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemPickedUp, const UDataTable*, InDataTable, FName, ItemName, int32, Amount);

AItem_Base* SpawnItemIntoWorld(UWorld* World, FItemData* Data, FTransform Transform, FName ItemName, USkeletalMeshComponent* OwningMesh);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTH_API UContainerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UContainerComponent();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // NOTE(philipp): This should only ever be called by the server
	UFUNCTION(BlueprintCallable)
	bool AddItems(FName ItemName, int32 Amount = 1);

    UFUNCTION(BlueprintCallable)
    void RemoveItems(FName ItemName, int32 Index, int32 Amount = 1);

    // NOTE(philipp): If this function returns -1 then the item wasn't found, otherwise the index is returned
    UFUNCTION(BlueprintCallable)
    int32 QueryItem(FName ItemName) const;

    // NOTE(philipp): Base events
    //                Each of these functions have a Server_, Client_ and Multicast_ Delegate that you can bind to
    void OnNewItemPickedUp(const UDataTable* InDataTable, FName ItemName, int32 Amount);
    void OnItemPickedUp(const UDataTable* InDataTable, FName ItemName, int32 Amount);

    // ### OnNewItemPickedUp begin ###
    UFUNCTION(Server, Unreliable)
    void Server_OnNewItemPickedUp(const UDataTable* InDataTable, FName ItemName, int32 Amount);
    UPROPERTY(BlueprintAssignable)
    FOnNewItemPickedUp Server_OnNewItemPickedUpDelegate;

    UFUNCTION(Client, Unreliable)
    void Client_OnNewItemPickedUp(const UDataTable* InDataTable, FName ItemName, int32 Amount);
    UPROPERTY(BlueprintAssignable)
    FOnNewItemPickedUp Client_OnNewItemPickedUpDelegate;

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_OnNewItemPickedUp(const UDataTable* InDataTable, FName ItemName, int32 Amount);
    UPROPERTY(BlueprintAssignable)
    FOnNewItemPickedUp Multicast_OnNewItemPickedUpDelegate;
    // ### OnNewItemPickedUp end ###

    // ### OnItemPickedUp begin ###
    UFUNCTION(Server, Unreliable)
    void Server_OnItemPickedUp(const UDataTable* InDataTable, FName ItemName, int32 Amount);
    UPROPERTY(BlueprintAssignable)
    FOnItemPickedUp Server_OnItemPickedUpDelegate;

    UFUNCTION(Client, Unreliable)
    void Client_OnItemPickedUp(const UDataTable* InDataTable, FName ItemName, int32 Amount);
    UPROPERTY(BlueprintAssignable)
    FOnItemPickedUp Client_OnItemPickedUpDelegate;

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_OnItemPickedUp(const UDataTable* InDataTable, FName ItemName, int32 Amount);
    UPROPERTY(BlueprintAssignable)
    FOnItemPickedUp Multicast_OnItemPickedUpDelegate;
    // ### OnItemPickedUp end ###

    // ### DropItems begin ###
    UFUNCTION(BlueprintCallable)
    void DropItems(FName ItemName, int32 Index, int32 Amount);

    UFUNCTION(Server, Reliable)
    void Server_DropItems(FName ItemName, int32 Index, int32 Amount);
    // ### DropItems end ###

    // ### EquipItem begin ###
    UFUNCTION(BlueprintCallable)
    void EquipItem(FName ItemName, int32 Index);

    UFUNCTION(Server, Reliable)
    void Server_EquipItem(FName ItemName, int32 Index);
    // ### EquipItem end ###

    // ### EquipItem begin ###
    UFUNCTION(BlueprintCallable)
    void UnequipItem();

    UFUNCTION(Server, Reliable)
    void Server_UnequipItem();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_UnequipItem(AItem_Base* Item);
    // ### EquipItem end ###

    // ### UseItem begin ###
    UFUNCTION(BlueprintCallable)
    void UseItem(FName ItemName, int32 Index, int32 Amount);

    UFUNCTION(Server, Reliable)
    void Server_UseItem(FName ItemName, int32 Index, int32 Amount);
    // ### EquipItem end ###


protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	TArray<FContainerItem> Items;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UDataTable* DataTable;
};
