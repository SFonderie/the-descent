#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RoomData.generated.h"

/** Defines a room's function. */
UENUM(BlueprintType)
enum class ERoomType : uint8
{
	/** Indicates that the room is a start block. */
	Start,

	/** Indicates that the room is a connector (two or more doors). */
	Connector,

	/** Indicates that the room is a single-door terminal block. */
	Terminal,

	/** Indicates that the room is an objective (end) block. */
	Boss,
};

/** Defines a room's door configuration. */
UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class ERoomDoorFlags : uint8
{
	/** Room has no door flags. */
	None = 0 UMETA(Hidden),

	/** Indicates that the room has a door on its lower-level's north side. */
	LowerDoorNorth = 1 << 0 UMETA(DisplayName = "Lower Northern Door"),

	/** Indicates that the room has a door on its lower-level's south side. */
	LowerDoorSouth = 1 << 1 UMETA(DisplayName = "Lower Southern Door"),

	/** Indicates that the room has a door on its lower-level's east side. */
	LowerDoorEast = 1 << 2 UMETA(DisplayName = "Lower Eastern Door"),

	/** Indicates that the room has a door on its lower-level's west side. */
	LowerDoorWest = 1 << 3 UMETA(DisplayName = "Lower Western Door"),

	/** Indicates that the room has a door on its upper-level's north side. */
	UpperDoorNorth = 1 << 4 UMETA(DisplayName = "Upper Northern Door"),

	/** Indicates that the room has a door on its upper-level's south side. */
	UpperDoorSouth = 1 << 5 UMETA(DisplayName = "Upper Southern Door"),

	/** Indicates that the room has a door on its upper-level's east side. */
	UpperDoorEast = 1 << 6 UMETA(DisplayName = "Upper Eastern Door"),

	/** Indicates that the room has a door on its upper-level's west side. */
	UpperDoorWest = 1 << 7 UMETA(DisplayName = "Upper Western Door"),
};

ENUM_CLASS_FLAGS(ERoomDoorFlags);

/** Provides key information about a room in an easily-accessible container. */
UCLASS(BlueprintType)
class DESCENTCORE_API URoomData : public UDataAsset
{
	GENERATED_BODY()

public:

	/** Reference to the level actually containing the room. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSoftObjectPtr<UWorld> Level;

	/** Define the room's function in the tileset. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	ERoomType RoomType = ERoomType::Connector;

	/** Defines the room's doors in its default rotation. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Bitmask, BitmaskEnum = ERoomDoorFlags))
	int32 DoorFlags = 0;

	/** Room width and depth measured in meters. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int32 RoomSize = 20;

	/** Room height measured in meters. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int32 RoomHeight = 12;

	/**
	 * Calculates the world position and direction of a door for the given room transform.
	 *
	 * @param RoomTransform Transform of the room from which to build a door.
	 * @param DoorType Door to build, if available.
	 * @param OutPoint Returned global door position.
	 * @param OutDirection Returned global door direction.
	 * @return Whether the door type is valid for the given room.
	 */
	UFUNCTION(BlueprintPure, Category = "Room Generation")
	bool GetConnectionVectorsFor(const FTransform& RoomTransform, ERoomDoorFlags DoorType, FVector& OutPoint, FVector& OutDirection) const;

	/** Returns the room transform needed to connect with the given entrance point and direction. */
	UFUNCTION(BlueprintPure, Category = "Room Generation")
	FTransform GetConnectionTransformFrom(const FVector EntryPoint, const FVector EntryDirection) const;
};
