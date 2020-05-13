// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "XD_ItemFactory.generated.h"

class UXD_ItemCoreBase;
class AXD_ItemBase;

/**
 *
 */
UCLASS()
class XD_ITEMSYSTEM_EDITOR_API UXD_ItemCoreFactory : public UFactory
{
	GENERATED_BODY()
public:
	UXD_ItemCoreFactory();

	UPROPERTY(EditAnywhere, Category = "道具")
	TSubclassOf<UXD_ItemCoreBase> ItemCoreClass;

	FText GetDisplayName() const override;
	FText GetToolTip() const override;

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ConfigureProperties() override;
};

UCLASS()
class XD_ITEMSYSTEM_EDITOR_API UItemEntityFactory : public UFactory
{
	GENERATED_BODY()
public:
	UItemEntityFactory();

	UPROPERTY(EditAnywhere, Category = "道具")
	TSubclassOf<AXD_ItemBase> ItemEntityClass;

	FText GetDisplayName() const override;
	FText GetToolTip() const override;
	uint32 GetMenuCategories() const override;
	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ConfigureProperties() override;
};
