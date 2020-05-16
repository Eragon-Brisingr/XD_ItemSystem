// Fill out your copyright notice in the Description page of Project Settings.


#include "XD_ItemActorFactory.h"
#include <Misc/MessageDialog.h>
#include <AssetData.h>

#include "Abstract/XD_ItemBase.h"
#include "Abstract/XD_ItemCoreBase.h"
#include "Bluprint/XD_ItemCoreBlueprint.h"

#define LOCTEXT_NAMESPACE "XD_ItemActorFactory" 

UXD_ItemActorFactory::UXD_ItemActorFactory(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
	DisplayName = LOCTEXT("ItemActorFactoryName", "XD_Item");
	NewActorClass = AXD_ItemBase::StaticClass();
	bUseSurfaceOrientation = true;
}

AActor* UXD_ItemActorFactory::GetDefaultActor(const FAssetData& AssetData)
{
	if (!AssetData.IsValid() || !AssetData.GetClass()->IsChildOf(UBlueprint::StaticClass()))
	{
		return nullptr;
	}

	const FString GeneratedClassPath = AssetData.GetTagValueRef<FString>(FBlueprintTags::GeneratedClassPath);
	if (GeneratedClassPath.IsEmpty())
	{
		return nullptr;
	}

	UClass* GeneratedClass = Cast<UClass>(StaticLoadObject(UClass::StaticClass(), nullptr, *GeneratedClassPath, nullptr, LOAD_NoWarn, nullptr));
	if (GeneratedClass == nullptr)
	{
		return nullptr;
	}

	UXD_ItemCoreBase* ItemCore = GeneratedClass->GetDefaultObject<UXD_ItemCoreBase>();
	if (ItemCore == nullptr)
	{
		return nullptr;
	}

	TSubclassOf<AActor> SpawnedItemClass = ItemCore->GetSpawnedItemClass();
	if (SpawnedItemClass == nullptr)
	{
		return nullptr;
	}
	return SpawnedItemClass.GetDefaultObject();
}

UObject* UXD_ItemActorFactory::GetAssetFromActorInstance(AActor* ActorInstance)
{
	if (AXD_ItemBase* Item = Cast<AXD_ItemBase>(ActorInstance))
	{
		if (UXD_ItemCoreBase* ItemCore = Item->ItemCore)
		{
			return ItemCore->GetClass()->ClassGeneratedBy;
		}
	}
	return nullptr;
}

bool UXD_ItemActorFactory::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	if (AssetData.IsValid())
	{
		UObject* AssetClass = AssetData.GetAsset();
		if (UXD_ItemCoreBlueprint* Blueprint = Cast<UXD_ItemCoreBlueprint>(AssetClass))
		{
			if (TSubclassOf<UXD_ItemCoreBase> ItemClass = CastChecked<UClass>(Blueprint->GeneratedClass))
			{
				const UXD_ItemCoreBase* ItemCore = CastChecked<UXD_ItemCoreBase>(ItemClass.GetDefaultObject());
				const TSubclassOf<AXD_ItemBase> ItemEntityClass = ItemCore->GetSpawnedItemClass();
				const FXD_ItemModelData& ItemModelData = ItemCore->GetCurrentItemModel();
				if (ItemEntityClass == nullptr || ItemModelData.ModelType == EItemModelType::None || ItemModelData.Model.IsNull())
				{
					OutErrorMsg = LOCTEXT("模型未配置", "模型未配置");
					return false;
				}
				return true;
			}
		}
	}
	return false;
}

bool UXD_ItemActorFactory::PreSpawnActor(UObject* Asset, FTransform& InOutLocation)
{
	return true;
}

AActor* UXD_ItemActorFactory::SpawnActor(UObject* Asset, ULevel* InLevel, const FTransform& Transform, EObjectFlags InObjectFlags, const FName Name)
{
	if (UXD_ItemCoreBlueprint* ItemBlueprint = Cast<UXD_ItemCoreBlueprint>(Asset))
	{
		TSubclassOf<UXD_ItemCoreBase> ItemClass = CastChecked<UClass>(ItemBlueprint->GeneratedClass);
		const UXD_ItemCoreBase* ItemCore = CastChecked<UXD_ItemCoreBase>(ItemClass.GetDefaultObject());
		int32 SpawnItemCoreNumber = 1;
		if ((InObjectFlags & RF_Transactional))
		{
			if (ItemCore->CanMergeItem())
			{
				static FText Title = LOCTEXT("物品模式选择", "物品模式选择");
				EAppReturnType::Type AppReturn = FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("是否创建为堆叠模式的物品？", "是否创建为堆叠模式的物品？"));
				if (AppReturn == EAppReturnType::Yes)
				{
					SpawnItemCoreNumber = ItemCore->GetMinItemMergeNumberValue();
				}
			}
		}
		AXD_ItemBase* Item = ItemCore->SpawnItemActorInLevel(InLevel, Transform.GetLocation(), Transform.GetRotation().Rotator(), SpawnItemCoreNumber, Name, InObjectFlags, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		Item->ItemInWorldSetting();
		return Item;
	}
	return nullptr;
}

void UXD_ItemActorFactory::PostSpawnActor(UObject* Asset, AActor* NewActor)
{

}

void UXD_ItemActorFactory::PostCreateBlueprint(UObject* Asset, AActor* CDO)
{
	// 不允许从道具实体制作蓝图
	ensureAlways(false);
}

#undef LOCTEXT_NAMESPACE
