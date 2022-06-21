// Fill out your copyright notice in the Description page of Project Settings.
#include "Structs/ItemStructs.h"

FItemData::FItemData()
{
    Type = EItemType::None;
    TypeName = FName("");
    MaxAmount = -1;
    Icon = nullptr;
    StaticMesh = nullptr;
    Material = nullptr;
    AnimationSet = FName("");
}

FItemAnimations::FItemAnimations()
{
    IdleToIdleUsableMontage = nullptr;
    IdleUsableToIdleMontage = nullptr;
}
