// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "GameplayItemFactory.generated.h"

class UGameplayItemCoreBase;
class AGameplayItemBase;

/**
 *
 */
UCLASS()
class GAMEPLAYITEMSYSTEM_EDITOR_API UGameplayItemCoreFactory : public UFactory
{
	GENERATED_BODY()
public:
	UGameplayItemCoreFactory();

	UPROPERTY(EditAnywhere, Category = "道具")
	TSubclassOf<UGameplayItemCoreBase> ItemCoreClass;

	FText GetDisplayName() const override;
	FText GetToolTip() const override;
	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ConfigureProperties() override;
};

UCLASS()
class GAMEPLAYITEMSYSTEM_EDITOR_API UItemEntityFactory : public UFactory
{
	GENERATED_BODY()
public:
	UItemEntityFactory();

	UPROPERTY(EditAnywhere, Category = "道具")
	TSubclassOf<UGameplayItemCoreBase> ItemCoreClass;

	bool CanCreateNew() const override;
	FText GetDisplayName() const override;
	FText GetToolTip() const override;
	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};
