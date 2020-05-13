// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <Engine/Blueprint.h>
#include "XD_ItemCoreBlueprint.generated.h"

/**
 *
 */

UCLASS()
class XD_ITEMSYSTEM_API UXD_ItemCoreBlueprint : public UBlueprint
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	UClass* GetBlueprintClass() const override;
	void GetReparentingRules(TSet<const UClass*>& AllowedChildrenOfClasses, TSet<const UClass*>& DisallowedChildrenOfClasses) const override;
#endif
};
