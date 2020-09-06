// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayItemActorFactory.h"
#include <Misc/MessageDialog.h>
#include <AssetData.h>

#include "Abstract/GameplayItemBase.h"
#include "Abstract/GameplayItemCoreBase.h"
#include "Bluprint/GameplayItemCoreBlueprint.h"

#define LOCTEXT_NAMESPACE "GameplayItemActorFactory" 

UGameplayItemActorFactory::UGameplayItemActorFactory(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
	DisplayName = LOCTEXT("ItemActorFactoryName", "GameplayItem");
	NewActorClass = AGameplayItemBase::StaticClass();
	bUseSurfaceOrientation = true;
}

AActor* UGameplayItemActorFactory::GetDefaultActor(const FAssetData& AssetData)
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

	UGameplayItemCoreBase* ItemCore = GeneratedClass->GetDefaultObject<UGameplayItemCoreBase>();
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

UObject* UGameplayItemActorFactory::GetAssetFromActorInstance(AActor* ActorInstance)
{
	if (AGameplayItemBase* Item = Cast<AGameplayItemBase>(ActorInstance))
	{
		if (UGameplayItemCoreBase* ItemCore = Item->ItemCore)
		{
			return ItemCore->GetClass()->ClassGeneratedBy;
		}
	}
	return nullptr;
}

bool UGameplayItemActorFactory::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	if (AssetData.IsValid())
	{
		UObject* AssetClass = AssetData.GetAsset();
		if (UGameplayItemCoreBlueprint* Blueprint = Cast<UGameplayItemCoreBlueprint>(AssetClass))
		{
			if (Blueprint->GeneratedClass == nullptr)
			{
				return false;
			}
			
			if (TSubclassOf<UGameplayItemCoreBase> ItemClass = CastChecked<UClass>(Blueprint->GeneratedClass))
			{
				const UGameplayItemCoreBase* ItemCore = CastChecked<UGameplayItemCoreBase>(ItemClass.GetDefaultObject());
				const TSubclassOf<AGameplayItemBase> ItemEntityClass = ItemCore->GetSpawnedItemClass();
				const FGameplayItemModelData& ItemModelData = ItemCore->GetCurrentItemModel();
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

bool UGameplayItemActorFactory::PreSpawnActor(UObject* Asset, FTransform& InOutLocation)
{
	return true;
}

AActor* UGameplayItemActorFactory::SpawnActor(UObject* Asset, ULevel* InLevel, const FTransform& Transform, EObjectFlags InObjectFlags, const FName Name)
{
	if (UGameplayItemCoreBlueprint* ItemBlueprint = Cast<UGameplayItemCoreBlueprint>(Asset))
	{
		TSubclassOf<UGameplayItemCoreBase> ItemClass = CastChecked<UClass>(ItemBlueprint->GeneratedClass);
		const UGameplayItemCoreBase* ItemCore = CastChecked<UGameplayItemCoreBase>(ItemClass.GetDefaultObject());
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
		AGameplayItemBase* Item = ItemCore->SpawnItemActorInLevel(InLevel, Transform.GetLocation(), Transform.GetRotation().Rotator(), SpawnItemCoreNumber, Name, InObjectFlags, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		Item->ItemInWorldSetting();
		return Item;
	}
	return nullptr;
}

void UGameplayItemActorFactory::PostSpawnActor(UObject* Asset, AActor* NewActor)
{

}

void UGameplayItemActorFactory::PostCreateBlueprint(UObject* Asset, AActor* CDO)
{
	// 不允许从道具实体制作蓝图
	ensureAlways(false);
}

#undef LOCTEXT_NAMESPACE
