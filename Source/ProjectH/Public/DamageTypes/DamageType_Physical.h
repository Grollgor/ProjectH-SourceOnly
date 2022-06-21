// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DamageTypes/DamageType_Base.h"
#include "DamageType_Physical.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTH_API UDamageType_Physical : public UDamageType_Base
{
	GENERATED_BODY()
public:
	UDamageType_Physical() { DamageType = EDamageType::Physical; }

	static void HandleDamage(class UAttributeComponent* Attributes, AWeapon_Base* Weapon, float Damage);
};

