// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "GameplayItemSystem_Editor.h"
#include <Modules/ModuleManager.h>
#include <PropertyEditorModule.h>
#include <Editor.h>
#include <AssetToolsModule.h>
#include <PropertyEditorDelegates.h>

#include "Abstract/GameplayItemCoreBase.h"
#include "Bluprint/GameplayItemCoreBlueprint.h"
#include "AssetTypeActions_ItemCore.h"
#include "GameplayItem_Customization.h"
#include "GameplayItemActorFactory.h"

struct FGameplayItem;

#define LOCTEXT_NAMESPACE "FGameplayItemSystemModule_Editor"

void FGameplayItemSystem_EditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	{
		PropertyModule.RegisterCustomClassLayout(TEXT("GameplayItemCoreBase"), FOnGetDetailCustomizationInstance::CreateStatic(&FGameplayItemCoreDetails::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(TEXT("GameplayItemCoreBase"), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FGameplayItemCoreCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(TEXT("GameplayItemModelData"), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FGameplayItemModelDataCustomization::MakeInstance));
	}

	GEditor->ActorFactories.Add(NewObject<UGameplayItemActorFactory>());

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	{
		AssetTypeActions_ItemCore = MakeShareable(new FAssetTypeActions_ItemCore());
		AssetTools.RegisterAssetTypeActions(AssetTypeActions_ItemCore.ToSharedRef());
	}

	UThumbnailManager::Get().RegisterCustomRenderer(UGameplayItemCoreBlueprint::StaticClass(), UItemCore_ThumbnailRenderer::StaticClass());
}

void FGameplayItemSystem_EditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout(TEXT("GameplayItemCoreBase"));
 		PropertyModule.UnregisterCustomPropertyTypeLayout(TEXT("GameplayItemCoreBase"));
		PropertyModule.UnregisterCustomPropertyTypeLayout(TEXT("GameplayItemModelData"));
	}

	if (GEditor)
	{
		GEditor->ActorFactories.RemoveAll([](const UActorFactory* ActorFactory) { return ActorFactory->IsA<UGameplayItemActorFactory>(); });
	}

	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		AssetTools.UnregisterAssetTypeActions(AssetTypeActions_ItemCore.ToSharedRef());
	}

	//UThumbnailManager::Get().UnregisterCustomRenderer(UGameplayItemCoreBlueprint::StaticClass());
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGameplayItemSystem_EditorModule, GameplayItemSystem_Editor)