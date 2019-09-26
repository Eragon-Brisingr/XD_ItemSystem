﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"
#include "ThumbnailRendering/BlueprintThumbnailRenderer.h"
#include "AssetTypeActions_ItemCore.generated.h"

/**
 * 
 */
class FCanvas;
class FRenderTarget;

UCLASS(CustomConstructor, Config = Editor)
class UItemCore_ThumbnailRenderer : public UBlueprintThumbnailRenderer
{
	GENERATED_BODY()
public:
	UItemCore_ThumbnailRenderer(const FObjectInitializer& ObjectInitializer);

	// UThumbnailRenderer implementation
	void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* RenderTarget, FCanvas* Canvas) override;
	bool CanVisualizeAsset(UObject* Object) override;

	void BeginDestroy() override;
private:
	class FStaticMeshThumbnailScene* StaticMeshThumbnailScene;
	class FSkeletalMeshThumbnailScene* SkeletalMeshThumbnailScene;
};


class XD_ITEMSYSTEM_EDITOR_API FAssetTypeActions_ItemCore : public FAssetTypeActions_Base
{
public:
	using Super = FAssetTypeActions_Base;

	// Inherited via FAssetTypeActions_Base
	FText GetName() const override;
	UClass* GetSupportedClass() const override;
	FColor GetTypeColor() const override;
	uint32 GetCategories() override;
	class UThumbnailInfo* GetThumbnailInfo(UObject* Asset) const override;
};