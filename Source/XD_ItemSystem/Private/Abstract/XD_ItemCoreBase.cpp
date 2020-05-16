// Fill out your copyright notice in the Description page of Project Settings.

#include "Abstract/XD_ItemCoreBase.h"
#include "Abstract/XD_ItemBase.h"
#include <Net/UnrealNetwork.h>
#include <Engine/World.h>
#include <Engine/BlueprintGeneratedClass.h>
#include <Engine/StaticMesh.h>
#include <Engine/SkeletalMesh.h>

#include "Inventory/XD_InventoryComponentBase.h"
#include "XD_ItemSystemUtility.h"

#define LOCTEXT_NAMESPACE "物品" 

#if WITH_EDITOR
uint8 FSpawnPreviewItemScope::Counter;

void FXD_ItemModelData::UpdateModelType()
{
	if (UObject* ModelObj = Model.LoadSynchronous())
	{
		if (ModelObj->IsA<UStaticMesh>())
		{
			ModelType = EItemModelType::StaticMesh;
		}
		else if (ModelObj->IsA<USkeletalMesh>())
		{
			ModelType = EItemModelType::SkeletalMesh;
		}
		else if (UClass* ModelClass = Cast<UClass>(ModelObj))
		{
			check(ModelClass->IsChildOf<AActor>());
			ModelType = EItemModelType::Actor;
		}
		else
		{
			checkNoEntry();
		}
	}
	else
	{
		ModelType = EItemModelType::None;
	}
}
#endif

FXD_ItemCoreSparseData::FXD_ItemCoreSparseData() :
	ItemNameValue(LOCTEXT("物品", "物品")),
	bCanCompositeInInventoryValue(true),
	bCanMergeItemValue(false)
{

}

UXD_ItemCoreBase::UXD_ItemCoreBase(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
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

	DOREPLIFETIME(UXD_ItemCoreBase, Number);
}

UWorld* UXD_ItemCoreBase::GetWorld() const
{
#if WITH_EDITOR
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return nullptr;
	}
#endif
	return GetOuter()->GetWorld();
}

#if WITH_EDITOR
void UXD_ItemCoreBase::PostLoad()
{
	Super::PostLoad();

// 	if (HasAnyFlags(RF_ClassDefaultObject))
// 	{
// 		FXD_ItemCoreSparseData* ItemCoreSparseData = GetXD_ItemCoreSparseData();
// 		ItemCoreSparseData->ItemModelValue.UpdateModelType();
// 		ItemCoreSparseData->MergeItemModelValue.UpdateModelType();
// 	}
}

void UXD_ItemCoreBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UXD_ItemCoreBase, Number))
	{
		UObject* Outer = GetOuter();
		if (Outer && (Outer->IsA<AXD_ItemBase>() || Outer->IsA<UPackage>()))
		{
			if (!CanMergeItem())
			{
				if (ensure(Number == 1) == false)
				{
					Number = 1;
				}
			}
			else
			{
				if (ensure(Number < GetMinItemMergeNumberValue() && Number != 1) == false)
				{
					Number = 1;
				}
			}
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

const FXD_ItemModelData& UXD_ItemCoreBase::GetCurrentItemModel() const
{
	return IsMergedItem() ? GetMergeItemModelValue() : GetItemModelValue();
}

AXD_ItemBase* UXD_ItemCoreBase::SpawnItemActorInLevel(ULevel* OuterLevel, const FVector& Location /*= FVector::ZeroVector*/, const FRotator& Rotation /*= FRotator::ZeroRotator*/, int32 ItemNumber /*= 1*/, const FName& Name /*= NAME_None*/, EObjectFlags InObjectFlags /*= RF_NoFlags*/, ESpawnActorCollisionHandlingMethod CollisionHandling /*= ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn*/) const
{
	if (ensure(OuterLevel))
	{
		FActorSpawnParameters ActorSpawnParameters;
		ActorSpawnParameters.bDeferConstruction = true;
		ActorSpawnParameters.OverrideLevel = OuterLevel;
		ActorSpawnParameters.Name = Name;
		ActorSpawnParameters.ObjectFlags = InObjectFlags;
		ActorSpawnParameters.SpawnCollisionHandlingOverride = CollisionHandling;
#if WITH_EDITOR
		if (FSpawnPreviewItemScope::IsSpawnPreviewItem())
		{
			ActorSpawnParameters.ObjectFlags |= RF_Transient;
			ActorSpawnParameters.bHideFromSceneOutliner = true;
		}
#endif
		if (AXD_ItemBase * SpawnedItem = OuterLevel->GetWorld()->SpawnActor<AXD_ItemBase>(GetSpawnedItemClass(ItemNumber), ActorSpawnParameters))
		{
#if WITH_EDITOR
			if (!ActorSpawnParameters.Name.IsNone())
			{
				SpawnedItem->SetActorLabel(ActorSpawnParameters.Name.ToString(), false);
			}
#endif
			SettingSpawnedItem(SpawnedItem, ItemNumber);
			SpawnedItem->FinishSpawning(FTransform(Rotation, Location));
			return SpawnedItem;
		}
	}
	return nullptr;
}

AXD_ItemBase* UXD_ItemCoreBase::SpawnItemActorForOwner(AActor* Owner, APawn* Instigator, const FVector& Location /*= FVector::ZeroVector*/, const FRotator& Rotation /*= FRotator::ZeroRotator*/, int32 ItemNumber /*= 1*/, ESpawnActorCollisionHandlingMethod CollisionHandling /*= ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn*/) const
{
	if (ensure(Owner))
	{
		FActorSpawnParameters ActorSpawnParameters;
		ActorSpawnParameters.bDeferConstruction = true;
		ActorSpawnParameters.Owner = Owner;
		ActorSpawnParameters.Instigator = Instigator;
		ActorSpawnParameters.OverrideLevel = Owner->GetLevel();
		ActorSpawnParameters.SpawnCollisionHandlingOverride = CollisionHandling;
		ActorSpawnParameters.Name = MakeUniqueObjectName(Owner->GetLevel(), GetClass());
#if WITH_EDITOR
		if (FSpawnPreviewItemScope::IsSpawnPreviewItem())
		{
			ActorSpawnParameters.ObjectFlags |= RF_Transient;
			ActorSpawnParameters.bHideFromSceneOutliner = true;
		}
#endif
		if (AXD_ItemBase* SpawnedItem = Owner->GetWorld()->SpawnActor<AXD_ItemBase>(GetSpawnedItemClass(ItemNumber), ActorSpawnParameters))
		{
#if WITH_EDITOR
			SpawnedItem->SetActorLabel(ActorSpawnParameters.Name.ToString(), false);
#endif
			SpawnedItem->bOnlyRelevantToOwner = true;
			SettingSpawnedItem(SpawnedItem, ItemNumber);
			SpawnedItem->FinishSpawning(FTransform(Rotation, Location));
			return SpawnedItem;
		}
	}
	return nullptr;
}

AXD_ItemBase* UXD_ItemCoreBase::SpawnPreviewItemActor(const UObject* WorldContextObject)
{
	FActorSpawnParameters ActorSpawnParameters;
	ActorSpawnParameters.bDeferConstruction = true;
	ActorSpawnParameters.ObjectFlags = RF_Transient;
	ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AXD_ItemBase* SpawnedItem = WorldContextObject->GetWorld()->SpawnActor<AXD_ItemBase>(GetSpawnedItemClass(Number), ActorSpawnParameters);
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

TSubclassOf<AXD_ItemBase> UXD_ItemCoreBase::GetSpawnedItemClass(int32 SpawnedNumber) const
{
	const FXD_ItemModelData& ModelData = GetCurrentItemModel();
	const EItemModelType ModelType = ModelData.ModelType;
	if (ModelType == EItemModelType::StaticMesh)
	{
		return GetStaticMeshEntityType();
	}
	else if (ModelType == EItemModelType::SkeletalMesh)
	{
		return GetSkeletalMeshEntityType();
	}
	else if (ModelType == EItemModelType::Actor)
	{
		return Cast<UClass>(ModelData.Model.LoadSynchronous());
	}
	ensure(false);
	return nullptr;
}

TSubclassOf<AXD_ItemBase> UXD_ItemCoreBase::GetBelongToEntityType() const
{
	return AXD_ItemBase::StaticClass();
}

TSubclassOf<AXD_ItemBase> UXD_ItemCoreBase::GetStaticMeshEntityType() const
{
	return AXD_Item_StaticMesh::StaticClass();
}

TSubclassOf<AXD_ItemBase> UXD_ItemCoreBase::GetSkeletalMeshEntityType() const
{
	return AXD_Item_SkeletalMesh::StaticClass();
}

UXD_ItemCoreBase* UXD_ItemCoreBase::DeepDuplicateCore(const UObject* Outer, const FName& Name) const
{
	return NewObject<UXD_ItemCoreBase>(const_cast<UObject*>(Outer), GetClass(), Name, RF_NoFlags, const_cast<UXD_ItemCoreBase*>(this));
}

void UXD_ItemCoreBase::SettingSpawnedItem(AXD_ItemBase* Item, int32 ThrowNumber) const
{
	Item->ItemCore = UXD_ItemCoreBase::DeepDuplicateCore(this, Item, GET_MEMBER_NAME_CHECKED(AXD_ItemBase, ItemCore));
	Item->ItemCore->Number = ThrowNumber;
}

bool UXD_ItemCoreBase::IsEqualWithItemCore(const UXD_ItemCoreBase* ItemCore) const
{
	if (this == ItemCore)
	{
		return true;
	}
	if (this && ItemCore)
	{
		return GetClass() == ItemCore->GetClass() && RecevieIsEqualWithItemCore(ItemCore);
	}
	return false;
}

void UXD_ItemCoreBase::WhenThrow(AActor* WhoThrowed, int32 ThrowNumber, ULevel* ThrowToLevel)
{
	if (WhoThrowed)
	{
		FVector ThrowLocation = WhoThrowed->GetActorLocation() + WhoThrowed->GetActorRotation().RotateVector(FVector(100.f, 0.f, 0.f));
		FRotator ThrowRotation = WhoThrowed->GetActorRotation();
		if (ThrowNumber > GetMinItemMergeNumberValue() && CanMergeItem())
		{
			AXD_ItemBase* SpawnedItem = SpawnItemActorInLevel(ThrowToLevel, ThrowLocation, ThrowRotation, ThrowNumber, NAME_None, RF_NoFlags);
			SpawnedItem->ItemInWorldSetting();
		}
		else
		{
			for (int i = 0; i < ThrowNumber; ++i)
			{
				AXD_ItemBase* SpawnedItem = SpawnItemActorInLevel(ThrowToLevel, ThrowLocation, ThrowRotation, 1, NAME_None, RF_NoFlags);
				SpawnedItem->ItemInWorldSetting();
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE