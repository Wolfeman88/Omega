// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "OmegaCharacter.h"
#include "OmegaProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Classes/GameFramework/CharacterMovementComponent.h"
#include "OmegaGunBase.h"
#include "CoverActorBase.h"
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
	GunActor->SetupAttachment(Mesh1P);
}

void AOmegaCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	GunActor->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	CurrentWeapon = Cast<AOmegaGunBase>(GunActor->GetChildActor());

	normalHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	normalSpeed = GetCharacterMovement()->MaxWalkSpeed;
	normalRadius = GetCapsuleComponent()->GetUnscaledCapsuleRadius();

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

	if (bDoQuickTurn) ProcessQuickTurnOnTick(DeltaSeconds);
	if (bIsSliding) HandleSliding(DeltaSeconds);

	UpdateReticleState();

	FVector positionDelta = GetActorLocation() - previousPosition;
	if (!positionDelta.IsNearlyZero())
	{
		RechargeShield(shieldRechargeFactor * DeltaSeconds * positionDelta.Size());
		previousPosition = GetActorLocation();
	}

	if (CoverState == ECoverState::CS_COVER) HandleInCover();
	else if (CoverState == ECoverState::CS_MOVING) HandleMovingToCover();
}

void AOmegaCharacter::DoCrouch()
{
	if (bIsSprinting)
	{
		bIsSliding = true;
		SlideDirection = UKismetMathLibrary::GetForwardVector(GetControlRotation());
		SlideSpeed = GetCharacterMovement()->Velocity.Size();
		DoSprint();
	}

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
	if (CoverState == ECoverState::CS_COVER) ExitCover();

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

void AOmegaCharacter::UpdateReticleState()
{
	FHitResult* hit = new FHitResult();
	FCollisionQueryParams params = FCollisionQueryParams(FName(TEXT("collision query")), false, this);
	FVector CamLoc;
	FRotator CamRot;
	GetActorEyesViewPoint(CamLoc, CamRot);

	ReticleState = EViewTargetState::VTS_DEFAULT;
	aimLocation = FVector::ZeroVector;

	bool bHitSuccess = GetWorld()->LineTraceSingleByChannel(*hit, CamLoc, CamLoc + CamRot.Vector() * CoverInteractDistance, ECollisionChannel::ECC_WorldDynamic, params);

	if (bHitSuccess)
	{
		ACoverActorBase* coverActor = Cast<ACoverActorBase>(hit->GetActor());

		if (coverActor)
		{
			ReticleState = EViewTargetState::VTS_COVER;
			aimLocation = hit->Location;
		}
		else
		{
			bHitSuccess = GetWorld()->LineTraceSingleByChannel(*hit, CamLoc, CamLoc + CamRot.Vector() * NPCInteractDistance, ECollisionChannel::ECC_WorldDynamic, params);

			if (false)
			{	// TODO: NPC and Pickup classes to detect and handle reticle - not worried about this yet
				if (GetWorld()->LineTraceSingleByChannel(*hit, CamLoc, CamLoc + CamRot.Vector() * NPCInteractDistance, ECollisionChannel::ECC_WorldDynamic, params))
				{
					ReticleState = EViewTargetState::VTS_NPC;
					aimLocation = hit->Location;
					// TODO: differentiate between talk and stealth attack reticle behavior
					// potentially dot product of both actors forward vectors - if positive (facing away), stealth; if negative (facing), talk
					// EViewTargetState::VTS_STEALTH
				}

				else if (GetWorld()->LineTraceSingleByChannel(*hit, CamLoc, CamLoc + CamRot.Vector() * PickupInteractDistance, ECollisionChannel::ECC_WorldDynamic, params))
				{
					ReticleState = EViewTargetState::VTS_OBJECT;
					aimLocation = hit->Location;
					// TODO: differentiate between objective, health, and ammo pickups
					// need to make c++ classes for those pickups and if/else check
					// EViewTargetState::VTS_HEALTH;
					// EViewTargetState::VTS_AMMO;
				}
			}
		}
	}
}

void AOmegaCharacter::Action()
{
	if (GetCharacterMovement()->IsFalling()) return;

	if (ReticleState == EViewTargetState::VTS_COVER)
	{
		if (CoverState == ECoverState::CS_MOVING) ExitCover();
		else EnterCover();
	}
	else if ((ReticleState == EViewTargetState::VTS_AMMO) ||
		(ReticleState == EViewTargetState::VTS_HEALTH) ||
		(ReticleState == EViewTargetState::VTS_OBJECT))
	{
		// pickup object
	}
	else if (ReticleState == EViewTargetState::VTS_NPC)
	{
		// talk to NPC
	}
	else if (ReticleState == EViewTargetState::VTS_HEALTH)
	{ 
		// initiate stealth kill
	}
	else
	{
		DoSprint();
	}
}

void AOmegaCharacter::HandleSliding(float DeltaTime)
{
	SetActorLocation(GetActorLocation() + SlideDirection * (SlideSpeed * DeltaTime));
	SlideSpeed -= SlideAcceleration;
	bIsSliding = SlideSpeed > 0.f;
}

void AOmegaCharacter::HandleMovingToCover()
{
	FVector DeltaPosition = coverEntryLocation - GetActorLocation();
	float DotX = UKismetMathLibrary::Dot_VectorVector(GetActorForwardVector(), DeltaPosition.GetSafeNormal());
	float DotY = UKismetMathLibrary::Dot_VectorVector(GetActorRightVector(), DeltaPosition.GetSafeNormal());
	AddMovementInput(GetActorForwardVector(), MovingToCoverSpeedRate * DotX);
	AddMovementInput(GetActorRightVector(), MovingToCoverSpeedRate * DotY);

	if ((FMath::Abs(DeltaPosition.X) <= CoverEntryThreshold) && (FMath::Abs(DeltaPosition.Y) <= CoverEntryThreshold))
	{
		if (bIsSprinting) DoSprint();
		CoverState = ECoverState::CS_COVER;
	}
}

void AOmegaCharacter::EnterCover()
{
	GetCapsuleComponent()->SetCapsuleRadius(coverRadiusFactor * normalRadius);

	FHitResult* hit = new FHitResult();
	FCollisionQueryParams params = FCollisionQueryParams(FName(TEXT("collision query")), false, this);
	FVector StartLoc = GetActorLocation();
	FVector StartRot = GetActorForwardVector();
	
	if (GetWorld()->LineTraceSingleByChannel(*hit, StartLoc, StartLoc + StartRot * CoverInteractDistance, ECollisionChannel::ECC_WorldDynamic, params))
	{
		if (hit->GetActor() == CoverActor) return;

		CoverActor = Cast<ACoverActorBase>(hit->GetActor());
		CoverNormalVector = hit->Normal;
		bool bIsShortCover = CoverActor->GetIsCrouchHeight();

		if ((!bIsShortCover && bIsCrouching) || (bIsShortCover && !bIsCrouching)) DoCrouch();	

		coverEntryLocation = hit->Location;
		coverEntryLocation += CoverNormalVector * (GetCapsuleComponent()->GetUnscaledCapsuleRadius());

		CoverState = ECoverState::CS_MOVING;
	}
	else
	{
		ExitCover();
	}
}

void AOmegaCharacter::ExitCover()
{
	GetCapsuleComponent()->SetCapsuleRadius(normalRadius);
	CoverState = ECoverState::CS_NONE;
	CoverNormalVector = FVector::ZeroVector;
	CoverActor = nullptr;
	coverEntryLocation = FVector::ZeroVector;
}

void AOmegaCharacter::HandleInCover()
{
	FVector PlayerLoc = GetActorLocation();
	FVector CoverActorLoc = CoverActor->GetActorLocation();
	float CapsuleRad = GetCapsuleComponent()->GetUnscaledCapsuleRadius() + CoverActorGap;

	float DeltaX = PlayerLoc.X - CoverActorLoc.X;
	float ClampedX = CoverActorLoc.X + CapsuleRad * (DeltaX / FMath::Abs(DeltaX));
	float DeltaY = PlayerLoc.Y - CoverActorLoc.Y;
	float ClampedY = CoverActorLoc.Y + CapsuleRad * (DeltaY / FMath::Abs(DeltaY));

	float PlayerCoverOrientationFactor = FMath::Abs(UKismetMathLibrary::Dot_VectorVector(CoverNormalVector, FVector::ForwardVector));

	PlayerLoc.X = (PlayerCoverOrientationFactor == 0.f) ? PlayerLoc.X : ClampedX;
	PlayerLoc.Y = (PlayerCoverOrientationFactor == 0.f) ? ClampedY : PlayerLoc.Y;

	SetActorLocation(PlayerLoc);

	FHitResult* hit = new FHitResult();
	FCollisionQueryParams params = FCollisionQueryParams(FName(TEXT("collision query")), false, this);

	if (!GetWorld()->LineTraceSingleByObjectType(*hit, GetActorLocation(), GetActorLocation() + CoverNormalVector * fMinCoverDistance, ECollisionChannel::ECC_WorldStatic, params) ||
		(UKismetMathLibrary::Dot_VectorVector(GetCharacterMovement()->GetLastInputVector(), CoverNormalVector) > CoverExitThresholdFactor))
	{
		ExitCover();
	}
}

void AOmegaCharacter::ProcessQuickTurnOnTick(float DeltaTime)
{
	if (quickTurnDelta > 0.f)
	{
		float turnRate = quickTurnRateScale * DeltaTime * BaseTurnRate * ((quickTurnDirection == EQuickTurnDirection::QTD_LEFT) ? -1 : 1);
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

	PlayerInputComponent->BindAction("Action", IE_Pressed, this, &AOmegaCharacter::Action);
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
		if (CurrentWeapon->PrimaryFire(aimLocation))
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
		if (CurrentWeapon->SecondaryFire(aimLocation))
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
	if (bIsSliding || CoverState == ECoverState::CS_MOVING) return;
	// TODO: have forward movement away from cover break cover

	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);

		if ((Value < 0.f) && (bIsSprinting)) DoSprint();
	}
	else if (bIsSprinting) DoSprint();
}

void AOmegaCharacter::MoveRight(float Value)
{
	if (bIsSliding || CoverState == ECoverState::CS_MOVING) return;
	// TODO: have strafing movement away from cover break cover

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
	// if ((FMath::Abs(Rate) > 1.f) && (bIsSprinting)) DoSprint();
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
