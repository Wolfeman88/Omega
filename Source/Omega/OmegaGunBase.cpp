// Fill out your copyright notice in the Description page of Project Settings.

#include "OmegaGunBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "OmegaProjectile.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AOmegaGunBase::AOmegaGunBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	GunSkeleton = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunSkeleton"));
	RootComponent = GunSkeleton;
}

// Called when the game starts or when spawned
void AOmegaGunBase::BeginPlay()
{
	Super::BeginPlay();

	currentClipAmmo = clipAmmoMax;
	currentGunAmmo = totalAmmoMax;

	currentSecondaryCharges = clipSecondaryChargeMax;

	ReloadTimer = *(new FTimerHandle());	
}

void AOmegaGunBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AOmegaGunBase::Reload()
{
	if (currentGunAmmo == 0) return;

	int32 bulletsNeeded = clipAmmoMax - currentClipAmmo;
	currentClipAmmo = ((currentGunAmmo -= bulletsNeeded) >= 0) ? clipAmmoMax : currentGunAmmo;
	GetWorldTimerManager().ClearTimer(ReloadTimer);
}

void AOmegaGunBase::FireProjectile(TSubclassOf<AOmegaProjectile> projectile)
{
	UWorld* const World = GetWorld();
	if (World)
	{
		const FRotator SpawnRotation = UGameplayStatics::GetPlayerController(this, 0)->GetControlRotation();
		// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
		const FVector SpawnLocation = GunSkeleton->GetSocketByName("Muzzle")->GetSocketTransform(GunSkeleton).GetLocation() + SpawnRotation.RotateVector(FVector::ForwardVector * 50.f);

		//Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		// spawn the projectile at the muzzle
		World->SpawnActor<AOmegaProjectile>(projectile, SpawnLocation, SpawnRotation, ActorSpawnParams);
	}
}

void AOmegaGunBase::StartReload()
{
	GetWorldTimerManager().SetTimer(ReloadTimer, this, &AOmegaGunBase::Reload, 1.2f);
}

bool AOmegaGunBase::PrimaryFire()
{	
	if (ReloadTimer.IsValid())
	{
		return false;
	}
	else if (currentClipAmmo == 0)
	{
		StartReload();
		return false;
	}

	// try and fire a projectile
	if (ProjectileClass) FireProjectile(ProjectileClass);
	else return false;

	// try and play the sound if specified
	if (PrimaryFireSound) UGameplayStatics::PlaySoundAtLocation(this, PrimaryFireSound, GetActorLocation());

	// check if reload necessary
	if (--currentClipAmmo == 0) StartReload();

	return true;
}

bool AOmegaGunBase::SecondaryFire()
{
	if (currentSecondaryCharges == 0) return false;

	// try and fire a projectile
	if (SecondaryProjectileClass) FireProjectile(SecondaryProjectileClass);
	else return false;

	// try and play the sound if specified
	if (SecondaryFireSound) UGameplayStatics::PlaySoundAtLocation(this, SecondaryFireSound, GetActorLocation());

	UWorld* w = GetWorld();
	if (w)
	{
		FTimerHandle* newestChargeTimer = new FTimerHandle();
		SecondaryChargeTimers.Add(newestChargeTimer);

		w->GetTimerManager().SetTimer(*newestChargeTimer, this, &AOmegaGunBase::AddCharge, 3.f);
		currentSecondaryCharges--;
	}

	return true;
}

FTimerHandle AOmegaGunBase::GetOldestSecondaryChargeTimer() const
{
	if (SecondaryChargeTimers.Num() == 0) return *(new FTimerHandle());
	return *SecondaryChargeTimers[0];
}

void AOmegaGunBase::AddCharge()
{
	if (currentSecondaryCharges >= clipSecondaryChargeMax) return;

	currentSecondaryCharges++;
	SecondaryChargeTimers[0]->Invalidate();
	SecondaryChargeTimers.RemoveAt(0);
}
