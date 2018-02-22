// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "OmegaCharacter.h"
#include "OmegaProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Classes/GameFramework/CharacterMovementComponent.h"
#include "OmegaGunBase.h"
#include "Components/ChildActorComponent.h"

#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AOmegaCharacter

AOmegaCharacter::AOmegaCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

	// creating a default child actor component to 'hold' the current weapon
	GunActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("GunActor"));
}

void AOmegaCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	GunActor->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	CurrentWeapon = Cast<AOmegaGunBase>(GunActor->GetChildActor());

	normalHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	normalSpeed = GetCharacterMovement()->MaxWalkSpeed;

	originalScopePosition = Mesh1P->RelativeLocation;
	originalFieldOfView = FirstPersonCameraComponent->FieldOfView;

	currentHealth = maxHealth;
	currentShield = maxShield;

	previousPosition = GetActorLocation();
	previousRotation = GetControlRotation();
}

void AOmegaCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bDoQuickTurn)
	{
		if (quickTurnDelta > 0.f)
		{
			float turnRate = quickTurnRateScale * DeltaSeconds * BaseTurnRate * ((quickTurnDirection == EQuickTurnDirection::QTD_LEFT) ? -1 : 1);
			float absTurnRate = FMath::Abs<float>(turnRate);
			turnRate = (quickTurnDelta - absTurnRate >= 0.f) ? turnRate : ((turnRate * quickTurnDelta) / absTurnRate);

			float inputYawScale = UGameplayStatics::GetPlayerController(this, 0)->InputYawScale;
			AddControllerYawInput(turnRate / inputYawScale);

			quickTurnDelta -= FMath::Abs<float>(turnRate);;
		}
		else 
		{
			bDoQuickTurn = false;
		}
	}

	/* this code is the start of the system to determine what player is looking at - drives UI */
	FHitResult* hit = new FHitResult();
	FCollisionQueryParams params = FCollisionQueryParams(FName(TEXT("collision query")), true, this);
	FVector CamLoc;
	FRotator CamRot;
	GetActorEyesViewPoint(CamLoc, CamRot);

	if (GetWorld()->LineTraceSingleByChannel(*hit, CamLoc, CamLoc + CamRot.Vector() * 5000.f, ECollisionChannel::ECC_PhysicsBody, params))
	{
		if (!hit->GetActor()->IsRootComponentMovable())
		{
			// GEngine->AddOnScreenDebugMessage(INDEX_NONE, 0.f, FColor::Blue, hit->GetActor()->GetName());
		}
	}

	FVector positionDelta = GetActorLocation() - previousPosition;
	// FRotator rotationDelta = GetControlRotation() - previousRotation;

	if (!positionDelta.IsNearlyZero())
	{
		RechargeShield(shieldRechargeFactor * DeltaSeconds * positionDelta.Size());
		previousPosition = GetActorLocation();
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AOmegaCharacter::DoCrouch()
{
	if (bIsSprinting) DoSprint();

	bIsCrouching = !bIsCrouching;

	GetCapsuleComponent()->SetCapsuleHalfHeight(normalHeight * ((bIsCrouching) ? crouchHeightFactor : 1.f));
	GetCharacterMovement()->MaxWalkSpeed = normalSpeed * ((bIsCrouching) ? crouchSpeedFactor : 1.f);
}

void AOmegaCharacter::StopCrouch()
{
	if (HoldCrouch && bIsCrouching) DoCrouch();
}

void AOmegaCharacter::DoSprint()
{
	if (bIsCrouching) DoCrouch();
	if (bIsScoped) ZoomIn();

	bIsSprinting = !bIsSprinting;

	GetCapsuleComponent()->SetCapsuleHalfHeight(normalHeight * ((bIsSprinting) ? sprintHeightFactor : 1.f));
	GetCharacterMovement()->MaxWalkSpeed = normalSpeed * ((bIsSprinting) ? sprintSpeedFactor : 1.f);
}

void AOmegaCharacter::StopSprint()
{
	if (HoldSprint && bIsSprinting) DoSprint();
}

void AOmegaCharacter::DoQuickTurn()
{
	if (bDoQuickTurn) return;

	bDoQuickTurn = true;
	quickTurnDelta = fQuickTurnAngle;
}

void AOmegaCharacter::ZoomIn()
{
	if (bIsSprinting)
	{
		DoSprint();
		return;
	}
	else if (bDoQuickTurn)
	{
		bDoQuickTurn = false;
		quickTurnDelta = 0.f;
		return;
	}

	bIsScoped = !bIsScoped;

	FirstPersonCameraComponent->SetFieldOfView(originalFieldOfView * ((bIsScoped) ? scopeZoomFactor : 1.f));
	GetCharacterMovement()->MaxWalkSpeed = normalSpeed * ((bIsScoped) ? scopeSpeedFactor : 1.f);

	// TODO: redo parenting scheme to make offset behavior easier to manage
}

void AOmegaCharacter::ZoomOut()
{
	if (HoldScope && bIsScoped) ZoomIn();
}

void AOmegaCharacter::ResetAim()
{
	APlayerController* currentPlayerController = UGameplayStatics::GetPlayerController(this, 0);
	FRotator currentRotation = currentPlayerController->GetControlRotation();
	currentPlayerController->SetControlRotation(*(new FRotator(0.f, currentRotation.Yaw, currentRotation.Roll)));
}

void AOmegaCharacter::StartReload()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartReload();
	}
}

void AOmegaCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("PrimaryFire", IE_Pressed, this, &AOmegaCharacter::OnPrimaryFire);
	PlayerInputComponent->BindAction("SecondaryFire", IE_Pressed, this, &AOmegaCharacter::OnSecondaryFire);

	PlayerInputComponent->BindAxis("MoveForward", this, &AOmegaCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AOmegaCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &AOmegaCharacter::TurnAbsolute);
	PlayerInputComponent->BindAxis("TurnRate", this, &AOmegaCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &AOmegaCharacter::LookUpAbsolute);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AOmegaCharacter::LookUpAtRate);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AOmegaCharacter::DoCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AOmegaCharacter::StopCrouch);

	PlayerInputComponent->BindAction("Action", IE_Pressed, this, &AOmegaCharacter::DoSprint);
	PlayerInputComponent->BindAction("Action", IE_Released, this, &AOmegaCharacter::StopSprint);

	PlayerInputComponent->BindAction("QuickTurn", IE_Pressed, this, &AOmegaCharacter::DoQuickTurn);

	PlayerInputComponent->BindAction("Special", IE_Pressed, this, &AOmegaCharacter::OnSpecial);

	PlayerInputComponent->BindAction("Scope", IE_Pressed, this, &AOmegaCharacter::ZoomIn);
	PlayerInputComponent->BindAction("Scope", IE_Released, this, &AOmegaCharacter::ZoomOut);

	PlayerInputComponent->BindAction("ResetAim", IE_Pressed, this, &AOmegaCharacter::ResetAim);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AOmegaCharacter::StartReload);
}

void AOmegaCharacter::ReceiveDamage(float damage)
{
	float remainingDamage = 0.f;
	float overkill = 0.f;

	if (currentShield > damage)
	{
		currentShield = currentShield - damage;
	}
	else
	{
		remainingDamage = damage - currentShield;
		currentShield = 0.f;
	}

	if (currentHealth > remainingDamage)
	{
		currentHealth = currentHealth - (remainingDamage - (remainingDamage * (armorFactor)));
	}
	else
	{
		overkill = remainingDamage - currentHealth;
		currentHealth = 0.f;
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Black, TEXT("You died..."));
		DisableInput(UGameplayStatics::GetPlayerController(this, 0));
	}
}

void AOmegaCharacter::RechargeShield(float regen)
{
	currentShield = FMath::Min(maxShield, currentShield + regen);
}

void AOmegaCharacter::RegainHealth(float health)
{
	currentHealth = FMath::Min(maxHealth, currentHealth + health);
}

void AOmegaCharacter::OnPrimaryFire()
{
	if (bIsSprinting)
	{
		DoSprint();
		return;
	}
	else if (bDoQuickTurn)
	{
		bDoQuickTurn = false;
		quickTurnDelta = 0.f;
		return;
	}
	else if (CurrentWeapon)
	{
		if (CurrentWeapon->PrimaryFire())
		{
			// try and play a firing animation if specified
			if (FireAnimation != NULL)
			{
				// Get the animation object for the arms mesh
				UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
				if (AnimInstance != NULL)
				{
					AnimInstance->Montage_Play(FireAnimation, 1.f);
				}
			}
		}
	}
}

void AOmegaCharacter::OnSecondaryFire()
{
	if (bIsSprinting)
	{
		DoSprint();
		return;
	}
	else if (bDoQuickTurn)
	{
		bDoQuickTurn = false;
		quickTurnDelta = 0.f;
		return;
	}
	else if (CurrentWeapon)
	{
		if (CurrentWeapon->SecondaryFire())
		{
			// try and play a firing animation if specified
			if (FireAnimation != NULL)
			{
				// Get the animation object for the arms mesh
				UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
				if (AnimInstance != NULL)
				{
					AnimInstance->Montage_Play(FireAnimation, 1.f);
				}
			}
		}
	}
}

void AOmegaCharacter::OnSpecial()
{
	// TODO: remove reference to GEngine and includes up top
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Special Ability activated!"));

	// make call to character-specific special ability function
	// e.g. duration buff, toggled mode, hold-to-engage behavior
}

void AOmegaCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AOmegaCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AOmegaCharacter::TurnAtRate(float Rate)
{
	if (bDoQuickTurn) return;
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AOmegaCharacter::TurnAbsolute(float Rate)
{
	if (bDoQuickTurn) return;
	AddControllerYawInput(Rate);
}

void AOmegaCharacter::LookUpAtRate(float Rate)
{
	if (bDoQuickTurn) return;
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AOmegaCharacter::LookUpAbsolute(float Rate)
{
	if (bDoQuickTurn) return;
	AddControllerPitchInput(Rate);
}
