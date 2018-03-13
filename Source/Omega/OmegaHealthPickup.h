// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "OmegaHealthPickup.generated.h"

/**
 * 
 */
UCLASS()
class OMEGA_API AOmegaHealthPickup : public APickup
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	float HealthValue = 45.f;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Pickup")
	virtual void Pickup(class AOmegaCharacter* ActingPlayer) override;
	
};
