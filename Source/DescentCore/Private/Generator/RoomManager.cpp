#include "Generator/RoomManager.h"
#include "Generator/RoomData.h"
#include "Generator/RoomDoor.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

ARoomManager::ARoomManager()
	: SpawnCenter(400, 0, 1000)
	, SpawnExtent(400, 800, 100)
{
	PrimaryActorTick.bCanEverTick = true;
}

void ARoomManager::LockRoom(bool bTryLockEntrance)
{
	for (ARoomDoor* DoorActor : ExitDoors)
	{
		DoorActor->IsLocked = true;
		DoorActor->TryClose();
	}

	if (EntranceDoor && bTryLockEntrance)
	{
		EntranceDoor->IsLocked = true;
		EntranceDoor->TryClose();
	}
}

void ARoomManager::UnlockRoom(bool bTryUnlockEntrance)
{
	for (ARoomDoor* DoorActor : ExitDoors)
	{
		DoorActor->IsLocked = false;
	}

	if (EntranceDoor && bTryUnlockEntrance)
	{
		EntranceDoor->IsLocked = false;
	}
}

void ARoomManager::BeginSpawn(const TArray<FSpawnParams>& SpawnSequence)
{
	// Clear the previous actor list if we respawn.
	if (HasActiveSpawns())
	{
		ClearSpawns();
	}

	for (const FSpawnParams& Params : SpawnSequence)
	{
		// Skip this parameter group if there are no actor types.
		if (Params.ActorTypes.IsEmpty())
		{
			continue;
		}

		for (int i = 0; i < Params.SpawnCount; i++)
		{
			// Randomly select an actor class to spawn.
			int32 Index = FMath::RandRange(0, Params.ActorTypes.Num() - 1);
			UClass* SpawnClass = Params.ActorTypes[Index].Get();

			if (SpawnClass)
			{
				// Extract some references for math use.
				const FVector& VolumeLocation = GetActorLocation();
				const FRotator& VolumeRotation = GetActorRotation();

				// Calculate a random x and y location within the room bounds.
				int32 LocationX = FMath::RandRange(SpawnCenter.X - SpawnExtent.X, SpawnCenter.X + SpawnExtent.X);
				int32 LocationY = FMath::RandRange(SpawnCenter.Y - SpawnExtent.Y, SpawnCenter.Y + SpawnExtent.Y);
				int32 LocationZ = FMath::RandRange(SpawnCenter.Z - SpawnExtent.Z, SpawnCenter.Z + SpawnExtent.Z);

				// Construct the actual spawn position vector.
				FVector SpawnPoint = FVector(LocationX, LocationY, LocationZ);
				SpawnPoint = VolumeRotation.RotateVector(SpawnPoint) + VolumeLocation;

				FActorSpawnParameters Parameters;
				Parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

				// Actually spawn the actor in.
				AActor* SpawnActor = GetWorld()->SpawnActor<AActor>(SpawnClass, SpawnPoint, VolumeRotation, Parameters);

				// Register the new actor.
				SpawnActors.Emplace(SpawnActor);

				if (Params.bRequireDestroy)
				{
					FScriptDelegate ScriptDelegate;
					ScriptDelegate.BindUFunction(this, TEXT("ReduceRequireCount")); // WARNING! HARD-CODED REFERENCE
					SpawnActor->OnDestroyed.Add(ScriptDelegate);
					++RequireCount;
				}
			}
		}
	}

	// Fire the event if no required enemies spawned.
	if (RequireCount == 0)
	{
		OnPlayerKillRequired();
	}
}

void ARoomManager::ClearSpawns()
{
	for (AActor* Actor : SpawnActors)
	{
		Actor->Destroy();
	}

	SpawnActors.Empty();
}

void ARoomManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	CheckPlayerEntrance();
}

void ARoomManager::CheckPlayerEntrance()
{
	if (ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0))
	{
		bool bIsInRoom = true;
		bool bNearDoor = false;

		FVector PlayerLocation = PlayerCharacter->GetActorLocation();
		FVector RoomLocation = RoomTransform.GetLocation();

		// Escape if the player is too far away to be in
		// the room (outside the encompassing circle).
		FVector RoomDelta = PlayerLocation - RoomLocation;
		float RoomRadius = Template->RoomSize * 75;

		if (RoomDelta.SquaredLength() > RoomRadius * RoomRadius)
		{
			return;
		}

		// Construct some intervals for a AABB check.
		FFloatInterval RoomX, RoomY, RoomZ;
		RoomX.Include(RoomLocation.X + Template->RoomSize * 50);
		RoomX.Include(RoomLocation.X - Template->RoomSize * 50);
		RoomY.Include(RoomLocation.Y + Template->RoomSize * 50);
		RoomY.Include(RoomLocation.Y - Template->RoomSize * 50);
		RoomZ.Include(RoomLocation.Z + Template->RoomHeight * 100);
		RoomZ.Include(0);

		// Make the AABB check to determine if the player is in the room.
		bIsInRoom = RoomX.Contains(PlayerLocation.X) && RoomY.Contains(PlayerLocation.Y) && RoomZ.Contains(PlayerLocation.Z);

		// If we have an entrance door, we should also check if the player is near it.
		if (EntranceDoor)
		{
			FVector DoorDelta = PlayerLocation - EntranceDoor->GetActorLocation();
			float DoorRadius = Template->RoomSize * 12.5f;

			bNearDoor = DoorDelta.SquaredLength() < DoorRadius * DoorRadius;
		}

		// If the player is inside of the room, not near
		// the entrance door, and has not already been
		// registered as inside the room, then invoke
		// the entrance event function.
		if (bIsInRoom && !bNearDoor && !bPlayerInside)
		{
			OnPlayerEnterRoom();
		}

		// Update the tracker so that we can retrigger.
		bPlayerInside = bIsInRoom && !bNearDoor;
	}
}

void ARoomManager::ReduceRequireCount()
{
	--RequireCount;

	if (RequireCount == 0) // Invoke only once.
	{
		OnPlayerKillRequired();
	}
}
