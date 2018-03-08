// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OmegaGunBase.generated.h"

UENUM(BlueprintType)
enum class EFireMode : uint8
{
	FM_Single	UMETA(DisplayName = "Semi-automatic"),
	FM_Burst	UMETA(DisplayName = "Burst-fire"),
	FM_Auto		UMETA(DisplayName = "Automatic")
};

UCLASS()
class OMEGA_API AOmegaGunBase : public AActor
{
	GENERATED_BODY()

	virtual void Reload();
	virtual void FireProjectile(TSubclassOf<class AOmegaProjectile> projectile, const FVector& AimTarget);
	virtual void FireHitscan(const FVector& AimTarg);

	TArray<FTimerHandle*> SecondaryChargeTimers;
	
public:	
	// Sets default values for this actor's properties
	AOmegaGunBase();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Gun")
	void StartReload();
	UFUNCTION(BlueprintCallable, Category = "Gun")
	virtual bool PrimaryFire(const FVector& AimTarget);
	UFUNCTION(BlueprintCallable, Category = "Gun")
	virtual bool SecondaryFire(const FVector& AimTarget);
	UFUNCTION(BlueprintCallable, Category = "Gun")
	virtual void AddCharge();

	/* these functions and variables handle ammo and reloading */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = "0.0", ClampMax = "3000.0"))
	int32 totalAmmoMax = 100;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = "0.0", ClampMax = "3000.0"))
	int32 clipAmmoMax = 25;
	UPROPERTY(BlueprintReadWrite, Category = "Ammo")
	int32 currentClipAmmo;
	UPROPERTY(BlueprintReadWrite, Category = "Ammo")
	int32 currentGunAmmo;
	UPROPERTY(BlueprintReadOnly, Category = "Ammo")
	FTimerHandle ReloadTimer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	int32 clipSecondaryChargeMax = 2;
	UPROPERTY(BlueprintReadWrite, Category = "Ammo")
	int32 currentSecondaryCharges;

	UFUNCTION(BlueprintCallable, Category = "Gun")
	FTimerHandle GetOldestSecondaryChargeTimer() const;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class AOmegaProjectile> ProjectileClass;
	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class AOmegaProjectile> SecondaryProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class USoundBase* PrimaryFireSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class USoundBase* SecondaryFireSound;

	/* these variables and functions handle the trigger configuration settings for the weapon */
	UPROPERTY(BlueprintReadWrite, Category = "Weapon Configuration")
	EFireMode TriggerConfig = EFireMode::FM_Single;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Configuration")
	float SingleFireRate = 0.5f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Configuration")
	float AutoFireRate = 0.1f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon Configuration")
	int32 BurstCount = 2;
	int32 BurstRemaining = 0;
	bool IsAbleToFire = true;
	bool IsTriggerHeld = false;
	bool IsBurstActive = false;

	UFUNCTION(BlueprintCallable, Category = "Fire Rate")
	void ResetIsAbleToFire();
	UFUNCTION(BlueprintCallable, Category = "Fire Rate")
	void AutomaticFire();
	UFUNCTION(BlueprintCallable, Category = "Fire Rate")
	void BurstFire();

	UFUNCTION(BlueprintCallable, Category = "Gun")
	void SetOwningPlayerRef(class AOmegaCharacter* OwningPlayer);

	UFUNCTION(BlueprintCallable, Category = "Gun")
	FTimerHandle GetPrimaryFireTimerHandle();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly, Category = "Appearance")
	class USkeletalMeshComponent* GunSkeleton;

	UPROPERTY(EditDefaultsOnly, Category = "Gun")
	float DefaultHitscanDamage = 10.f;
	UPROPERTY(EditDefaultsOnly, Category = "Gun")
	float DefaultHitscanForce = 100000.f;
	UPROPERTY(EditDefaultsOnly, Category = "Gun")
	float HitscanRangeBuffer = 50.f;

private:
	AOmegaCharacter* OwningPlayerRef = nullptr;
	FTimerHandle PrimaryFireRateTimerHandle;
};
