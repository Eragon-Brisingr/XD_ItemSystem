// Fill out your copyright notice in the Description page of Project Settings.

#include "XD_ItemCoreBase.h"
#include "XD_ItemBase.h"
#include "XD_InventoryComponentBase.h"
#include "XD_ObjectFunctionLibrary.h"
#include <UnrealNetwork.h>


bool UXD_ItemCoreBase::IsSupportedForNetworking() const
{
	return true;
}

void UXD_ItemCoreBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	if (UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(GetClass()))
	{
		BPClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
	}

	DOREPLIFETIME(UXD_ItemCoreBase, ItemClass);
	DOREPLIFETIME(UXD_ItemCoreBase, Number);
}

void UXD_ItemCoreBase::PostInitProperties()
{
	Super::PostInitProperties();
}

#if WITH_EDITOR
void UXD_ItemCoreBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UXD_ItemCoreBase, Number))
	{
		if (AXD_ItemBase* Item = Cast<AXD_ItemBase>(GetOuter()))
		{
			if (!Item->CanCompositeItem())
			{
				Number = 1;
			}
			Item->InitRootMesh();
		}
	}
}
#endif //WITH_EDITOR

void UXD_ItemCoreBase::OnRep_Number(int32 PreNumber)
{
	if (OwingInventory)
	{
		if (PreNumber < Number)
		{
			OwingInventory->OnAddItem.Broadcast(this, Number - PreNumber, Number);
		}
		else
		{
			OwingInventory->OnRemoveItem.Broadcast(this, PreNumber - Number, Number);
		}
	}
}

class AXD_ItemBase* UXD_ItemCoreBase::GetItemDefaultActor() const
{
	return GetItemDefalutActorImpl<AXD_ItemBase>();
}

AActor* UXD_ItemCoreBase::GetOwner() const
{
	return OwingInventory ? OwingInventory->GetOwner() : nullptr;
}

class AXD_ItemBase* UXD_ItemCoreBase::SpawnItemActorInLevel(ULevel* OuterLevel, const FVector& Location /*= FVector::ZeroVector*/, const FRotator& Rotation /*= FRotator::ZeroRotator*/, ESpawnActorCollisionHandlingMethod CollisionHandling /*= ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn*/)
{
	if (OuterLevel)
	{
		FActorSpawnParameters ActorSpawnParameters;
		ActorSpawnParameters.OverrideLevel = OuterLevel;
		ActorSpawnParameters.SpawnCollisionHandlingOverride = CollisionHandling;
		if (AXD_ItemBase* SpawnedItem = OuterLevel->GetWorld()->SpawnActor<AXD_ItemBase>(ItemClass, Location, Rotation, ActorSpawnParameters))
		{
			return SettingSpawnedItem(SpawnedItem);
		}
	}
	return nullptr;
}

class AXD_ItemBase* UXD_ItemCoreBase::SpawnItemActorForOwner(AActor* Owner, APawn* Instigator, const FVector& Location /*= FVector::ZeroVector*/, const FRotator& Rotation /*= FRotator::ZeroRotator*/, ESpawnActorCollisionHandlingMethod CollisionHandling /*= ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn*/)
{
	if (Owner)
	{
		FActorSpawnParameters ActorSpawnParameters;
		ActorSpawnParameters.Owner = Owner;
		ActorSpawnParameters.Instigator = Instigator;
		ActorSpawnParameters.OverrideLevel = Owner->GetLevel();
		ActorSpawnParameters.SpawnCollisionHandlingOverride = CollisionHandling;
		if (AXD_ItemBase* SpawnedItem = Owner->GetWorld()->SpawnActor<AXD_ItemBase>(ItemClass, Location, Rotation, ActorSpawnParameters))
		{
			return SettingSpawnedItem(SpawnedItem);
		}
	}
	return nullptr;
}

class AXD_ItemBase* UXD_ItemCoreBase::SettingSpawnedItem(class AXD_ItemBase* Item) const
{
	Item->InnerItemCore = UXD_ObjectFunctionLibrary::DuplicateObject(this, Item);
	if (Item->CanCompositeItem() == false)
	{
		Item->InnerItemCore->Number = 1;
	}
	return Item;
}

bool UXD_ItemCoreBase::EqualForItemCore_Implementation(const UXD_ItemCoreBase* ItemCore) const
{
	if (this == ItemCore)
	{
		return true;
	}
	if (this && ItemCore)
	{
		if (GetClass() == ItemCore->GetClass())
		{
			return ItemClass == ItemCore->ItemClass;
		}
	}
	return false;
}

void UXD_ItemCoreBase::UseItem(class APawn* ItemOwner, EUseItemInput UseItemInput)
{
	GetItemDefaultActor()->UseItemImpl(this, ItemOwner, UseItemInput);
}

FText UXD_ItemCoreBase::GetItemName() const
{
	return GetItemDefaultActor()->GetItemNameImpl(this);
}

float UXD_ItemCoreBase::GetWeight() const
{
	return GetItemDefaultActor()->GetWeightImpl(this);
}

float UXD_ItemCoreBase::GetPrice() const
{
	return GetItemDefaultActor()->GetPriceImpl(this);
}

FText UXD_ItemCoreBase::GetItemTypeDesc() const
{
	return GetItemDefaultActor()->GetItemTypeDescImpl(this);
}

float UXD_ItemCoreBase::GetTradePrice(class APawn* Role, class UXD_InventoryComponentBase* Trader, bool IsBuy) const
{
	return GetPrice() * 1.f;
}

FText UXD_ItemCoreBase::GetDescribe() const
{
	return ItemClass ? GetItemDefaultActor()->Describe : FText::GetEmpty();
}

void UXD_ItemCoreBase::BeThrowed(AActor* WhoThrowed, int32 RemoveNumber, ULevel* ThrowLevel)
{
	GetItemDefaultActor()->BeThrowedImpl(WhoThrowed, this, RemoveNumber, ThrowLevel);
}

void UXD_ItemCoreBase::WhenRemoveFromInventory(class AActor* ItemOwner, int32 RemoveNumber, int32 ExistNumber)
{
	GetItemDefaultActor()->WhenRemoveFromInventory(ItemOwner, this, RemoveNumber, ExistNumber);
}

