// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "OmegaGameMode.h"
#include "OmegaHUD.h"
#include "OmegaCharacter.h"
#include "UObject/ConstructorHelpers.h"

AOmegaGameMode::AOmegaGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AOmegaHUD::StaticClass();
}
