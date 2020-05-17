// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <Engine/Blueprint.h>
#include <Engine/BlueprintGeneratedClass.h>
#include "GameplayItemCoreBlueprint.generated.h"

/**
 *
 */

UCLASS()
class GAMEPLAYITEMSYSTEM_API UGameplayItemCoreBlueprint : public UBlueprint
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	UClass* GetBlueprintClass() const override;
	void GetReparentingRules(TSet<const UClass*>& AllowedChildrenOfClasses, TSet<const UClass*>& DisallowedChildrenOfClasses) const override;
#endif
};

UCLASS()
class GAMEPLAYITEMSYSTEM_API UGameplayItemCoreGenerateClass : public UBlueprintGeneratedClass
{
	GENERATED_BODY()
public:

};
