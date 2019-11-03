// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "XD_ItemCoreFactory.generated.h"

class UXD_ItemCoreBase;

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

	virtual FText GetDisplayName() const override;
	virtual FText GetToolTip() const override;

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ConfigureProperties() override;
};
