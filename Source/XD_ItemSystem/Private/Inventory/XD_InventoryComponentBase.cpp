// Fill out your copyright notice in the Description page of Project Settings.

#include "XD_InventoryComponentBase.h"
#include "XD_ItemCoreBase.h"
#include "XD_ObjectFunctionLibrary.h"
#include <Kismet/KismetSystemLibrary.h>


// Sets default values for this component's properties
UXD_InventoryComponentBase::UXD_InventoryComponentBase()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
	SetIsReplicated(true);
}


// Called when the game starts
void UXD_InventoryComponentBase::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UXD_InventoryComponentBase::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UXD_InventoryComponentBase::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UXD_InventoryComponentBase, ItemCoreList);
}

bool UXD_InventoryComponentBase::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
	bool IsFailed = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (UXD_ItemCoreBase* ItemCore : ItemCoreList)
	{
		if (ItemCore)
		{
			IsFailed |= Channel->ReplicateSubobject(ItemCore, *Bunch, *RepFlags);
		}
	}

	return IsFailed;
}

void UXD_InventoryComponentBase::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

void UXD_InventoryComponentBase::WhenLoad_Implementation()
{
	OnRep_ItemList();
}

class UXD_ItemCoreBase* UXD_InventoryComponentBase::AddItemCore(const class UXD_ItemCoreBase* ItemCore, int32 Number /*= 1*/)
{
	if (!ItemCore || Number <= 0)
		return nullptr;
	int32 ItemIndex = ItemCoreList.IndexOfByPredicate([ItemCore](UXD_ItemCoreBase* ElementItem) {return ElementItem->EqualForItemCore(ItemCore); });
	if (ItemIndex != INDEX_NONE)
	{
		int32 PreNumber = ItemCoreList[ItemIndex]->Number;
		ItemCoreList[ItemIndex]->Number += Number;
		ItemCoreList[ItemIndex]->OnRep_Number(PreNumber);
		return ItemCoreList[ItemIndex];
	}
	else
	{
		//确保Item唯一，防止其它Item引用
		UXD_ItemCoreBase* NewItemCore = UXD_ObjectFunctionLibrary::DuplicateObject(ItemCore, this);
		NewItemCore->Number = Number;
		ItemCoreList.Add(NewItemCore);
		OnRep_ItemList();
		return NewItemCore;
	}
}

int32 UXD_InventoryComponentBase::RemoveItemCore(const class UXD_ItemCoreBase* ItemCore, int32 Number /*= 1*/)
{
	int32 ItemIndex = ItemCoreList.IndexOfByPredicate([ItemCore](auto& ElementItem) {return ElementItem->EqualForItemCore(ItemCore); });
	if (ItemIndex != INDEX_NONE)
	{
		if (ItemCoreList[ItemIndex]->Number - Number <= 0)
		{
			int32 RetNum = ItemCoreList[ItemIndex]->Number;
			ItemCoreList.RemoveAt(ItemIndex);
			OnRep_ItemList();
			return RetNum;
		}
		else
		{
			int32 PreNumber = ItemCoreList[ItemIndex]->Number;
			ItemCoreList[ItemIndex]->Number -= Number;
			ItemCoreList[ItemIndex]->OnRep_Number(PreNumber);
			return Number;
		}
	}
	return 0;
}

class UXD_ItemCoreBase* UXD_InventoryComponentBase::AddItemCoreByType(TSubclassOf<class AXD_ItemBase> Item, int32 Number /*= 1*/)
{
	if (Item)
	{
		return AddItemCore(Item->GetDefaultObject<AXD_ItemBase>()->GetItemCore(), Number);
	}
	return nullptr;
}

void UXD_InventoryComponentBase::GetItemFromOther(UXD_InventoryComponentBase* OtherInventory, class UXD_ItemCoreBase* ItemCore, int32 Number /*= 1*/)
{
	int32 AddNumber = OtherInventory->RemoveItemCore(ItemCore, Number);
	if (AddNumber > 0)
	{
		AddItemCore(ItemCore, AddNumber);
		OnGetItemFromOther.Broadcast(GetOwner(), ItemCore, AddNumber, false);
		OtherInventory->OnRemoveItemByOther.Broadcast(GetOwner(), ItemCore, AddNumber, false);
	}
}

ULevel* UXD_InventoryComponentBase::GetThrowedLevel()
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

void UXD_InventoryComponentBase::ThrowItemCore_Implementation(class UXD_ItemCoreBase* ItemCore, int32 Number /*= 1*/)
{
	if (ULevel* ThrowLevel = GetThrowedLevel())
	{
		int32 RemoveNumber = RemoveItemCore(ItemCore, Number);
		if (RemoveNumber > 0)
		{
			ItemCore->BeThrowed(GetOwner(), RemoveNumber, ThrowLevel);
		}
	}
}

int32 UXD_InventoryComponentBase::GetItemNumber(class AXD_ItemBase* Item)
{
	return Item ? GetItemNumberByCore(Item->GetItemCore()) : 0;
}

int32 UXD_InventoryComponentBase::GetItemNumberByCore(const class UXD_ItemCoreBase* ItemCore)
{
	if (auto FindItem = ItemCoreList.FindByPredicate([ItemCore](auto& ElementItem) {return ElementItem->EqualForItemCore(ItemCore); }))
	{
		return (*FindItem)->Number;
	}
	return 0;
}

int32 UXD_InventoryComponentBase::GetItemNumberByClass(TSubclassOf<class AXD_ItemBase> ItemClass)
{
	int32 Number = 0;
	for (UXD_ItemCoreBase* ElementItem : ItemCoreList)
	{
		if (ElementItem->ItemClass->IsChildOf(ItemClass))
		{
			Number += ElementItem->Number;
		}
	}
	return Number;
}

void UXD_InventoryComponentBase::ClearItem()
{
	ItemCoreList.Empty();
	OnRep_ItemList();
}

class UXD_ItemCoreBase* UXD_InventoryComponentBase::FindItemByType(TSubclassOf<class AXD_ItemBase> ItemType) const
{
	UXD_ItemCoreBase* const* p_NeedFindItem = ItemCoreList.FindByPredicate([&](UXD_ItemCoreBase* Item) {return Item->ItemClass->IsChildOf(ItemType); });
	return p_NeedFindItem ? *p_NeedFindItem : nullptr;
}

TArray<class UXD_ItemCoreBase*> UXD_InventoryComponentBase::FindItemsByType(TSubclassOf<class AXD_ItemBase> ItemType) const
{
	TArray<class UXD_ItemCoreBase*> Items;
	for (UXD_ItemCoreBase* ItemCore : ItemCoreList)
	{
		if (ItemCore->ItemClass->IsChildOf(ItemType))
		{
			Items.Add(ItemCore);
		}
	}
	return Items;
}

class UXD_ItemCoreBase* UXD_InventoryComponentBase::FindItemByItemCore(class UXD_ItemCoreBase* ItemCore) const
{
	int32 ItemIndex = ItemCoreList.IndexOfByPredicate([ItemCore](auto& ElementItem) {return ElementItem->EqualForItemCore(ItemCore); });
	if (ItemIndex != INDEX_NONE)
	{
		return ItemCoreList[ItemIndex];
	}
	return nullptr;
}

void UXD_InventoryComponentBase::AddItemArray(const TArray<FXD_Item>& Items)
{
	for (const FXD_Item& Item : Items)
	{
		if (Item.ItemCore)
		{
			AddItemCore(Item.ItemCore, Item.ItemCore->Number);
		}
	}
}

void UXD_InventoryComponentBase::OnRep_ItemList()
{
	for (UXD_ItemCoreBase* AddItem : TSet<UXD_ItemCoreBase*>(ItemCoreList).Difference(TSet<UXD_ItemCoreBase*>(PreItemCoreList)))
	{
		if (AddItem)
		{
			AddItem->OwingInventory = this;
			OnAddItem.Broadcast(AddItem, AddItem->Number, AddItem->Number);
		}
	}

	if (OnRemoveItem.IsBound())
	{
		for (UXD_ItemCoreBase* RemoveItem : TSet<UXD_ItemCoreBase*>(PreItemCoreList).Difference(TSet<UXD_ItemCoreBase*>(ItemCoreList)))
		{
			if (RemoveItem)
			{
				OnRemoveItem.Broadcast(RemoveItem, RemoveItem->Number, 0);
				RemoveItem->WhenRemoveFromInventory(GetOwner(), RemoveItem->Number, 0);
			}
		}
	}

	PreItemCoreList = ItemCoreList;
}
