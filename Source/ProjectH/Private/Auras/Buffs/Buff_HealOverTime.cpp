// Fill out your copyright notice in the Description page of Project Settings.
#include "Auras/Buffs/Buff_HealOverTime.h"

#include "Components/AttributeComponent.h"

void UBuff_HealOverTime::Server_OnAuraTicked(float DeltaTime)
{
    Super::Server_OnAuraTicked(DeltaTime);

    AActor* Actor = GetOwningActor();
    check(Actor);
    UAttributeComponent* Attributes = Cast<UAttributeComponent>(Actor->GetComponentByClass(UAttributeComponent::StaticClass()));
    check(Attributes);
    Attributes->GiveHealth(5.f * DeltaTime);
}
