#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

/** Game module class representing DescentCore. */
class FDescentCoreModule : public IModuleInterface
{

public:

	/** Marks DescentCore as a gameplay module. Always returns true. */
	virtual bool IsGameModule() const override
	{
		return true;
	}
};
