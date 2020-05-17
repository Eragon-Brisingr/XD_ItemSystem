// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"
#include "IDetailCustomization.h"

/**
 * 
 */
class GAMEPLAYITEMSYSTEM_EDITOR_API FGameplayItemCoreCustomization : public IPropertyTypeCustomization
{
	// 特殊Meta：
	// ConfigUseItem[配置用道具]，为true一直显示数量
	void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShareable(new FGameplayItemCoreCustomization); }
};


class GAMEPLAYITEMSYSTEM_EDITOR_API FGameplayItemModelDataCustomization : public IPropertyTypeCustomization
{
	void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShareable(new FGameplayItemModelDataCustomization); }
};

class GAMEPLAYITEMSYSTEM_EDITOR_API FGameplayItemCoreDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance() { return MakeShareable(new FGameplayItemCoreDetails()); }
	void CustomizeDetails(class IDetailLayoutBuilder& DetailBuilder) override;
};
