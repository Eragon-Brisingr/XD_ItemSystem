// Fill out your copyright notice in the Description page of Project Settings.

#include "XD_ItemCoreBase.h"
#include "XD_ItemBase.h"
#include "XD_InventoryComponentBase.h"
#include "XD_ObjectFunctionLibrary.h"
#include <UnrealNetwork.h>
#include "XD_ItemSystemUtility.h"
#include "Engine/World.h"


UXD_ItemCoreBase::UXD_ItemCoreBase()
	:ItemClass(nullptr)
{
	
}

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

AActor* UXD_ItemCoreBase::GetOwner() const
{
	return OwingInventory ? OwingInventory->GetOwner() : nullptr;
}

class AXD_ItemBase* UXD_ItemCoreBase::SpawnItemActorInLevel(ULevel* OuterLevel, int32 ItemNumber /*= 1*/, const FVector& Location /*= FVector::ZeroVector*/, const FRotator& Rotation /*= FRotator::ZeroRotator*/, ESpawnActorCollisionHandlingMethod CollisionHandling /*= ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn*/) const
{
	if (OuterLevel)
	{
		FActorSpawnParameters ActorSpawnParameters;
		ActorSpawnParameters.bDeferConstruction = true;
		ActorSpawnParameters.OverrideLevel = OuterLevel;
		ActorSpawnParameters.SpawnCollisionHandlingOverride = CollisionHandling;
		if (AXD_ItemBase* SpawnedItem = OuterLevel->GetWorld()->SpawnActor<AXD_ItemBase>(GetItemClass(), ActorSpawnParameters))
		{
			SettingSpawnedItem(SpawnedItem, ItemNumber);
			SpawnedItem->FinishSpawning(FTransform(Rotation, Location));
			return SpawnedItem;
		}
	}
	return nullptr;
}

class AXD_ItemBase* UXD_ItemCoreBase::SpawnItemActorForOwner(AActor* Owner, APawn* Instigator, int32 ItemNumber /*= 1*/, const FVector& Location /*= FVector::ZeroVector*/, const FRotator& Rotation /*= FRotator::ZeroRotator*/, ESpawnActorCollisionHandlingMethod CollisionHandling /*= ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn*/) const
{
	if (Owner)
	{
		FActorSpawnParameters ActorSpawnParameters;
		ActorSpawnParameters.bDeferConstruction = true;
		ActorSpawnParameters.Owner = Owner;
		ActorSpawnParameters.Instigator = Instigator;
		ActorSpawnParameters.OverrideLevel = Owner->GetLevel();
		ActorSpawnParameters.SpawnCollisionHandlingOverride = CollisionHandling;
		if (AXD_ItemBase* SpawnedItem = Owner->GetWorld()->SpawnActor<AXD_ItemBase>(GetItemClass(), ActorSpawnParameters))
		{
			SettingSpawnedItem(SpawnedItem, ItemNumber);
			SpawnedItem->FinishSpawning(FTransform(Rotation, Location));
			return SpawnedItem;
		}
	}
	return nullptr;
}

UXD_ItemCoreBase* UXD_ItemCoreBase::DeepDuplicateCore(const UObject* Outer) const
{
	return UXD_ObjectFunctionLibrary::DuplicateObject(this, Outer);
}

AXD_ItemBase* UXD_ItemCoreBase::SpawnPreviewItemActor(const UObject* WorldContextObject)
{
	FActorSpawnParameters ActorSpawnParameters;
	ActorSpawnParameters.bDeferConstruction = true;
	ActorSpawnParameters.ObjectFlags = RF_Transient;
	ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AXD_ItemBase* SpawnedItem = WorldContextObject->GetWorld()->SpawnActor<AXD_ItemBase>(GetItemClass(), ActorSpawnParameters);
	if (SpawnedItem)
	{
		SettingSpawnedItem(SpawnedItem, Number);
		SpawnedItem->SetReplicates(false);
		SpawnedItem->FinishSpawning(FTransform::Identity);
		SpawnedItem->SetItemSimulatePhysics(false);
		return SpawnedItem;
	}
	return nullptr;
}

void UXD_ItemCoreBase::SettingSpawnedItem(class AXD_ItemBase* Item, int32 ThrowNumber) const
{
	Item->InnerItemCore = UXD_ItemCoreBase::DeepDuplicateCore(this, Item);
	Item->InnerItemCore->Number = Item->CanCompositeItem() ? ThrowNumber : 1;

	if (Number < Item->MinItemCompositeNumber && Number != 1)
	{
		Item->InnerItemCore->Number = 1;
		ItemSystem_Warning_LOG("SpawnItemActor : 无法叠加%s，申请道具数量%d，最小叠加数量%d，设置为1", *UXD_ObjectFunctionLibrary::GetClassName(GetItemClass()), ThrowNumber, Item->MinItemCompositeNumber);
	}
}

bool UXD_ItemCoreBase::CanCompositeInInventory() const
{
	return GetItemClass().GetDefaultObject()->bCanCompositeInInventory;
}

bool UXD_ItemCoreBase::IsEqualWithItemCore(const UXD_ItemCoreBase* ItemCore) const
{
	if (this == ItemCore)
	{
		return true;
	}
	if (this && ItemCore)
	{
		return GetClass() == ItemCore->GetClass() 
				&& GetItemClass() == ItemCore->GetItemClass() 
				&& RecevieIsEqualWithItemCore(ItemCore);
	}
	return false;
}

FText UXD_ItemCoreBase::GetItemName() const
{
	return GetItemDefaultActor()->GetItemNameImpl(this);
}

void UXD_ItemCoreBase::WhenThrow(AActor* WhoThrowed, int32 RemoveNumber, ULevel* ThrowLevel)
{
	GetItemDefaultActor()->WhenThrow(WhoThrowed, this, RemoveNumber, ThrowLevel);
}

void UXD_ItemCoreBase::WhenRemoveFromInventory(class AActor* ItemOwner, int32 RemoveNumber, int32 ExistNumber)
{
	GetItemDefaultActor()->WhenRemoveFromInventory(ItemOwner, this, RemoveNumber, ExistNumber);
}

