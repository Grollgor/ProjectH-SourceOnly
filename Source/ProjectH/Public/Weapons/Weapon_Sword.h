// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Weapons/Weapon_Base.h"
#include "Weapon_Sword.generated.h"

UCLASS()
class PROJECTH_API AWeapon_Sword : public AWeapon_Base
{
	GENERATED_BODY()
public:
    AWeapon_Sword();

    UFUNCTION()
    void Server_OnUseImpl();
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_Attack(int32 Combo);

    virtual void SetCanAdvanceCombo(bool bCanAdvance) override;
    virtual void SetCanStartNewCombo(bool bCanStartCombo) override;
    virtual void DoDamage() override;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
    uint8 bCanAdvanceCombo : 1;

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
    uint8 bCanStartNewCombo : 1;

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
    int32 ComboState;
};
