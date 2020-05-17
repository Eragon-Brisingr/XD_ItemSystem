// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayItem_Customization.h"
#include <DetailWidgetRow.h>
#include <IDetailChildrenBuilder.h>
#include <Editor.h>
#include <Widgets/Input/SButton.h>
#include <Widgets/Images/SImage.h>
#include <Widgets/Text/STextBlock.h>
#include <Widgets/SBoxPanel.h>
#include <Widgets/Notifications/SNotificationList.h>
#include <Widgets/Layout/SBox.h>
#include <PropertyCustomizationHelpers.h>
#include <Framework/Notifications/NotificationManager.h>
#include <AssetToolsModule.h>
#include <IAssetTools.h>
#include <DetailLayoutBuilder.h>
#include <IPropertyUtilities.h>

#include "Abstract/GameplayItemBase.h"
#include "Abstract/GameplayItemCoreBase.h"
#include "Inventory/GameplayInventoryComponentBase.h"
#include "Bluprint/GameplayItemEntityBlueprint.h"
#include "GameplayItemFactory.h"

#define LOCTEXT_NAMESPACE "GameplayItemCore类型自定义面板控件"

void FGameplayItemCoreCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	UObject* Value = nullptr;
	FPropertyAccess::Result AccessResult = PropertyHandle->GetValue(Value);
	UGameplayItemCoreBase* ItemCore = Cast<UGameplayItemCoreBase>(Value);

	bool bShowNumber = ItemCore && PropertyHandle->GetBoolMetaData(TEXT("ConfigUseItem"));
	if (!bShowNumber && ItemCore)
	{
		if (UObject* Outer = ItemCore->GetOuter())
		{
			if (Outer->IsA<UGameplayInventoryComponentBase>() && PropertyHandle->GetProperty()->GetFName() == GET_MEMBER_NAME_CHECKED(UGameplayInventoryComponentBase, ItemCoreList))
			{
				if (ItemCore->CanCompositeInInventory())
				{
					bShowNumber = true;
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
							if (UGameplayItemCoreBase * ItemCore = Cast<UGameplayItemCoreBase>(Value))
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
						TSharedPtr<IPropertyHandle> NumberPropertyHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(UGameplayItemCoreBase, Number));
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

void FGameplayItemCoreCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	UObject* Value;
	if (PropertyHandle->GetValue(Value) != FPropertyAccess::MultipleValues && Value)
	{
		static TArray<FName> ExcludePropertyNames { GET_MEMBER_NAME_CHECKED(UGameplayItemCoreBase, Number) };
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

void FGameplayItemModelDataCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	TSharedPtr<IPropertyHandle> Model_Handle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FGameplayItemModelData, Model));
	
	Model_Handle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=]
	{
		TArray<UObject*> Outers;
		PropertyHandle->GetOuterObjects(Outers);
		for (UObject* Outer : Outers)
		{
			if (FGameplayItemModelData* ItemModelData = reinterpret_cast<FGameplayItemModelData*>(PropertyHandle->GetValueBaseAddress(reinterpret_cast<uint8*>(Outer->GetClass()->GetOrCreateSparseClassData()))))
			{
				if (UGameplayItemEntityBlueprint* GameplayItemEntityBlueprint = Cast<UGameplayItemEntityBlueprint>(ItemModelData->Model.Get()))
				{
					UGameplayItemCoreBase* ItemCore = Cast<UGameplayItemCoreBase>(Outer);
					AGameplayItemBase* ItemEntity = Cast<AGameplayItemBase>(GameplayItemEntityBlueprint->GeneratedClass.GetDefaultObject());
					if (ItemCore && ItemEntity)
					{
						if (ItemCore->IsA(ItemEntity->BelongToCoreType))
						{
							ItemModelData->Model = *GameplayItemEntityBlueprint->GeneratedClass;
						}
						else
						{
							FNotificationInfo NotificationInfo(FText::Format(LOCTEXT("Model赋值实体类型不匹配报错", "道具实体所属的类型为[{0}]，与当前类型[{1}]不匹配"), ItemEntity->BelongToCoreType->GetDisplayNameText(), ItemCore->GetClass()->GetDisplayNameText()));
							NotificationInfo.bFireAndForget = true;
							FSlateNotificationManager::Get().AddNotification(NotificationInfo);

							ItemModelData->Model.Reset();
						}
					}
					else
					{
						ItemModelData->Model.Reset();
					}
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
		.MinDesiredWidth(250.f)
		.HAlign(HAlign_Left)
		[
			Model_Handle->CreatePropertyValueWidget()
		];
}

void FGameplayItemModelDataCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	static TArray<FName> ExcludePropertyNames{ GET_MEMBER_NAME_CHECKED(FGameplayItemModelData, Model) };
	uint32 ChildNumber;
	ChildBuilder.AddCustomRow(FText::GetEmpty()).NameContent()
	[
		SNullWidget::NullWidget
	]
	.ValueContent()
	[
		SNew(SButton)
		.ToolTipText(LOCTEXT("创建物品核心对应的实体Actor", "创建物品核心对应的实体Actor"))
		.Text(FText::FromString(TEXT("创建实体Actor")))
		.OnClicked_Lambda([=]()
		{
			TArray<UObject*> Outers;
			PropertyHandle->GetOuterObjects(Outers);
			if (Outers.Num() > 0)
			{
				if (UGameplayItemCoreBase* ItemCore = Cast<UGameplayItemCoreBase>(Outers[0]))
				{
					TSharedPtr<IPropertyHandle> Model_Handle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FGameplayItemModelData, Model));

					UItemEntityFactory* FactoryInstance = NewObject<UItemEntityFactory>();
					FactoryInstance->ItemCoreClass = ItemCore->GetClass();

					FAssetToolsModule& AssetToolsModule = FAssetToolsModule::GetModule();
					UObject* NewAsset = AssetToolsModule.Get().CreateAssetWithDialog(FactoryInstance->GetSupportedClass(), FactoryInstance);
					if (NewAsset)
					{
						Model_Handle->SetValue(NewAsset);
					}
				}
			}
			return FReply::Handled();
		})
	];
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

void FGameplayItemCoreDetails::CustomizeDetails(class IDetailLayoutBuilder& DetailBuilder)
{
	TSharedRef<IPropertyHandle> Number_Handle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UGameplayItemCoreBase, Number), UGameplayItemCoreBase::StaticClass());

	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);

	if (ObjectsBeingCustomized.Num() != 1)
	{
		DetailBuilder.HideProperty(Number_Handle);
		return;
	}

	if (UGameplayItemCoreBase* ItemCore = Cast<UGameplayItemCoreBase>(ObjectsBeingCustomized[0].Get()))
	{
		if (ItemCore->HasAnyFlags(RF_ClassDefaultObject) || ItemCore->Number == 1)
		{
			DetailBuilder.HideProperty(Number_Handle);

			TSharedRef<IPropertyHandle> HACK_SOFTOBJECT_SLOT_Handle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(FGameplayItemCoreSparseData, HACK_SOFTOBJECT_SLOT), FGameplayItemCoreSparseData::StaticStruct());
			DetailBuilder.HideProperty(HACK_SOFTOBJECT_SLOT_Handle);

			TSharedRef<IPropertyHandle> CanMergeItemValue_Handle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(FGameplayItemCoreSparseData, bCanMergeItemValue), FGameplayItemCoreSparseData::StaticStruct());
			bool CanMergeItem = false;
			CanMergeItemValue_Handle->GetValue(CanMergeItem);
			if (CanMergeItem == false)
			{
				DetailBuilder.HideProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(FGameplayItemCoreSparseData, MinItemMergeNumberValue), FGameplayItemCoreSparseData::StaticStruct()));
				DetailBuilder.HideProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(FGameplayItemCoreSparseData, MergeItemModelValue), FGameplayItemCoreSparseData::StaticStruct()));
			}

			CanMergeItemValue_Handle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([PropertyUtilities = DetailBuilder.GetPropertyUtilities()]
			{
				PropertyUtilities->ForceRefresh();
			}));
		}
		else
		{
			const FString MinItemMergeNumberValue = FString::FromInt(ItemCore->GetMinItemMergeNumberValue());
			Number_Handle->SetInstanceMetaData(TEXT("ClampMin"), MinItemMergeNumberValue);
			Number_Handle->SetInstanceMetaData(TEXT("UIMin"), MinItemMergeNumberValue);
		}
	}
}

#undef LOCTEXT_NAMESPACE
