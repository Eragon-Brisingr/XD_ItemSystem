// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XD_PropertyCustomizationEx.h"

class AXD_ItemBase;

/**
 * 
 */
class XD_ITEMSYSTEM_EDITOR_API FXD_Item_Customization : public IPropertyTypeCustomizationMakeInstanceable<FXD_Item_Customization>
{
public:
	FXD_Item_Customization();

	/** IPropertyTypeCustomization interface */
	void CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	void CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

	TSubclassOf<AXD_ItemBase> BaseItemClass;
private:
	class UXD_ItemCoreBase* GetItemCore(TSharedPtr<IPropertyHandle> ItemCore_PropertyHandle) const;
};

class XD_ITEMSYSTEM_EDITOR_API FXD_ItemCore_Customization : public IPropertyTypeCustomizationMakeInstanceable<FXD_ItemCore_Customization>
{
public:
	/** IPropertyTypeCustomization interface */
	void CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	void CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
};