﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XD_ItemType.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FXD_Item
{
	GENERATED_BODY()
public:
	FXD_Item()
	{
#if WITH_EDITORONLY_DATA
		bShowNumber = true;
#endif
	}
	FXD_Item(class UXD_ItemCoreBase* ItemCore) 
		:ItemCore(ItemCore)
	{
#if WITH_EDITORONLY_DATA
		bShowNumber = true;
#endif
	}

	explicit operator bool() const { return ItemCore ? true : false; }

	operator class UXD_ItemCoreBase*() const { return ItemCore; }

	UXD_ItemCoreBase* operator->() const { return ItemCore; }

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditDefaultsOnly, Category = "Item", meta = (DisplayName = "显示数量"))
	uint8 bShowNumber : 1;

	UPROPERTY(EditDefaultsOnly, Category = "Item", meta = (DisplayName = "显示道具类型"))
	TSubclassOf<class AXD_ItemBase> ShowItemType;

	UPROPERTY(EditAnywhere, Category = "Item", meta = (DisplayName = "道具类型"))
	TSubclassOf<class AXD_ItemBase> ItemClass;
#endif
	UPROPERTY(VisibleAnywhere, Instanced, BlueprintReadOnly, Category = "Item", meta = (DisplayName = "道具详情"))
	class UXD_ItemCoreBase* ItemCore;

	friend uint32 GetTypeHash(const FXD_Item& Item);

	friend bool operator==(const FXD_Item& LHS, const FXD_Item& RHS);
};

UENUM(BlueprintType)
enum class EUseItemInput :uint8
{
	LeftMouse,
	RightMouse
};