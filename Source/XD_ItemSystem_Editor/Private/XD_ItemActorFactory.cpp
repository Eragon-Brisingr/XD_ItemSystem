// Fill out your copyright notice in the Description page of Project Settings.


#include "XD_ItemActorFactory.h"
#include "Abstract/XD_ItemBase.h"
#include "Abstract/XD_ItemCoreBase.h"
#include "AssetData.h"
#include "Bluprint/XD_ItemCoreBlueprint.h"

#define LOCTEXT_NAMESPACE "XD_ItemActorFactory" 

UXD_ItemActorFactory::UXD_ItemActorFactory(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
	DisplayName = LOCTEXT("ItemActorFactoryName", "XD_Item");
	NewActorClass = AXD_ItemBase::StaticClass();
	bUseSurfaceOrientation = true;
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
				UXD_ItemCoreBase* ItemCore = CastChecked<UXD_ItemCoreBase>(ItemClass.GetDefaultObject());
				const FXD_ItemModelData& ItemModelData = ItemCore->GetCurrentItemModel();
				if (ItemModelData.ModelType == EItemModelType::None || ItemModelData.Model.IsNull())
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

AActor* UXD_ItemActorFactory::SpawnActor(UObject* Asset, ULevel* InLevel, const FTransform& Transform, EObjectFlags InObjectFlags, const FName Name)
{
	if (UXD_ItemCoreBlueprint* ItemBlueprint = Cast<UXD_ItemCoreBlueprint>(Asset))
	{
		TSubclassOf<UXD_ItemCoreBase> ItemClass = CastChecked<UClass>(ItemBlueprint->GeneratedClass);
		UXD_ItemCoreBase* ItemCore = CastChecked<UXD_ItemCoreBase>(ItemClass.GetDefaultObject());
		AXD_ItemBase* Item = ItemCore->SpawnItemActorInLevel(InLevel, ItemCore->Number, Name, InObjectFlags, Transform.GetLocation(), Transform.GetRotation().Rotator(), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		Item->WhenItemInWorldSetting();
		return Item;
	}
	return nullptr;
}

void UXD_ItemActorFactory::PostSpawnActor(UObject* Asset, AActor* NewActor)
{

}

#undef LOCTEXT_NAMESPACE
