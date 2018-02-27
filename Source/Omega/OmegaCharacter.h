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

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UChildActorComponent* GunActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
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

private:
	float normalHeight = 0.f;
	float normalSpeed = 0.f;
	const float fQuickTurnAngle = 180.f;
	FVector originalScopePosition;
	float originalFieldOfView;
	void StartReload();

	FVector previousPosition;
	FRotator previousRotation;

	FVector aimLocation;
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};

