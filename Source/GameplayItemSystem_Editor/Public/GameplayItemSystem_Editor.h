// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include <Modules/ModuleManager.h>

class FAssetTypeActions_ItemCore;

class FGameplayItemSystem_EditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	void StartupModule() override;
	void ShutdownModule() override;

private:
	TSharedPtr<FAssetTypeActions_ItemCore> AssetTypeActions_ItemCore;
};
