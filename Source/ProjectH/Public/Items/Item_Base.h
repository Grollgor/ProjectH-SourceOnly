// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"

#include "Engine/DataTable.h"

#include "Net/UnrealNetwork.h"

#include "GameFramework/Actor.h"

#include "Structs/ItemStructs.h"

#include "Interfaces/ComboInterface.h"

#include "Item_Base.generated.h"

class UStaticMeshComponent;
class USkeletalMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEquip);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUnequip);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUse);

UCLASS(Blueprintable)
class PROJECTH_API AItem_Base : public AActor, public IComboInterface
{
	GENERATED_BODY()
	
public:	
    AItem_Base();

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_EnablePhysics();

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_SetVisuals(FName ItemName);

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_SetMesh(USkeletalMeshComponent* Mesh);

    UFUNCTION(Server, Reliable)
    void Server_SetCombatState();
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_SetCombatState(bool bIsInCombat);

    // NOTE(philipp): Base events
    //                Each of these functions have a Server_, Client_ and Multicast_ Delegate that you can bind to
    void OnEquip();
    void OnUnequip();
    void OnUse();

protected:
    virtual void OnConstruction(const FTransform& Transform) override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // NOTE(philipp): Just bind to the delegates if you want to implement functionality for the following events.
    //                They are available in C++ and Blueprints.
    // ### OnEquip begin ###
public:
    UFUNCTION(Server, Unreliable)
    void Server_OnEquip();
    UPROPERTY(BlueprintAssignable)
    FOnEquip Server_OnEquipDelegate;

    UFUNCTION(Client, Unreliable)
    void Client_OnEquip();
    UPROPERTY(BlueprintAssignable)
    FOnEquip Client_OnEquipDelegate;

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_OnEquip();
    UPROPERTY(BlueprintAssignable)
    FOnEquip Multicast_OnEquipDelegate;
    // ### OnEquip end ###

    // ### OnUnequip begin ###
public:
    UFUNCTION(Server, Unreliable)
    void Server_OnUnequip();
    UPROPERTY(BlueprintAssignable)
    FOnUnequip Server_OnUnequipDelegate;

    UFUNCTION(Client, Unreliable)
    void Client_OnUnequip();
    UPROPERTY(BlueprintAssignable)
    FOnUnequip Client_OnUnequipDelegate;

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_OnUnequip();
    UPROPERTY(BlueprintAssignable)
    FOnUnequip Multicast_OnUnequipDelegate;
    // ### OnUnequip end ###

    // ### OnUse begin ###
public:
    UFUNCTION(Server, Unreliable)
    void Server_OnUse();
    UPROPERTY(BlueprintAssignable)
    FOnUse Server_OnUseDelegate;

    UFUNCTION(Client, Unreliable)
    void Client_OnUse();
    UPROPERTY(BlueprintAssignable)
    FOnUse Client_OnUseDelegate;

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_OnUse();
    UPROPERTY(BlueprintAssignable)
    FOnUse Multicast_OnUseDelegate;
    // ### OnUse end ###

public:
    virtual void SetCanAdvanceCombo(bool bCanAdvance) override {}
    virtual void SetCanStartNewCombo(bool bCanStartCombo) override {}
    virtual void DoDamage() override {};
    virtual AItem_Base* GetEquippedItem() { return nullptr; }

public:
    UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly)
    FName Name;

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
    int32 Amount;

    UPROPERTY()
    AActor* OwningActor;

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
    uint8 bInCombat : 1;

    UPROPERTY()
    USkeletalMeshComponent* OwningMesh;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UDataTable* DataTable;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UDataTable* AnimationTable;

    UPROPERTY()
    UStaticMeshComponent* StaticMesh;

    UPROPERTY(VisibleAnywhere)
    FItemAnimations Anims;
};
