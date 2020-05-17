// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/GameplayItemFunctionLibrary.h"
#include "Abstract/GameplayItemBase.h"
#include "Abstract/GameplayItemCoreBase.h"

int32 UGameplayItemFunctionLibrary::GetItemNumber(const TArray<UGameplayItemCoreBase*>& ItemCoreList, const AGameplayItemBase* Item)
{
	return Item ? GetItemNumberByCore(ItemCoreList, Item->GetItemCoreConst()) : 0;
}

int32 UGameplayItemFunctionLibrary::GetItemNumberByCore(const TArray<UGameplayItemCoreBase*>& ItemCoreList, const UGameplayItemCoreBase* ItemCore)
{
	if (ItemCore)
	{
		if (ItemCore->CanCompositeInInventory())
		{
			if (auto FindItem = ItemCoreList.FindByPredicate([ItemCore](auto& ElementItem) {return ElementItem->IsEqualWithItemCore(ItemCore); }))
			{
				return (*FindItem)->Number;
			}
		}
		else
		{
			int Number = 0;
			for (UGameplayItemCoreBase* ItemCoreElement : ItemCoreList)
			{
				if (ItemCoreElement->IsEqualWithItemCore(ItemCore))
				{
					Number += 1;
				}
			}
			return Number;
		}
	}
	return 0;
}

int32 UGameplayItemFunctionLibrary::GetItemNumberByType(const TArray<UGameplayItemCoreBase*>& ItemCoreList, TSubclassOf<UGameplayItemCoreBase> ItemClass)
{
	int32 Number = 0;
	for (UGameplayItemCoreBase* ElementItem : ItemCoreList)
	{
		if (ElementItem->IsA(ItemClass))
		{
			Number += ElementItem->Number;
		}
	}
	return Number;
}

bool UGameplayItemFunctionLibrary::ContainItem(const TArray<UGameplayItemCoreBase*>& ItemCoreList, const AGameplayItemBase* Item)
{
	return GetItemNumber(ItemCoreList, Item) > 0;
}

bool UGameplayItemFunctionLibrary::ContainItemByCore(const TArray<UGameplayItemCoreBase*>& ItemCoreList, const UGameplayItemCoreBase* ItemCore)
{
	return GetItemNumberByCore(ItemCoreList, ItemCore) > 0;
}

bool UGameplayItemFunctionLibrary::ContainItemByType(const TArray<UGameplayItemCoreBase*>& ItemCoreList, TSubclassOf<UGameplayItemCoreBase> ItemClass)
{
	return GetItemNumberByType(ItemCoreList, ItemClass) > 0;
}

UGameplayItemCoreBase* UGameplayItemFunctionLibrary::FindItemByType(const TArray<UGameplayItemCoreBase*>& ItemCoreList, TSubclassOf<UGameplayItemCoreBase> ItemType)
{
	UGameplayItemCoreBase* const* p_NeedFindItem = ItemCoreList.FindByPredicate([&](UGameplayItemCoreBase* Item) {return Item->IsA(ItemType); });
	return p_NeedFindItem ? *p_NeedFindItem : nullptr;
}

TArray<UGameplayItemCoreBase*> UGameplayItemFunctionLibrary::FindItemsByType(const TArray<UGameplayItemCoreBase*>& ItemCoreList, TSubclassOf<UGameplayItemCoreBase> ItemType)
{
	TArray<UGameplayItemCoreBase*> Items;
	for (UGameplayItemCoreBase* ItemCore : ItemCoreList)
	{
		if (ItemCore->IsA(ItemType))
		{
			Items.Add(ItemCore);
		}
	}
	return Items;
}

UGameplayItemCoreBase* UGameplayItemFunctionLibrary::FindItemByItemCore(const TArray<UGameplayItemCoreBase*>& ItemCoreList, UGameplayItemCoreBase* ItemCore)
{
	int32 ItemIndex = ItemCoreList.IndexOfByPredicate([ItemCore](auto& ElementItem) {return ElementItem->IsEqualWithItemCore(ItemCore); });
	if (ItemIndex != INDEX_NONE)
	{
		return ItemCoreList[ItemIndex];
	}
	return nullptr;
}

void UGameplayItemFunctionLibrary::AddItemNumberImpl(TArray<UGameplayItemCoreBase*>& ItemCoreList, UGameplayItemCoreBase* AddItemCore, int32 AddNumber, const UObject* Outer)
{
	int32 Idx = ItemCoreList.IndexOfByPredicate([&](UGameplayItemCoreBase* E) {return E->IsEqualWithItemCore(AddItemCore); });
	if (Idx == INDEX_NONE)
	{
		UGameplayItemCoreBase* AddedItemCore = UGameplayItemCoreBase::DeepDuplicateCore(AddItemCore, Outer);
		AddedItemCore->Number = AddNumber;
		ItemCoreList.Add(AddedItemCore);
	}
	else
	{
		ItemCoreList[Idx]->Number += AddNumber;
	}
}

void UGameplayItemFunctionLibrary::RemoveItemNumberImpl(TArray<UGameplayItemCoreBase*>& ItemCoreList, UGameplayItemCoreBase* RemoveItemCore, int32 RemoveNumber)
{
	int32 Idx = ItemCoreList.IndexOfByPredicate([&](UGameplayItemCoreBase* E) {return E->IsEqualWithItemCore(RemoveItemCore); });
	if (Idx != INDEX_NONE)
	{
		ItemCoreList[Idx]->Number += RemoveNumber;
		if (ItemCoreList[Idx]->Number <= 0)
		{
			ItemCoreList.RemoveAt(Idx);
		}
	}
}
