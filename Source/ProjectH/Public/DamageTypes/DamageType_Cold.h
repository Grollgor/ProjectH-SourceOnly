// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DamageTypes/DamageType_Base.h"
#include "DamageType_Cold.generated.h"

class UAttributeComponent;
class AWeapon_Base;

UCLASS()
class PROJECTH_API UDamageType_Cold : public UDamageType_Base
{
	GENERATED_BODY()
public:
	UDamageType_Cold() { DamageType = EDamageType::Ice; }

	static void HandleDamage(UAttributeComponent* Attributes, AWeapon_Base* Weapon, float Damage);
};
