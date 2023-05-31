// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUpActors.h"

// Sets default values
APickUpActors::APickUpActors()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
	
	SetRootComponent(StaticMesh);

	
}

// Called when the game starts or when spawned
void APickUpActors::BeginPlay()
{
	Super::BeginPlay();

}


//void APickUpActors::PickUp_Implementation()
//{

//}

//void APickUpActors::Utilize_Implementation()
//{

//}

// Called every frame
void APickUpActors::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

