// Fill out your copyright notice in the Description page of Project Settings.

#include "XD_Item_Customization.h"
#include <DetailWidgetRow.h>
#include <IDetailChildrenBuilder.h>
#include <Widgets/Input/SButton.h>
#include <Editor.h>
#include <Widgets/Images/SImage.h>
#include <Widgets/Text/STextBlock.h>
#include <PropertyCustomizationHelpers.h>
#include <Engine/StaticMesh.h>
#include <Engine/SkeletalMesh.h>

#include "Abstract/XD_ItemCoreBase.h"
#include "Inventory/XD_InventoryComponentBase.h"
#include "Bluprint/ItemEntityBlueprint.h"

#define LOCTEXT_NAMESPACE "XD_ItemCore类型自定义面板控件"

void FXD_ItemCoreCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	UObject* Value = nullptr;
	FPropertyAccess::Result AccessResult = PropertyHandle->GetValue(Value);

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
					if (AccessResult == FPropertyAccess::MultipleValues)
					{
						return SNew(STextBlock).Text(LOCTEXT("无法编辑多个道具", "无法编辑多个道具"));
					}

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
	if (PropertyHandle->GetValue(Value) != FPropertyAccess::MultipleValues && Value)
	{
		static TArray<FName> ExcludePropertyNames { GET_MEMBER_NAME_CHECKED(UXD_ItemCoreBase, Number) };
		TSharedRef<IPropertyHandle> ItemCoreHandle = PropertyHandle->GetChildHandle(0).ToSharedRef();
		uint32 ChildNumber;
		ItemCoreHandle->GetNumChildren(ChildNumber);
		for (uint32 ChildIndex = 0; ChildIndex < ChildNumber; ++ChildIndex)
		{
			const TSharedRef<IPropertyHandle> ChildHandle = ItemCoreHandle->GetChildHandle(ChildIndex).ToSharedRef();
			const FProperty* Property = ChildHandle->GetProperty();
			// EditDefaultOnly的不显示
			if (Property && !Property->HasAnyPropertyFlags(EPropertyFlags::CPF_DisableEditOnInstance) && !ExcludePropertyNames.Contains(*Property->GetNameCPP()))
			{
				ChildBuilder.AddProperty(ChildHandle);
			}
		}
	}
}

void FXD_ItemModelDataCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	TSharedPtr<IPropertyHandle> Model_Handle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FXD_ItemModelData, Model));
	
	Model_Handle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=]
	{
		TArray<UObject*> Outers;
		PropertyHandle->GetOuterObjects(Outers);
		for (UObject* Outer : Outers)
		{
			if (FXD_ItemModelData* ItemModelData = reinterpret_cast<FXD_ItemModelData*>(PropertyHandle->GetValueBaseAddress(reinterpret_cast<uint8*>(Outer->GetClass()->GetOrCreateSparseClassData()))))
			{
				if (UItemEntityBlueprint* ItemEntityBlueprint = Cast<UItemEntityBlueprint>(ItemModelData->Model.Get()))
				{
					ItemModelData->Model = *ItemEntityBlueprint->GeneratedClass;
				}
				ItemModelData->UpdateModelType();
			}
		}
	}));

	HeaderRow.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
	.ValueContent()
		.HAlign(HAlign_Fill)
		[
			Model_Handle->CreatePropertyValueWidget()
		];
}

void FXD_ItemModelDataCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	static TArray<FName> ExcludePropertyNames{ GET_MEMBER_NAME_CHECKED(FXD_ItemModelData, Model) };
	uint32 ChildNumber;
	if (PropertyHandle->GetNumChildren(ChildNumber) != FPropertyAccess::Fail)
	{
		for (uint32 ChildIndex = 0; ChildIndex < ChildNumber; ++ChildIndex)
		{
			const TSharedRef<IPropertyHandle> ChildHandle = PropertyHandle->GetChildHandle(ChildIndex).ToSharedRef();
			const FProperty* Property = ChildHandle->GetProperty();
			if (Property && !ExcludePropertyNames.Contains(*Property->GetNameCPP()))
			{
				ChildBuilder.AddProperty(ChildHandle);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
