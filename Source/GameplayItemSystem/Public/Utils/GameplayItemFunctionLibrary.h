// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayItemFunctionLibrary.generated.h"

class UGameplayItemCoreBase;
class AGameplayItemBase;

/**
 * 
 */
UCLASS()
class GAMEPLAYITEMSYSTEM_API UGameplayItemFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "背包")
	static int32 GetItemNumber(const TArray<UGameplayItemCoreBase*>& ItemCoreList, const AGameplayItemBase* Item);

	UFUNCTION(BlueprintPure, Category = "背包")
	static int32 GetItemNumberByCore(const TArray<UGameplayItemCoreBase*>& ItemCoreList, const UGameplayItemCoreBase* ItemCore);

	UFUNCTION(BlueprintPure, Category = "背包")
	static int32 GetItemNumberByType(const TArray<UGameplayItemCoreBase*>& ItemCoreList, TSubclassOf<UGameplayItemCoreBase> ItemClass);

	UFUNCTION(BlueprintPure, Category = "背包")
	static bool ContainItem(const TArray<UGameplayItemCoreBase*>& ItemCoreList, const AGameplayItemBase* Item);

	UFUNCTION(BlueprintPure, Category = "背包")
	static bool ContainItemByCore(const TArray<UGameplayItemCoreBase*>& ItemCoreList, const UGameplayItemCoreBase* ItemCore);

	UFUNCTION(BlueprintPure, Category = "背包")
	static bool ContainItemByType(const TArray<UGameplayItemCoreBase*>& ItemCoreList, TSubclassOf<UGameplayItemCoreBase> ItemClass);

	UFUNCTION(BlueprintPure, Category = "背包", meta = (DeterminesOutputType = "ItemType"))
	static UGameplayItemCoreBase* FindItemByType(const TArray<UGameplayItemCoreBase*>& ItemCoreList, TSubclassOf<UGameplayItemCoreBase> ItemType);

	UFUNCTION(BlueprintPure, Category = "背包", meta = (DeterminesOutputType = "ItemType"))
	static TArray<UGameplayItemCoreBase*> FindItemsByType(const TArray<UGameplayItemCoreBase*>& ItemCoreList, TSubclassOf<UGameplayItemCoreBase> ItemType);

	UFUNCTION(BlueprintPure, Category = "背包")
	static UGameplayItemCoreBase* FindItemByItemCore(const TArray<UGameplayItemCoreBase*>& ItemCoreList, UGameplayItemCoreBase* ItemCore);

private:
	static void AddItemNumberImpl(TArray<UGameplayItemCoreBase*>& ItemCoreList, UGameplayItemCoreBase* AddItemCore, int32 AddNumber, const UObject* Outer);
	static void RemoveItemNumberImpl(TArray<UGameplayItemCoreBase*>& ItemCoreList, UGameplayItemCoreBase* RemoveItemCore, int32 RemoveNumber);
public:
	template<typename T>
	static void AddItemNumber(TArray<T*>& ItemCoreList, UGameplayItemCoreBase* AddItemCore, int32 AddNumber, const UObject* Outer)
	{
		static_assert(TIsDerivedFrom<T, UGameplayItemCoreBase>::IsDerived, "T must be derived from UGameplayItemCoreBase");
		AddItemNumberImpl(ArrayCast<UGameplayItemCoreBase*>(ItemCoreList), AddItemCore, AddNumber, Outer);
	}

	template<typename T>
	static void RemoveItemNumber(TArray<T*>& ItemCoreList, UGameplayItemCoreBase* RemoveItemCore, int32 RemoveNumber)
	{
		static_assert(TIsDerivedFrom<T, UGameplayItemCoreBase>::IsDerived, "T must be derived from UGameplayItemCoreBase");
		RemoveItemNumberImpl(ArrayCast<UGameplayItemCoreBase*>(ItemCoreList), RemoveItemCore, RemoveNumber);
	}
};
