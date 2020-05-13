// Fill out your copyright notice in the Description page of Project Settings.
#include "XD_ItemFactory.h"
#include <Kismet2/KismetEditorUtilities.h>
#include <KismetCompilerModule.h>
#include <Modules/ModuleManager.h>
#include <ClassViewerModule.h>
#include <Kismet2/SClassPickerDialog.h>
#include <AssetTypeCategories.h>
#include <ClassViewerFilter.h>
#include <Bluprint/ItemEntityBlueprint.h>

#include "Abstract/XD_ItemBase.h"
#include "Abstract/XD_ItemCoreBase.h"
#include "Bluprint/XD_ItemCoreBlueprint.h"
#include "Bluprint/XD_ItemCoreGenerateClass.h"
#include "Bluprint/ItemEntityBlueprint.h"

#define LOCTEXT_NAMESPACE "XD_ItemCoreFactory"

class UARPG_EditorSettings;

UXD_ItemCoreFactory::UXD_ItemCoreFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UXD_ItemCoreBlueprint::StaticClass();
}

FText UXD_ItemCoreFactory::GetDisplayName() const
{
	return LOCTEXT("创建新道具", "创建新道具");
}

FText UXD_ItemCoreFactory::GetToolTip() const
{
	return LOCTEXT("创建新道具", "创建新道具");
}

UObject* UXD_ItemCoreFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return FKismetEditorUtilities::CreateBlueprint(ItemCoreClass, InParent, InName, EBlueprintType::BPTYPE_Normal, UXD_ItemCoreBlueprint::StaticClass(), UXD_ItemCoreGenerateClass::StaticClass());
}

bool UXD_ItemCoreFactory::ConfigureProperties()
{
	ItemCoreClass = nullptr;

	FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");
	FClassViewerInitializationOptions Options;

	Options.Mode = EClassViewerMode::ClassPicker;

	class FXD_ItemCoreFilterViewer : public IClassViewerFilter
	{
	public:
		const EClassFlags DisallowedClassFlags = CLASS_Deprecated;
		const EClassFlags AllowedClassFlags = CLASS_Abstract;

		virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
		{
			return InClass->HasAnyClassFlags(AllowedClassFlags) && !InClass->HasAnyClassFlags(DisallowedClassFlags) && InClass->IsChildOf<UXD_ItemCoreBase>();
		}

		virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef<const IUnloadedBlueprintData> InUnloadedClassData, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
		{
			return InUnloadedClassData->HasAnyClassFlags(AllowedClassFlags) && !InUnloadedClassData->HasAnyClassFlags(DisallowedClassFlags) && InUnloadedClassData->IsChildOf(UXD_ItemCoreBase::StaticClass());
		}
	};

	Options.ClassFilter = MakeShareable<FXD_ItemCoreFilterViewer>(new FXD_ItemCoreFilterViewer);
	Options.NameTypeToDisplay = EClassViewerNameTypeToDisplay::Dynamic;
	Options.DisplayMode = EClassViewerDisplayMode::TreeView;

	const FText TitleText = LOCTEXT("选择创建道具类型", "选择创建道具类型");
	UClass* ChosenClass = nullptr;
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UXD_ItemCoreBase::StaticClass());

	if (bPressedOk)
	{
		ItemCoreClass = ChosenClass;
	}

	return bPressedOk;
}

UItemEntityFactory::UItemEntityFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UItemEntityBlueprint::StaticClass();
}

FText UItemEntityFactory::GetDisplayName() const
{
	return LOCTEXT("创建道具实体", "创建道具实体");
}

FText UItemEntityFactory::GetToolTip() const
{
	return LOCTEXT("创建道具实体", "创建道具实体");
}

uint32 UItemEntityFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::Gameplay;
}

UObject* UItemEntityFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return FKismetEditorUtilities::CreateBlueprint(ItemEntityClass, InParent, InName, EBlueprintType::BPTYPE_Normal, UItemEntityBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
}

bool UItemEntityFactory::ConfigureProperties()
{
	ItemEntityClass = nullptr;

	FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");
	FClassViewerInitializationOptions Options;

	Options.Mode = EClassViewerMode::ClassPicker;

	class FXD_ItemCoreFilterViewer : public IClassViewerFilter
	{
	public:
		const EClassFlags DisallowedClassFlags = CLASS_Deprecated;
		const EClassFlags AllowedClassFlags = CLASS_Abstract;

		virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
		{
			return InClass->HasAnyClassFlags(AllowedClassFlags) && !InClass->HasAnyClassFlags(DisallowedClassFlags) && InClass->IsChildOf<AXD_ItemBase>();
		}

		virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef<const IUnloadedBlueprintData> InUnloadedClassData, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
		{
			return InUnloadedClassData->HasAnyClassFlags(AllowedClassFlags) && !InUnloadedClassData->HasAnyClassFlags(DisallowedClassFlags) && InUnloadedClassData->IsChildOf(AXD_ItemBase::StaticClass());
		}
	};

	Options.ClassFilter = MakeShareable<FXD_ItemCoreFilterViewer>(new FXD_ItemCoreFilterViewer);
	Options.NameTypeToDisplay = EClassViewerNameTypeToDisplay::Dynamic;
	Options.DisplayMode = EClassViewerDisplayMode::TreeView;

	const FText TitleText = LOCTEXT("选择道具实体类型", "选择道具实体类型");
	UClass* ChosenClass = nullptr;
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, AXD_ItemBase::StaticClass());

	if (bPressedOk)
	{
		ItemEntityClass = ChosenClass;
	}

	return bPressedOk;
}

#undef LOCTEXT_NAMESPACE
