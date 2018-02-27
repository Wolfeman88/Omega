// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OmegaGunBase.generated.h"

UCLASS()
class OMEGA_API AOmegaGunBase : public AActor
{
	GENERATED_BODY()

	virtual void Reload();
	virtual void FireProjectile(TSubclassOf<class AOmegaProjectile> projectile, const FVector& AimTarget);

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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly, Category = "Appearance")
	class USkeletalMeshComponent* GunSkeleton;

};
