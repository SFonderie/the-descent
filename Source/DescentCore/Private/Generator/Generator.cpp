#include "Generator/Generator.h"
#include "Generator/RoomDoor.h"
#include "Generator/RoomManager.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Engine/World.h"

AGenerator::AGenerator()
{
	PrimaryActorTick.bCanEverTick = true;
}

/** Helper struct representing an incomplete room. */
struct FIntermediateRoomData
{
	ARoomManager* Manager = nullptr;
	URoomData* RoomData = nullptr;
	FTransform Transform;
	FVector GridOffset;
	int32 EmptyDoors = 0;
	int32 PathIndex = 0;
};

void AGenerator::GenerateLevel()
{
	// Only invoke while there is no level.
	if (HasGenerated())
	{
		return;
	}

	// Variables used in the generator loop.
	FTransform RoomTransform = GetActorTransform();
	URoomData* RoomSelection = GetRandomRoomTile(ERoomType::Start);
	ARoomDoor* DoorActor = nullptr;
	FVector CurrentOffset;

	// No start room? Exit.
	if (!RoomSelection)
	{
		return;
	}

	// Track the generator's progress.
	TArray<FIntermediateRoomData> ExistingRooms;
	int32 RoomsRemaining = GenerateLength;

	// Generate the golden path.
	while (RoomsRemaining > 0)
	{
		bool LoadSuccess = false;

		// Load the level using our currently-selected room and current room transform.
		ULevelStreamingDynamic* Level = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(
			this,
			RoomSelection->Level,
			RoomTransform,
			LoadSuccess
		);

		// Save the level instance.
		if (LoadSuccess)
		{
			LevelStreams.Add(Level);
		}

		// Create a Room Manager and place it at the room's position.
		ARoomManager* Manager = SpawnManager(RoomTransform);

		// Load the manager's entrance door.
		Manager->EntranceDoor = DoorActor;

		// We'll need this in a moment for bad reasons.
		int32 South = (int32)ERoomDoorFlags::LowerDoorSouth;

		// Store room data for collision and backfill.
		FIntermediateRoomData NewRoomData;
		NewRoomData.Manager = Manager;
		NewRoomData.RoomData = RoomSelection;
		NewRoomData.Transform = RoomTransform;
		NewRoomData.GridOffset = CurrentOffset; // We always enter through the "southern" door, so exclude it.
		NewRoomData.EmptyDoors = RoomSelection->DoorFlags - ((RoomSelection->DoorFlags & South) != 0 ? South : 0);
		NewRoomData.PathIndex = GenerateLength - RoomsRemaining;

		// Used to track doors.
		FVector DoorPosition;
		FVector DoorDirection;
		int32 CurrentDoor = 0;
		int32 IgnoreDoors = 0;
		bool bDidCollide = false;

		// Do-while loop that randomly picks a door and checks collisions.
		// It will continue trying new doors until it finds one that can't collide.
		// If all doors collide, then the generator will exit.
		do
		{
			// Exit if there are no un-ignored doors.
			// Ignored doors are confirmed to overlap.
			if (NewRoomData.EmptyDoors - IgnoreDoors == 0)
			{
				// CurrentDoor = 0; // No door could be found.
				break;
			}

			// Nested do-while loops; spicy isn't it?
			// This loop randomly picks a door via a bit-shift.
			// And ends when it finds an empty door slot.
			do
			{
				CurrentDoor = 1 << FMath::RandRange(0, 7);
			}
			while ((NewRoomData.EmptyDoors - IgnoreDoors & CurrentDoor) == 0);

			// Once we have a "valid" door, calculate its world position and world direction vectors for collision checks.
			RoomSelection->GetConnectionVectorsFor(RoomTransform, (ERoomDoorFlags)CurrentDoor, DoorPosition, DoorDirection);

			// Start with no collision.
			bDidCollide = false;

			// Check the previous rooms for overlapping grid positions.
			for (FIntermediateRoomData PrevRoom : ExistingRooms)
			{
				if (FVector::PointsAreSame(PrevRoom.GridOffset, CurrentOffset + DoorDirection))
				{
					IgnoreDoors += CurrentDoor; // Don't try this door again.
					bDidCollide = true; // Repeat the loop to try again.
					break; // Skip the remaining collisions.
				}
			}
		}
		while (bDidCollide);

		// Spawn an exit door and register it with the manager.
		DoorActor = SpawnDoor(DoorPosition, DoorDirection);
		Manager->ExitDoors.Emplace(DoorActor);

		// Exclude the door we exit through.
		NewRoomData.EmptyDoors -= CurrentDoor;

		// At last register the new room.
		ExistingRooms.Emplace(NewRoomData);

		// Update the room transform and the current offset to the next room position.
		RoomTransform = RoomSelection->GetConnectionTransformFrom(DoorPosition, DoorDirection);
		CurrentOffset += DoorDirection;

		// Decrement.
		--RoomsRemaining;

		// Pick a new connector room. If this is the last loop iteration, then select a boss room.
		RoomSelection = GetRandomRoomTile(RoomsRemaining > 1 ? ERoomType::Connector : ERoomType::Boss);

		// If we can't build the golden path any further 
		// (no rooms available) then just cap it off.
		if (!RoomSelection)
		{
			break;
		}
	}

	// Now do another pass to fill holes.
	for (int32 Rooms = 0; Rooms < GenerateLength; Rooms++)
	{
		// Extract the intermediate data for each room from the start.
		FIntermediateRoomData RoomInfo = ExistingRooms[Rooms];

		// Redefine in scope.
		FVector DoorPosition;
		FVector DoorDirection;
		int32 CurrentDoor = 0;
		bool bDidCollide = false;

		// Loop until all doors are filled.
		while (RoomInfo.EmptyDoors != 0)
		{
			// Randomly select a door, using a similar rule as above.
			// We don't need to worry about infinite loops here since
			// the parent loop checks for the infinite condition.
			do
			{
				CurrentDoor = 1 << FMath::RandRange(0, 7);
			}
			while ((RoomInfo.EmptyDoors & CurrentDoor) == 0);

			// Once we have a valid door, calculate its world position and world direction vectors for collision checks.
			RoomInfo.RoomData->GetConnectionVectorsFor(RoomInfo.Transform, (ERoomDoorFlags)CurrentDoor, DoorPosition, DoorDirection);

			// Start with no collision.
			bDidCollide = false;

			// Check the previous rooms for overlapping grid positions.
			for (FIntermediateRoomData PrevRoom : ExistingRooms)
			{
				if (FVector::PointsAreSame(PrevRoom.GridOffset, RoomInfo.GridOffset + DoorDirection))
				{
					bDidCollide = true; // Repeat the loop to try again.
					break; // Skip the remaining collisions.
				}
			}

			if (bDidCollide)
			{
				// We can't put a room here, so spawn a locked door actor instead.
				SpawnDoor(DoorPosition, DoorDirection, true);
			}
			else
			{
				// We can put a room here, so pick a random terminal to fill in.
				URoomData* TerminalRoom = GetRandomRoomTile(ERoomType::Terminal);

				// No terminal? Put a door there instead.
				if (!TerminalRoom)
				{
					SpawnDoor(DoorPosition, DoorDirection, true);
					RoomInfo.EmptyDoors -= CurrentDoor;
					continue;
				}

				// We will need a terminal transform as well, so we calculate one from the current room.
				FTransform TerminalTransform = RoomInfo.RoomData->GetConnectionTransformFrom(DoorPosition, DoorDirection);

				bool LoadSuccess = false;

				// Load the level using our currently-selected terminal and current room transform.
				ULevelStreamingDynamic* Level = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(
					this,
					TerminalRoom->Level,
					TerminalTransform,
					LoadSuccess
				);

				// Save the level instance.
				if (LoadSuccess)
				{
					LevelStreams.Add(Level);
				}

				// Create another exit door and register it with the previous room manager.
				ARoomDoor* ExitDoor = SpawnDoor(DoorPosition, DoorDirection);
				RoomInfo.Manager->ExitDoors.Emplace(ExitDoor);

				// Now create another manager for the terminal and track it as well.
				ARoomManager* Manager = SpawnManager(TerminalTransform);

				// We need to register the new terminal for collision.
				FIntermediateRoomData TerminalData;
				TerminalData.Manager = Manager;
				TerminalData.RoomData = TerminalRoom;
				TerminalData.Transform = TerminalTransform;
				TerminalData.GridOffset = RoomInfo.GridOffset + DoorDirection;
				TerminalData.PathIndex = RoomInfo.PathIndex;

				// Add it and we're done.
				ExistingRooms.Emplace(TerminalData);
			}

			// Mark the current door as filled.
			RoomInfo.EmptyDoors -= CurrentDoor;
		}
	}

	// Transfer the intermediates to final data.
	for (FIntermediateRoomData Intermediate : ExistingRooms)
	{
		ARoomManager* Manager = Intermediate.Manager;
		Manager->Template = Intermediate.RoomData;
		Manager->GridPosition = Intermediate.GridOffset;
		Manager->RoomTransform = Intermediate.Transform;
		Manager->PathIndex = Intermediate.PathIndex;
		RoomGrid.Emplace(Manager);
	}
}

void AGenerator::ReleaseLevel()
{
	// These are technically weak pointers, so they need to go first.
	RoomGrid.Empty();

	// Unload all of the actors. Since Managers are always emplaced
	// before Doors, Managers should not be able to reference already
	// deleted doors via this loop.
	for (AActor* Actor : ActorSpawns)
	{
		Actor->Destroy();
	}

	// Unload all of the level streams.
	for (ULevelStreamingDynamic* Level : LevelStreams)
	{
		Level->SetIsRequestingUnloadAndRemoval(true);
	}

	// Clear the arrays.
	ActorSpawns.Empty();
	LevelStreams.Empty();
}

ARoomDoor* AGenerator::SpawnDoor(const FVector& DoorPosition, const FVector& DoorDirection, bool bSpawnSealed)
{
	if (UClass* ActorClass = bSpawnSealed ? SealClass.Get() : DoorClass.Get())
	{
		ARoomDoor* Door = GetWorld()->SpawnActor<ARoomDoor>(ActorClass, DoorPosition, DoorDirection.Rotation());
		ActorSpawns.Emplace(Door);
		return Door;
	}

	return nullptr;
}

ARoomManager* AGenerator::SpawnManager(const FTransform& RoomTransform)
{
	if (UClass* SpawnClass = ManagerClass.Get())
	{
		ARoomManager* Manager = GetWorld()->SpawnActor<ARoomManager>(SpawnClass, RoomTransform);
		ActorSpawns.Emplace(Manager);
		return Manager;
	}

	return nullptr;
}

URoomData* AGenerator::GetRandomRoomTile(ERoomType RoomType, int32 MaxTries) const
{
	URoomData* RoomSelected = nullptr;

	do
	{
		// Select a random room and confirm that its type matches.
		RoomSelected = Tileset[FMath::RandRange(0, Tileset.Num() - 1)];
		--MaxTries;

		// Exit if we exceed the trial count.
		// This is only here to prevent the
		// possibility of an infinite loop.
		if (MaxTries < 0)
		{
			return nullptr;
		}

	}
	while (RoomSelected->RoomType != RoomType);

	return RoomSelected;
}

bool AGenerator::HasGenerated() const
{
	return !LevelStreams.IsEmpty();
}
