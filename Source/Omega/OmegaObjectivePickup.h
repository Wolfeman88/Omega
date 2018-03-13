// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "OmegaObjectivePickup.generated.h"

/**
 * 
 */
UCLASS()
class OMEGA_API AOmegaObjectivePickup : public APickup
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	int32 AmmoValue = 50.f;

public:
	UFUNCTION(BlueprintCallable, Category = "Pickup")
	virtual void Pickup(class AOmegaCharacter* ActingPlayer) override;	
	
};
