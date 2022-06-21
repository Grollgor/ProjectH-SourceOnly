// Fill out your copyright notice in the Description page of Project Settings.
#include "Consumable/Consumable_InstantHeal.h"

#include "Components/AttributeComponent.h"
#include "Components/ContainerComponent.h"

void AConsumable_InstantHeal::BeginPlay()
{
    if (HasAuthority())
    {
        Server_OnUseDelegate.AddDynamic(this, &AConsumable_InstantHeal::Server_OnUseImpl);
    }
}

void AConsumable_InstantHeal::EndPlay(EEndPlayReason::Type EndPlayReason)
{
    if (HasAuthority())
    {
        Server_OnUseDelegate.RemoveDynamic(this, &AConsumable_InstantHeal::Server_OnUseImpl);
    }
}

void AConsumable_InstantHeal::Server_OnUseImpl()
{
    UAttributeComponent* Attributes = Cast<UAttributeComponent>(OwningActor->GetComponentByClass(UAttributeComponent::StaticClass()));
    check(Attributes);
    Attributes->GiveHealth(50.f);

    UContainerComponent* Container = Cast<UContainerComponent>(OwningActor->GetComponentByClass(UContainerComponent::StaticClass()));
    check(Container);
    Container->AddItems(FName("itemEmptyBottle"));
}
