// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "GameplayItemSystem.h"
#include "GameplayItemSystemUtility.h"
#if WITH_EDITOR
#include <ISettingsModule.h>
#include <ISettingsSection.h>
#endif

#define LOCTEXT_NAMESPACE "FGameplayItemSystemModule"

void FGameplayItemSystemModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

#if WITH_EDITOR
	// register settings
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	if (SettingsModule != nullptr)
	{
		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "Plugins", "GameplayItemSystemSettings",
			LOCTEXT("GameplayItemSystemSettings", "GameplayItemSystemSettings"),
			LOCTEXT("GameplayItemSystemSettingsDescription", "Configure the GameplayItemSystemSettings plug-in."),
			GetMutableDefault<UGameplayItemSystemSettings>()
		);
	}
#endif //WITH_EDITOR
}

void FGameplayItemSystemModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGameplayItemSystemModule, GameplayItemSystem)