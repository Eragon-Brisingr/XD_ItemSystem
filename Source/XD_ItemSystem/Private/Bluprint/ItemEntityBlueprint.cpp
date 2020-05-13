// Fill out your copyright notice in the Description page of Project Settings.


#include "Bluprint/ItemEntityBlueprint.h"

#include "Abstract/XD_ItemBase.h"

void UItemEntityBlueprint::GetReparentingRules(TSet<const UClass*>& AllowedChildrenOfClasses, TSet<const UClass*>& DisallowedChildrenOfClasses) const
{
	AllowedChildrenOfClasses.Add(AXD_ItemBase::StaticClass());
}
