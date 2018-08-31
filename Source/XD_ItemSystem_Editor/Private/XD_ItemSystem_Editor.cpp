// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "XD_ItemSystem_Editor.h"
#include <ModuleManager.h>
#include <PropertyEditorModule.h>
#include "XD_PropertyCustomizationEx.h"
#include "XD_Item_Customization.h"

#define LOCTEXT_NAMESPACE "FXD_ItemSystemModule_Editor"

void FXD_ItemSystem_EditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

		RegisterCustomProperty(struct FXD_Item, FXD_Item_Customization);
	}
}

void FXD_ItemSystem_EditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FXD_ItemSystem_EditorModule, XD_ItemSystem_Editor)