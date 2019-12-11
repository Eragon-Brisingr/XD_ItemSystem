// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/XD_ItemFunctionLibrary.h"
#include "Abstract/XD_ItemBase.h"
#include "Abstract/XD_ItemCoreBase.h"

int32 UXD_ItemFunctionLibrary::GetItemNumber(const TArray<UXD_ItemCoreBase*>& ItemCoreList, const AXD_ItemBase* Item)
{
	return Item ? GetItemNumberByCore(ItemCoreList, Item->GetItemCoreConst()) : 0;
}

int32 UXD_ItemFunctionLibrary::GetItemNumberByCore(const TArray<UXD_ItemCoreBase*>& ItemCoreList, const UXD_ItemCoreBase* ItemCore)
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
			for (UXD_ItemCoreBase* ItemCoreElement : ItemCoreList)
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

int32 UXD_ItemFunctionLibrary::GetItemNumberByType(const TArray<UXD_ItemCoreBase*>& ItemCoreList, TSubclassOf<UXD_ItemCoreBase> ItemClass)
{
	int32 Number = 0;
	for (UXD_ItemCoreBase* ElementItem : ItemCoreList)
	{
		if (ElementItem->IsA(ItemClass))
		{
			Number += ElementItem->Number;
		}
	}
	return Number;
}

bool UXD_ItemFunctionLibrary::ContainItem(const TArray<UXD_ItemCoreBase*>& ItemCoreList, const AXD_ItemBase* Item)
{
	return GetItemNumber(ItemCoreList, Item) > 0;
}

bool UXD_ItemFunctionLibrary::ContainItemByCore(const TArray<UXD_ItemCoreBase*>& ItemCoreList, const UXD_ItemCoreBase* ItemCore)
{
	return GetItemNumberByCore(ItemCoreList, ItemCore) > 0;
}

bool UXD_ItemFunctionLibrary::ContainItemByType(const TArray<UXD_ItemCoreBase*>& ItemCoreList, TSubclassOf<UXD_ItemCoreBase> ItemClass)
{
	return GetItemNumberByType(ItemCoreList, ItemClass) > 0;
}

UXD_ItemCoreBase* UXD_ItemFunctionLibrary::FindItemByType(const TArray<UXD_ItemCoreBase*>& ItemCoreList, TSubclassOf<UXD_ItemCoreBase> ItemType)
{
	UXD_ItemCoreBase* const* p_NeedFindItem = ItemCoreList.FindByPredicate([&](UXD_ItemCoreBase* Item) {return Item->IsA(ItemType); });
	return p_NeedFindItem ? *p_NeedFindItem : nullptr;
}

TArray<UXD_ItemCoreBase*> UXD_ItemFunctionLibrary::FindItemsByType(const TArray<UXD_ItemCoreBase*>& ItemCoreList, TSubclassOf<UXD_ItemCoreBase> ItemType)
{
	TArray<UXD_ItemCoreBase*> Items;
	for (UXD_ItemCoreBase* ItemCore : ItemCoreList)
	{
		if (ItemCore->IsA(ItemType))
		{
			Items.Add(ItemCore);
		}
	}
	return Items;
}

UXD_ItemCoreBase* UXD_ItemFunctionLibrary::FindItemByItemCore(const TArray<UXD_ItemCoreBase*>& ItemCoreList, UXD_ItemCoreBase* ItemCore)
{
	int32 ItemIndex = ItemCoreList.IndexOfByPredicate([ItemCore](auto& ElementItem) {return ElementItem->IsEqualWithItemCore(ItemCore); });
	if (ItemIndex != INDEX_NONE)
	{
		return ItemCoreList[ItemIndex];
	}
	return nullptr;
}

void UXD_ItemFunctionLibrary::AddItemNumberImpl(TArray<UXD_ItemCoreBase*>& ItemCoreList, UXD_ItemCoreBase* AddItemCore, int32 AddNumber, const UObject* Outer)
{
	int32 Idx = ItemCoreList.IndexOfByPredicate([&](UXD_ItemCoreBase* E) {return E->IsEqualWithItemCore(AddItemCore); });
	if (Idx == INDEX_NONE)
	{
		UXD_ItemCoreBase* AddedItemCore = UXD_ItemCoreBase::DeepDuplicateCore(AddItemCore, Outer);
		AddedItemCore->Number = AddNumber;
		ItemCoreList.Add(AddedItemCore);
	}
	else
	{
		ItemCoreList[Idx]->Number += AddNumber;
	}
}

void UXD_ItemFunctionLibrary::RemoveItemNumberImpl(TArray<UXD_ItemCoreBase*>& ItemCoreList, UXD_ItemCoreBase* RemoveItemCore, int32 RemoveNumber)
{
	int32 Idx = ItemCoreList.IndexOfByPredicate([&](UXD_ItemCoreBase* E) {return E->IsEqualWithItemCore(RemoveItemCore); });
	if (Idx != INDEX_NONE)
	{
		ItemCoreList[Idx]->Number += RemoveNumber;
		if (ItemCoreList[Idx]->Number <= 0)
		{
			ItemCoreList.RemoveAt(Idx);
		}
	}
}
