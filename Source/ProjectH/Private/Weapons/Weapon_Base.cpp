// Fill out your copyright notice in the Description page of Project Settings.
#include "Weapons/Weapon_Base.h"

AWeapon_Base::AWeapon_Base()
{
	static ConstructorHelpers::FObjectFinder<UDataTable> WeaponDataTableObject(TEXT("DataTable'/Game/ProjectH/Core/DataTables/DT_WeaponData.DT_WeaponData'"));
	if (WeaponDataTableObject.Succeeded())
	{
		WeaponDataTable = WeaponDataTableObject.Object;
    }
}

void AWeapon_Base::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    FItemData* Data = DataTable->FindRow<FItemData>(Name, TEXT(""));
    if (Data)
    {
        FWeaponData* WeaponDataPtr = WeaponDataTable->FindRow<FWeaponData>(Data->TypeName, TEXT(""), true);
        if (WeaponDataPtr)
        {
            WeaponData = *WeaponDataPtr;
        }
    }
}