// Fill out your copyright notice in the Description page of Project Settings.

#include "XD_ItemType.h"
#include "XD_ItemCoreBase.h"



uint32 GetTypeHash(const FItem& Item)
{
	return GetTypeHash(Item.ItemCore);
}

bool operator==(const FItem& LHS, const FItem& RHS)
{
	if (LHS.ItemCore == RHS.ItemCore)
	{
		return true;
	}
	if (LHS.ItemCore && RHS.ItemCore)
	{
		return LHS.ItemCore->EqualForItemCore(RHS.ItemCore);
	}
	return false;
}

