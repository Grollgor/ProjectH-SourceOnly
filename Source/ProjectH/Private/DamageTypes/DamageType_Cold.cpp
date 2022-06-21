// Fill out your copyright notice in the Description page of Project Settings.
#include "DamageTypes/DamageType_Cold.h"

#include "Components/AttributeComponent.h"

#include "Weapons/Weapon_Base.h"

void UDamageType_Cold::HandleDamage(UAttributeComponent* Attributes, AWeapon_Base* Weapon, float Damage)
{
    if (Weapon)
    {
        DefaultHandleDamage(Attributes, Weapon->WeaponData.ColdDamage, Attributes->ColdResistance, EDamageType::Ice);

        Attributes->ColdStatus.CurrentBuildup = FMath::Min(Attributes->ColdStatus.CurrentBuildup + Weapon->WeaponData.ColdApplied, Attributes->ColdStatus.BuildupCap);
        if (Attributes->ColdStatus.CurrentBuildup > Attributes->ColdStatus.ActivationThreshold)
        {
            Attributes->Server_OnStartFreezing();
        }
    }
    else
    {
        DefaultHandleDamage(Attributes, Damage, Attributes->ColdResistance, EDamageType::Ice);
    }
}