// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "XD_ItemSystem.h"
#include "XD_ItemSystemUtility.h"
#if WITH_EDITOR
#include <ISettingsModule.h>
#include <ISettingsSection.h>
#endif

#define LOCTEXT_NAMESPACE "FXD_ItemSystemModule"

void FXD_ItemSystemModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

#if WITH_EDITOR
	// register settings
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	if (SettingsModule != nullptr)
	{
		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "Plugins", "XD_ItemSystemSettings",
			LOCTEXT("XD_ItemSystemSettings", "XD_ItemSystemSettings"),
			LOCTEXT("XD_ItemSystemSettingsDescription", "Configure the XD_ItemSystemSettings plug-in."),
			GetMutableDefault<UXD_ItemSystemSettings>()
		);
	}
#endif //WITH_EDITOR
}

void FXD_ItemSystemModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FXD_ItemSystemModule, XD_ItemSystem)