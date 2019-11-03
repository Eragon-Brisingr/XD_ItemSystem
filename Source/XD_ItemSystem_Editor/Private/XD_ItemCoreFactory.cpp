// Fill out your copyright notice in the Description page of Project Settings.

#include "XD_ItemCoreFactory.h"
#include "KismetEditorUtilities.h"
#include "KismetCompilerModule.h"
#include "ModuleManager.h"
#include "ClassViewerModule.h"
#include "SClassPickerDialog.h"
#include "XD_ItemCoreBase.h"
#include "AssetTypeCategories.h"
#include "ClassViewerFilter.h"
#include "XD_ItemCoreBlueprint.h"
#include "XD_ItemCoreGenerateClass.h"

#define LOCTEXT_NAMESPACE "XD_ItemCoreFactory"

class UARPG_EditorSettings;

UXD_ItemCoreFactory::UXD_ItemCoreFactory()
{
	bCreateNew = true;

	bEditAfterNew = true;

	SupportedClass = UXD_ItemCoreBase::StaticClass();
}

FText UXD_ItemCoreFactory::GetDisplayName() const
{
	return LOCTEXT("创造新道具", "创造新道具");
}

FText UXD_ItemCoreFactory::GetToolTip() const
{
	return LOCTEXT("创造新道具", "创造新道具");
}

UObject* UXD_ItemCoreFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
// 	UClass* BlueprintClass = nullptr;
// 	UClass* BlueprintGeneratedClass = nullptr;
// 	IKismetCompilerInterface& KismetCompilerModule = FModuleManager::LoadModuleChecked<IKismetCompilerInterface>("KismetCompiler");
// 	KismetCompilerModule.GetBlueprintTypesForClass(ItemCoreClass, BlueprintClass, BlueprintGeneratedClass);

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

	const FText TitleText = LOCTEXT("选择创造道具类型", "选择创造道具类型");
	UClass* ChosenClass = nullptr;
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UXD_ItemCoreBase::StaticClass());

	if (bPressedOk)
	{
		ItemCoreClass = ChosenClass;
	}

	return bPressedOk;
}

#undef LOCTEXT_NAMESPACE
