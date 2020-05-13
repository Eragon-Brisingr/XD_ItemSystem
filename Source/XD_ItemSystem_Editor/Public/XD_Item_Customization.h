// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"

/**
 * 
 */
class XD_ITEMSYSTEM_EDITOR_API FXD_ItemCoreCustomization : public IPropertyTypeCustomization
{
	// 特殊Meta：
	// ConfigUseItem[配置用道具]，为true一直显示数量
	void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShareable(new FXD_ItemCoreCustomization); }
};


class XD_ITEMSYSTEM_EDITOR_API FXD_ItemModelDataCustomization : public IPropertyTypeCustomization
{
	void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShareable(new FXD_ItemModelDataCustomization); }
};
