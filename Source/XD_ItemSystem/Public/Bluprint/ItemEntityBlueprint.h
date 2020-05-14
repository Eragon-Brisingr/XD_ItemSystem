// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <Engine/Blueprint.h>
#include <Engine/BlueprintGeneratedClass.h>
#include "ItemEntityBlueprint.generated.h"

/**
 * 
 */
UCLASS()
class XD_ITEMSYSTEM_API UItemEntityBlueprint : public UBlueprint
{
	GENERATED_BODY()
public:
#if WITH_EDITOR
	UClass* GetBlueprintClass() const override;
	void GetReparentingRules(TSet<const UClass*>& AllowedChildrenOfClasses, TSet<const UClass*>& DisallowedChildrenOfClasses) const override;
#endif
};

UCLASS()
class XD_ITEMSYSTEM_API UItemEntityGenerateClass : public UBlueprintGeneratedClass
{
	GENERATED_BODY()
public:

};

