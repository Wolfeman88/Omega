// Fill out your copyright notice in the Description page of Project Settings.

#include "OmegaAmmoPickup.h"
#include "OmegaCharacter.h"

void AOmegaAmmoPickup::Pickup(AOmegaCharacter * ActingPlayer)
{
	ActingPlayer->RegainAmmo(AmmoValue);
	Super::Pickup(ActingPlayer);
}
