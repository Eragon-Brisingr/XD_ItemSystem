// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetTypeActions_ItemCore.h"
#include <ThumbnailHelpers.h>
#include <ThumbnailRendering/SceneThumbnailInfo.h>

#include "Abstract/XD_ItemCoreBase.h"
#include "Bluprint/XD_ItemCoreBlueprint.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencer_EditorModule"

UItemCore_ThumbnailRenderer::UItemCore_ThumbnailRenderer(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{

}

void UItemCore_ThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* RenderTarget, FCanvas* Canvas)
{
	UXD_ItemCoreBlueprint* ItemCoreBlueprint = Cast<UXD_ItemCoreBlueprint>(Object);
	if (ItemCoreBlueprint == nullptr || ItemCoreBlueprint->GeneratedClass == nullptr)
	{
		return;
	}

	UXD_ItemCoreBase* ItemCore = ItemCoreBlueprint->GeneratedClass->GetDefaultObject<UXD_ItemCoreBase>();
	if (ItemCore == nullptr)
	{
		return;
	}
	TSoftObjectPtr<UObject> ItemModelPtr = ItemCore->GetCurrentItemModel();
	if (ItemModelPtr.IsNull())
	{
		return;
	}
	UObject* ItemModel = ItemModelPtr.LoadSynchronous();

	if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(ItemModel))
	{
		if (StaticMeshThumbnailScene == nullptr || ensure(StaticMeshThumbnailScene->GetWorld() != nullptr) == false)
		{
			if (StaticMeshThumbnailScene)
			{
				FlushRenderingCommands();
				delete StaticMeshThumbnailScene;
			}
			StaticMeshThumbnailScene = new FStaticMeshThumbnailScene();
		}

		StaticMeshThumbnailScene->SetStaticMesh(StaticMesh);
		StaticMeshThumbnailScene->GetScene()->UpdateSpeedTreeWind(0.0);

		FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(RenderTarget, StaticMeshThumbnailScene->GetScene(), FEngineShowFlags(ESFIM_Game))
			.SetWorldTimes(FApp::GetCurrentTime() - GStartTime, FApp::GetDeltaTime(), FApp::GetCurrentTime() - GStartTime));

		ViewFamily.EngineShowFlags.DisableAdvancedFeatures();
		ViewFamily.EngineShowFlags.MotionBlur = 0;
		ViewFamily.EngineShowFlags.LOD = 0;

		StaticMeshThumbnailScene->GetView(&ViewFamily, X, Y, Width, Height);
		RenderViewFamily(Canvas, &ViewFamily);
		StaticMeshThumbnailScene->SetStaticMesh(nullptr);
	}
	else if (USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(ItemModel))
	{
		if (SkeletalMeshThumbnailScene == nullptr)
		{
			SkeletalMeshThumbnailScene = new FSkeletalMeshThumbnailScene();
		}

		SkeletalMeshThumbnailScene->SetSkeletalMesh(SkeletalMesh);
		FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(RenderTarget, SkeletalMeshThumbnailScene->GetScene(), FEngineShowFlags(ESFIM_Game))
			.SetWorldTimes(FApp::GetCurrentTime() - GStartTime, FApp::GetDeltaTime(), FApp::GetCurrentTime() - GStartTime));

		ViewFamily.EngineShowFlags.DisableAdvancedFeatures();
		ViewFamily.EngineShowFlags.MotionBlur = 0;
		ViewFamily.EngineShowFlags.LOD = 0;

		SkeletalMeshThumbnailScene->GetView(&ViewFamily, X, Y, Width, Height);
		RenderViewFamily(Canvas, &ViewFamily);
		SkeletalMeshThumbnailScene->SetSkeletalMesh(nullptr);
	}
	else if (UClass* ActorClass = Cast<UClass>(ItemModel))
	{
		Super::Draw(ActorClass->ClassGeneratedBy, X, Y, Width, Height, RenderTarget, Canvas);
	}
}

bool UItemCore_ThumbnailRenderer::CanVisualizeAsset(UObject* Object)
{
	UXD_ItemCoreBlueprint* ItemCoreBlueprint = Cast<UXD_ItemCoreBlueprint>(Object);
	if (ItemCoreBlueprint == nullptr || ItemCoreBlueprint->GeneratedClass == nullptr)
	{
		return false;
	}

	UXD_ItemCoreBase* ItemCore = ItemCoreBlueprint->GeneratedClass->GetDefaultObject<UXD_ItemCoreBase>();
	if (ItemCore == nullptr)
	{
		return false;
	}
	TSoftObjectPtr<UObject> ItemModel = ItemCore->GetCurrentItemModel();
	if (ItemModel.IsNull())
	{
		return false;
	}

	if (UClass* ActorClass = Cast<UClass>(ItemModel.LoadSynchronous()))
	{
		return Super::CanVisualizeAsset(ActorClass->ClassGeneratedBy);
	}
	return true;
}

void UItemCore_ThumbnailRenderer::BeginDestroy()
{
	if (StaticMeshThumbnailScene != nullptr)
	{
		delete StaticMeshThumbnailScene;
		StaticMeshThumbnailScene = nullptr;
	}
	if (SkeletalMeshThumbnailScene != nullptr)
	{
		delete SkeletalMeshThumbnailScene;
		SkeletalMeshThumbnailScene = nullptr;
	}

	Super::BeginDestroy();
}

FText FAssetTypeActions_ItemCore::GetName() const
{
	return LOCTEXT("物品核心", "物品核心");
}

UClass* FAssetTypeActions_ItemCore::GetSupportedClass() const
{
	return UXD_ItemCoreBase::StaticClass();
}

FColor FAssetTypeActions_ItemCore::GetTypeColor() const
{
	return FColor::Blue;
}

uint32 FAssetTypeActions_ItemCore::GetCategories()
{
	return EAssetTypeCategories::Gameplay;
}

class UThumbnailInfo* FAssetTypeActions_ItemCore::GetThumbnailInfo(UObject* Asset) const
{
	if (UXD_ItemCoreBlueprint* ItemCoreBlueprint = Cast<UXD_ItemCoreBlueprint>(Asset))
	{
		if (ItemCoreBlueprint->ThumbnailInfo == nullptr)
		{
			ItemCoreBlueprint->ThumbnailInfo = NewObject<USceneThumbnailInfo>(ItemCoreBlueprint, NAME_None, RF_Transactional);
		}
		return ItemCoreBlueprint->ThumbnailInfo;
	}
	return nullptr;
}

#undef LOCTEXT_NAMESPACE
