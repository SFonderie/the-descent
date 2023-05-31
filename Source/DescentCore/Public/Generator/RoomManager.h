#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomManager.generated.h"

// Forward declarations.
class URoomData;
class ARoomDoor;

/** Information about an individual spawn grouping. */
USTRUCT(BlueprintType)
struct DESCENTCORE_API FSpawnParams
{
	GENERATED_BODY()

public:

	/** Randomly selects from these actors when spawning this group. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<TSubclassOf<AActor>> ActorTypes;

	/** Spawn this number of actors for this group. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 SpawnCount = 0;

	/** Mark true if these actors must be destroyed to "beat" the spawn. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bRequireDestroy = false;
};

/** Represents and manages a generated room instance. */
UCLASS()
class DESCENTCORE_API ARoomManager : public AActor
{
	GENERATED_BODY()

public:

	/** Center of the spawn volumes relative to the manager. */
	UPROPERTY(BlueprintReadOnly, Category = "Room Manager", EditAnywhere)
	FVector SpawnCenter;

	/** Half radius extent of the spawn volumes relative to the manager. */
	UPROPERTY(BlueprintReadOnly, Category = "Room Manager", EditAnywhere)
	FVector SpawnExtent;

	/** Room data asset used to construct this room. */
	UPROPERTY(BlueprintReadOnly, Category = "Room Data")
	URoomData* Template = nullptr;

	/** Tracks the room's index in the golden path. Terminals use their associated Connector. */
	UPROPERTY(BlueprintReadOnly, Category = "Room Data")
	int32 PathIndex = 0;

	/** Unit grid position for this room. */
	UPROPERTY(BlueprintReadOnly, Category = "Room Data")
	FVector GridPosition;

	/** Transform of the streamed room instance. */
	UPROPERTY(BlueprintReadOnly)
	FTransform RoomTransform;

	/** Tracks the room's entrance door. */
	UPROPERTY(BlueprintReadOnly, Category = "Room Data")
	ARoomDoor* EntranceDoor = nullptr;

	/** Tracks the room's exit doorways. */
	UPROPERTY(BlueprintReadOnly, Category = "Room Data")
	TArray<ARoomDoor*> ExitDoors;

	/** Constructs the Manager. */
	ARoomManager();

	/** Invoked whenever the player enters the room managed by this manager. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Room Manager|Events")
	void OnPlayerEnterRoom();

	/** Invoked when the player kills all required enemies. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Room Manager|Events")
	void OnPlayerKillRequired();

	/** Locks all exit doors and closes them. */
	UFUNCTION(BlueprintCallable, Category = "Room Manager|Events")
	virtual void LockRoom(bool bTryLockEntrance = true);

	/** Unlocks all exit doors. */
	UFUNCTION(BlueprintCallable, Category = "Room Manager|Events")
	virtual void UnlockRoom(bool bTryUnlockEntrance = false);

	/** Spawns the given parameter sequence. */
	UFUNCTION(BlueprintCallable, Category = "Room Manager|Spawns")
	virtual void BeginSpawn(const TArray<FSpawnParams>& SpawnSequence);

	/** Clears spawned actors and resets the spawn gate. */
	UFUNCTION(BlueprintCallable, Category = "Room Manager|Spawns")
	virtual void ClearSpawns();

	UFUNCTION(BlueprintPure, Category = "Room Manager|State")
	bool IsPlayerInside()
	{
		return bPlayerInside;
	}

	UFUNCTION(BlueprintPure, Category = "Room Manager|State")
	bool HasActiveSpawns()
	{
		return !SpawnActors.IsEmpty();
	}

	UFUNCTION(BlueprintPure, Category = "Room Manager|State")
	bool HasRequiredSpawns()
	{
		return RequireCount > 0;
	}

	/**
	 * Updates the Manager once per frame.
	 *
	 * @param DeltaSeconds Seconds since last update.
	 */
	virtual void Tick(float DeltaSeconds) override;

private:

	/** Checks the player location for entrances. */
	void CheckPlayerEntrance();

	/** Delegate callback used to reduce the tracked require count. */
	UFUNCTION()
	void ReduceRequireCount();

	/** Tracks whether the player is in the room. */
	bool bPlayerInside = false;

	/** Tracks the currently spawned actors. */
	TArray<AActor*> SpawnActors;

	/** Tracks the remaining required actors. */
	int32 RequireCount = 0;
};
