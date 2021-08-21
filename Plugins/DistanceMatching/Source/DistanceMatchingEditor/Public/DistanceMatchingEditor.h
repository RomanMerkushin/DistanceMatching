// Copyright Roman Merkushin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FDistanceMatchingEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
