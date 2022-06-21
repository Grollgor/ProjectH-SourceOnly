// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Items/Item_Base.h"

#include "Pickup_Base.generated.h"

class UStaticMeshComponent;
class UCapsuleComponent;

// TODO(philipp):
// - Only let certain actors pick up items, not just anyone with an inventory
// - Respawn items?
//   - Make sure we don't spawn too many objects

UCLASS(Blueprintable)
class PROJECTH_API APickup_Base : public AActor
{
	GENERATED_BODY()
	
public:	
	APickup_Base();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnItemPickUp(AActor* Item);

public:
	UPROPERTY(Replicated, EditAnywhere, Category="Item")
	FName ItemName;

    UPROPERTY(EditAnywhere, Category = "Item")
    FVector ItemScale;

	UPROPERTY(EditAnywhere, Category="Item")
	FVector ItemOffset;

    UPROPERTY(EditAnywhere, Category = "Item")
    FRotator ItemRotation;

	UPROPERTY(Replicated, VisibleAnywhere, Category="Debug")
	AItem_Base* ActiveItem;

	UPROPERTY(EditAnywhere, Category="Visuals")
	UStaticMesh* Mesh;

protected:
    UPROPERTY(EditAnywhere)
    UCapsuleComponent* CapsuleCollider;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UDataTable* DataTable;

private:
	UPROPERTY()
	UStaticMeshComponent* StaticMesh;
};
