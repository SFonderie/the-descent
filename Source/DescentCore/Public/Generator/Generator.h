#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Generator/RoomData.h"
#include "Generator.generated.h"

class ARoomDoor;
class ARoomManager;
class ULevelStreamingDynamic;

/** Actor responsible for building a golden path and branches. */
UCLASS()
class DESCENTCORE_API AGenerator : public AActor
{
	GENERATED_BODY()

public:

	/** Array of all room types the generator knows to use. */
	UPROPERTY(BlueprintReadOnly, Category = "Generation", EditAnywhere)
	TArray<URoomData*> Tileset;

	/** Manager class used for all spawned tiles. */
	UPROPERTY(BlueprintReadOnly, Category = "Generation", EditAnywhere)
	TSubclassOf<ARoomManager> ManagerClass;

	/** Door type spawned into open doorways to act as a connector. */
	UPROPERTY(BlueprintReadOnly, Category = "Generation", EditAnywhere)
	TSubclassOf<ARoomDoor> DoorClass;

	/** Door type spawned into sealed doorways to hide the opening. */
	UPROPERTY(BlueprintReadOnly, Category = "Generation", EditAnywhere)
	TSubclassOf<ARoomDoor> SealClass;

	/** Length of the generated golden level path. */
	UPROPERTY(BlueprintReadOnly, Category = "Generation", EditAnywhere)
	int32 GenerateLength = 8;

	/** Generated level data managers. Only populated while HasGenerated is true. */
	UPROPERTY(BlueprintReadOnly, Category = "Generation")
	TArray<ARoomManager*> RoomGrid;

	/** Constructs the Generator. */
	AGenerator();

	/** Generates a level from the generator's position. */
	UFUNCTION(BlueprintCallable, Category = "Generation")
	void GenerateLevel();

	/** Unloads the currently-generated level. */
	UFUNCTION(BlueprintCallable, Category = "Generation")
	void ReleaseLevel();

	/** Spawns an open or sealed door at the given position with the given direction. */
	UFUNCTION(BlueprintCallable, Category = "Generation")
	ARoomDoor* SpawnDoor(const FVector& DoorPosition, const FVector& DoorDirection, bool bSpawnSealed = false);

	/** Checks to see if the level is currently generated. */
	UFUNCTION(BlueprintPure, Category = "Generation")
	bool HasGenerated() const;

private:

	/** Spawns a room manager at the given transform. */
	ARoomManager* SpawnManager(const FTransform& RoomTransform);

	/** Helper function that returns a randomly-selected room tile with the given type. */
	URoomData* GetRandomRoomTile(ERoomType RoomType, int32 MaxTries = 100) const;

	/** Holds pointers to the active level instances. */
	TArray<ULevelStreamingDynamic*> LevelStreams;

	/** Holds pointers to actors spawned during generation. */
	TArray<AActor*> ActorSpawns;
};
