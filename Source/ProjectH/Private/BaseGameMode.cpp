// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseGameMode.h"

#include "GameFramework/GameSession.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"

#include "PlayerCharacter.h"
#include "BasePlayerController.h"
#include "BaseGameInstance.h"

ABaseGameMode::ABaseGameMode()
{
    DefaultPawnClass = APlayerCharacter::StaticClass();
    PlayerControllerClass = ABasePlayerController::StaticClass();

    bAllExistingPlayersRegistered = false;
}

void ABaseGameMode::PreLogout(APlayerController* PlayerController)
{
    check(IsValid(PlayerController));

    FUniqueNetIdRepl UniqueNetIdRepl;
    if (PlayerController->IsLocalPlayerController())
    {
        ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
        if (IsValid(LocalPlayer))
        {
            UniqueNetIdRepl = LocalPlayer->GetPreferredUniqueNetId();
        }
        else
        {
            UNetConnection* RemoteNetConnection = Cast<UNetConnection>(PlayerController->Player);
            check(IsValid(RemoteNetConnection));
            UniqueNetIdRepl = RemoteNetConnection->PlayerId;
        }
    }
    else
    {
        UNetConnection* RemoteNetConnection = Cast<UNetConnection>(PlayerController->Player);
        check(IsValid(RemoteNetConnection));
        UniqueNetIdRepl = RemoteNetConnection->PlayerId;
    }

    TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIdRepl.GetUniqueNetId();
    if(!UniqueNetId) return;

    IOnlineSubsystem* Subsystem = Online::GetSubsystem(PlayerController->GetWorld());
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

    UBaseGameInstance* GameInstance = Cast<UBaseGameInstance>(GetGameInstance());
    check(IsValid(GameInstance));
    if (!Session->UnregisterPlayer(FName(GameInstance->LocalSessionName), *UniqueNetId))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to unregister a player from the session"));
    }
}

void ABaseGameMode::BeginPlay()
{
    UBaseGameInstance* Instance = Cast<UBaseGameInstance>(GetGameInstance());
    if (Instance && Instance->bIsHostingSession)
    {
        IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
        IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

        CreateSessionDelegateHandle = Session->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionComplete::FDelegate::CreateUObject(this, &ABaseGameMode::HandleCreateSessionComplete));

        TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShared<FOnlineSessionSettings>();
        SessionSettings->NumPublicConnections = 4;
        SessionSettings->bShouldAdvertise = true;
        SessionSettings->bUsesPresence = true;

        int64 Value = 2509;
        SessionSettings->Settings.Add(FName(TEXT("ALWAYSPRESENT")), FOnlineSessionSetting(Value, EOnlineDataAdvertisementType::ViaOnlineService));

        if (!Session->CreateSession(0, FName(Instance->LocalSessionName), *SessionSettings))
        {
            UE_LOG(LogTemp, Error, TEXT("Call to create session failed"));
        }
    }
}

void ABaseGameMode::EndPlay(EEndPlayReason::Type EndPlayReason)
{
    UBaseGameInstance* Instance = Cast<UBaseGameInstance>(GetGameInstance());
    if (Instance && Instance->bIsHostingSession)
    {
        IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
        IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

        DestroySessionDelegateHandle = Session->AddOnDestroySessionCompleteDelegate_Handle(FOnDestroySessionComplete::FDelegate::CreateUObject(this, &ABaseGameMode::HandleDestroySessionComplete));

        if (!Session->DestroySession(FName(Instance->LocalSessionName)))
        {
            UE_LOG(LogTemp, Error, TEXT("Call to destroy session failed"));
        }
    }
}

void ABaseGameMode::PostLogin(APlayerController* NewPlayer)
{
    if (!bAllExistingPlayersRegistered)
    {
        Super::PostLogin(NewPlayer);
        return;
    }

    FUniqueNetIdRepl UniqueNetIdRepl;
    if (NewPlayer->IsLocalPlayerController())
    {
        ULocalPlayer* LocalPlayer = NewPlayer->GetLocalPlayer();
        if (IsValid(LocalPlayer))
        {
            UniqueNetIdRepl = LocalPlayer->GetPreferredUniqueNetId();
        }
        else
        {
            UNetConnection* RemoteNetConnection = Cast<UNetConnection>(NewPlayer->Player);
            check(IsValid(RemoteNetConnection));
            UniqueNetIdRepl = RemoteNetConnection->PlayerId;
        }
    }
    else
    {
        UNetConnection* RemoteNetConnection = Cast<UNetConnection>(NewPlayer->Player);
        check(IsValid(RemoteNetConnection));
        UniqueNetIdRepl = RemoteNetConnection->PlayerId;
    }

    TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIdRepl.GetUniqueNetId();
    check(UniqueNetId);

    IOnlineSubsystem* Subsystem = Online::GetSubsystem(NewPlayer->GetWorld());
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

    UBaseGameInstance* GameInstance = Cast<UBaseGameInstance>(GetGameInstance());
    check(IsValid(GameInstance))
    if (!Session->RegisterPlayer(FName(GameInstance->LocalSessionName), *UniqueNetId, false))
    {
        GameSession->KickPlayer(NewPlayer, FText::FromString(TEXT("Failed to register you with the game session")));
    }
    Super::PostLogin(NewPlayer);
}

void ABaseGameMode::RegisterExistingPlayers()
{
    for (auto It = GetWorld()->GetPlayerControllerIterator(); It; It++)
    {
        APlayerController* PlayerController = It->Get();

        FUniqueNetIdRepl UniqueNetIdRepl;
        if (PlayerController->IsLocalPlayerController())
        {
            ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
            if (IsValid(LocalPlayer))
            {
                UniqueNetIdRepl = LocalPlayer->GetPreferredUniqueNetId();
            }
            else
            {
                UNetConnection* RemoteNetConnection = Cast<UNetConnection>(PlayerController->Player);
                check(IsValid(RemoteNetConnection));
                UniqueNetIdRepl = RemoteNetConnection->PlayerId;
            }
        }
        else
        {
            UNetConnection* RemoteNetConnection = Cast<UNetConnection>(PlayerController->Player);
            check(IsValid(RemoteNetConnection));
            UniqueNetIdRepl = RemoteNetConnection->PlayerId;
        }

        TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIdRepl.GetUniqueNetId();
        check(UniqueNetId);

        IOnlineSubsystem* Subsystem = Online::GetSubsystem(PlayerController->GetWorld());
        IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

        UBaseGameInstance* GameInstance = Cast<UBaseGameInstance>(GetGameInstance());
        check(IsValid(GameInstance))
        if (!Session->RegisterPlayer(FName(GameInstance->LocalSessionName), *UniqueNetId, false))
        {
            GameSession->KickPlayer(PlayerController, FText::FromString(TEXT("Failed to register you with the game session")));
        }
    }
    bAllExistingPlayersRegistered = true;
}

void ABaseGameMode::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        RegisterExistingPlayers();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create online session"));
    }

    IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
    Session->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionDelegateHandle);
    CreateSessionDelegateHandle.Reset();
}

void ABaseGameMode::HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (!bWasSuccessful)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to destroy session"));
    }

    IOnlineSubsystem* Subsystem = Online::GetSubsystem(this->GetWorld());
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
    Session->ClearOnDestroySessionCompleteDelegate_Handle(this->DestroySessionDelegateHandle);
    this->DestroySessionDelegateHandle.Reset();
}
