// Fill out your copyright notice in the Description page of Project Settings.

#include "Bluprint/XD_ItemCoreBlueprint.h"

#include "Abstract/XD_ItemCoreBase.h"

#if WITH_EDITOR
UClass* UXD_ItemCoreBlueprint::GetBlueprintClass() const
{
	return UXD_ItemCoreGenerateClass::StaticClass();
}

void UXD_ItemCoreBlueprint::GetReparentingRules(TSet<const UClass*>& AllowedChildrenOfClasses, TSet<const UClass*>& DisallowedChildrenOfClasses) const
{
	AllowedChildrenOfClasses.Add(UXD_ItemCoreBase::StaticClass());
}

#endif
