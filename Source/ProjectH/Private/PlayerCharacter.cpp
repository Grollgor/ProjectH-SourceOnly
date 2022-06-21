// Fill out your copyright notice in the Description page of Project Settings.
#include "PlayerCharacter.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "Camera/CameraComponent.h"

#include "Animation/AnimMontage.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

#include "DrawDebugHelpers.h"

#include "Components/AttributeComponent.h"
#include "Components/ContainerComponent.h"

#include "Items/Item_Base.h"
#include "Weapons/Weapon_Base.h"

APlayerCharacter::APlayerCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.f);
	SetReplicates(true);
	SetReplicateMovement(true);
	PrimaryActorTick.bCanEverTick = true;

	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	CameraArm->SetupAttachment(GetCapsuleComponent());
    CameraArm->SetRelativeLocation({ 0.f, 0.f, 96.f });
    CameraArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraArm);

	EquippedItem = nullptr;
	TargetActor = nullptr;
	TimeSinceTargetLastSeen = 0.f;

	bUseFreeCamera = true;
    GetCharacterMovement()->bOrientRotationToMovement = bUseFreeCamera;
    bUseControllerRotationYaw = !bUseFreeCamera;
	CameraArm->SetUsingAbsoluteRotation(bUseFreeCamera);

	Tags.Add(FName("Actor.IsTargetable"));

	VFOV = 70.f;

	Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));
	Attributes->SetNetAddressable();
	Attributes->SetIsReplicated(true);

	Inventory = CreateDefaultSubobject<UContainerComponent>(TEXT("Inventory"));
	Inventory->SetNetAddressable();
	Inventory->SetIsReplicated(true);
}

void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerCharacter, bUseFreeCamera);

    DOREPLIFETIME(APlayerCharacter, EquippedItem);

	DOREPLIFETIME(APlayerCharacter, RespawnLocation);
	DOREPLIFETIME(APlayerCharacter, RespawnRotation);
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	RespawnLocation = GetActorLocation();
	RespawnRotation = GetActorRotation();

	if (HasAuthority())
	{
		Attributes->Server_OnDeathDelegate.AddDynamic(this, &APlayerCharacter::OnDeath);
	}
}

void APlayerCharacter::EndPlay(EEndPlayReason::Type EndReason)
{
    if (HasAuthority())
    {
        Attributes->Server_OnDeathDelegate.RemoveDynamic(this, &APlayerCharacter::OnDeath);
    }

	Super::EndPlay(EndReason);
}

void APlayerCharacter::Interact()
{
	Server_Interact();
}

void APlayerCharacter::Attack()
{
	if (!EquippedItem) return;
	EquippedItem->OnUse();
}

void APlayerCharacter::Server_Interact_Implementation()
{
    TArray<FHitResult> HitResults;
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Emplace(this);
    if (EquippedItem)
    {
        ActorsToIgnore.Emplace(EquippedItem);
    }

    FVector TargetLocation = GetActorLocation();

    UKismetSystemLibrary::SphereTraceMulti(GetWorld(), TargetLocation, TargetLocation, 150.f,
                                           UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility),
                                           false, ActorsToIgnore, EDrawDebugTrace::None, HitResults, true);

    for (FHitResult& HitResult : HitResults)
    {
        AItem_Base* Item = Cast<AItem_Base>(HitResult.GetActor());
        if (!Item) continue;

		Inventory->AddItems(Item->Name, Item->Amount);

        Item->SetActorLocation({ 0.f, 0.f, -10000.f });
        Item->Destroy();
        Item->MarkAsGarbage();
		break;
    }
}

void APlayerCharacter::Client_OnDeath_Implementation(FRotator Rotation)
{
	GetController()->SetControlRotation(Rotation);
}

void APlayerCharacter::OnDeath(EDamageType DamageType)
{
	// TODO(philipp): Disable player input here, and only respawn player after x seconds of time

	SetActorLocation(RespawnLocation);
	SetActorRotation(RespawnRotation);

    if (Attributes->FireStatus.bIsActive)
    {
        Attributes->OnEndBurning();
    }
	Attributes->FireStatus.CurrentBuildup = 0.f;
	Attributes->FireStatus.bIsActive = false;

    if (Attributes->ColdStatus.bIsActive)
    {
        Attributes->OnEndFreezing();
    }
    Attributes->ColdStatus.CurrentBuildup = 0.f;
    Attributes->ColdStatus.bIsActive = false;

    for (FHealthContainer& HealthBar : Attributes->HealthBars)
    {
        HealthBar.CurrentHealth = HealthBar.MaxHealth;
    }
    Attributes->ActiveHealthBar = Attributes->HealthBars.Num() - 1;
	Attributes->OnHealthReceived(Attributes->HealthBars[Attributes->ActiveHealthBar].CurrentHealth, Attributes->ActiveHealthBar);

	Client_OnDeath(RespawnRotation);
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    DrawDebugCylinder(GetWorld(), RespawnLocation, RespawnLocation + FVector(0.f, 0.f, 500.f), 100.f, 10, FColor::Green, false, -1.0f, 0U, 2.f);

	if (IsLocallyControlled())
	{
		// NOTE(philipp): unreal doesn't seem to have an option to fix the FOV to the vertical axis, so I have to do this here manually
		FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
		float AspectRatio = ViewportSize.X / ViewportSize.Y;
		Camera->SetAspectRatio(AspectRatio);
		Camera->SetFieldOfView(2.f * FMath::RadiansToDegrees(FMath::Atan(AspectRatio * FMath::Tan(FMath::DegreesToRadians(VFOV / 2.f)))));

		if (TargetActor)
		{
            FRotator NewRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetActor->GetActorLocation());
            AController* Con = GetController();
            check(Con);
            Con->SetControlRotation(NewRot);
		}
	}

	if (HasAuthority() && !bUseFreeCamera && TargetActor)
	{
		FHitResult HitResult;
		if (GetWorld()->LineTraceSingleByChannel(HitResult, GetActorLocation(), TargetActor->GetActorLocation(), ECC_Visibility))
		{
			if (HitResult.GetActor() != TargetActor)
			{
				TimeSinceTargetLastSeen += DeltaTime;
			}
			else
			{
				TimeSinceTargetLastSeen = 0.f;
			}
		}

		FVector Direction = GetActorLocation() - TargetActor->GetActorLocation();
		if (Direction.SquaredLength() >= 5000 * 5000 || TimeSinceTargetLastSeen > 5.f)
		{
			TimeSinceTargetLastSeen = 0.f;
			ToggleFreeCam();
		}
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

    PlayerInputComponent->BindAction("ToggleFreeCam", IE_Pressed, this, &APlayerCharacter::ToggleFreeCam);
    PlayerInputComponent->BindAction("ToggleCombat", IE_Pressed, this, &APlayerCharacter::ToggleCombat);

    PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &APlayerCharacter::Interact);

    PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &APlayerCharacter::Attack);

    PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);

    PlayerInputComponent->BindAxis("Turn", this, &APlayerCharacter::Turn);
    PlayerInputComponent->BindAxis("LookUp", this, &APlayerCharacter::LookUp);

	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void APlayerCharacter::SetRespawnPoint(FVector Location, FRotator Rotation)
{
	RespawnLocation = Location + FVector(0.f, 0.f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	RespawnRotation = Rotation;
}

void APlayerCharacter::MoveForward(float Value)
{
	if (Attributes->ColdStatus.bIsActive) return;

	if (bUseFreeCamera)
	{
		FRotator ControlRot = GetControlRotation();
		ControlRot.Pitch = 0.f;
		ControlRot.Roll = 0.f;
		AddMovementInput(ControlRot.Vector(), Value);
	}
	else
	{
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void APlayerCharacter::MoveRight(float Value)
{
    if (Attributes->ColdStatus.bIsActive) return;

	if (bUseFreeCamera)
	{
		FRotator ControlRot = GetControlRotation();
		ControlRot.Pitch = 0.f;
		ControlRot.Roll = 0.f;
		FVector RightVector = FRotationMatrix(ControlRot).GetScaledAxis(EAxis::Y);
		AddMovementInput(RightVector, Value);
	}
	else
	{
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void APlayerCharacter::Turn(float Value)
{
	if (bUseFreeCamera)
	{
		AddControllerYawInput(Value * 1.f);
	}
}

void APlayerCharacter::LookUp(float Value)
{
	if (bUseFreeCamera)
	{
		AddControllerPitchInput(Value * 1.f);
	}
}

void APlayerCharacter::Server_ToggleFreeCam_Implementation()
{
	FRotator NewRot;
	if (bUseFreeCamera)
	{
		TArray<FHitResult> HitResults;
		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Emplace(this);

		FVector Direction = GetControlRotation().Vector();
		FVector TargetLocation = GetActorLocation() + Direction * 5000.f;
		if(UKismetSystemLibrary::SphereTraceMulti(GetWorld(), GetActorLocation(), TargetLocation, 500.f, UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility), false, ActorsToIgnore, EDrawDebugTrace::None, HitResults, true))
		{
			for (FHitResult& HitResult : HitResults)
			{
				if (HitResult.GetActor()->ActorHasTag(FName("Actor.IsTargetable")))
				{
					FHitResult HitResultLT;
					if (GetWorld()->LineTraceSingleByChannel(HitResultLT, GetActorLocation(), HitResult.GetActor()->GetActorLocation(), ECC_Visibility))
					{
						if (HitResultLT.GetActor() != HitResult.GetActor()) continue;
					}

                    FVector SelfToActorVector = HitResult.GetActor()->GetActorLocation() - GetActorLocation();
                    SelfToActorVector.Normalize();
                    if(FVector::DotProduct(GetControlRotation().Vector(), SelfToActorVector) <= 0.f) continue;

					bUseFreeCamera = !bUseFreeCamera;

                    TargetActor = HitResult.GetActor();
                    NewRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetActor->GetActorLocation());
                    AController* Con = GetController();
                    Con->SetControlRotation(NewRot);
					break;
				}
			}
		}
	}
	else
	{
		bUseFreeCamera = !bUseFreeCamera;
		TargetActor = nullptr;

        if (!bUseFreeCamera)
        {
            float Yaw = GetActorRotation().Yaw;
            AController* Con = GetController();
			NewRot = Con->GetControlRotation();
			NewRot.Yaw = Yaw;
            Con->SetControlRotation(NewRot);
        }
	}

    GetCharacterMovement()->bOrientRotationToMovement = bUseFreeCamera;
    bUseControllerRotationYaw = !bUseFreeCamera;
    CameraArm->SetUsingAbsoluteRotation(bUseFreeCamera);

    Client_ToggleFreeCam(bUseFreeCamera, NewRot, TargetActor);
}

void APlayerCharacter::Client_ToggleFreeCam_Implementation(bool bInUseFreeCamera, FRotator Rotation, AActor* Target)
{
	TargetActor = Target;

    if (!bInUseFreeCamera)
    {
		AController* Con = GetController();
		check(Con);
        Con->SetControlRotation(Rotation);
    }

    GetCharacterMovement()->bOrientRotationToMovement = bInUseFreeCamera;
    bUseControllerRotationYaw = !bInUseFreeCamera;
    CameraArm->SetUsingAbsoluteRotation(bInUseFreeCamera);
}

void APlayerCharacter::ToggleFreeCam()
{
	Server_ToggleFreeCam();
}

void APlayerCharacter::ToggleCombat()
{
	if (!EquippedItem) return;
	EquippedItem->Server_SetCombatState();
}

