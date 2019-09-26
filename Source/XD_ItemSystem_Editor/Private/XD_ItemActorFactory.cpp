// Fill out your copyright notice in the Description page of Project Settings.


#include "XD_ItemActorFactory.h"
#include "XD_ItemBase.h"
#include "XD_ItemCoreBase.h"
#include "AssetData.h"
#include "Engine/Blueprint.h"

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
		if (UBlueprint* Blueprint = Cast<UBlueprint>(AssetClass))
		{
			// TODO：资源有效性检查
			if (Blueprint->GeneratedClass->IsChildOf<UXD_ItemCoreBase>())
			{
				//OutErrorMsg = LOCTEXT("CanCreateActorFrom_NoTileMap", "No tile map was specified.");
				return true;
			}
		}
	}
	return true;
}

AActor* UXD_ItemActorFactory::SpawnActor(UObject* Asset, ULevel* InLevel, const FTransform& Transform, EObjectFlags InObjectFlags, const FName Name)
{
	UBlueprint* ItemBlueprint = CastChecked<UBlueprint>(Asset);
	TSubclassOf<UXD_ItemCoreBase> ItemClass = CastChecked<UClass>(ItemBlueprint->GeneratedClass);
	UXD_ItemCoreBase* ItemCore = CastChecked<UXD_ItemCoreBase>(ItemClass.GetDefaultObject());
	AXD_ItemBase* Item = ItemCore->SpawnItemActorInLevel(InLevel, ItemCore->Number, Name, InObjectFlags, Transform.GetLocation(), Transform.GetRotation().Rotator(), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	return Item;
}

void UXD_ItemActorFactory::PostSpawnActor(UObject* Asset, AActor* NewActor)
{

}

#undef LOCTEXT_NAMESPACE
