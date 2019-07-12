// Fill out your copyright notice in the Description page of Project Settings.

#include "XD_ItemType.h"
#include "XD_ItemCoreBase.h"
#include "XD_ItemBase.h"


uint32 GetTypeHash(const FXD_Item& Item)
{
	return GetTypeHash(Item.ItemCore);
}

bool operator==(const FXD_Item& LHS, const FXD_Item& RHS)
{
	if (LHS.ItemCore == RHS.ItemCore)
	{
		return true;
	}
	if (LHS.ItemCore && RHS.ItemCore)
	{
		return LHS.ItemCore->IsEqualWithItemCore(RHS.ItemCore);
	}
	return false;
}

FXD_Item::FXD_Item(UXD_ItemCoreBase* ItemCore): ItemCore(ItemCore)
{
#if WITH_EDITORONLY_DATA
	bShowNumber = true;
	ShowItemType = AXD_ItemBase::StaticClass();
#endif
}

FXD_Item::operator bool() const
{
#if WITH_EDITOR
	return ItemCore ? ItemCore->GetItemClass() == ItemClass : false;
#endif
	return ItemCore ? true : false;
}
