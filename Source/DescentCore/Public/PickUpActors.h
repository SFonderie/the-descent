// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickUpInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "PickUpActors.generated.h"

UCLASS()
class DESCENTCORE_API APickUpActors : public AActor, public IPickUpInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickUpActors();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MeshComponent")
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	int Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float PlaybackSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float CoolDown;



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//void PickUp_Implementation() override;

	//void Utilize_Implementation() override;

};
