// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DebugDamageSphere.generated.h"

class UDamageType_Base;

UCLASS()
class ADebugDamageSphere : public AActor
{
	GENERATED_BODY()
	
public:
	ADebugDamageSphere();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
    virtual void OnConstruction(const FTransform& Transform) override;

private:
    UFUNCTION()
    void Server_OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
    UPROPERTY(EditAnywhere)
    class USphereComponent* Collision;

private:
    UPROPERTY(EditAnywhere)
    float Radius;

    UPROPERTY(EditAnywhere)
    float Damage;

    UPROPERTY(EditAnywhere)
    TSubclassOf<UDamageType_Base> DamageType;
};
