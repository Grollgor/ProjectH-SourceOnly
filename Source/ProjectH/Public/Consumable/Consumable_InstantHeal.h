// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Items/Item_Base.h"
#include "Consumable_InstantHeal.generated.h"

UCLASS()
class PROJECTH_API AConsumable_InstantHeal : public AItem_Base
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason);

	UFUNCTION()
	void Server_OnUseImpl();
};
