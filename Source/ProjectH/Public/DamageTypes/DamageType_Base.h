// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "CoreMinimal.h"

#include "GameFramework/DamageType.h"

#include "DamageType_Base.generated.h"

class AWeapon_Base;

UENUM(BlueprintType, meta=(ScriptName="PHDamageType"))
enum class EDamageType : uint8
{
	None,
	Physical,
	Fire,
	Ice,
};

UCLASS()
class PROJECTH_API UDamageType_Base : public UDamageType
{
	GENERATED_BODY()
public:
    static void ApplyDamage(class UAttributeComponent* Attributes, AWeapon_Base* Weapon, float Damage, EDamageType DamageType);

public:
	EDamageType DamageType = EDamageType::None;
};

void DefaultHandleDamage(class UAttributeComponent* Attributes, float Damage, float Resistance, EDamageType DamageType);
