// Fill out your copyright notice in the Description page of Project Settings.
#include "Auras/Aura_Base.h"

#include "Net/UnrealNetwork.h"

UAura_Base::UAura_Base()
{
    DataTable = nullptr;
    static ConstructorHelpers::FObjectFinder<UDataTable> DataTableObj(TEXT("DataTable'/Game/ProjectH/Core/DataTables/DT_Auras.DT_Auras'"));
    if (DataTableObj.Succeeded())
    {
        DataTable = DataTableObj.Object;
    }
}

void UAura_Base::Client_OnAuraAppliedFnc_Implementation()
{
    Client_OnAuraApplied();
}

void UAura_Base::Multicast_OnAuraAppliedFnc_Implementation()
{
    Multicast_OnAuraApplied();
}

void UAura_Base::Server_OnAuraApplied()
{
    FAuraData* Data = DataTable->FindRow<FAuraData>(AuraName, TEXT(""));
    check(Data);
    TimeRemaining = Data->Duration;

    Client_OnAuraApplied();
    Multicast_OnAuraApplied();
}

void UAura_Base::Client_OnAuraApplied()
{
}

void UAura_Base::Multicast_OnAuraApplied()
{
}

void UAura_Base::Server_OnAuraTicked(float DeltaTime)
{
    TimeRemaining -= DeltaTime;
}

void UAura_Base::Client_OnAuraTicked(float DeltaTime)
{
}

void UAura_Base::Multicast_OnAuraTicked(float DeltaTime)
{
}

void UAura_Base::Server_OnAuraExpired()
{
}

void UAura_Base::Client_OnAuraExpired()
{
}

void UAura_Base::Multicast_OnAuraExpired()
{
}

void UAura_Base::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UAura_Base, AuraName);
    DOREPLIFETIME(UAura_Base, TimeRemaining);

    // NOTE(philipp): Make sure that blueprint properties get replicated
    if (const UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(GetClass()))
    {
        BPClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
    }
}

int32 UAura_Base::GetFunctionCallspace(UFunction* Function, FFrame* Stack)
{
    check(GetOuter() != nullptr);
    return GetOuter()->GetFunctionCallspace(Function, Stack);
}

bool UAura_Base::CallRemoteFunction(UFunction* Function, void* Parms, struct FOutParmRec* OutParms, FFrame* Stack)
{
    check(!HasAnyFlags(RF_ClassDefaultObject));

    AActor* Owner = GetOwningActor();
    UNetDriver* NetDriver = Owner->GetNetDriver();
    if (NetDriver)
    {
        NetDriver->ProcessRemoteFunction(Owner, Function, Parms, OutParms, Stack, this);
        return true;
    }
    return false;
}

UWorld* UAura_Base::GetWorld() const
{
    if (const UObject* MyOuter = GetOuter())
    {
        return MyOuter->GetWorld();
    }
    return nullptr;
}

void UAura_Base::Destroy()
{
    if (!IsValid(this))
    {
        checkf(GetOwningActor()->HasAuthority(), TEXT("Destroy:: Object does not have authority to destroy itself!"));

        MarkAsGarbage();
    }
}
