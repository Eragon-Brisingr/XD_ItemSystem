// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "XD_ItemSystem_Editor.h"
#include <Modules/ModuleManager.h>
#include <PropertyEditorModule.h>
#include <Editor.h>
#include <AssetToolsModule.h>
#include <PropertyEditorDelegates.h>

#include "Abstract/XD_ItemCoreBase.h"
#include "Bluprint/XD_ItemCoreBlueprint.h"
#include "AssetTypeActions_ItemCore.h"
#include "XD_Item_Customization.h"
#include "XD_ItemActorFactory.h"

struct FXD_Item;

#define LOCTEXT_NAMESPACE "FXD_ItemSystemModule_Editor"

void FXD_ItemSystem_EditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	{
		PropertyModule.RegisterCustomClassLayout(TEXT("XD_ItemCoreBase"), FOnGetDetailCustomizationInstance::CreateStatic(&FXD_ItemCoreDetails::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(TEXT("XD_ItemCoreBase"), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FXD_ItemCoreCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(TEXT("XD_ItemModelData"), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FXD_ItemModelDataCustomization::MakeInstance));
	}

	GEditor->ActorFactories.Add(NewObject<UXD_ItemActorFactory>());

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	{
		AssetTypeActions_ItemCore = MakeShareable(new FAssetTypeActions_ItemCore());
		AssetTools.RegisterAssetTypeActions(AssetTypeActions_ItemCore.ToSharedRef());
	}

	UThumbnailManager::Get().RegisterCustomRenderer(UXD_ItemCoreBlueprint::StaticClass(), UItemCore_ThumbnailRenderer::StaticClass());
}

void FXD_ItemSystem_EditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout(TEXT("XD_ItemCoreBase"));
 		PropertyModule.UnregisterCustomPropertyTypeLayout(TEXT("XD_ItemCoreBase"));
		PropertyModule.UnregisterCustomPropertyTypeLayout(TEXT("XD_ItemModelData"));
	}

	if (GEditor)
	{
		GEditor->ActorFactories.RemoveAll([](const UActorFactory* ActorFactory) { return ActorFactory->IsA<UXD_ItemActorFactory>(); });
	}

	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		AssetTools.UnregisterAssetTypeActions(AssetTypeActions_ItemCore.ToSharedRef());
	}

	//UThumbnailManager::Get().UnregisterCustomRenderer(UXD_ItemCoreBlueprint::StaticClass());
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FXD_ItemSystem_EditorModule, XD_ItemSystem_Editor)