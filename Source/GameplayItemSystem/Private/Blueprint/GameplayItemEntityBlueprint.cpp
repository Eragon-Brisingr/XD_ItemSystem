// Fill out your copyright notice in the Description page of Project Settings.


#include "Bluprint/GameplayItemEntityBlueprint.h"

#include "Abstract/GameplayItemCoreBase.h"
#include "Abstract/GameplayItemBase.h"

#if WITH_EDITOR
UClass* UGameplayItemEntityBlueprint::GetBlueprintClass() const
{
	return UGameplayItemEntityGenerateClass::StaticClass();
}

void UGameplayItemEntityBlueprint::GetReparentingRules(TSet<const UClass*>& AllowedChildrenOfClasses, TSet<const UClass*>& DisallowedChildrenOfClasses) const
{
	if (GeneratedClass)
	{
		if (AGameplayItemBase* ItemEntity = Cast<AGameplayItemBase>(GeneratedClass.GetDefaultObject()))
		{
			AllowedChildrenOfClasses.Add(ItemEntity->BelongToCoreType);
		}
	}
}
#endif
