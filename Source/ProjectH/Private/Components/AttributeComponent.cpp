// Fill out your copyright notice in the Description page of Project Settings.
#include "Components/AttributeComponent.h"

#include "Engine/ActorChannel.h"

#include "Kismet/GameplayStatics.h"

#include "GameFramework/Character.h"
#include "GameFramework/PawnMovementComponent.h"

#include "DamageTypes/DamageType_Base.h"
#include "DamageTypes/DamageType_Fire.h"

#include "Auras/Aura_Base.h"

#include "Weapons/Weapon_Base.h"

UAttributeComponent::UAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);

	FHealthContainer DefaultContainer;
	DefaultContainer.MaxHealth = 100.f;
	DefaultContainer.CurrentHealth = DefaultContainer.MaxHealth;
	DefaultContainer.bCanFallToZero = true;
	HealthBars.Add(DefaultContainer);

	ActiveHealthBar = 0;

	FireResistance = 0.f;
	ColdResistance = 0.f;

    FireStatus.bCanDecay = true;
    FireStatus.bCanActivate = true;
	FireStatus.BuildupCap = 125.f;
	FireStatus.DecayPerSecond = 5.f;
	FireStatus.ActivationThreshold = 100.f;
	FireStatus.DeactivationThreshold = 70.f;

    ColdStatus.bCanDecay = true;
	ColdStatus.bCanActivate = true;
    ColdStatus.BuildupCap = 125.f;
    ColdStatus.DecayPerSecond = 5.f;
    ColdStatus.ActivationThreshold = 100.f;
    ColdStatus.DeactivationThreshold = 70.f;

	bDisplayDebugStats = false;

	AuraDataTable = nullptr;
	static ConstructorHelpers::FObjectFinder<UDataTable> AuraDataObj(TEXT("DataTable'/Game/ProjectH/Core/DataTables/DT_Auras.DT_Auras'"));
	if (AuraDataObj.Succeeded())
	{
		AuraDataTable = AuraDataObj.Object;
	}
}

void UAttributeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UAttributeComponent, HealthBars);
    DOREPLIFETIME(UAttributeComponent, ActiveHealthBar);

    DOREPLIFETIME(UAttributeComponent, PhysicalResistance);

    DOREPLIFETIME(UAttributeComponent, FireResistance);
    DOREPLIFETIME(UAttributeComponent, FireStatus);

    DOREPLIFETIME(UAttributeComponent, ColdResistance);
    DOREPLIFETIME(UAttributeComponent, ColdStatus);

    DOREPLIFETIME(UAttributeComponent, BuffList);
    DOREPLIFETIME(UAttributeComponent, DebuffList);
}

void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	OwnerAsCharacter = Cast<ACharacter>(Owner);
	check(OwnerAsCharacter);

	if (OwnerAsCharacter->HasAuthority())
	{
		OwnerAsCharacter->OnTakeAnyDamage.AddDynamic(this, &UAttributeComponent::TakeDamage);
	}
}

void UAttributeComponent::EndPlay(EEndPlayReason::Type EndReason)
{
	if (OwnerAsCharacter->HasAuthority())
	{
		OwnerAsCharacter->OnTakeAnyDamage.RemoveDynamic(this, &UAttributeComponent::TakeDamage);
	}

	Super::EndPlay(EndReason);
}

void UAttributeComponent::TakeDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* IntigatedBy, AActor* DamageCauser)
{
	check(DamagedActor == GetOwner());

	const UDamageType_Base* DamageTypeClass = Cast<UDamageType_Base>(DamageType);
	AWeapon_Base* Weapon = Cast<AWeapon_Base>(DamageCauser);
	if (DamageTypeClass)
	{
		DamageTypeClass->ApplyDamage(this, Weapon, Damage, DamageTypeClass->DamageType);
	}
}

inline static void DecayStatus(FStatusEffect* Status, float DeltaTime)
{
    if (Status->bCanDecay && Status->CurrentBuildup > 0.f)
    {
        Status->CurrentBuildup -= Status->DecayPerSecond * DeltaTime;
        if (Status->CurrentBuildup < 0.f)
        {
            Status->CurrentBuildup = 0.f;
        }
    }
}

// NOTE(philipp): If the state changes from effect active to not active this functions returns true, otherwise false
static bool DealDotDamage(UAttributeComponent* Attributes, FStatusEffect* Status, float DeltaTime, AActor* ActorToDamage)
{
    if ((Status->bCanActivate && Status->CurrentBuildup >= Status->ActivationThreshold) || Status->bIsActive)
    {
        Status->bIsActive = true;
        UGameplayStatics::ApplyDamage(ActorToDamage, 5.f * DeltaTime, nullptr, nullptr, UDamageType_Fire::StaticClass());
        if (Status->CurrentBuildup < Status->DeactivationThreshold)
        {
            Status->bIsActive = false;
			return true;
        }
    }
	return false;
}

static void TickAuraList(TArray<UAura_Base*>& AuraList, float DeltaTime, AActor* Owner)
{
    for (int32 Index = AuraList.Num() - 1;  Index >= 0 ; Index--)
    {
        if (Owner->HasAuthority())
        {
            AuraList[Index]->Server_OnAuraTicked(DeltaTime);
        }
        if (Owner->GetInstigatorController() && Owner->GetInstigatorController()->IsLocalPlayerController())
        {
			AuraList[Index]->Client_OnAuraTicked(DeltaTime);
        }
		AuraList[Index]->Multicast_OnAuraTicked(DeltaTime);

		if (AuraList[Index]->TimeRemaining <= 0.f)
		{
			AuraList[Index]->Multicast_OnAuraExpired();
			if (Owner->GetInstigatorController()->IsLocalPlayerController())
			{
				AuraList[Index]->Client_OnAuraExpired();
			}
            if (Owner->HasAuthority())
            {
                AuraList[Index]->Server_OnAuraExpired();
				AuraList[Index]->Destroy();
                AuraList.RemoveAt(Index);
            }
		}
    }
}

void UAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TickAuraList(BuffList, DeltaTime, GetOwner());
    TickAuraList(DebuffList, DeltaTime, GetOwner());

	DecayStatus(&FireStatus, DeltaTime);
	DecayStatus(&ColdStatus, DeltaTime);

	if (DealDotDamage(this, &FireStatus, DeltaTime, GetOwner())) Server_OnEndBurning();
	if (DealDotDamage(this, &ColdStatus, DeltaTime, GetOwner())) Server_OnEndFreezing();

#if WITH_EDITOR
	if (OwnerAsCharacter->HasAuthority() && bDisplayDebugStats)
	{
		uint64 LogIndex = 10000;
		GEngine->AddOnScreenDebugMessage(LogIndex++, 10.f, FColor::Green, TEXT("ATTRIBUTES"));
		GEngine->AddOnScreenDebugMessage(LogIndex++, 10.f, FColor::Green, TEXT("Health"));
		for (uint64 Index = 0; Index < HealthBars.Num(); Index++)
		{
			FString String = FString::Printf(TEXT("HealthBar%d: %.1f/%.1f"), Index, HealthBars[Index].CurrentHealth, HealthBars[Index].MaxHealth);
			GEngine->AddOnScreenDebugMessage(LogIndex++, 10.f, FColor::Green, String);
		}

        GEngine->AddOnScreenDebugMessage(LogIndex++, 10.f, FColor::Green, TEXT("FireStatus"));
        FString FireBuildupString = FString::Printf(TEXT("Buildup: %.1f/%.1f"), FireStatus.CurrentBuildup, FireStatus.BuildupCap);
        GEngine->AddOnScreenDebugMessage(LogIndex++, 10.f, FColor::Green, FireBuildupString);

        GEngine->AddOnScreenDebugMessage(LogIndex++, 10.f, FColor::Green, TEXT("ColdStatus"));
        FString ColdBuildupString = FString::Printf(TEXT("Buildup: %.1f/%.1f"), ColdStatus.CurrentBuildup, ColdStatus.BuildupCap);
        GEngine->AddOnScreenDebugMessage(LogIndex++, 10.f, FColor::Green, ColdBuildupString);
	}
#endif
}

bool UAttributeComponent::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

    bWroteSomething |= Channel->ReplicateSubobjectList(BuffList, *Bunch, *RepFlags);
    bWroteSomething |= Channel->ReplicateSubobjectList(DebuffList, *Bunch, *RepFlags);

	return bWroteSomething;
}

void UAttributeComponent::GiveHealth(float Health)
{
	FHealthContainer* Bar = &HealthBars[ActiveHealthBar];
	Bar->CurrentHealth += Health;
	if (Bar->CurrentHealth > Bar->MaxHealth)
	{
		Bar->CurrentHealth = Bar->MaxHealth;
		if (ActiveHealthBar < HealthBars.Num() - 1)
		{
			ActiveHealthBar++;
		}
	}
	OnHealthReceived(HealthBars[ActiveHealthBar].CurrentHealth, ActiveHealthBar);
}

void UAttributeComponent::ApplyBuff(FName BuffName)
{
	check(AuraDataTable);
	FAuraData* Data = AuraDataTable->FindRow<FAuraData>(BuffName, TEXT(""));
	check(Data);

	UAura_Base* Aura = NewObject<UAura_Base>(this, Data->Class);
	Aura->AuraName = BuffName;
	int32 Index = BuffList.Add(Aura);
	Server_OnAuraApplied(BuffList[Index]);
}

void UAttributeComponent::ApplyDebuff(FName DebuffName)
{

    check(AuraDataTable);
    FAuraData* Data = AuraDataTable->FindRow<FAuraData>(DebuffName, TEXT(""));
    check(Data);

    UAura_Base* Aura = NewObject<UAura_Base>(this, Data->Class);
	Aura->AuraName = DebuffName;
    int32 Index = DebuffList.Add(Aura);
    Server_OnAuraApplied(DebuffList[Index]);
}

void UAttributeComponent::Server_OnAuraApplied_Implementation(UAura_Base* Aura)
{
	Aura->Server_OnAuraApplied();
}

void UAttributeComponent::OnHealthReceived(float NewHealth, int32 NewActiveHealthBar)
{
	Server_OnHealthReceived(NewHealth, NewActiveHealthBar);
}

void UAttributeComponent::Server_OnHealthReceived_Implementation(float NewHealth, int32 NewActiveHealthBar)
{
	Server_OnHealthReceivedDelegate.Broadcast(NewHealth, NewActiveHealthBar);

	Client_OnHealthReceived(NewHealth, NewActiveHealthBar);
	Multicast_OnHealthReceived(NewHealth, NewActiveHealthBar);
}

void UAttributeComponent::Client_OnHealthReceived_Implementation(float NewHealth, int32 NewActiveHealthBar)
{
	Client_OnHealthReceivedDelegate.Broadcast(NewHealth, NewActiveHealthBar);
}

void UAttributeComponent::Multicast_OnHealthReceived_Implementation(float NewHealth, int32 NewActiveHealthBar)
{
	Multicast_OnHealthReceivedDelegate.Broadcast(NewHealth, NewActiveHealthBar);
}

void UAttributeComponent::OnDamageTaken(EDamageType DamageType, float Damage, float NewHealth)
{
	Server_OnDamageTaken(DamageType, Damage, NewHealth);
}

void UAttributeComponent::Server_OnDamageTaken_Implementation(EDamageType DamageType, float Damage, float NewHealth)
{
	Server_OnDamageTakenDelegate.Broadcast(DamageType, Damage, NewHealth);

	Client_OnDamageTaken(DamageType, Damage, NewHealth);
	Multicast_OnDamageTaken(DamageType, Damage, NewHealth);
}

void UAttributeComponent::Client_OnDamageTaken_Implementation(EDamageType DamageType, float Damage, float NewHealth)
{
	Client_OnDamageTakenDelegate.Broadcast(DamageType, Damage, NewHealth);
}

void UAttributeComponent::Multicast_OnDamageTaken_Implementation(EDamageType DamageType, float Damage, float NewHealth)
{
	Multicast_OnDamageTakenDelegate.Broadcast(DamageType, Damage, NewHealth);
}

void UAttributeComponent::OnHealthBarDepleted(EDamageType DamageType)
{
	Server_OnHealthBarDepleted(DamageType);
}

void UAttributeComponent::Server_OnHealthBarDepleted_Implementation(EDamageType DamageType)
{
	Server_OnHealthBarDepletedDelegate.Broadcast(DamageType);

	Client_OnHealthBarDepleted(DamageType);
	Multicast_OnHealthBarDepleted(DamageType);
}

void UAttributeComponent::Client_OnHealthBarDepleted_Implementation(EDamageType DamageType)
{
	Client_OnHealthBarDepletedDelegate.Broadcast(DamageType);
}

void UAttributeComponent::Multicast_OnHealthBarDepleted_Implementation(EDamageType DamageType)
{
	Multicast_OnHealthBarDepletedDelegate.Broadcast(DamageType);
}

void UAttributeComponent::OnDeath(EDamageType DamageType)
{
	Server_OnDeath(DamageType);
}

void UAttributeComponent::Server_OnDeath_Implementation(EDamageType DamageType)
{
	Server_OnDeathDelegate.Broadcast(DamageType);

	Client_OnDeath(DamageType);
	Multicast_OnDeath(DamageType);
}

void UAttributeComponent::Client_OnDeath_Implementation(EDamageType DamageType)
{
	Client_OnDeathDelegate.Broadcast(DamageType);
}

void UAttributeComponent::Multicast_OnDeath_Implementation(EDamageType DamageType)
{
	Multicast_OnDeathDelegate.Broadcast(DamageType);
}

void UAttributeComponent::OnStartBurning()
{
	Server_OnStartBurning();
}

void UAttributeComponent::Server_OnStartBurning_Implementation()
{
	Server_OnStartBurningDelegate.Broadcast();

	Client_OnStartBurning();
	Multicast_OnStartBurning();
}

void UAttributeComponent::Client_OnStartBurning_Implementation()
{
	Client_OnStartBurningDelegate.Broadcast();
}

void UAttributeComponent::Multicast_OnStartBurning_Implementation()
{
	Multicast_OnStartBurningDelegate.Broadcast();
}

void UAttributeComponent::OnEndBurning()
{
	Server_OnEndBurning();
}

void UAttributeComponent::Server_OnEndBurning_Implementation()
{
	Server_OnEndBurningDelegate.Broadcast();

	Client_OnEndBurning();
	Multicast_OnEndBurning();
}

void UAttributeComponent::Client_OnEndBurning_Implementation()
{
	Client_OnEndBurningDelegate.Broadcast();
}

void UAttributeComponent::Multicast_OnEndBurning_Implementation()
{
	Multicast_OnEndBurningDelegate.Broadcast();
}

void UAttributeComponent::OnStartFreezing()
{
	Server_OnStartFreezing();
}

void UAttributeComponent::Server_OnStartFreezing_Implementation()
{
	Server_OnStartFreezingDelegate.Broadcast();

	Client_OnStartFreezing();
	Multicast_OnStartFreezing();
}

void UAttributeComponent::Client_OnStartFreezing_Implementation()
{
	Client_OnStartFreezingDelegate.Broadcast();
}

void UAttributeComponent::Multicast_OnStartFreezing_Implementation()
{
	Multicast_OnStartFreezingDelegate.Broadcast();
}

void UAttributeComponent::OnEndFreezing()
{
	Server_OnEndFreezing();
}

void UAttributeComponent::Server_OnEndFreezing_Implementation()
{
	Server_OnEndFreezingDelegate.Broadcast();

	Client_OnEndFreezing();
	Multicast_OnEndFreezing();
}

void UAttributeComponent::Client_OnEndFreezing_Implementation()
{
	Client_OnEndFreezingDelegate.Broadcast();
}

void UAttributeComponent::Multicast_OnEndFreezing_Implementation()
{
	Multicast_OnEndFreezingDelegate.Broadcast();
}

