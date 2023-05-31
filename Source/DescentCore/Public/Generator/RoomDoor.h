#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomDoor.generated.h"

/**
 * Very abstract base class for door actors. Provides lock logic.
 */
UCLASS(Abstract)
class DESCENTCORE_API ARoomDoor : public AActor
{
	GENERATED_BODY()
	
public:

	/** Whether the door is locked or unlocked. */
	UPROPERTY(BlueprintReadWrite, Category = "Door", EditAnywhere)
	bool IsLocked = false;

	/** Whether the door is open or closed. To set, call TryOpen or TryClose. */
	UPROPERTY(BlueprintReadOnly, Category = "Door", EditAnywhere)
	bool IsOpen = false;

	/** Constructs the Door. */
	ARoomDoor();

	/** Tries to open the door, optionally ignoring the lock. */
	UFUNCTION(BlueprintCallable, Category = "Door")
	void TryOpen(bool bIgnoreLock = false);

	/** Tries to close the door, optionally ignoring the lock. */
	UFUNCTION(BlueprintCallable, Category = "Door")
	void TryClose(bool bIgnoreLock = true);
};
