// Fill out your copyright notice in the Description page of Project Settings.
#include "Weapons/Weapon_Sword.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "DamageTypes/DamageType_Physical.h"
#include "DamageTypes/DamageType_Fire.h"
#include "DamageTypes/DamageType_Cold.h"

AWeapon_Sword::AWeapon_Sword()
{
    bCanStartNewCombo = true;
    bCanAdvanceCombo = true;
    ComboState = 0;
}

void AWeapon_Sword::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AWeapon_Sword, bCanAdvanceCombo);
    DOREPLIFETIME(AWeapon_Sword, bCanStartNewCombo);
    DOREPLIFETIME(AWeapon_Sword, ComboState);
}

void AWeapon_Sword::Server_OnUseImpl()
{
    if (!bInCombat) return;

    // TODO(philipp): this should probably be moved to client code
    if ((ComboState == -1 && bCanStartNewCombo) ||
        (ComboState >= 0 && bCanAdvanceCombo))
    {
        Multicast_Attack(++ComboState);
        if (ComboState >= Anims.UseMontages.Num() - 1)
        {
            ComboState = -1;
        }
    }
}

void AWeapon_Sword::Multicast_Attack_Implementation(int32 Combo)
{
    UAnimMontage* Montage = Anims.UseMontages[Combo];
    UAnimInstance* AnimInstance = OwningMesh->GetAnimInstance();
    if (AnimInstance)
    {
        AnimInstance->Montage_Play(Montage);
    }
}

void AWeapon_Sword::BeginPlay()
{
    if (HasAuthority())
    {
        Server_OnUseDelegate.AddDynamic(this, &AWeapon_Sword::Server_OnUseImpl);
    }
}

void AWeapon_Sword::EndPlay(EEndPlayReason::Type EndPlayReason)
{
    if (HasAuthority())
    {
        Server_OnUseDelegate.RemoveDynamic(this, &AWeapon_Sword::Server_OnUseImpl);
    }
}

void AWeapon_Sword::SetCanAdvanceCombo(bool bCanAdvance)
{
    bCanAdvanceCombo = bCanAdvance;
    if (!bCanAdvanceCombo)
    {
        ComboState = -1;
    }
}

void AWeapon_Sword::SetCanStartNewCombo(bool bCanStartCombo)
{
    bCanStartNewCombo = bCanStartCombo;
}

void AWeapon_Sword::DoDamage()
{
    AActor* Parent = GetAttachParentActor();
    check(Parent);

    if (WeaponData.Range == 0.f) return;

    TArray<FHitResult> HitResults;
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Emplace(Parent);
    ActorsToIgnore.Emplace(this);

    FVector TargetLocation = Parent->GetActorLocation();

    UKismetSystemLibrary::SphereTraceMulti(GetWorld(), TargetLocation, TargetLocation, WeaponData.Range,
        UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility),
        false, ActorsToIgnore, EDrawDebugTrace::None, HitResults, true);

    for (FHitResult& Result : HitResults)
    {
        AActor* HitActor = Result.GetActor();
        if (!HitActor->ActorHasTag(FName("Actor.IsTargetable"))) continue;

        FVector OwnerToActorVector = HitActor->GetActorLocation() - Parent->GetActorLocation();
        OwnerToActorVector.Normalize();
        if (FVector::DotProduct(Parent->GetActorRotation().Vector(), OwnerToActorVector) > 0.f)
        {
            UGameplayStatics::ApplyDamage(HitActor, WeaponData.PhysicalDamage, nullptr, this, UDamageType_Physical::StaticClass());
            UGameplayStatics::ApplyDamage(HitActor, WeaponData.FireDamage, nullptr, this, UDamageType_Fire::StaticClass());
            UGameplayStatics::ApplyDamage(HitActor, WeaponData.ColdDamage, nullptr, this, UDamageType_Cold::StaticClass());
        }
    }
}