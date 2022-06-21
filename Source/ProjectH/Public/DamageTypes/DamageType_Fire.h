// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DamageTypes/DamageType_Base.h"
#include "DamageType_Fire.generated.h"

class AWeapon_Base;

UCLASS()
class PROJECTH_API UDamageType_Fire : public UDamageType_Base
{
	GENERATED_BODY()
public:
	UDamageType_Fire() { DamageType = EDamageType::Fire; }

	static void HandleDamage(class UAttributeComponent* Attributes, AWeapon_Base* Weapon, float Damage);
};
