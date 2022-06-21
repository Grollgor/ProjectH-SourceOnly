// Fill out your copyright notice in the Description page of Project Settings.
#include "DamageTypes/DamageType_Fire.h"

#include "Components/AttributeComponent.h"

#include "Weapons/Weapon_Base.h"

void UDamageType_Fire::HandleDamage(UAttributeComponent* Attributes, AWeapon_Base* Weapon, float Damage)
{
    if (Weapon)
    {
        DefaultHandleDamage(Attributes, Weapon->WeaponData.FireDamage, Attributes->FireResistance, EDamageType::Fire);

        Attributes->FireStatus.CurrentBuildup = FMath::Min(Attributes->FireStatus.CurrentBuildup + Weapon->WeaponData.FireApplied, Attributes->FireStatus.BuildupCap);
        if (Attributes->FireStatus.CurrentBuildup > Attributes->FireStatus.ActivationThreshold)
        {
            Attributes->Server_OnStartBurning();
        }
    }
    else
    {
        DefaultHandleDamage(Attributes, Damage, Attributes->FireResistance, EDamageType::Fire);
    }
}