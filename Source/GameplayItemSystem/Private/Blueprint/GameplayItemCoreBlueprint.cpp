// Fill out your copyright notice in the Description page of Project Settings.

#include "Bluprint/GameplayItemCoreBlueprint.h"

#include "Abstract/GameplayItemCoreBase.h"

#if WITH_EDITOR
UClass* UGameplayItemCoreBlueprint::GetBlueprintClass() const
{
	return UGameplayItemCoreGenerateClass::StaticClass();
}

void UGameplayItemCoreBlueprint::GetReparentingRules(TSet<const UClass*>& AllowedChildrenOfClasses, TSet<const UClass*>& DisallowedChildrenOfClasses) const
{
	AllowedChildrenOfClasses.Add(UGameplayItemCoreBase::StaticClass());
}

#endif
