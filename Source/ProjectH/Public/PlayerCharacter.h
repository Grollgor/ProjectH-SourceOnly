// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../ProjectH.h"
#include "GameFramework/Character.h"

#include "Interfaces/ComboInterface.h"

#include "PlayerCharacter.generated.h"

class UAnimMontage;
class UStaticMeshComponent;
class UPrimitiveComponent;
class USpringArmComponent;
class UCameraComponent;

class AItem_Base;
class UAttributeComponent;
class UContainerComponent;

UCLASS()
class PROJECTH_API APlayerCharacter : public ACharacter, public IComboInterface
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void SetRespawnPoint(FVector Location, FRotator Rotation);

	void MoveForward(float Value);
	void MoveRight(float Value);

	void Turn(float Value);
	void LookUp(float Value);

	UFUNCTION(Server, Unreliable)
	void Server_ToggleFreeCam();
	UFUNCTION(Client, Unreliable)
	void Client_ToggleFreeCam(bool bInUseFreeCamera, FRotator Rotation, AActor* Target);
	void ToggleFreeCam();

	void ToggleCombat();

	UAttributeComponent* GetAttributeComponent() { return Attributes; }
	UContainerComponent* GetContainerComponent() { return Inventory; }

	// Combo Interface
    virtual void SetCanAdvanceCombo(bool bCanAdvance) override {}
    virtual void SetCanStartNewCombo(bool bCanStartCombo) override {}
    virtual void DoDamage() override {};
    virtual AItem_Base* GetEquippedItem() { return EquippedItem; }

protected:
    virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndReason);

	UFUNCTION()
	void Interact();

	UFUNCTION()
	void Attack();

	UFUNCTION(Server, Reliable)
	void Server_Interact();

private:
	UFUNCTION(Client, Unreliable)
	void Client_OnDeath(FRotator Rotation);
	UFUNCTION()
	void OnDeath(EDamageType DamageType);

public:
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	uint8 bUseFreeCamera : 1;

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    AItem_Base* EquippedItem;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UAttributeComponent* Attributes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UContainerComponent* Inventory;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float VFOV;

    UPROPERTY(Replicated, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    FVector RespawnLocation;

    UPROPERTY(Replicated, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    FRotator RespawnRotation;

private:
	UPROPERTY(VisibleAnywhere)
	AActor* TargetActor;

	UPROPERTY(VisibleAnywhere)
	float TimeSinceTargetLastSeen;
};
