// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XD_PropertyCustomizationEx.h"

/**
 * 
 */
class XD_ITEMSYSTEM_EDITOR_API FXD_Item_Customization : public IPropertyTypeCustomizationMakeInstanceable<FXD_Item_Customization>
{
public:
	/** IPropertyTypeCustomization interface */
	virtual void CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:
	class UXD_ItemCoreBase* GetItemCore(TSharedPtr<IPropertyHandle> ItemCore_PropertyHandle) const;
};
