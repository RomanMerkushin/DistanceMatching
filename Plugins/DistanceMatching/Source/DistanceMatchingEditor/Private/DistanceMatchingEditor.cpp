// Copyright Roman Merkushin. All Rights Reserved.

#include "DistanceMatchingEditor.h"

#define LOCTEXT_NAMESPACE "FDistanceMatchingEditorModule"

void FDistanceMatchingEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FDistanceMatchingEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDistanceMatchingEditorModule, DistanceMatchingEditor)
