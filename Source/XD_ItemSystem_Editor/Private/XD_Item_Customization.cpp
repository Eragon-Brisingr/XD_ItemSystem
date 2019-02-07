// Fill out your copyright notice in the Description page of Project Settings.

#include "XD_Item_Customization.h"
#include <DetailWidgetRow.h>
#include <IDetailChildrenBuilder.h>
#include <SNumericEntryBox.h>
#include "XD_ItemCoreBase.h"
#include "XD_PropertyCustomizationEx.h"
#include "XD_ItemBase.h"
#include "XD_TemplateLibrary.h"

#define LOCTEXT_NAMESPACE "XD_Item类型自定义面板控件"

void FXD_Item_Customization::CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TSharedPtr<IPropertyHandle> ItemClass_PropertyHandle = FPropertyCustomizeHelper::GetPropertyHandleByName(StructPropertyHandle, GET_MEMBER_NAME_STRING_CHECKED(FXD_Item, ItemClass));
	TSharedPtr<IPropertyHandle> ItemCore_PropertyHandle = FPropertyCustomizeHelper::GetPropertyHandleByName(StructPropertyHandle, GET_MEMBER_NAME_STRING_CHECKED(FXD_Item, ItemCore));

	ItemClass_PropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=]()
	{
		UObject* ClassObject;
		ItemClass_PropertyHandle->GetValue(ClassObject);

		if (ClassObject)
		{
			UObject* Outer = FPropertyCustomizeHelper::GetOuter(ItemCore_PropertyHandle.ToSharedRef());

			TSubclassOf<AXD_ItemBase> ItemClass = CastChecked<UClass>(ClassObject);
			UXD_ItemCoreBase* ItemCore = AXD_ItemBase::CreateItemCoreByType(ItemClass, Outer);
			ItemCore->ItemClass = ItemClass;
			ItemCore->SetFlags(RF_Public | RF_ArchetypeObject);
			FPropertyCustomizeHelper::SetObjectValue(ItemCore_PropertyHandle, ItemCore);
		}
		else
		{
			FPropertyCustomizeHelper::SetObjectValue(ItemCore_PropertyHandle, nullptr);
		}
	}));
	FXD_Item& Item = FPropertyCustomizeHelper::Value<FXD_Item>(StructPropertyHandle);

 	TSharedRef<SWidget> PropertyValueWidget = ItemClass_PropertyHandle->CreatePropertyValueWidget(false);

 //#include <Private/PropertyEditorHelpers.h>
 //#include <Private/UserInterface/PropertyEditor/SPropertyEditorClass.h>
 //#define offsetof(type, member) (size_t)&(((type*)0)->member)
 //	constexpr long offset = offsetof(SPropertyValueWidget, ValueEditorWidget);
 	//SPropertyValueWidget::ValueEditorWidget
 	TSharedPtr<SWidget>& ValueEditorWidget = GetObjectMemberByOffset<TSharedPtr<SWidget>>(&PropertyValueWidget.Get(), 768L);
 	//SPropertyEditorClass::MetaClass
 	const UClass*& MetaClass = GetObjectMemberByOffset<const UClass*>(ValueEditorWidget.Get(), 800L);

 	if (TSubclassOf<AXD_ItemBase> ShowItemType = Item.ShowItemType)
 	{
 		MetaClass = ShowItemType;
		if (Item.ItemCore && !Item.ItemCore->ItemClass->IsChildOf(Item.ShowItemType))
		{
			Item.ItemClass = Item.ShowItemType;
			Item.ItemCore = AXD_ItemBase::CreateItemCoreByType(ShowItemType, FPropertyCustomizeHelper::GetOuter(ItemCore_PropertyHandle.ToSharedRef()));
			Item.ItemCore->SetFlags(RF_Public | RF_ArchetypeObject);
		}
 	}
	else
	{
		MetaClass = BaseItemClass;
	}

	if (!Item.bShowNumber)
	{
		HeaderRow.NameContent()
			[
				StructPropertyHandle->CreatePropertyNameWidget()
			]
		.ValueContent()
			[
				PropertyValueWidget
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
							ItemCore->Number = NewNumber;
						}
					})
				];
		if (FPropertyCustomizeHelper::IsInArray(StructPropertyHandle))
		{
			HeaderRow.NameContent()
				[
					PropertyValueWidget
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
					PropertyValueWidget
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
	FPropertyCustomizeHelper::StructBuilderDrawPropertys(StructBuilder, StructPropertyHandle, { GET_MEMBER_NAME_STRING_CHECKED(FXD_Item, ItemCore), GET_MEMBER_NAME_STRING_CHECKED(FXD_Item, ItemClass) });

	TSharedPtr<IPropertyHandle> ItemCore_PropertyHandle = FPropertyCustomizeHelper::GetPropertyHandleByName(StructPropertyHandle, GET_MEMBER_NAME_STRING_CHECKED(FXD_Item, ItemCore));
	if (FPropertyCustomizeHelper::GetNumChildren(ItemCore_PropertyHandle.ToSharedRef()) > 0)
	{
		TSharedPtr<class IPropertyHandle> ItemCoreInstance_PropertyHandle = ItemCore_PropertyHandle->GetChildHandle(0);
		for (uint32 Idx = 0; Idx < FPropertyCustomizeHelper::GetNumChildren(ItemCoreInstance_PropertyHandle.ToSharedRef()); ++Idx)
		{
			TSharedPtr<class IPropertyHandle> ItemCoreInstanceChild_PropertyHandle = ItemCoreInstance_PropertyHandle->GetChildHandle(Idx);

			if (UProperty* Property = ItemCoreInstanceChild_PropertyHandle->GetProperty())
			{
				FString ItemCoreMemberCppName = Property->GetNameCPP();
				if (ItemCoreMemberCppName != GET_MEMBER_NAME_STRING_CHECKED(UXD_ItemCoreBase, ItemClass)
					&& ItemCoreMemberCppName != GET_MEMBER_NAME_STRING_CHECKED(UXD_ItemCoreBase, Number))
				{
					StructBuilder.AddProperty(ItemCoreInstance_PropertyHandle->GetChildHandle(Idx).ToSharedRef());
				}
			}
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

#undef LOCTEXT_NAMESPACE
