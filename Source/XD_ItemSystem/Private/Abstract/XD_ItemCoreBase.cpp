// Fill out your copyright notice in the Description page of Project Settings.

#include "XD_ItemCoreBase.h"
#include "XD_ItemBase.h"
#include "XD_InventoryComponentBase.h"
#include "XD_ObjectFunctionLibrary.h"
#include <UnrealNetwork.h>
#include "XD_ItemSystemUtility.h"
#include "Engine/World.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "NotificationManager.h"
#include "SNotificationList.h"

#define LOCTEXT_NAMESPACE "物品" 

UXD_ItemCoreBase::UXD_ItemCoreBase(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer), 
	ItemName(LOCTEXT("物品", "物品")),
	bCanCompositeInInventory(true)
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

void UXD_ItemCoreBase::PostInitProperties()
{
	Super::PostInitProperties();
}

#if WITH_EDITOR
void UXD_ItemCoreBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UXD_ItemCoreBase, Number))
	{
		if (AXD_ItemBase* ItemBase = Cast<AXD_ItemBase>(GetOuter()))
		{
			if (!CanMergeItem())
			{
				Number = 1;
				FNotificationInfo NotificationInfo(LOCTEXT("不可合并道具数量改变错误提示", "不可合并的道具，数量不可变"));
				TSharedPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(NotificationInfo);
				NotificationItem->SetCompletionState(SNotificationItem::CS_Fail);
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

TSoftObjectPtr<UObject> UXD_ItemCoreBase::GetCurrentItemModel() const
{
	return IsMergedItem() ? ItemMergeMesh : ItemMesh;
}

AActor* UXD_ItemCoreBase::GetOwner() const
{
	return OwingInventory ? OwingInventory->GetOwner() : nullptr;
}

AXD_ItemBase* UXD_ItemCoreBase::SpawnItemActorInLevel(ULevel* OuterLevel, int32 ItemNumber /*= 1*/, const FVector& Location /*= FVector::ZeroVector*/, const FRotator& Rotation /*= FRotator::ZeroRotator*/, ESpawnActorCollisionHandlingMethod CollisionHandling /*= ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn*/) const
{
	return SpawnItemActorInLevel(OuterLevel, ItemNumber, MakeUniqueObjectName(OuterLevel, GetClass()), RF_NoFlags, Location, Rotation, CollisionHandling);
}

AXD_ItemBase* UXD_ItemCoreBase::SpawnItemActorInLevel(ULevel* OuterLevel, int32 ItemNumber /*= 1*/, const FName& Name /*= NAME_None*/, EObjectFlags InObjectFlags /*= RF_NoFlags*/, const FVector& Location /*= FVector::ZeroVector*/, const FRotator& Rotation /*= FRotator::ZeroRotator*/, ESpawnActorCollisionHandlingMethod CollisionHandling /*= ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn*/) const
{
	if (ensure(OuterLevel))
	{
		FActorSpawnParameters ActorSpawnParameters;
		ActorSpawnParameters.bDeferConstruction = true;
		ActorSpawnParameters.OverrideLevel = OuterLevel;
		ActorSpawnParameters.Name = Name;
		ActorSpawnParameters.ObjectFlags = InObjectFlags;
		ActorSpawnParameters.SpawnCollisionHandlingOverride = CollisionHandling;
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

AXD_ItemBase* UXD_ItemCoreBase::SpawnItemActorForOwner(AActor* Owner, APawn* Instigator, int32 ItemNumber /*= 1*/, const FVector& Location /*= FVector::ZeroVector*/, const FRotator& Rotation /*= FRotator::ZeroRotator*/, ESpawnActorCollisionHandlingMethod CollisionHandling /*= ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn*/) const
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
		if (AXD_ItemBase* SpawnedItem = Owner->GetWorld()->SpawnActor<AXD_ItemBase>(GetSpawnedItemClass(ItemNumber), ActorSpawnParameters))
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

TSubclassOf<AXD_ItemBase> UXD_ItemCoreBase::GetSpawnedItemClass(int32 SpawnedNumber) const
{
	TSoftObjectPtr<UObject> MeshPtr = GetCurrentItemModel();
	// TODO：异步加载
	UObject* Mesh = MeshPtr.LoadSynchronous();
	if (Mesh->IsA<UStaticMesh>())
	{
		return GetStaticMeshActor();
	}
	else if (Mesh->IsA<USkeletalMesh>())
	{
		return GetSkeletalMeshActor();
	}
	checkNoEntry();
	return nullptr;
}

TSubclassOf<AXD_ItemBase> UXD_ItemCoreBase::GetStaticMeshActor() const
{
	return AXD_Item_StaticMesh::StaticClass();
}

TSubclassOf<AXD_ItemBase> UXD_ItemCoreBase::GetSkeletalMeshActor() const
{
	return AXD_Item_SkeletalMesh::StaticClass();
}

UXD_ItemCoreBase* UXD_ItemCoreBase::DeepDuplicateCore(const UObject* Outer, const FName& Name) const
{
	return UXD_ObjectFunctionLibrary::DuplicateObject(this, Outer, Name);
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

FText UXD_ItemCoreBase::GetItemName_Implementation() const
{
	return ItemName;
}

void UXD_ItemCoreBase::WhenThrow(AActor* WhoThrowed, int32 ThrowNumber, ULevel* ThrowToLevel)
{
	if (WhoThrowed)
	{
		FVector ThrowLocation = WhoThrowed->GetActorLocation() + WhoThrowed->GetActorRotation().RotateVector(FVector(100.f, 0.f, 0.f));
		FRotator ThrowRotation = WhoThrowed->GetActorRotation();
		if (ThrowNumber > MinItemMergeNumber && CanMergeItem())
		{
			AXD_ItemBase* SpawnedItem = SpawnItemActorInLevel(ThrowToLevel, ThrowNumber, ThrowLocation, ThrowRotation);
			SpawnedItem->WhenItemInWorldSetting();
		}
		else
		{
			for (int i = 0; i < ThrowNumber; ++i)
			{
				AXD_ItemBase* SpawnedItem = SpawnItemActorInLevel(ThrowToLevel, 1, ThrowLocation, ThrowRotation);
				SpawnedItem->WhenItemInWorldSetting();
			}
		}
	}
}

void UXD_ItemCoreBase::WhenRemoveFromInventory(class AActor* ItemOwner, int32 RemoveNumber, int32 ExistNumber)
{

}

#undef LOCTEXT_NAMESPACE