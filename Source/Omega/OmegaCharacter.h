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

UCLASS(config=Game)
class AOmegaCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* FP_Gun;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* FP_MuzzleLocation;

	/** Gun mesh: VR view (attached to the VR controller directly, no arm, just the actual gun) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* VR_Gun;

	/** Location on VR gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* VR_MuzzleLocation;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	/** Motion controller (right hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMotionControllerComponent* R_MotionController;

	/** Motion controller (left hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMotionControllerComponent* L_MotionController;

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

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class AOmegaProjectile> ProjectileClass;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class AOmegaProjectile> SecondaryProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* FireAnimation;

	/** Whether to use motion controller location for aiming. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	uint32 bUsingMotionControllers : 1;

protected:
	
	/** Fires the primary projectile/ability of the weapon. */
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void OnPrimaryFire();	
	
	/** Fires the secondary projectile/ability of the weapon. */
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void OnSecondaryFire();

	/** Fires the secondary projectile/ability of the weapon. */
	UFUNCTION(BlueprintCallable, Category = "Special")
	void OnSpecial();

	/** Resets HMD orientation and position in VR. */
	void OnResetVR();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
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

	struct TouchData
	{
		TouchData() { bIsPressed = false;Location=FVector::ZeroVector;}
		bool bIsPressed;
		ETouchIndex::Type FingerIndex;
		FVector Location;
		bool bMoved;
	};
	void BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location);
	TouchData	TouchItem;

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

private:
	float normalHeight = 0.f;
	float normalSpeed = 0.f;
	const float fQuickTurnAngle = 180.f;
	FVector originalScopePosition;
	float originalFieldOfView;
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	/* 
	 * Configures input for touchscreen devices if there is a valid touch interface for doing so 
	 *
	 * @param	InputComponent	The input component pointer to bind controls to
	 * @returns true if touch controls were enabled.
	 */
	bool EnableTouchscreenMovement(UInputComponent* InputComponent);

public:
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};

