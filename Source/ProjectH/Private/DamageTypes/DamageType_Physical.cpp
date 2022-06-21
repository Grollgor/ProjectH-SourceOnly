// Fill out your copyright notice in the Description page of Project Settings.
#include "DamageTypes/DamageType_Physical.h"

#include "Components/AttributeComponent.h"

#include "Weapons/Weapon_Base.h"

void UDamageType_Physical::HandleDamage(class UAttributeComponent* Attributes, AWeapon_Base* Weapon, float Damage)
{
    DefaultHandleDamage(Attributes, Weapon ? Weapon->WeaponData.PhysicalDamage : Damage, Attributes->PhysicalResistance, EDamageType::Physical);
}
