// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "OmegaCharacter.generated.h"

class UInputComponent;
class UCharacterMovementComponent;

UENUM(BlueprintType)
enum class EQuickTurnDirection : uint8
{
	QTD_LEFT 	UMETA(DisplayName = "Left"),
	QTD_RIGHT	UMETA(DisplayName = "Right")
};

UENUM(BlueprintType)
enum class EViewTargetState : uint8
{
	VTS_DEFAULT		UMETA(DisplayName = "Default"),
	VTS_COVER		UMETA(DisplayName = "Cover"),
	VTS_AMMO		UMETA(DisplayName = "Ammo"),
	VTS_HEALTH		UMETA(DisplayName = "Health"),
	VTS_OBJECT		UMETA(DisplayName = "Objective"),
	VTS_NPC			UMETA(DisplayName = "NPC"),
	VTS_STEALTH		UMETA(DisplayName = "Stealth")
};

UENUM(BlueprintType)
enum class ECoverState : uint8
{
	CS_NONE		UMETA(DisplayName = "None"),
	CS_MOVING	UMETA(DisplayName = "Moving to Cover"),
	CS_COVER	UMETA(DisplayName = "In Cover")
};

UCLASS(config=Game)
class AOmegaCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UChildActorComponent* GunActor_Primary;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UChildActorComponent* GunActor_Secondary;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class AOmegaGunBase* CurrentWeapon;

public:
	AOmegaCharacter();

protected:
	virtual void BeginPlay();

	virtual void Tick(float DeltaSeconds) override;

public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* FireAnimation;

	UFUNCTION(BlueprintCallable, Category = "Health")
	void ReceiveDamage(float damage);
	UFUNCTION(BlueprintCallable, Category = "Health")
	void RechargeShield(float regen);
	UFUNCTION(BlueprintCallable, Category = "Health")
	void RegainHealth(float health);

	UFUNCTION(BlueprintCallable, Category = "Reticle")
	FVector GetAimLocation();

	UFUNCTION(BlueprintCallable, Category = "Ammo")
	void RegainAmmo(int32 ammo);

protected:
	
	/** Fires the primary projectile/ability of the weapon. */
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void OnPrimaryFire();
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void OnPrimaryFireEnd();
	
	/** Fires the secondary projectile/ability of the weapon. */
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void OnSecondaryFire();

	/** activates player-specific special ability */
	UFUNCTION(BlueprintImplementableEvent, Category = "Special")
	void OnSpecial();

	/** fires off player-specific melee ability */
	UFUNCTION(BlueprintCallable, Category = "Melee")
	void OnMelee();

	/** Handles moving forward/backward */
	UFUNCTION(BlueprintCallable, Category = "Character Movement")
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	UFUNCTION(BlueprintCallable, Category = "Character Movement")
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	UFUNCTION(BlueprintCallable, Category = "Character Movement")
	void TurnAtRate(float Rate);

	UFUNCTION(BlueprintCallable, Category = "Character Movement")
	void TurnAbsolute(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	UFUNCTION(BlueprintCallable, Category = "Character Movement")
	void LookUpAtRate(float Rate);
	
	UFUNCTION(BlueprintCallable, Category = "Character Movement")
	void LookUpAbsolute(float Rate);
	
	/* these functions and variables handle crouching, un-crouching, and movement changes while crouching*/
	UFUNCTION(BlueprintCallable, Category = "Crouching")
	void DoCrouch();
	UFUNCTION(BlueprintCallable, Category = "Crouching")
	void StopCrouch();

	UPROPERTY(EditAnywhere, Category = "Crouching")
	bool HoldCrouch = true;
	UPROPERTY(EditAnywhere, Category = "Crouching")
	float crouchHeightFactor = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Crouching")
	float crouchSpeedFactor = 0.5f;
	UPROPERTY(BlueprintReadOnly, Category = "Crouching")
	bool bIsCrouching = false;

	/* these functions and variables handle sprinting, un-sprinting, and movement changes while sprinting*/
	UFUNCTION(BlueprintCallable, Category = "Sprinting")
	void DoSprint();
	UFUNCTION(BlueprintCallable, Category = "Sprinting")
	void StopSprint();

	UPROPERTY(EditAnywhere, Category = "Sprinting")
	bool HoldSprint = true;
	UPROPERTY(EditAnywhere, Category = "Sprinting")
	float sprintHeightFactor = 0.8f;
	UPROPERTY(EditAnywhere, Category = "Sprinting")
	float sprintSpeedFactor = 2.f;
	UPROPERTY(BlueprintReadOnly, Category = "Sprinting")
	bool bIsSprinting = false;

	/* these functions and variables handle quick turning */
	UFUNCTION(BlueprintCallable, Category = "Quick Turn")
	void DoQuickTurn();

	UPROPERTY(EditAnywhere, Category = "Quick Turn", meta = (ClampMin = "1.0", ClampMax = "50.0"))
	float quickTurnRateScale = 10.f;
	UPROPERTY(EditAnywhere, Category = "Quick Turn")
	EQuickTurnDirection quickTurnDirection;
	UPROPERTY(BlueprintReadWrite, Category = "Quick Turn")
	float quickTurnDelta = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Quick Turn")
	bool bDoQuickTurn = false;

	/* these functions and variables handle scoping in/aim-down sights */
	UFUNCTION(BlueprintCallable, Category = "Scope")
	void ZoomIn();
	UFUNCTION(BlueprintCallable, Category = "Scope")
	void ZoomOut();
	
	UPROPERTY(EditAnywhere, Category = "Scope")
	FVector scopedInOffset;
	UPROPERTY(EditAnywhere, Category = "Scope")
	float scopeZoomFactor = 0.75f;
	UPROPERTY(EditAnywhere, Category = "Scope")
	bool HoldScope = true;
	UPROPERTY(BlueprintReadWrite, Category = "Scope")
	bool bIsScoped = false;
	UPROPERTY(EditAnywhere, Category = "Scope")
	float scopeSpeedFactor = 0.5f;

	/* this function handles reseting the aim to 0 pitch */
	UFUNCTION(BlueprintCallable, Category = "Reset Aim")
	void ResetAim();

	/* these variables and functions handle player and shield health, and armor */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (ClampMin = 50.f, ClampMax = 500.f))
	float maxHealth = 100.f;
	UPROPERTY(BlueprintReadWrite, Category = "Health", meta = (ClampMin = 0.f))
	float currentHealth;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (ClampMin = 50.f, ClampMax = 500.f))
	float maxShield = 100.f;
	UPROPERTY(BlueprintReadWrite, Category = "Health", meta = (ClampMin = 0.f))
	float currentShield;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (ClampMin = 0.f, ClampMax = 2.f))
	float shieldRechargeFactor = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (ClampMin = 0.f, ClampMax = 1.f))
	float armorFactor = 0.2f;

	/* these variables and functions handle crosshair behavior */
	UPROPERTY(BlueprintReadOnly, Category = "Reticle")
	EViewTargetState ReticleState = EViewTargetState::VTS_DEFAULT;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reticle")
	float MaxAimDistance = 10000.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reticle")
	float CoverInteractDistance = 750.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reticle")
	float PickupInteractDistance = 500.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reticle")
	float NPCInteractDistance = 250.f;

	UFUNCTION(BlueprintCallable, Category = "Reticle")
	void UpdateReticleState();

	/* this function handles the base action/interact/sprint/cover decision */
	UFUNCTION(BlueprintCallable, Category = "Action")
	void Action();

	/* these variables handle sliding behavior*/
	UFUNCTION(BlueprintCallable, Category = "Sliding")
	void HandleSliding(float DeltaTime);

	UPROPERTY(BlueprintReadWrite, Category = "Sliding", meta = (ClampMin = 1.f, ClampMax = 10.f))
	float SlideAcceleration = 7.f;
	UPROPERTY(BlueprintReadOnly, Category = "Sliding")
	bool bIsSliding = false;
	UPROPERTY(BlueprintReadOnly, Category = "Sliding")
	FVector SlideDirection;
	UPROPERTY(BlueprintReadOnly, Category = "Sliding")
	float SlideSpeed = 0.f;

	/* these variables and functions handle cover movement and behavior */
	UFUNCTION(BlueprintCallable, Category = "Cover")
	void HandleMovingToCover();			// for now just going to teleport - will add actual movement later
	UFUNCTION(BlueprintCallable, Category = "Cover")
	void HandleInCover();
	UFUNCTION(BlueprintCallable, Category = "Cover")
	void EnterCover();
	UFUNCTION(BlueprintCallable, Category = "Cover")
	void ExitCover();

	UPROPERTY(BlueprintReadOnly, Category = "Cover")
	ECoverState CoverState = ECoverState::CS_NONE;
	UPROPERTY(BlueprintReadOnly, Category = "Cover")
	class ACoverActorBase* CoverActor = nullptr;
	UPROPERTY(BlueprintReadOnly, Category = "Cover")
	FVector CoverNormalVector;
	UPROPERTY(EditDefaultsOnly, Category = "Cover", meta = (ClampMin = -50.f, ClampMax = -100.f))
	float fMinCoverDistance = -70.f;
	UPROPERTY(EditDefaultsOnly, Category = "Cover", meta = (ClampMin = -50.f, ClampMax = -100.f))
	float CoverActorGap = 5.f;
	UPROPERTY(EditDefaultsOnly, Category = "Cover")
	float coverRadiusFactor = 0.75f;
	UPROPERTY(EditDefaultsOnly, Category = "Cover", meta = (ClampMin = 1.f, ClampMax = 5.f))
	float MovingToCoverSpeedRate = 1.25f;
	UPROPERTY(EditDefaultsOnly, Category = "Cover", meta = (ClampMin = 5.f, ClampMax = 50.f))
	float CoverEntryThreshold = 10.f;
	UPROPERTY(EditDefaultsOnly, Category = "Cover", meta = (ClampMin = 0.2f, ClampMax = 2.f))
	float CoverExitThresholdFactor = 0.6f;

	/* these functions handle melee */
	UPROPERTY(EditDefaultsOnly, Category = "Melee")
	float DefaultMeleeForce = 800000.f;
	UPROPERTY(EditDefaultsOnly, Category = "Melee")
	float DefaultMeleeDamage = 5.f;

	/* these functions and variables handle changing weapons */
	UFUNCTION(BlueprintCallable, Category = "Weapon Swap")
	void StartWeaponSwap();
	UFUNCTION(BlueprintCallable, Category = "Weapon Swap")
	void FinishWeaponSwap();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Swap", meta = (ClampMin = 0.5f, ClampMax = 3.f))
	float WeaponSwapTime = 1.f;

	/** these variables and functions handle leaning behavior in cover */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Leaning", meta = (ClampMin = 0.f, ClampMax = 100.f))
	float LeanDisplacementMax = 50.f;

	/* these variables and functions handle special ability behavior */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Special")
	float SpecialAbilityCooldown = 10.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Special")
	float SpecialActivationTime = 0.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Special")
	float SpecialDuration = 0.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Special")
	class UTexture2D* SpecialIcon = nullptr;
	
	UPROPERTY(BlueprintReadWrite, Category = "Special")
	FTimerHandle SpecialCooldownTimer;
	UPROPERTY(BlueprintReadWrite, Category = "Special")
	FTimerHandle SpecialDurationTimer;
	UPROPERTY(BlueprintReadWrite, Category = "Special")
	FTimerHandle SpecialActivationTimer;

private:
	// original character values for reset after leaving sprint/crouch/aim/etc. states
	float normalHeight = 0.f;
	float normalSpeed = 0.f;
	float normalRadius = 0.f;
	FVector originalScopePosition;
	float originalFieldOfView;
	FVector previousPosition;
	FRotator previousRotation;

	// how far to turn as part of the quick turn action
	const float fQuickTurnAngle = 180.f;
	void ProcessQuickTurnOnTick(float DeltaTime);

	// internal utility to trigger a reload on the gun
	void StartReload();

	FVector aimLocation;
	FVector coverEntryLocation;

	// internal state and variables for weapon swapping
	bool IsWeaponPrimary = true;
	FTimerHandle WeaponSwapTimerHandle;

	class APickup* OverlappedPickupRef;
	bool IsOverlappingPickup = false;

	FVector InitialLeanDisplacement;
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }


	UFUNCTION(BlueprintCallable, Category = "Overlap")
	void SetOverlappingReticle(APickup* OverlappedPickup);
	UFUNCTION(BlueprintCallable, Category = "Overlap")
	void ClearOverlappingReticle();
};

