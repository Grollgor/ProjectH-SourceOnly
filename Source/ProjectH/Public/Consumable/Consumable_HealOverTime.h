// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item_Base.h"
#include "Consumable_HealOverTime.generated.h"

UCLASS()
class PROJECTH_API AConsumable_HealOverTime : public AItem_Base
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndGameReason);

	UFUNCTION()
	void Server_OnUseImpl();
};
