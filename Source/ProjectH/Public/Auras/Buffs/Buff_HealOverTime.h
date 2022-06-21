// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Auras/Aura_Base.h"
#include "Buff_HealOverTime.generated.h"

UCLASS()
class PROJECTH_API UBuff_HealOverTime : public UAura_Base
{
	GENERATED_BODY()
public:
	virtual void Server_OnAuraTicked(float DeltaTime) override;
};
