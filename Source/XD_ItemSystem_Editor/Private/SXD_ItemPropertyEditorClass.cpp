// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SXD_ItemPropertyEditorClass.h"
#include "Engine/Blueprint.h"
#include "Misc/FeedbackContext.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Layout/SBox.h"

#include "DragAndDrop/ClassDragDropOp.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "ClassViewerModule.h"
#include "ClassViewerFilter.h"


#define LOCTEXT_NAMESPACE "PropertyEditor"

class FXD_ItemPropertyEditorClassFilter : public IClassViewerFilter
{
public:
	/** The meta class for the property that classes must be a child-of. */
	const UClass* ClassPropertyMetaClass;

	/** The interface that must be implemented. */
	const UClass* InterfaceThatMustBeImplemented;

	/** Whether or not abstract classes are allowed. */
	bool bAllowAbstract;

	bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs ) override
	{
		bool bMatchesFlags = !InClass->HasAnyClassFlags(CLASS_Hidden|CLASS_HideDropDown|CLASS_Deprecated) &&
			(bAllowAbstract || !InClass->HasAnyClassFlags(CLASS_Abstract));

		if(bMatchesFlags && InClass->IsChildOf(ClassPropertyMetaClass)
			&& (!InterfaceThatMustBeImplemented || InClass->ImplementsInterface(InterfaceThatMustBeImplemented)))
		{
			return true;
		}

		return false;
	}

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef< const IUnloadedBlueprintData > InClass, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
	{
		bool bMatchesFlags = !InClass->HasAnyClassFlags(CLASS_Hidden|CLASS_HideDropDown|CLASS_Deprecated) &&
			(bAllowAbstract || !InClass->HasAnyClassFlags(CLASS_Abstract));

		if(bMatchesFlags && InClass->IsChildOf(ClassPropertyMetaClass)
			&& (!InterfaceThatMustBeImplemented || InClass->ImplementsInterface(InterfaceThatMustBeImplemented)))
		{
			return true;
		}

		return false;
	}
};

void SXD_ItemPropertyEditorClass::GetDesiredWidth(float& OutMinDesiredWidth, float& OutMaxDesiredWidth)
{
	OutMinDesiredWidth = 125.0f;
	OutMaxDesiredWidth = 400.0f;
}

void SXD_ItemPropertyEditorClass::Construct(const FArguments& InArgs, const TSharedPtr< class FPropertyEditor >& InPropertyEditor)
{
	check(InArgs._MetaClass);
	check(InArgs._SelectedClass.IsSet());
	check(InArgs._OnSetClass.IsBound());

	MetaClass = InArgs._MetaClass;
	RequiredInterface = InArgs._RequiredInterface;
	bAllowAbstract = InArgs._AllowAbstract;
	bIsBlueprintBaseOnly = InArgs._IsBlueprintBaseOnly;
	bAllowNone = InArgs._AllowNone;
	bAllowOnlyPlaceable = false;
	bShowViewOptions = InArgs._ShowViewOptions;
	bShowTree = InArgs._ShowTree;
	bShowDisplayNames = InArgs._ShowDisplayNames;

	SelectedClass = InArgs._SelectedClass;
	OnSetClass = InArgs._OnSetClass;

	SAssignNew(ComboButton, SComboButton)
		.OnGetMenuContent(this, &SXD_ItemPropertyEditorClass::GenerateClassPicker)
		.ContentPadding(FMargin(2.0f, 2.0f))
		.ToolTipText(this, &SXD_ItemPropertyEditorClass::GetDisplayValueAsString)
		.ButtonContent()
		[
			SNew(STextBlock)
			.Text(this, &SXD_ItemPropertyEditorClass::GetDisplayValueAsString)
			.Font(InArgs._Font)
		];

	ChildSlot
	[
		ComboButton.ToSharedRef()
	];
}

/** Util to give better names for BP generated classes */
static FString GetClassDisplayName(const UObject* Object, bool bShowDisplayNames)
{
	const UClass* Class = Cast<UClass>(Object);
	if (Class != NULL)
	{
		UBlueprint* BP = UBlueprint::GetBlueprintFromClass(Class);
		if(BP != NULL)
		{
			return BP->GetName();
		}
		if (bShowDisplayNames && Class->HasMetaData(TEXT("DisplayName")))
		{
			return Class->GetMetaData(TEXT("DisplayName"));
		}
	}
	return (Object) ? Object->GetName() : "None";
}

FText SXD_ItemPropertyEditorClass::GetDisplayValueAsString() const
{
	static bool bIsReentrant = false;

	// Guard against re-entrancy which can happen if the delegate executed below (SelectedClass.Get()) forces a slow task dialog to open, thus causing this to lose context and regain focus later starting the loop over again
	if( !bIsReentrant )
	{
		return FText::FromString(GetClassDisplayName(SelectedClass.Get(), bShowDisplayNames));
	}
	else
	{
		return FText::GetEmpty();
	}

}

TSharedRef<SWidget> SXD_ItemPropertyEditorClass::GenerateClassPicker()
{
	FClassViewerInitializationOptions Options;
	Options.bShowUnloadedBlueprints = true;
	Options.bShowNoneOption = bAllowNone;

	TSharedPtr<FXD_ItemPropertyEditorClassFilter> ClassFilter = MakeShareable(new FXD_ItemPropertyEditorClassFilter);
	Options.ClassFilter = ClassFilter;
	ClassFilter->ClassPropertyMetaClass = MetaClass;
	ClassFilter->InterfaceThatMustBeImplemented = RequiredInterface;
	ClassFilter->bAllowAbstract = bAllowAbstract;
	Options.bIsBlueprintBaseOnly = bIsBlueprintBaseOnly;
	Options.bIsPlaceableOnly = bAllowOnlyPlaceable;
	Options.NameTypeToDisplay = (bShowDisplayNames ? EClassViewerNameTypeToDisplay::DisplayName : EClassViewerNameTypeToDisplay::ClassName);
	Options.DisplayMode = bShowTree ? EClassViewerDisplayMode::TreeView : EClassViewerDisplayMode::ListView;
	Options.bAllowViewOptions = bShowViewOptions;

	FOnClassPicked OnPicked(FOnClassPicked::CreateRaw(this, &SXD_ItemPropertyEditorClass::OnClassPicked));

	return SNew(SBox)
		.WidthOverride(280)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			.MaxHeight(500)
			[
				FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer").CreateClassViewer(Options, OnPicked)
			]
		];
}

void SXD_ItemPropertyEditorClass::OnClassPicked(UClass* InClass)
{
	if(!InClass)
	{
		SendToObjects(TEXT("None"));
	}
	else
	{
		SendToObjects(InClass->GetPathName());
	}

	ComboButton->SetIsOpen(false);
}

void SXD_ItemPropertyEditorClass::SendToObjects(const FString& NewValue)
{
	if (!NewValue.IsEmpty() && NewValue != TEXT("None"))
	{
		UClass* NewClass = FindObject<UClass>(ANY_PACKAGE, *NewValue);
		if(!NewClass)
		{
			NewClass = LoadObject<UClass>(nullptr, *NewValue);
		}
		OnSetClass.Execute(NewClass);
	}
	else
	{
		OnSetClass.Execute(nullptr);
	}
}

void SXD_ItemPropertyEditorClass::OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	TSharedPtr<FAssetDragDropOp> UnloadedClassOp = DragDropEvent.GetOperationAs<FAssetDragDropOp>();
	if (UnloadedClassOp.IsValid())
	{
		bool bAllAssetWereLoaded = true;

		FString AssetPath;
		FString PathName;

		// Find the class/blueprint path
		if (UnloadedClassOp->HasAssets())
		{
			AssetPath = UnloadedClassOp->GetAssets()[0].ObjectPath.ToString();
		}
		else if (UnloadedClassOp->HasAssetPaths())
		{
			AssetPath = UnloadedClassOp->GetAssetPaths()[0];
		}

		// Check to see if the asset can be found, otherwise load it.
		UObject* Object = FindObject<UObject>(nullptr, *AssetPath);
		if (Object == nullptr)
		{
			// Load the package.
			GWarn->BeginSlowTask(LOCTEXT("OnDrop_LoadPackage", "Fully Loading Package For Drop"), true, false);

			Object = LoadObject<UObject>(nullptr, *AssetPath);

			GWarn->EndSlowTask();
		}

		if (UClass* Class = Cast<UClass>(Object))
		{
			// This was pointing to a class directly
			UnloadedClassOp->SetToolTip(FText::GetEmpty(), FEditorStyle::GetBrush(TEXT("Graph.ConnectorFeedback.OK")));
		}
		else if (UBlueprint* Blueprint = Cast<UBlueprint>(Object))
		{
			if (Blueprint->GeneratedClass)
			{
				// This was pointing to a blueprint, get generated class
				UnloadedClassOp->SetToolTip(FText::GetEmpty(), FEditorStyle::GetBrush(TEXT("Graph.ConnectorFeedback.OK")));
			}
		}
		else
		{
			UnloadedClassOp->SetToolTip(FText::GetEmpty(), FEditorStyle::GetBrush(TEXT("Graph.ConnectorFeedback.Error")));
		}
	}
}

void SXD_ItemPropertyEditorClass::OnDragLeave(const FDragDropEvent& DragDropEvent)
{
	TSharedPtr<FAssetDragDropOp> UnloadedClassOp = DragDropEvent.GetOperationAs<FAssetDragDropOp>();
	if (UnloadedClassOp.IsValid())
	{
		UnloadedClassOp->ResetToDefaultToolTip();
	}
}

FReply SXD_ItemPropertyEditorClass::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	TSharedPtr<FClassDragDropOp> ClassOperation = DragDropEvent.GetOperationAs<FClassDragDropOp>();
	if (ClassOperation.IsValid())
	{
		// We can only drop one item into the combo box, so drop the first one.
		FString ClassPath = ClassOperation->ClassesToDrop[0]->GetPathName();

		// Set the property, it will be verified as valid.
		SendToObjects(ClassPath);

		return FReply::Handled();
	}

	TSharedPtr<FAssetDragDropOp> UnloadedClassOp = DragDropEvent.GetOperationAs<FAssetDragDropOp>();
	if (UnloadedClassOp.IsValid())
	{
		bool bAllAssetWereLoaded = true;

		FString AssetPath;

		// Find the class/blueprint path
		if (UnloadedClassOp->HasAssets())
		{
			AssetPath = UnloadedClassOp->GetAssets()[0].ObjectPath.ToString();
		}
		else if (UnloadedClassOp->HasAssetPaths())
		{
			AssetPath = UnloadedClassOp->GetAssetPaths()[0];
		}

		// Check to see if the asset can be found, otherwise load it.
		UObject* Object = FindObject<UObject>(nullptr, *AssetPath);
		if(Object == nullptr)
		{
			// Load the package.
			GWarn->BeginSlowTask(LOCTEXT("OnDrop_LoadPackage", "Fully Loading Package For Drop"), true, false);

			Object = LoadObject<UObject>(nullptr, *AssetPath);

			GWarn->EndSlowTask();
		}

		if (UClass* Class = Cast<UClass>(Object))
		{
			// This was pointing to a class directly
			SendToObjects(Class->GetPathName());
		}
		else if (UBlueprint* Blueprint = Cast<UBlueprint>(Object))
		{
			if (Blueprint->GeneratedClass)
			{
				// This was pointing to a blueprint, get generated class
				SendToObjects(Blueprint->GeneratedClass->GetPathName());
			}
		}

		return FReply::Handled();
	}

	return FReply::Unhandled();
}

#undef LOCTEXT_NAMESPACE
