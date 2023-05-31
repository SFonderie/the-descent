#include "Generator/RoomDoor.h"

ARoomDoor::ARoomDoor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ARoomDoor::TryOpen(bool bIgnoreLock)
{
	// Open if the door is unlocked
	// Open if we ignore the lock.
	IsOpen = !IsLocked || bIgnoreLock;
}

void ARoomDoor::TryClose(bool bIgnoreLock)
{
	// Close if the door is locked.
	// Close if do not ignore the lock.
	IsOpen = IsLocked && !bIgnoreLock;
}
