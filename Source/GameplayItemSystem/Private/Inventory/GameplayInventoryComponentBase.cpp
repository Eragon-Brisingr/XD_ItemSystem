// Fill out your copyright notice in the Description page of Project Settings.

#include "Inventory/GameplayInventoryComponentBase.h"
#include <Engine/ActorChannel.h>
#include <Net/UnrealNetwork.h>
#include <Kismet/KismetSystemLibrary.h>

#include "Utils/GameplayItemFunctionLibrary.h"
#include "Abstract/GameplayItemBase.h"
#include "Abstract/GameplayItemCoreBase.h"


// Sets default values for this component's properties
UGameplayInventoryComponentBase::UGameplayInventoryComponentBase()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void UGameplayInventoryComponentBase::BeginPlay()
{
	Super::BeginPlay();

	// ...
}


// Called every frame
void UGameplayInventoryComponentBase::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UGameplayInventoryComponentBase::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGameplayInventoryComponentBase, ItemCoreList);
}

bool UGameplayInventoryComponentBase::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
	bool IsFailed = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (UGameplayItemCoreBase* ItemCore : ItemCoreList)
	{
		if (ensureAlways(ItemCore))
		{
			IsFailed |= Channel->ReplicateSubobject(ItemCore, *Bunch, *RepFlags);
		}
	}

	return IsFailed;
}

#if WITH_EDITOR
void UGameplayInventoryComponentBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

TArray<UGameplayItemCoreBase*> UGameplayInventoryComponentBase::AddItemCore(const UGameplayItemCoreBase* ItemCore, int32 Number /*= 1*/)
{
	if (ensureAlways(ItemCore && Number > 0) == false)
	{
		return {};
	}

	if (ItemCore->CanCompositeInInventory())
	{
		int32 ItemIndex = ItemCoreList.IndexOfByPredicate([ItemCore](UGameplayItemCoreBase* ElementItem) {return ElementItem->IsEqualWithItemCore(ItemCore); });
		if (ItemIndex != INDEX_NONE)
		{
			UGameplayItemCoreBase* NeedAddItemCore = ItemCoreList[ItemIndex];
			int32 PreNumber = NeedAddItemCore->Number;
			NeedAddItemCore->Number += Number;
			NeedAddItemCore->OnRep_Number(PreNumber);
			return { NeedAddItemCore };
		}
		else
		{
			UGameplayItemCoreBase* NewItemCore = UGameplayItemCoreBase::DeepDuplicateCore(ItemCore, this);
			NewItemCore->Number = Number;
			ItemCoreList.Add(NewItemCore);
			WhenItemCoreAdded(NewItemCore);
			return { NewItemCore };
		}
	}

	TArray<UGameplayItemCoreBase*> Res;
	for (int i = 0; i < Number; ++i)
	{
		UGameplayItemCoreBase* NewItemCore = UGameplayItemCoreBase::DeepDuplicateCore(ItemCore, this);
		Res.Add(NewItemCore);
		NewItemCore->Number = 1;
		ItemCoreList.Add(NewItemCore);
		WhenItemCoreAdded(NewItemCore);
	}
	return Res;
}

template<typename TPredicate>
int32 RemoveItemByPredicate(UGameplayInventoryComponentBase* Inventory, int32 &Number, const TPredicate& Predicate)
{
	TArray<UGameplayItemCoreBase *> &ItemCoreList = Inventory->ItemCoreList;
	int32 RemovedNumber = 0;
	for (int i = 0; i < ItemCoreList.Num(); ++i)
	{
		if (UGameplayItemCoreBase* ItemCore = ItemCoreList[i])
		{
			if (Predicate(ItemCore))
			{
				if (ItemCore->Number - Number <= 0)
				{
					RemovedNumber += ItemCore->Number;
					Number -= ItemCore->Number;

					ItemCoreList.RemoveAt(i--);
					Inventory->WhenItemCoreRemoved(ItemCore);
				}
				else
				{
					RemovedNumber += Number;
					int32 PreNumber = ItemCore->Number;
					ItemCore->Number -= Number;
					ItemCore->OnRep_Number(PreNumber);
				}
			}
		}
	}
	return RemovedNumber;
}

int32 UGameplayInventoryComponentBase::RemoveItemCore(const UGameplayItemCoreBase* ItemCore, int32 Number /*= 1*/)
{
	return RemoveItemByPredicate(this, Number, [&](UGameplayItemCoreBase* ElementItem) {return ElementItem->IsEqualWithItemCore(ItemCore); });
}

TArray<UGameplayItemCoreBase*> UGameplayInventoryComponentBase::AddItemCoreByType(TSubclassOf<UGameplayItemCoreBase> Item, int32 Number /*= 1*/)
{
	if (Item)
	{
		return AddItemCore(Item.GetDefaultObject(), Number);
	}
	return {};
}

int32 UGameplayInventoryComponentBase::RemoveItemCoreByType(TSubclassOf<UGameplayItemCoreBase> Item, int32 Number)
{
	return RemoveItemByPredicate(this, Number, [&](UGameplayItemCoreBase* ItemCore) {return ItemCore->IsA(Item); });
}

void UGameplayInventoryComponentBase::GetItemFromOther(UGameplayInventoryComponentBase* OtherInventory, UGameplayItemCoreBase* ItemCore, int32 Number /*= 1*/)
{
	int32 AddNumber = OtherInventory->RemoveItemCore(ItemCore, Number);
	if (AddNumber > 0)
	{
		AddItemCore(ItemCore, AddNumber);
		OnGetItemFromOther.Broadcast(GetOwner(), ItemCore, AddNumber, false);
		OtherInventory->OnRemoveItemByOther.Broadcast(GetOwner(), ItemCore, AddNumber, false);
	}
}

ULevel* UGameplayInventoryComponentBase::GetThrowedLevel()
{
	//寻找当前关卡
	TArray<AActor*> TraceActors;
	UKismetSystemLibrary::SphereOverlapActors(this, GetOwner()->GetActorLocation(), 1000.f, { TEnumAsByte<EObjectTypeQuery>(ECollisionChannel::ECC_WorldStatic) }, nullptr, {}, TraceActors);
	for (AActor* Actor : TraceActors)
	{
		if (Actor->GetLevel())
		{
			return Actor->GetLevel();
		}
	}
	return nullptr;
}

void UGameplayInventoryComponentBase::ThrowItemCore(UGameplayItemCoreBase* ItemCore, int32 Number /*= 1*/)
{
	if (!ItemCore)
	{
		return;
	}

	if (ULevel* ThrowLevel = GetThrowedLevel())
	{
		int32 RemoveNumber = RemoveItemCore(ItemCore, Number);
		if (RemoveNumber > 0)
		{
			ItemCore->WhenThrow(GetOwner(), RemoveNumber, ThrowLevel);
		}
	}
}

void UGameplayInventoryComponentBase::ClearItem()
{
	for (UGameplayItemCoreBase* ItemCore : ItemCoreList)
	{
		WhenItemCoreRemoved(ItemCore);
	}
	ItemCoreList.Empty();
}

void UGameplayInventoryComponentBase::AddItemArray(const TArray<UGameplayItemCoreBase*>& Items)
{
	for (UGameplayItemCoreBase* ItemCore : Items)
	{
		if (ItemCore)
		{
			AddItemCore(ItemCore, ItemCore->Number);
		}
	}
}

void UGameplayInventoryComponentBase::OnRep_ItemList()
{
	for (UGameplayItemCoreBase* AddItem : TSet<UGameplayItemCoreBase*>(ItemCoreList).Difference(TSet<UGameplayItemCoreBase*>(PreItemCoreList)))
	{
		WhenItemCoreAdded(AddItem);
	}

	for (UGameplayItemCoreBase* RemoveItem : TSet<UGameplayItemCoreBase*>(PreItemCoreList).Difference(TSet<UGameplayItemCoreBase*>(ItemCoreList)))
	{
		WhenItemCoreRemoved(RemoveItem);
	}

	PreItemCoreList = ItemCoreList;
}

void UGameplayInventoryComponentBase::WhenItemCoreAdded(UGameplayItemCoreBase* AddedItemCore)
{
	if (ensureAlways(AddedItemCore))
	{
		AddedItemCore->OwingInventory = this;
		OnAddItem.Broadcast(AddedItemCore, AddedItemCore->Number, AddedItemCore->Number);
	}
}

void UGameplayInventoryComponentBase::WhenItemCoreRemoved(UGameplayItemCoreBase* RemovedItemCore)
{
	if (ensureAlways(RemovedItemCore))
	{
		OnRemoveItem.Broadcast(RemovedItemCore, RemovedItemCore->Number, 0);
		RemovedItemCore->WhenRemoveFromInventory(GetOwner(), RemovedItemCore->Number, 0);
	}
}

int32 UGameplayInventoryComponentBase::GetItemNumber(const AGameplayItemBase* Item) const
{
	return UGameplayItemFunctionLibrary::GetItemNumber(ItemCoreList, Item);
}

int32 UGameplayInventoryComponentBase::GetItemNumberByCore(const UGameplayItemCoreBase* ItemCore) const
{
	return UGameplayItemFunctionLibrary::GetItemNumberByCore(ItemCoreList, ItemCore);
}

int32 UGameplayInventoryComponentBase::GetItemNumberByType(TSubclassOf<UGameplayItemCoreBase> ItemClass) const
{
	return UGameplayItemFunctionLibrary::GetItemNumberByType(ItemCoreList, ItemClass);
}

bool UGameplayInventoryComponentBase::ContainItem(const AGameplayItemBase* Item) const
{
	return UGameplayItemFunctionLibrary::ContainItem(ItemCoreList, Item);
}

bool UGameplayInventoryComponentBase::ContainItemByCore(const UGameplayItemCoreBase* ItemCore) const
{
	return UGameplayItemFunctionLibrary::ContainItemByCore(ItemCoreList, ItemCore);
}

bool UGameplayInventoryComponentBase::ContainItemByType(TSubclassOf<UGameplayItemCoreBase> ItemClass) const
{
	return UGameplayItemFunctionLibrary::ContainItemByType(ItemCoreList, ItemClass);
}

UGameplayItemCoreBase* UGameplayInventoryComponentBase::FindItemByType(TSubclassOf<UGameplayItemCoreBase> ItemType) const
{
	return UGameplayItemFunctionLibrary::FindItemByType(ItemCoreList, ItemType);
}

TArray<UGameplayItemCoreBase*> UGameplayInventoryComponentBase::FindItemsByType(TSubclassOf<UGameplayItemCoreBase> ItemType) const
{
	return UGameplayItemFunctionLibrary::FindItemsByType(ItemCoreList, ItemType);
}

UGameplayItemCoreBase* UGameplayInventoryComponentBase::FindItemByItemCore(UGameplayItemCoreBase* ItemCore) const
{
	return UGameplayItemFunctionLibrary::FindItemByItemCore(ItemCoreList, ItemCore);
}
