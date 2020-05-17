// Fill out your copyright notice in the Description page of Project Settings.
#include "GameplayItemFactory.h"
#include <Kismet2/KismetEditorUtilities.h>
#include <KismetCompilerModule.h>
#include <Modules/ModuleManager.h>
#include <ClassViewerModule.h>
#include <Kismet2/SClassPickerDialog.h>
#include <AssetTypeCategories.h>
#include <ClassViewerFilter.h>
#include <Bluprint/GameplayItemEntityBlueprint.h>

#include "Abstract/GameplayItemBase.h"
#include "Abstract/GameplayItemCoreBase.h"
#include "Bluprint/GameplayItemCoreBlueprint.h"
#include "Bluprint/GameplayItemEntityBlueprint.h"

#define LOCTEXT_NAMESPACE "GameplayItemCoreFactory"

class UARPG_EditorSettings;

UGameplayItemCoreFactory::UGameplayItemCoreFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UGameplayItemCoreBlueprint::StaticClass();
}

FText UGameplayItemCoreFactory::GetDisplayName() const
{
	return LOCTEXT("创建新道具", "创建新道具");
}

FText UGameplayItemCoreFactory::GetToolTip() const
{
	return LOCTEXT("创建新道具", "创建新道具");
}

UObject* UGameplayItemCoreFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return FKismetEditorUtilities::CreateBlueprint(ItemCoreClass, InParent, InName, EBlueprintType::BPTYPE_Normal, UGameplayItemCoreBlueprint::StaticClass(), UGameplayItemCoreGenerateClass::StaticClass());
}

bool UGameplayItemCoreFactory::ConfigureProperties()
{
	ItemCoreClass = nullptr;

	FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");
	FClassViewerInitializationOptions Options;

	Options.Mode = EClassViewerMode::ClassPicker;

	class FGameplayItemCoreFilterViewer : public IClassViewerFilter
	{
	public:
		const EClassFlags DisallowedClassFlags = CLASS_Deprecated;
		const EClassFlags AllowedClassFlags = CLASS_Abstract;

		virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
		{
			return InClass->HasAnyClassFlags(AllowedClassFlags) && !InClass->HasAnyClassFlags(DisallowedClassFlags) && InClass->IsChildOf<UGameplayItemCoreBase>();
		}

		virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef<const IUnloadedBlueprintData> InUnloadedClassData, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
		{
			return InUnloadedClassData->HasAnyClassFlags(AllowedClassFlags) && !InUnloadedClassData->HasAnyClassFlags(DisallowedClassFlags) && InUnloadedClassData->IsChildOf(UGameplayItemCoreBase::StaticClass());
		}
	};

	Options.ClassFilter = MakeShareable<FGameplayItemCoreFilterViewer>(new FGameplayItemCoreFilterViewer);
	Options.NameTypeToDisplay = EClassViewerNameTypeToDisplay::Dynamic;
	Options.DisplayMode = EClassViewerDisplayMode::TreeView;

	const FText TitleText = LOCTEXT("选择创建道具类型", "选择创建道具类型");
	UClass* ChosenClass = nullptr;
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UGameplayItemCoreBase::StaticClass());

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
	SupportedClass = UGameplayItemEntityBlueprint::StaticClass();
}

bool UItemEntityFactory::CanCreateNew() const
{
	return ItemCoreClass ? true : false;
}

FText UItemEntityFactory::GetDisplayName() const
{
	return LOCTEXT("创建道具实体", "创建道具实体");
}

FText UItemEntityFactory::GetToolTip() const
{
	return LOCTEXT("创建道具实体", "创建道具实体");
}

UObject* UItemEntityFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	check(ItemCoreClass && ItemCoreClass.GetDefaultObject()->GetBelongToEntityType());
	UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(ItemCoreClass.GetDefaultObject()->GetBelongToEntityType(), InParent, InName, EBlueprintType::BPTYPE_Normal, UGameplayItemEntityBlueprint::StaticClass(), UGameplayItemEntityGenerateClass::StaticClass());
	AGameplayItemBase* ItemEntity = CastChecked<AGameplayItemBase>(Blueprint->GeneratedClass.GetDefaultObject());
	ItemEntity->BelongToCoreType = ItemCoreClass;
	return Blueprint;
}

#undef LOCTEXT_NAMESPACE
