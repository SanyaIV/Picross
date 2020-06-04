// Copyright Epic Games, Inc. All Rights Reserved.

#include "Array3D.h"

#define LOCTEXT_NAMESPACE "FArray3DModule"

void FArray3DModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FArray3DModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FArray3DModule, Array3D)