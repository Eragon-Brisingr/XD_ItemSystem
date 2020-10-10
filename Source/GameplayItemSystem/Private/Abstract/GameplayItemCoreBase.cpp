// Fill out your copyright notice in the Description page of Project Settings.

#include "Abstract/GameplayItemCoreBase.h"
#include "Abstract/GameplayItemBase.h"
#include <Net/UnrealNetwork.h>
#include <Engine/World.h>
#include <Engine/BlueprintGeneratedClass.h>
#include <Engine/StaticMesh.h>
#include <Engine/SkeletalMesh.h>

#include "Inventory/GameplayInventoryComponentBase.h"
#include "GameplayItemSystemUtility.h"

#define LOCTEXT_NAMESPACE "物品" 

#if WITH_EDITOR
uint8 FSpawnPreviewItemScope::Counter;

void FGameplayItemModelData::UpdateModelType()
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

FGameplayItemCoreSparseData::FGameplayItemCoreSparseData() :
	ItemNameValue(LOCTEXT("物品", "物品")),
	bCanCompositeInInventoryValue(true),
	bCanMergeItemValue(false)
{

}

UGameplayItemCoreBase::UGameplayItemCoreBase(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{

}

bool UGameplayItemCoreBase::IsSupportedForNetworking() const
{
	return true;
}

void UGameplayItemCoreBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	if (UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(GetClass()))
	{
		BPClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
	}

	DOREPLIFETIME(UGameplayItemCoreBase, Number);
}

UWorld* UGameplayItemCoreBase::GetWorld() const
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
void UGameplayItemCoreBase::PostLoad()
{
	Super::PostLoad();

// 	if (HasAnyFlags(RF_ClassDefaultObject))
// 	{
// 		FGameplayItemCoreSparseData* ItemCoreSparseData = GetGameplayItemCoreSparseData();
// 		ItemCoreSparseData->ItemModelValue.UpdateModelType();
// 		ItemCoreSparseData->MergeItemModelValue.UpdateModelType();
// 	}
}

void UGameplayItemCoreBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UGameplayItemCoreBase, Number))
	{
		UObject* Outer = GetOuter();
		if (Outer && (Outer->IsA<AGameplayItemBase>() || Outer->IsA<UPackage>()))
		{
			if (!CanMergeItem())
			{
				if (ensure(Number == 1) == false)
				{
					Number = 1;
				}
			}
			else if (Number != 1)
			{
				const int32 MinItemMergeNumberValue = GetMinItemMergeNumberValue();
				if (ensure(Number >= MinItemMergeNumberValue) == false)
				{
					Number = MinItemMergeNumberValue;
				}
			}
		}
	}
}
#endif //WITH_EDITOR

void UGameplayItemCoreBase::OnRep_Number(int32 PreNumber)
{
	if (OwingInventory)
	{
		if (PreNumber < Number)
		{
			OwingInventory->WhenItemCoreAdded(this, Number - PreNumber, Number);
		}
		else
		{
			OwingInventory->WhenItemCoreRemoved(this, PreNumber - Number, Number);
		}
	}
}

const FGameplayItemModelData& UGameplayItemCoreBase::GetCurrentItemModel() const
{
	return IsMergedItem() ? GetMergeItemModelValue() : GetItemModelValue();
}

AGameplayItemBase* UGameplayItemCoreBase::SpawnItemActorInWorld(UObject* WorldContextObject, int32 ItemNumber, FVector Location, FRotator Rotation, ESpawnActorCollisionHandlingMethod CollisionHandling) const
{
	if (WorldContextObject)
	{
		UWorld* World = WorldContextObject->GetWorld();
		AActor* WorldContextActor = Cast<AActor>(WorldContextObject);
		ULevel* Level = WorldContextActor ? WorldContextActor->GetLevel() : World->PersistentLevel;
		return SpawnItemActorInLevel(Level, Location, Rotation, ItemNumber, NAME_None, RF_NoFlags, CollisionHandling);
	}
	return nullptr;
}

AGameplayItemBase* UGameplayItemCoreBase::SpawnItemActorInLevel(ULevel* Level, const FVector& Location /*= FVector::ZeroVector*/, const FRotator& Rotation /*= FRotator::ZeroRotator*/, int32 ItemNumber /*= 1*/, const FName& Name /*= NAME_None*/, EObjectFlags InObjectFlags /*= RF_NoFlags*/, ESpawnActorCollisionHandlingMethod CollisionHandling /*= ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn*/) const
{
	if (ensure(Level))
	{
		FActorSpawnParameters ActorSpawnParameters;
		ActorSpawnParameters.bDeferConstruction = true;
		ActorSpawnParameters.OverrideLevel = Level;
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
		if (AGameplayItemBase* SpawnedItem = Level->GetWorld()->SpawnActor<AGameplayItemBase>(GetSpawnedItemClass(ItemNumber), ActorSpawnParameters))
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

AGameplayItemBase* UGameplayItemCoreBase::SpawnItemActorForOwner(AActor* Owner, APawn* Instigator, const FVector& Location /*= FVector::ZeroVector*/, const FRotator& Rotation /*= FRotator::ZeroRotator*/, int32 ItemNumber /*= 1*/, ESpawnActorCollisionHandlingMethod CollisionHandling /*= ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn*/) const
{
	const TSubclassOf<AGameplayItemBase> SpawnedItemClass = GetSpawnedItemClass(ItemNumber);
	if (ensure(Owner))
	{
		FActorSpawnParameters ActorSpawnParameters;
		ActorSpawnParameters.bDeferConstruction = true;
		ActorSpawnParameters.Owner = Owner;
		ActorSpawnParameters.Instigator = Instigator;
		ActorSpawnParameters.OverrideLevel = Owner->GetLevel();
		ActorSpawnParameters.SpawnCollisionHandlingOverride = CollisionHandling;
		ActorSpawnParameters.Name = MakeUniqueObjectName(ActorSpawnParameters.OverrideLevel, SpawnedItemClass, *FString::Printf(TEXT("%s_%s"), *Owner->GetName(), *GetClass()->GetName()));
#if WITH_EDITOR
		if (FSpawnPreviewItemScope::IsSpawnPreviewItem())
		{
			ActorSpawnParameters.ObjectFlags |= RF_Transient;
			ActorSpawnParameters.bHideFromSceneOutliner = true;
		}
#endif
		if (AGameplayItemBase* SpawnedItem = Owner->GetWorld()->SpawnActor<AGameplayItemBase>(SpawnedItemClass, ActorSpawnParameters))
		{
#if WITH_EDITOR
			SpawnedItem->SetActorLabel(ActorSpawnParameters.Name.ToString(), false);
#endif
			SpawnedItem->bNetUseOwnerRelevancy = true;
			SettingSpawnedItem(SpawnedItem, ItemNumber);
			SpawnedItem->FinishSpawning(FTransform(Rotation, Location));
			return SpawnedItem;
		}
	}
	return nullptr;
}

AGameplayItemBase* UGameplayItemCoreBase::SpawnPreviewItemActor(const UObject* WorldContextObject) const
{
	FActorSpawnParameters ActorSpawnParameters;
	ActorSpawnParameters.bDeferConstruction = true;
	ActorSpawnParameters.ObjectFlags = RF_Transient;
	ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AGameplayItemBase* SpawnedItem = WorldContextObject->GetWorld()->SpawnActor<AGameplayItemBase>(GetSpawnedItemClass(Number), ActorSpawnParameters);
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

TSubclassOf<AGameplayItemBase> UGameplayItemCoreBase::GetSpawnedItemClass(int32 SpawnedNumber) const
{
	const FGameplayItemModelData& ModelData = GetCurrentItemModel();
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

TSubclassOf<AGameplayItemBase> UGameplayItemCoreBase::GetBelongToEntityType() const
{
	return AGameplayItemBase::StaticClass();
}

TSubclassOf<AGameplayItemBase> UGameplayItemCoreBase::GetStaticMeshEntityType() const
{
	return AGameplayItem_StaticMesh::StaticClass();
}

TSubclassOf<AGameplayItemBase> UGameplayItemCoreBase::GetSkeletalMeshEntityType() const
{
	return AGameplayItem_SkeletalMesh::StaticClass();
}

UGameplayItemCoreBase* UGameplayItemCoreBase::DeepDuplicateCore(const UObject* Outer, const FName& Name) const
{
	return NewObject<UGameplayItemCoreBase>(const_cast<UObject*>(Outer), GetClass(), Name, RF_NoFlags, const_cast<UGameplayItemCoreBase*>(this));
}

void UGameplayItemCoreBase::SettingSpawnedItem(AGameplayItemBase* Item, int32 ThrowNumber) const
{
	Item->ItemCore = DeepDuplicateCore(this, Item, GET_MEMBER_NAME_CHECKED(AGameplayItemBase, ItemCore));
	Item->ItemCore->Number = ThrowNumber;
}

bool UGameplayItemCoreBase::IsEqualWithItemCore(const UGameplayItemCoreBase* ItemCore) const
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

void UGameplayItemCoreBase::WhenThrow(AActor* WhoThrowed, int32 ThrowNumber, ULevel* ThrowToLevel)
{
	if (WhoThrowed)
	{
		FVector ThrowLocation = WhoThrowed->GetActorLocation() + WhoThrowed->GetActorRotation().RotateVector(FVector(100.f, 0.f, 0.f));
		FRotator ThrowRotation = WhoThrowed->GetActorRotation();
		if (ThrowNumber > GetMinItemMergeNumberValue() && CanMergeItem())
		{
			AGameplayItemBase* SpawnedItem = SpawnItemActorInLevel(ThrowToLevel, ThrowLocation, ThrowRotation, ThrowNumber, NAME_None, RF_NoFlags);
			SpawnedItem->ItemInWorldSetting();
		}
		else
		{
			for (int i = 0; i < ThrowNumber; ++i)
			{
				AGameplayItemBase* SpawnedItem = SpawnItemActorInLevel(ThrowToLevel, ThrowLocation, ThrowRotation, 1, NAME_None, RF_NoFlags);
				SpawnedItem->ItemInWorldSetting();
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE