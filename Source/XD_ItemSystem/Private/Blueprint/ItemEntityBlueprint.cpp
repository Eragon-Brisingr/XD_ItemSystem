// Fill out your copyright notice in the Description page of Project Settings.


#include "Bluprint/ItemEntityBlueprint.h"

#include "Abstract/XD_ItemCoreBase.h"
#include "Abstract/XD_ItemBase.h"

#if WITH_EDITOR
UClass* UItemEntityBlueprint::GetBlueprintClass() const
{
	return UItemEntityGenerateClass::StaticClass();
}

void UItemEntityBlueprint::GetReparentingRules(TSet<const UClass*>& AllowedChildrenOfClasses, TSet<const UClass*>& DisallowedChildrenOfClasses) const
{
	if (GeneratedClass)
	{
		if (AXD_ItemBase* ItemEntity = Cast<AXD_ItemBase>(GeneratedClass.GetDefaultObject()))
		{
			AllowedChildrenOfClasses.Add(ItemEntity->BelongToCoreType);
		}
	}
}
#endif
