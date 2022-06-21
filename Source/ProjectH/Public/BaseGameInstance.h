// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"

#include "Engine/GameInstance.h"

#include "BaseGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTH_API UBaseGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UBaseGameInstance();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="OnlineSubsytem")
	FString LocalSessionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OnlineSubsytem")
    uint8 bIsHostingSession : 1;
};
