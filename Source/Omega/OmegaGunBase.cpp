// Fill out your copyright notice in the Description page of Project Settings.

#include "OmegaGunBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "OmegaProjectile.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "OmegaCharacter.h"
#include "DrawDebugHelpers.h"

// Sets default values
AOmegaGunBase::AOmegaGunBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	GunSkeleton = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunSkeleton"));
	RootComponent = GunSkeleton;
}

void AOmegaGunBase::ResetIsAbleToFire()
{
	IsAbleToFire = true;

	if (OwningPlayerRef && IsTriggerHeld) PrimaryFire(OwningPlayerRef->GetAimLocation());
}

void AOmegaGunBase::AutomaticFire()
{
	if (!IsTriggerHeld)
	{
		PrimaryFireRateTimerHandle.Invalidate();
		GetWorldTimerManager().SetTimer(PrimaryFireRateTimerHandle, this, &AOmegaGunBase::ResetIsAbleToFire, SingleFireRate);
	}
	else
	{
		IsAbleToFire = true;
		PrimaryFire(OwningPlayerRef->GetAimLocation());
		GetWorldTimerManager().SetTimer(PrimaryFireRateTimerHandle, this, &AOmegaGunBase::AutomaticFire, AutoFireRate);
	}
}

void AOmegaGunBase::BurstFire()
{
	BurstRemaining--;

	if (BurstRemaining == 0 && IsBurstActive)
	{
		PrimaryFireRateTimerHandle.Invalidate();
		IsBurstActive = false;
		GetWorldTimerManager().SetTimer(PrimaryFireRateTimerHandle, this, &AOmegaGunBase::ResetIsAbleToFire, SingleFireRate);
	}
	else
	{
		IsAbleToFire = true;
		PrimaryFire(OwningPlayerRef->GetAimLocation());
		GetWorldTimerManager().SetTimer(PrimaryFireRateTimerHandle, this, &AOmegaGunBase::BurstFire, AutoFireRate);
	}
}

void AOmegaGunBase::SetOwningPlayerRef(AOmegaCharacter* OwningPlayer)
{
	OwningPlayerRef = OwningPlayer;
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
	currentClipAmmo = ((currentGunAmmo - bulletsNeeded) >= 0) ? clipAmmoMax : currentGunAmmo + currentClipAmmo;
	currentGunAmmo = FMath::Max(currentGunAmmo - bulletsNeeded, 0);
	GetWorldTimerManager().ClearTimer(ReloadTimer);
}

void AOmegaGunBase::FireProjectile(TSubclassOf<AOmegaProjectile> projectile, const FVector& AimTarget)
{
	UWorld* const World = GetWorld();
	if (World)
	{
		// TODO: once weapon gimbal in place, rework to use combined "weapon rotation" vector instead of control rotation
		FVector MuzzleLocation = GunSkeleton->GetSocketLocation("Muzzle");
		FRotator MuzzleRotation = UGameplayStatics::GetPlayerController(this, 0)->GetControlRotation();
		const FVector SpawnLocation = MuzzleLocation + MuzzleRotation.RotateVector(FVector::ForwardVector * 50.f);
		FRotator AimRotation = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, AimTarget);

		// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position

		//Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		// spawn the projectile at the muzzle
		World->SpawnActor<AOmegaProjectile>(projectile, SpawnLocation, AimRotation, ActorSpawnParams);
	}
}

void AOmegaGunBase::FireHitscan(const FVector & AimTarg)
{
	UWorld* const World = GetWorld();
	if (World)
	{
		//const FName HitscanTrace = "Trace Tag";
		//World->DebugDrawTraceTag = HitscanTrace;

		FHitResult* hit = new FHitResult();
		FCollisionQueryParams params = FCollisionQueryParams(FName(TEXT("collision query")), false, this);
		params.AddIgnoredActor(OwningPlayerRef);
		//params.TraceTag = "Trace Tag";

		FVector MuzzleLocation = GunSkeleton->GetSocketLocation("Muzzle");
		FRotator MuzzleRotation = UGameplayStatics::GetPlayerController(this, 0)->GetControlRotation();

		if (World->LineTraceSingleByObjectType(*hit, MuzzleLocation, AimTarg + HitscanRangeBuffer * MuzzleRotation.Vector(), FCollisionObjectQueryParams::AllObjects, params))
		{
			DrawDebugLine(World, MuzzleLocation, hit->Location, FColor::Blue, false, 10.f, 0, 20.f);

			if ((hit->GetActor() != NULL) && (hit->GetComponent() != NULL))
			{
				if (hit->GetComponent()->IsSimulatingPhysics())
				{
					hit->GetComponent()->AddImpulseAtLocation((AimTarg - MuzzleLocation).SafeNormal() * DefaultHitscanForce, GetActorLocation());
				}

				AOmegaCharacter* omegaActor = Cast<AOmegaCharacter>(hit->GetActor());

				if (omegaActor)
				{
					omegaActor->ReceiveDamage(DefaultHitscanDamage);
				}
			}
		}
		else
		{
			DrawDebugLine(World, MuzzleLocation, AimTarg, FColor::Red, false, 10.f, 0, 20.f);
		}
	}
}

void AOmegaGunBase::StartReload()
{
	if (currentGunAmmo == 0 || currentClipAmmo == clipAmmoMax) return;

	GetWorldTimerManager().SetTimer(ReloadTimer, this, &AOmegaGunBase::Reload, 1.2f);
}

bool AOmegaGunBase::PrimaryFire(const FVector& AimTarget)
{	
	if (ReloadTimer.IsValid() || !IsAbleToFire)
	{
		return false;
	}
	else if (currentClipAmmo == 0)
	{
		StartReload();
		return false;
	}

	IsAbleToFire = false;

	// try and fire a projectile
	if (ProjectileClass) FireProjectile(ProjectileClass, AimTarget);
	else FireHitscan(AimTarget); 

	// try and play the sound if specified
	if (PrimaryFireSound) UGameplayStatics::PlaySoundAtLocation(this, PrimaryFireSound, GetActorLocation());

	// check if reload necessary
	if (--currentClipAmmo == 0) StartReload();

	if (TriggerConfig == EFireMode::FM_Single)
	{
		GetWorldTimerManager().SetTimer(PrimaryFireRateTimerHandle, this, &AOmegaGunBase::ResetIsAbleToFire, SingleFireRate);
	}
	else if (TriggerConfig == EFireMode::FM_Burst)
	{
		BurstRemaining = (IsBurstActive) ? BurstRemaining : BurstCount;
		IsBurstActive = true;
		GetWorldTimerManager().SetTimer(PrimaryFireRateTimerHandle, this, &AOmegaGunBase::BurstFire, AutoFireRate);
	}
	else
	{
		GetWorldTimerManager().SetTimer(PrimaryFireRateTimerHandle, this, &AOmegaGunBase::AutomaticFire, AutoFireRate);
	}

	return true;
}

bool AOmegaGunBase::SecondaryFire(const FVector& AimTarget)
{
	if (currentSecondaryCharges == 0) return false;

	// try and fire a projectile
	if (SecondaryProjectileClass) FireProjectile(SecondaryProjectileClass, AimTarget);
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
