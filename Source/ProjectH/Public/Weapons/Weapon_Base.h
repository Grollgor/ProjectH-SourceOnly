// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "Items/Item_Base.h"

#include "Weapon_Base.generated.h"

class UStaticMeshComponent;
class UMaterial;

USTRUCT(BlueprintType)
struct PROJECTH_API FWeaponData : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float PhysicalDamage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float FireDamage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float ColdDamage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float FireApplied;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float ColdApplied;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Range;
};

UCLASS(Blueprintable)
class PROJECTH_API AWeapon_Base : public AItem_Base
{
	GENERATED_BODY()
	
public:
	AWeapon_Base();

protected:
    virtual void OnConstruction(const FTransform& Transform);

public:
    virtual void SetCanAdvanceCombo(bool bCanAdvance) override {}
    virtual void SetCanStartNewCombo(bool bCanStartCombo) override {}
    virtual void DoDamage() override {}

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FWeaponData WeaponData;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UDataTable* WeaponDataTable;
};
