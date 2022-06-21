// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"

#include "Engine/DataTable.h"

#include "ItemStructs.generated.h"

class AItem_Base;

UENUM(BlueprintType)
enum class EItemType : uint8
{
    None,
    Resource,
    Consumable,
    Weapon
};

USTRUCT(BlueprintType)
struct PROJECTH_API FItemData : public FTableRowBase
{
    GENERATED_BODY()

    FItemData();

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EItemType Type;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "This is used to look up type specific data in a data table"))
    FName TypeName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText Name;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 MaxAmount;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UTexture2D* Icon;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UStaticMesh* StaticMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "This is used to override the material on the mesh, if this is not set the default material will be used"))
    UMaterialInterface* Material;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "This is used to spawn the item in the world and also holds the functionality if there is any"))
    TSubclassOf<AItem_Base> Class;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName AnimationSet;
};

USTRUCT(BlueprintType)
struct PROJECTH_API FItemAnimations : public FTableRowBase
{
    GENERATED_BODY()

    FItemAnimations();

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UAnimMontage* IdleToIdleUsableMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UAnimMontage* IdleUsableToIdleMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<UAnimMontage*> UseMontages;
};