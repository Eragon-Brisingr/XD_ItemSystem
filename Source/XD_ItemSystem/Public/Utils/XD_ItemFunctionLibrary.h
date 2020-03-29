// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XD_ItemFunctionLibrary.generated.h"

class UXD_ItemCoreBase;
class AXD_ItemBase;

/**
 * 
 */
UCLASS()
class XD_ITEMSYSTEM_API UXD_ItemFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "背包")
	static int32 GetItemNumber(const TArray<UXD_ItemCoreBase*>& ItemCoreList, const AXD_ItemBase* Item);

	UFUNCTION(BlueprintPure, Category = "背包")
	static int32 GetItemNumberByCore(const TArray<UXD_ItemCoreBase*>& ItemCoreList, const UXD_ItemCoreBase* ItemCore);

	UFUNCTION(BlueprintPure, Category = "背包")
	static int32 GetItemNumberByType(const TArray<UXD_ItemCoreBase*>& ItemCoreList, TSubclassOf<UXD_ItemCoreBase> ItemClass);

	UFUNCTION(BlueprintPure, Category = "背包")
	static bool ContainItem(const TArray<UXD_ItemCoreBase*>& ItemCoreList, const AXD_ItemBase* Item);

	UFUNCTION(BlueprintPure, Category = "背包")
	static bool ContainItemByCore(const TArray<UXD_ItemCoreBase*>& ItemCoreList, const UXD_ItemCoreBase* ItemCore);

	UFUNCTION(BlueprintPure, Category = "背包")
	static bool ContainItemByType(const TArray<UXD_ItemCoreBase*>& ItemCoreList, TSubclassOf<UXD_ItemCoreBase> ItemClass);

	UFUNCTION(BlueprintPure, Category = "背包", meta = (DeterminesOutputType = "ItemType"))
	static UXD_ItemCoreBase* FindItemByType(const TArray<UXD_ItemCoreBase*>& ItemCoreList, TSubclassOf<UXD_ItemCoreBase> ItemType);

	UFUNCTION(BlueprintPure, Category = "背包", meta = (DeterminesOutputType = "ItemType"))
	static TArray<UXD_ItemCoreBase*> FindItemsByType(const TArray<UXD_ItemCoreBase*>& ItemCoreList, TSubclassOf<UXD_ItemCoreBase> ItemType);

	UFUNCTION(BlueprintPure, Category = "背包")
	static UXD_ItemCoreBase* FindItemByItemCore(const TArray<UXD_ItemCoreBase*>& ItemCoreList, UXD_ItemCoreBase* ItemCore);

private:
	static void AddItemNumberImpl(TArray<UXD_ItemCoreBase*>& ItemCoreList, UXD_ItemCoreBase* AddItemCore, int32 AddNumber, const UObject* Outer);
	static void RemoveItemNumberImpl(TArray<UXD_ItemCoreBase*>& ItemCoreList, UXD_ItemCoreBase* RemoveItemCore, int32 RemoveNumber);
public:
	template<typename T>
	static void AddItemNumber(TArray<T*>& ItemCoreList, UXD_ItemCoreBase* AddItemCore, int32 AddNumber, const UObject* Outer)
	{
		static_assert(TIsDerivedFrom<T, UXD_ItemCoreBase>::IsDerived, "T must be derived from UXD_ItemCoreBase");
		AddItemNumberImpl(ArrayCast<UXD_ItemCoreBase*>(ItemCoreList), AddItemCore, AddNumber, Outer);
	}

	template<typename T>
	static void RemoveItemNumber(TArray<T*>& ItemCoreList, UXD_ItemCoreBase* RemoveItemCore, int32 RemoveNumber)
	{
		static_assert(TIsDerivedFrom<T, UXD_ItemCoreBase>::IsDerived, "T must be derived from UXD_ItemCoreBase");
		RemoveItemNumberImpl(ArrayCast<UXD_ItemCoreBase*>(ItemCoreList), RemoveItemCore, RemoveNumber);
	}
};
