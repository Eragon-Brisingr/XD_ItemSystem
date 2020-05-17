// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <Engine/Blueprint.h>
#include <Engine/BlueprintGeneratedClass.h>
#include "GameplayItemEntityBlueprint.generated.h"

/**
 * 
 */
UCLASS()
class GAMEPLAYITEMSYSTEM_API UGameplayItemEntityBlueprint : public UBlueprint
{
	GENERATED_BODY()
public:
#if WITH_EDITOR
	UClass* GetBlueprintClass() const override;
	void GetReparentingRules(TSet<const UClass*>& AllowedChildrenOfClasses, TSet<const UClass*>& DisallowedChildrenOfClasses) const override;
#endif
};

UCLASS()
class GAMEPLAYITEMSYSTEM_API UGameplayItemEntityGenerateClass : public UBlueprintGeneratedClass
{
	GENERATED_BODY()
public:

};

