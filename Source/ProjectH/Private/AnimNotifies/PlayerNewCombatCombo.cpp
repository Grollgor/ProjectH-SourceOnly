// Fill out your copyright notice in the Description page of Project Settings.
#include "AnimNotifies/PlayerNewCombatCombo.h"

#include "Interfaces/ComboInterface.h"

#include "Items/Item_Base.h"

void UPlayerNewCombatCombo::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
    IComboInterface* ComboInterface = Cast<IComboInterface>(MeshComp->GetOwner());
    if (ComboInterface)
    {
        AItem_Base* Item = ComboInterface->GetEquippedItem();
        if (!Item) return;
        Item->SetCanStartNewCombo(false);
    }
}

void UPlayerNewCombatCombo::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    IComboInterface* ComboInterface = Cast<IComboInterface>(MeshComp->GetOwner());
    if (ComboInterface)
    {
        AItem_Base* Item = ComboInterface->GetEquippedItem();
        if (!Item) return;
        Item->SetCanStartNewCombo(true);
        Item->SetCanAdvanceCombo(true);
    }
}
