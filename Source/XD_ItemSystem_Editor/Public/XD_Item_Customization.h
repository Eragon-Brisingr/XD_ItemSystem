// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XD_PropertyCustomizationEx.h"

/**
 * 
 */
class XD_ITEMSYSTEM_EDITOR_API FXD_ItemCoreCustomization : public IPropertyTypeCustomizationMakeInstanceable<FXD_ItemCoreCustomization>
{
	// 特殊Meta：
	// ConfigUseItem[配置用道具]，为true一直显示数量

	void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
};
