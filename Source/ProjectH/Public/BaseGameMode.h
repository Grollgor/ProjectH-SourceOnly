// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BaseGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTH_API ABaseGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	ABaseGameMode();

	void PreLogout(APlayerController* PlayerController);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

private:
	void RegisterExistingPlayers();

	void HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful);

private:
	uint8 bAllExistingPlayersRegistered : 1;

    FDelegateHandle CreateSessionDelegateHandle;
    FDelegateHandle DestroySessionDelegateHandle;
};
