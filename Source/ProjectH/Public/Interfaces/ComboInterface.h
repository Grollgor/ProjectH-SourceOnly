// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ComboInterface.generated.h"

UINTERFACE(MinimalAPI)
class UComboInterface : public UInterface
{
	GENERATED_BODY()
};

// TODO(philipp): A better name for this would probably be good
class PROJECTH_API IComboInterface
{
	GENERATED_BODY()
public:
	UFUNCTION()
	virtual void SetCanAdvanceCombo(bool bCanAdvance) = 0;

	UFUNCTION()
	virtual void SetCanStartNewCombo(bool bCanStartCombo) = 0;

	UFUNCTION()
	virtual void DoDamage() = 0;

	UFUNCTION()
	virtual AItem_Base* GetEquippedItem() = 0;
};