// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "../ProjectH.h"

#include "Engine/DataTable.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"

class ACharacter;

class UAura_Base;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthReceivedDelegate, float, NewHealth, int32, ActiveHealthBar);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDamageTakenDelegate, EDamageType, DamageType, float, Damage, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthBarDepletedDelegate, EDamageType, DamageType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeathDelegate, EDamageType, DamageType);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStartBurningDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEndBurningDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStartFreezingDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEndFreezingDelegate);

USTRUCT(BlueprintType)
struct PROJECTH_API FHealthContainer
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
    float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
    uint8 bCanFallToZero : 1;
};

USTRUCT(BlueprintType)
struct PROJECTH_API FStatusEffect
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status")
    float CurrentBuildup;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status")
    float BuildupCap;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status")
    float ActivationThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status")
    float DeactivationThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status")
    float DecayPerSecond;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status")
    uint8 bCanDecay : 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status")
    uint8 bIsActive : 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status")
    uint8 bCanActivate : 1;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable )
class PROJECTH_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttributeComponent();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

    void GiveHealth(float Health);

    void ApplyBuff(FName BuffName);
    void ApplyDebuff(FName DebuffName);

    // NOTE(philipp): Base events
    //                Each of these functions have a Server_, Client_ and Multicast_ Delegate that you can bind to
    void OnHealthReceived(float NewHealth, int32 NewActiveHealthBar);
    void OnDamageTaken(EDamageType DamageType, float Damage, float NewHealth);
    void OnHealthBarDepleted(EDamageType DamageType);
    void OnDeath(EDamageType DamageType);
    void OnStartBurning();
    void OnEndBurning();
    void OnStartFreezing();
    void OnEndFreezing();

protected:
    UFUNCTION(Server, Unreliable)
    void Server_OnAuraApplied(UAura_Base* Aura);

public:
    // ### OnHealthReveived begin ###
    UFUNCTION(Server, Unreliable)
    void Server_OnHealthReceived(float NewHealth, int32 NewActiveHealthBar);
    UPROPERTY(BlueprintAssignable)
    FOnHealthReceivedDelegate Server_OnHealthReceivedDelegate;

    UFUNCTION(Client, Unreliable)
    void Client_OnHealthReceived(float NewHealth, int32 NewActiveHealthBar);
    UPROPERTY(BlueprintAssignable)
    FOnHealthReceivedDelegate Client_OnHealthReceivedDelegate;

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_OnHealthReceived(float NewHealth, int32 NewActiveHealthBar);
    UPROPERTY(BlueprintAssignable)
    FOnHealthReceivedDelegate Multicast_OnHealthReceivedDelegate;
    // ### OnHealthReveived End ###

    // ### OnDamageTaken begin ###
    UFUNCTION(Server, Unreliable)
    void Server_OnDamageTaken(EDamageType DamageType, float Damage, float NewHealth);
    UPROPERTY(BlueprintAssignable)
    FOnDamageTakenDelegate Server_OnDamageTakenDelegate;

    UFUNCTION(Client, Unreliable)
    void Client_OnDamageTaken(EDamageType DamageType, float Damage, float NewHealth);
    UPROPERTY(BlueprintAssignable)
    FOnDamageTakenDelegate Client_OnDamageTakenDelegate;

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_OnDamageTaken(EDamageType DamageType, float Damage, float NewHealth);
    UPROPERTY(BlueprintAssignable)
    FOnDamageTakenDelegate Multicast_OnDamageTakenDelegate;
    // ### OnDamageTaken End ###

    // ### OnHealthBarDepleted begin ###
    UFUNCTION(Server, Unreliable)
    void Server_OnHealthBarDepleted(EDamageType DamageType);
    UPROPERTY(BlueprintAssignable)
    FOnHealthBarDepletedDelegate Server_OnHealthBarDepletedDelegate;

    UFUNCTION(Client, Unreliable)
    void Client_OnHealthBarDepleted(EDamageType DamageType);
    UPROPERTY(BlueprintAssignable)
    FOnHealthBarDepletedDelegate Client_OnHealthBarDepletedDelegate;

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_OnHealthBarDepleted(EDamageType DamageType);
    UPROPERTY(BlueprintAssignable)
    FOnHealthBarDepletedDelegate Multicast_OnHealthBarDepletedDelegate;
    // ### OnHealthBarDepleted End ###

    // ### OnDeath begin ###
    UFUNCTION(Server, Unreliable)
    void Server_OnDeath(EDamageType DamageType);
    UPROPERTY(BlueprintAssignable)
    FOnDeathDelegate Server_OnDeathDelegate;

    UFUNCTION(Client, Unreliable)
    void Client_OnDeath(EDamageType DamageType);
    UPROPERTY(BlueprintAssignable)
    FOnDeathDelegate Client_OnDeathDelegate;

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_OnDeath(EDamageType DamageType);
    UPROPERTY(BlueprintAssignable)
    FOnDeathDelegate Multicast_OnDeathDelegate;
    // ### OnDeath End ###

    // ### OnStartBurning begin ###
    UFUNCTION(Server, Unreliable)
    void Server_OnStartBurning();
    UPROPERTY(BlueprintAssignable)
    FOnStartBurningDelegate Server_OnStartBurningDelegate;

    UFUNCTION(Client, Unreliable)
    void Client_OnStartBurning();
    UPROPERTY(BlueprintAssignable)
    FOnStartBurningDelegate Client_OnStartBurningDelegate;

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_OnStartBurning();
    UPROPERTY(BlueprintAssignable)
    FOnStartBurningDelegate Multicast_OnStartBurningDelegate;
    // ### OnStartBurning End ###

    // ### OnEndBurning begin ###
    UFUNCTION(Server, Unreliable)
    void Server_OnEndBurning();
    UPROPERTY(BlueprintAssignable)
    FOnEndBurningDelegate Server_OnEndBurningDelegate;

    UFUNCTION(Client, Unreliable)
    void Client_OnEndBurning();
    UPROPERTY(BlueprintAssignable)
    FOnEndBurningDelegate Client_OnEndBurningDelegate;

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_OnEndBurning();
    UPROPERTY(BlueprintAssignable)
    FOnEndBurningDelegate Multicast_OnEndBurningDelegate;
    // ### OnEndBurning End ###

    // ### OnStartFreezing begin ###
    UFUNCTION(Server, Unreliable)
    void Server_OnStartFreezing();
    UPROPERTY(BlueprintAssignable)
    FOnStartFreezingDelegate Server_OnStartFreezingDelegate;

    UFUNCTION(Client, Unreliable)
    void Client_OnStartFreezing();
    UPROPERTY(BlueprintAssignable)
    FOnStartFreezingDelegate Client_OnStartFreezingDelegate;

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_OnStartFreezing();
    UPROPERTY(BlueprintAssignable)
    FOnStartFreezingDelegate Multicast_OnStartFreezingDelegate;
    // ### OnStartFreezing End ###

    // ### OnEndFreezing begin ###
    UFUNCTION(Server, Unreliable)
    void Server_OnEndFreezing();
    UPROPERTY(BlueprintAssignable)
    FOnEndFreezingDelegate Server_OnEndFreezingDelegate;

    UFUNCTION(Client, Unreliable)
    void Client_OnEndFreezing();
    UPROPERTY(BlueprintAssignable)
    FOnEndFreezingDelegate Client_OnEndFreezingDelegate;

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_OnEndFreezing();
    UPROPERTY(BlueprintAssignable)
    FOnEndFreezingDelegate Multicast_OnEndFreezingDelegate;
    // ### OnEndFreezing End ###
protected:
	virtual void BeginPlay() override;
    virtual void EndPlay(EEndPlayReason::Type EndReason) override;

    UFUNCTION()
    void TakeDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* IntigatedBy, AActor* DamageCauser);

public:
    // NOTE(philipp): It isn't valid to have no health containers
    UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category="Health")
    TArray<FHealthContainer> HealthBars;
    
    // NOTE(philipp): This index goes down when going towards death
    UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category="Health")
    int32 ActiveHealthBar;

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Health")
    float PhysicalResistance;

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Health")
    float FireResistance;

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Health")
    float ColdResistance;

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Health")
    FStatusEffect FireStatus;

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Health")
    FStatusEffect ColdStatus;

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Aura")
    TArray<UAura_Base*> BuffList;

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Aura")
    TArray<UAura_Base*> DebuffList;

protected:
#if WITH_EDITORONLY_DATA
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 bDisplayDebugStats : 1;
#endif

    UPROPERTY(BlueprintReadOnly, Category="Owner")
    ACharacter* OwnerAsCharacter;

    UPROPERTY()
    UDataTable* AuraDataTable;
};
