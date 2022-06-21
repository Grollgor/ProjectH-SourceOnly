// Fill out your copyright notice in the Description page of Project Settings.
#include "Consumable/Consumable_HealOverTime.h"

#include "Components/AttributeComponent.h"
#include "Components/ContainerComponent.h"

void AConsumable_HealOverTime::BeginPlay()
{
    if (HasAuthority())
    {
        Server_OnUseDelegate.AddDynamic(this, &AConsumable_HealOverTime::Server_OnUseImpl);
    }
}

void AConsumable_HealOverTime::EndPlay(EEndPlayReason::Type EndGameReason)
{
    if (HasAuthority())
    {
        Server_OnUseDelegate.RemoveDynamic(this, &AConsumable_HealOverTime::Server_OnUseImpl);
    }
}

void AConsumable_HealOverTime::Server_OnUseImpl()
{
    UAttributeComponent* Attributes = Cast<UAttributeComponent>(OwningActor->GetComponentByClass(UAttributeComponent::StaticClass()));
    check(Attributes);
    Attributes->ApplyBuff(FName("buffHealOverTime"));

    UContainerComponent* Container = Cast<UContainerComponent>(OwningActor->GetComponentByClass(UContainerComponent::StaticClass()));
    check(Container);
    Container->AddItems(FName("itemEmptyBottle"));
}
