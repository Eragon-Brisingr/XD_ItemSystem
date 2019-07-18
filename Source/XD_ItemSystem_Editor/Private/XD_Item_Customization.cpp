// Fill out your copyright notice in the Description page of Project Settings.

#include "XD_Item_Customization.h"
#include <DetailWidgetRow.h>
#include <IDetailChildrenBuilder.h>
#include <SNumericEntryBox.h>
#include "XD_ItemCoreBase.h"
#include "XD_PropertyCustomizationEx.h"
#include "XD_ItemBase.h"
#include "XD_TemplateLibrary.h"
#include "PropertyCustomizationHelpers.h"

#define LOCTEXT_NAMESPACE "XD_Item类型自定义面板控件"

FXD_Item_Customization::FXD_Item_Customization()
{
	BaseItemClass = AXD_ItemBase::StaticClass();
}

void FXD_Item_Customization::CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TSharedPtr<IPropertyHandle> ItemClass_PropertyHandle = FPropertyCustomizeHelper::GetPropertyHandleByName(StructPropertyHandle, GET_MEMBER_NAME_CHECKED(FXD_Item, ItemClass));
	TSharedPtr<IPropertyHandle> ItemCore_PropertyHandle = FPropertyCustomizeHelper::GetPropertyHandleByName(StructPropertyHandle, GET_MEMBER_NAME_CHECKED(FXD_Item, ItemCore));

	UClass* MetaClass = BaseItemClass;
	FXD_Item* Item = FPropertyCustomizeHelper::Value<FXD_Item>(StructPropertyHandle);
	if (Item)
	{
		if (TSubclassOf<AXD_ItemBase> ShowItemType = Item->ShowItemType)
		{
			MetaClass = ShowItemType;
			if (Item->ItemCore && !Item->ItemCore->GetItemClass()->IsChildOf(Item->ShowItemType))
			{
				Item->ItemClass = Item->ShowItemType;
				Item->ItemCore = AXD_ItemBase::CreateItemCoreByType(ShowItemType, FPropertyCustomizeHelper::GetOuter(ItemCore_PropertyHandle.ToSharedRef()));
				Item->ItemCore->SetFlags(RF_Public | RF_ArchetypeObject);
			}
		}
	}

	TSharedRef<SClassPropertyEntryBox> ItemPropertyEditorClass = SNew(SClassPropertyEntryBox)
				.MetaClass(MetaClass)
				.AllowNone(false)
				.SelectedClass_Lambda([=]()
				{
					FXD_Item* Item = FPropertyCustomizeHelper::Value<FXD_Item>(StructPropertyHandle);
					return Item ? Item->ItemClass : nullptr;
				})
				.OnSetClass_Lambda([=](const UClass* Class)
				{
					TSubclassOf<AXD_ItemBase> ItemClass = const_cast<UClass*>(Class);
					if (FXD_Item* Item = FPropertyCustomizeHelper::Value<FXD_Item>(StructPropertyHandle))
					{
						StructPropertyHandle->NotifyPreChange();
						if (ItemClass)
						{
							UObject* Outer = FPropertyCustomizeHelper::GetOuter(ItemCore_PropertyHandle.ToSharedRef());

							UXD_ItemCoreBase* ItemCore = AXD_ItemBase::CreateItemCoreByType(ItemClass, Outer);
							ItemCore->ItemClass = ItemClass;
							ItemCore->SetFlags(RF_Public | RF_ArchetypeObject);
							Item->ItemCore = ItemCore;
						}
						else
						{
							Item->ItemCore = nullptr;
						}
						Item->ItemClass = ItemClass;
						StructPropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
					}
				});

	if (Item && !Item->bShowNumber)
	{
		HeaderRow.NameContent()
			[
				StructPropertyHandle->CreatePropertyNameWidget()
			]
		.ValueContent()
			[
				ItemPropertyEditorClass
			];
	}
	else 
	{
		TSharedRef<SHorizontalBox> NumberBox = 
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(FMargin(4.f))
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("道具数量", "数量"))
				]
				+ SHorizontalBox::Slot()
				.Padding(FMargin(4.f))
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Center)
				[
					SNew(SNumericEntryBox<int32>)
					.AllowSpin(true)
					.MinValue(1)
					.MaxValue(TOptional<int32>())
					.MinSliderValue(0)
					.MaxSliderValue(TOptional<int32>())
					.Delta(1)
					.MinDesiredValueWidth(200.0f)
					.IsEnabled_Lambda([=]()
					{
						return !StructPropertyHandle->IsEditConst() && GetItemCore(ItemCore_PropertyHandle) ? true : false;
					})
					.Value_Lambda([=]()
					{
						if (UXD_ItemCoreBase* ItemCore = GetItemCore(ItemCore_PropertyHandle))
						{
							return TOptional<int32>(ItemCore->Number);
						}
						return TOptional<int32>(0);
					})
					.OnValueChanged_Lambda([=](int NewNumber)
					{
						if (UXD_ItemCoreBase* ItemCore = GetItemCore(ItemCore_PropertyHandle))
						{
							StructPropertyHandle->NotifyPreChange();
							ItemCore->Number = NewNumber;
							StructPropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
						}
					})
				];
		if (FPropertyCustomizeHelper::IsInArray(StructPropertyHandle))
		{
			HeaderRow.NameContent()
				[
					ItemPropertyEditorClass
				]
			.ValueContent()
				.MinDesiredWidth(300.f)
				[
					NumberBox
				];
		}
		else
		{
			HeaderRow.NameContent()
				[
					StructPropertyHandle->CreatePropertyNameWidget()
				]
			.ValueContent()
				.MinDesiredWidth(300.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					[
						ItemPropertyEditorClass
					]
					+ SHorizontalBox::Slot()
					[
						NumberBox
					]
				];
		}
	}
}

void FXD_Item_Customization::CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	FPropertyCustomizeHelper::StructBuilderDrawPropertys(StructBuilder, StructPropertyHandle, { GET_MEMBER_NAME_CHECKED(FXD_Item, ItemCore), GET_MEMBER_NAME_CHECKED(FXD_Item, ItemClass) });

	TSharedPtr<IPropertyHandle> ItemCore_PropertyHandle = FPropertyCustomizeHelper::GetPropertyHandleByName(StructPropertyHandle, GET_MEMBER_NAME_CHECKED(FXD_Item, ItemCore));
	if (FPropertyCustomizeHelper::GetNumChildren(ItemCore_PropertyHandle.ToSharedRef()) > 0)
	{
		TSharedPtr<class IPropertyHandle> ItemCoreInstance_PropertyHandle = ItemCore_PropertyHandle->GetChildHandle(0);
		if (ItemCoreInstance_PropertyHandle.IsValid())
		{
			FPropertyCustomizeHelper::StructBuilderDrawPropertys(StructBuilder, ItemCoreInstance_PropertyHandle.ToSharedRef(), { GET_MEMBER_NAME_CHECKED(UXD_ItemCoreBase, ItemClass), GET_MEMBER_NAME_CHECKED(UXD_ItemCoreBase, Number) });
		}
	}
}

class UXD_ItemCoreBase* FXD_Item_Customization::GetItemCore(TSharedPtr<IPropertyHandle> ItemCore_PropertyHandle) const
{
	if (ItemCore_PropertyHandle.IsValid())
	{
		UObject* Object = nullptr;
		ItemCore_PropertyHandle->GetValue(Object);
		if (UXD_ItemCoreBase* ItemCore = Cast<UXD_ItemCoreBase>(Object))
		{
			return ItemCore;
		}
	}
	return nullptr;
}

void FXD_ItemCore_Customization::CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	UXD_ItemCoreBase* ItemCore = FPropertyCustomizeHelper::GetValue<UXD_ItemCoreBase*>(StructPropertyHandle);

	TSharedPtr<IPropertyHandle> ItemClass_PropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(UXD_ItemCoreBase, ItemClass));
	TSharedPtr<IPropertyHandle> Number_PropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(UXD_ItemCoreBase, Number));

	if (!ItemCore)
	{
		HeaderRow.NameContent()
			[
				StructPropertyHandle->CreatePropertyNameWidget()
			]
		.ValueContent()
			.MinDesiredWidth(300.f)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Error_Null", "错误：空的道具核心"))
				]
				+ SHorizontalBox::Slot()
				[
					StructPropertyHandle->CreatePropertyValueWidget()
				]
			];
	}
	else
	{
		HeaderRow.NameContent()
			[
				StructPropertyHandle->CreatePropertyNameWidget()
			]
		.ValueContent()
			.MinDesiredWidth(300.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(2.f)
				[
					SNew(STextBlock)
					.Text(ItemCore->ItemClass ? ItemCore->ItemClass->GetDisplayNameText() : LOCTEXT("Error_ClassNull", "错误：空的道具类"))
				]
				+ SHorizontalBox::Slot()
				[
					Number_PropertyHandle->CreatePropertyNameWidget()
				]
				+ SHorizontalBox::Slot()
				[
					Number_PropertyHandle->CreatePropertyValueWidget()
				]
			];
	}
}

void FXD_ItemCore_Customization::CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TSharedPtr<class IPropertyHandle> Instance = StructPropertyHandle->GetChildHandle(0);
	if (Instance.IsValid())
	{
		FPropertyCustomizeHelper::StructBuilderDrawPropertys(StructBuilder, Instance.ToSharedRef(), { GET_MEMBER_NAME_CHECKED(UXD_ItemCoreBase, ItemClass), GET_MEMBER_NAME_CHECKED(UXD_ItemCoreBase, Number) });
	}
}

#undef LOCTEXT_NAMESPACE
