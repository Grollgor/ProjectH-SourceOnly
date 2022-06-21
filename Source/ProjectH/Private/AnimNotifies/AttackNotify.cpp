// Fill out your copyright notice in the Description page of Project Settings.
#include "AnimNotifies/AttackNotify.h"

#include "Interfaces/ComboInterface.h"

#include "Items/Item_Base.h"

void UAttackNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    IComboInterface* ComboInterface = Cast<IComboInterface>(MeshComp->GetOwner());
    if (ComboInterface)
    {
        AItem_Base* Item = ComboInterface->GetEquippedItem();
        if (!Item) return;
        Item->DoDamage();
    }
}
