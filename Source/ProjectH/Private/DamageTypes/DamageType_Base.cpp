// Fill out your copyright notice in the Description page of Project Settings.
#include "DamageTypes/DamageType_Base.h"

#include "Components/AttributeComponent.h"

#include "Weapons/Weapon_Base.h"

#include "DamageTypes/DamageType_Physical.h"
#include "DamageTypes/DamageType_Fire.h"
#include "DamageTypes/DamageType_Cold.h"

void DefaultHandleDamage(UAttributeComponent* Attributes, float Damage, float Resistance, EDamageType DamageType)
{
    FHealthContainer* HealthBar = &Attributes->HealthBars[Attributes->ActiveHealthBar];
    float DamageTaken = Damage * (1.f - Resistance);
    HealthBar->CurrentHealth -= DamageTaken;
    Attributes->OnDamageTaken(DamageType, DamageTaken, HealthBar->CurrentHealth);

    if (HealthBar->CurrentHealth > 0.f) return;
    if (!HealthBar->bCanFallToZero)
    {
        HealthBar->CurrentHealth = 1.f;
        return;
    }

    if (Attributes->ActiveHealthBar == 0)
    {
        Attributes->OnDeath(DamageType);
    }
    else
    {
        Attributes->ActiveHealthBar--;
        Attributes->OnHealthBarDepleted(DamageType);
    }
}

void UDamageType_Base::ApplyDamage(UAttributeComponent* Attributes, AWeapon_Base* Weapon, float Damage, EDamageType DamageType)
{
    switch (DamageType)
    {
    case EDamageType::None:
    {
        UE_LOG(LogTemp, Warning, TEXT("DamageType_None used"));
    } break;
    case EDamageType::Physical:
    {
        UDamageType_Physical::HandleDamage(Attributes, Weapon, Damage);
    } break;
    case EDamageType::Fire:
    {
        UDamageType_Fire::HandleDamage(Attributes, Weapon, Damage);
    } break;
    case EDamageType::Ice:
    {
        UDamageType_Cold::HandleDamage(Attributes, Weapon, Damage);
    } break;
    default:
    {
        UE_LOG(LogTemp, Warning, TEXT("Encountered unknown DamageType %d"), DamageType);
    } break;
    }
}