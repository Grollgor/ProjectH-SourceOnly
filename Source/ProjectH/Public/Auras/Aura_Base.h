// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "Engine/DataTable.h"

#include "Components/ActorComponent.h"

#include "Aura_Base.generated.h"

class UAura_Base;

USTRUCT(BlueprintType)
struct PROJECTH_API FAuraData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UTexture2D* Icon;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<UAura_Base> Class;
};

UCLASS(Blueprintable)
class PROJECTH_API UAura_Base : public UObject
{
	GENERATED_BODY()
public:
    UAura_Base();

    UFUNCTION(Client, Unreliable)
    void Client_OnAuraAppliedFnc();

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_OnAuraAppliedFnc();

    // ### Begin OnAuraApplied ###
    virtual void Server_OnAuraApplied();
    virtual void Client_OnAuraApplied();
    virtual void Multicast_OnAuraApplied();
    // ### End OnAuraApplied ###

    // ### Begin OnAuraTicked ###
    virtual void Server_OnAuraTicked(float DeltaTime);
    virtual void Client_OnAuraTicked(float DeltaTime);
    virtual void Multicast_OnAuraTicked(float DeltaTime);
    // ### End OnAuraTicked ###

    // ### Begin OnAuraExpired ###
    virtual void Server_OnAuraExpired();
    virtual void Client_OnAuraExpired();
    virtual void Multicast_OnAuraExpired();
    // ### End OnAuraExpired ###

    // NOTE(philipp): Those functions are required to add replication support
    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
    virtual bool IsSupportedForNetworking() const { return true; }
    virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;
    virtual bool CallRemoteFunction(UFunction* Function, void* Parms, struct FOutParmRec* OutParms, FFrame* Stack) override;

    virtual UWorld* GetWorld() const override;

    // NOTE(philipp): This class should only ever be part of the AttributeComponent
    UFUNCTION(BlueprintPure, Category="AuraBase")
    AActor* GetOwningActor() const { return GetTypedOuter<UActorComponent>()->GetOwner(); }

    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="AuraBase")
    void Destroy();

public:
    UPROPERTY(Replicated)
    FName AuraName;

    UPROPERTY(Replicated)
    float TimeRemaining;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UDataTable* DataTable;
};
