// Fill out your copyright notice in the Description page of Project Settings.

#include "XD_Item_Customization.h"
#include <DetailWidgetRow.h>
#include <IDetailChildrenBuilder.h>
#include "XD_ItemCoreBase.h"
#include "XD_PropertyCustomizationEx.h"
#include "PropertyCustomizationHelpers.h"
#include "XD_InventoryComponentBase.h"
#include "SButton.h"
#include "Editor.h"
#include "SImage.h"
#include "STextBlock.h"

#define LOCTEXT_NAMESPACE "XD_ItemCore类型自定义面板控件"

void FXD_ItemCoreCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	UObject* Value = nullptr;
	PropertyHandle->GetValue(Value);

	bool bShowNumber = Value && PropertyHandle->GetBoolMetaData(TEXT("ConfigUseItem"));
	if (!bShowNumber)
	{
		if (UXD_ItemCoreBase* ItemCore = Cast<UXD_ItemCoreBase>(Value))
		{
			if (UObject* Outer = ItemCore->GetOuter())
			{
				if (Outer->IsA<UXD_InventoryComponentBase>() && PropertyHandle->GetProperty()->GetFName() == GET_MEMBER_NAME_CHECKED(UXD_InventoryComponentBase, ItemCoreList))
				{
					if (ItemCore->CanCompositeInInventory())
					{
						bShowNumber = true;
					}
				}
			}
		}
	}

	HeaderRow.NameContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				PropertyHandle->CreatePropertyNameWidget()
			]
			+ SHorizontalBox::Slot()
			.Padding(4.f, 0.f)
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text_Lambda([=]()
					{
						FText ItemName = LOCTEXT("道具为空", "空");
						UObject* Value = nullptr;
						FPropertyAccess::Result AccessResult = PropertyHandle->GetValue(Value);
						switch (AccessResult)
						{
						case FPropertyAccess::MultipleValues:
							ItemName = LOCTEXT("选择了多个道具", "多个道具");
							break;
						case FPropertyAccess::Success:
							if (UXD_ItemCoreBase * ItemCore = Cast<UXD_ItemCoreBase>(Value))
							{
								ItemName = ItemCore->GetItemName();
							}
							break;
						}
						return FText::Format(LOCTEXT("道具名Format", "[{0}]"), ItemName);
					})
				.ToolTipText(LOCTEXT("道具名", "道具名"))
			]
		]
	.ValueContent()
		.HAlign(HAlign_Fill)	
		[
			([&]()->TSharedRef<SWidget>
				{
					TSharedRef<SHorizontalBox> HorizontalBox = SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Fill)
						[
							PropertyHandle->CreatePropertyValueWidget(false)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SButton)
							.ToolTipText(LOCTEXT("定位至道具类", "定位至道具类"))
							.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
							.IsEnabled_Lambda([=]()
								{
									UObject* Value = nullptr;
									PropertyHandle->GetValue(Value);
									return Value ? true : false;
								})
							.OnClicked_Lambda([=]()
								{
									UObject* Value = nullptr;
									PropertyHandle->GetValue(Value);
									if (Value)
									{
										if (UObject* Blueprint = Value->GetClass()->ClassGeneratedBy)
										{
											TArray<UObject*> SyncedObject{ Blueprint };
											GEditor->SyncBrowserToObjects(SyncedObject);
										}
									}
									return FReply::Handled();
								})
							[
								SNew(SImage)
								.Image(FSlateIcon(FEditorStyle::GetStyleSetName(), "Symbols.SearchGlass").GetIcon())
							]
						];

					if (bShowNumber)
					{
						TSharedPtr<IPropertyHandle> NumberPropertyHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(UXD_ItemCoreBase, Number));
						if (NumberPropertyHandle.IsValid())
						{
							HorizontalBox->AddSlot()
								.HAlign(HAlign_Fill)
								.FillWidth(0.5f)
								[
									NumberPropertyHandle->CreatePropertyValueWidget(false)
								];
						}
					}

					return HorizontalBox;
				}())
		];
}

void FXD_ItemCoreCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	UObject* Value;
	PropertyHandle->GetValue(Value);
	if (Value)
	{
		TArray<FName> ExcludePropertyNames { GET_MEMBER_NAME_CHECKED(UXD_ItemCoreBase, Number) };
		TSharedRef<IPropertyHandle> ItemCoreHandle = PropertyHandle->GetChildHandle(0).ToSharedRef();
		for (uint32 ChildIndex = 0; ChildIndex < FPropertyCustomizeHelper::GetNumChildren(ItemCoreHandle); ++ChildIndex)
		{
			const TSharedRef<IPropertyHandle> ChildHandle = ItemCoreHandle->GetChildHandle(ChildIndex).ToSharedRef();
			UProperty* Property = ChildHandle->GetProperty();
			// EditDefaultOnly的不显示
			if (Property && !Property->HasAnyPropertyFlags(EPropertyFlags::CPF_DisableEditOnInstance) && !ExcludePropertyNames.Contains(*Property->GetNameCPP()))
			{
				ChildBuilder.AddProperty(ChildHandle);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
