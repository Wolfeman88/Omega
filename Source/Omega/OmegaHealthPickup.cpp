// Fill out your copyright notice in the Description page of Project Settings.

#include "OmegaHealthPickup.h"
#include "OmegaCharacter.h"

void AOmegaHealthPickup::Pickup(AOmegaCharacter* ActingPlayer)
{
	ActingPlayer->RegainHealth(HealthValue);
	Super::Pickup(ActingPlayer);
}
