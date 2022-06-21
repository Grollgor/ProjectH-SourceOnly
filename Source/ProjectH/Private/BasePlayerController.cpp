// Fill out your copyright notice in the Description page of Project Settings.
#include "BasePlayerController.h"

#include "BaseGameMode.h"

ABasePlayerController::ABasePlayerController()
{
}

void ABasePlayerController::BeginPlay()
{
    bShowMouseCursor = false;

    InputMode = FInputModeGameOnly();

    SetInputMode(InputMode);
}

void ABasePlayerController::OnNetCleanup(class UNetConnection* Connection)
{
    if (GetLocalRole() == ROLE_Authority && PlayerState)
    {
        ABaseGameMode* GameMode = Cast<ABaseGameMode>(GetWorld()->GetAuthGameMode());
        if (GameMode)
        {
            GameMode->PreLogout(this);
        }
    }
    Super::OnNetCleanup(Connection);
}
