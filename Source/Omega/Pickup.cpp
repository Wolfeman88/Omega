// Fill out your copyright notice in the Description page of Project Settings.

#include "Pickup.h"
#include "Components/StaticMeshComponent.h"
#include "OmegaCharacter.h"

// Sets default values
APickup::APickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	BaseComp = CreateDefaultSubobject<USceneComponent>(TEXT("Base"));
	RootComponent = BaseComp;

	PickupComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pickup"));
	PickupComp->SetupAttachment(RootComponent);
	PickupComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

	PickupComp->OnComponentBeginOverlap.AddDynamic(this, &APickup::OnOverlapStart);
	PickupComp->OnComponentEndOverlap.AddDynamic(this, &APickup::OnOverlapEnd);
}

// Called when the game starts or when spawned
void APickup::BeginPlay()
{
	Super::BeginPlay();
	
}

void APickup::Pickup(class AOmegaCharacter* ActingPlayer)
{
	this->Destroy();
}

void APickup::OnOverlapStart(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	AOmegaCharacter* OverlappedActor = Cast<AOmegaCharacter>(OtherActor);

	if (OverlappedActor) OverlappedActor->SetOverlappingReticle(this);
}

void APickup::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AOmegaCharacter* OverlappedActor = Cast<AOmegaCharacter>(OtherActor);

	if (OverlappedActor) OverlappedActor->ClearOverlappingReticle();
}

