// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <SubclassOf.h>
#include "XD_ItemType.generated.h"

class AXD_ItemBase;
class UXD_ItemCoreBase;

/**
 * 
 */

USTRUCT(BlueprintType)
struct XD_ITEMSYSTEM_API FXD_Item
{
	GENERATED_BODY()
public:
	FXD_Item(UXD_ItemCoreBase* ItemCore = nullptr);

	explicit operator bool() const;

	operator UXD_ItemCoreBase*() const { return ItemCore; }

	UXD_ItemCoreBase* operator->() const { return ItemCore; }

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditDefaultsOnly, Category = "Item", meta = (DisplayName = "显示数量"))
	uint8 bShowNumber : 1;

	UPROPERTY(EditDefaultsOnly, Category = "Item", meta = (DisplayName = "显示道具类型"))
	TSubclassOf<AXD_ItemBase> ShowItemType;

	UPROPERTY(EditAnywhere, Category = "Item", meta = (DisplayName = "道具类型"))
	TSubclassOf<AXD_ItemBase> ItemClass;
#endif
	UPROPERTY(VisibleAnywhere, Instanced, BlueprintReadOnly, Category = "Item", meta = (DisplayName = "道具详情"), SaveGame)
	UXD_ItemCoreBase* ItemCore;

	friend uint32 GetTypeHash(const FXD_Item& Item) { return GetTypeHash((UObject*)Item.ItemCore); }

	friend bool operator==(const FXD_Item& LHS, const FXD_Item& RHS);
};
