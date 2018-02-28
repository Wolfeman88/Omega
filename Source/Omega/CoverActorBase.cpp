// Fill out your copyright notice in the Description page of Project Settings.

#include "CoverActorBase.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ACoverActorBase::ACoverActorBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CoverMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoverMeshComp"));
	RootComponent = CoverMeshComp;
}

bool ACoverActorBase::GetIsCrouchHeight()
{
	return bIsCrouchHeight;
}
