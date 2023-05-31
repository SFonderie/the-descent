#include "Generator/RoomData.h"

bool URoomData::GetConnectionVectorsFor(const FTransform& RoomTransform, ERoomDoorFlags DoorType, FVector& OutPoint, FVector& OutDirection) const
{
	// Convert to door flags for ease of use.
	ERoomDoorFlags RoomDoorFlags = (ERoomDoorFlags)DoorFlags;

	// If there is no flag overlap, the door is impossible.
	if ((RoomDoorFlags & DoorType) == ERoomDoorFlags::None)
	{
		return false;
	}

	bool bAscent = false;

	// Switch to get the direction.
	switch (DoorType)
	{
	case ERoomDoorFlags::LowerDoorNorth:
		OutDirection = FVector::ForwardVector;
		break;

	case ERoomDoorFlags::LowerDoorSouth:
		OutDirection = FVector::BackwardVector;
		break;

	case ERoomDoorFlags::LowerDoorEast:
		OutDirection = FVector::RightVector;
		break;

	case ERoomDoorFlags::LowerDoorWest:
		OutDirection = FVector::LeftVector;
		break;

	case ERoomDoorFlags::UpperDoorNorth:
		OutDirection = FVector::ForwardVector;
		bAscent = true;
		break;

	case ERoomDoorFlags::UpperDoorSouth:
		OutDirection = FVector::BackwardVector;
		bAscent = true;
		break;

	case ERoomDoorFlags::UpperDoorEast:
		OutDirection = FVector::RightVector;
		bAscent = true;
		break;

	case ERoomDoorFlags::UpperDoorWest:
		OutDirection = FVector::LeftVector;
		bAscent = true;
		break;
	}

	// Transform the direction from local to global coordinates.
	OutDirection = RoomTransform.TransformVectorNoScale(OutDirection);

	// Add half the room size in the out direction to the transform point
	// to get the location of the given door (without height).
	OutPoint = RoomTransform.GetLocation() + OutDirection * RoomSize * 50;

	if (bAscent)
	{
		// Then add the full height if appropriate.
		OutPoint += FVector::UpVector * RoomHeight * 100;
	}

	return true;
}

FTransform URoomData::GetConnectionTransformFrom(const FVector EntryPoint, const FVector EntryDirection) const
{
	// Math only works if the direction vector is normal.
	FVector UnitDirection = EntryDirection.GetSafeNormal();

	// Determine the rotation from the unit direction.
	FRotator RoomRotation = UnitDirection.Rotation();

	// Determine the position by adding the direction plus the half the
	// room size in centimeters (magic number is half a meter).
	FVector RoomPosition = EntryPoint + UnitDirection * RoomSize * 50;

	// Construct and return the new room transform.
	return FTransform(RoomRotation, RoomPosition);
}
