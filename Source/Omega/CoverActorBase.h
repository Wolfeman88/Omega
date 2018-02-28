// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoverActorBase.generated.h"

UCLASS()
class OMEGA_API ACoverActorBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACoverActorBase();

	UFUNCTION(BlueprintCallable, Category = "Cover")
	bool GetIsCrouchHeight();
	
protected:
	UPROPERTY(VisibleAnywhere, Category = "Cover")
	class UStaticMeshComponent* CoverMeshComp;

	UPROPERTY(EditDefaultsOnly, Category = "Cover")
	bool bIsCrouchHeight = false;

};
