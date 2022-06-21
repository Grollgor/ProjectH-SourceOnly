// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RespawnPoint.generated.h"

UCLASS()
class PROJECTH_API ARespawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	ARespawnPoint();

    UFUNCTION()
    void Server_OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	virtual void BeginPlay() override;
    virtual void EndPlay(EEndPlayReason::Type EndReason) override;

protected:
    UPROPERTY(EditAnywhere)
    class USphereComponent* Collision;

    UPROPERTY(EditAnywhere)
    class UArrowComponent* Arrow;

    // NOTE(philipp): I might wanna swap this to a skeletal mesh later and have animations for when the point
    //                activates, like in Borderlands for example
    UPROPERTY(EditAnywhere)
    class UStaticMeshComponent* StaticMesh;
};
