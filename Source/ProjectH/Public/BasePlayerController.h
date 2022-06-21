// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../ProjectH.h"
#include "GameFramework/PlayerController.h"
#include "BasePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTH_API ABasePlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	ABasePlayerController();

	virtual void BeginPlay() override;

	virtual void OnNetCleanup(class UNetConnection* Connection) override;

private:
	struct FInputModeGameOnly InputMode;
};
