// Fill out your copyright notice in the Description page of Project Settings.

#include "XD_InventoryComponentBase.h"
#include "XD_ItemCoreBase.h"
#include "XD_ObjectFunctionLibrary.h"
#include <Kismet/KismetSystemLibrary.h>
#include "XD_ItemBase.h"
#include "UnrealNetwork.h"


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
#if WITH_EDITOR
	InitItemsType = AXD_ItemBase::StaticClass();
#endif
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

void UXD_InventoryComponentBase::WhenGameInit_Implementation()
{
	AddItemArray(InitItems);
}

void UXD_InventoryComponentBase::WhenPostLoad_Implementation()
{
	OnRep_ItemList();
}

#if WITH_EDITOR
void UXD_InventoryComponentBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (InitItemsType && PropertyName == GET_MEMBER_NAME_CHECKED(UXD_InventoryComponentBase, InitItems))
	{
		for (FXD_Item& Item : InitItems)
		{
			Item.ShowItemType = InitItemsType;
		}
	}
}
#endif

TArray<class UXD_ItemCoreBase*> UXD_InventoryComponentBase::AddItemCore(const class UXD_ItemCoreBase* ItemCore, int32 Number /*= 1*/)
{
	if (!ItemCore || Number <= 0)
		return {};

	if (ItemCore->CanCompositeInInventory())
	{
		int32 ItemIndex = ItemCoreList.IndexOfByPredicate([ItemCore](UXD_ItemCoreBase* ElementItem) {return ElementItem->IsEqualWithItemCore(ItemCore); });
		if (ItemIndex != INDEX_NONE)
		{
			UXD_ItemCoreBase* NeedAddItemCore = ItemCoreList[ItemIndex];
			int32 PreNumber = NeedAddItemCore->Number;
			NeedAddItemCore->Number += Number;
			NeedAddItemCore->OnRep_Number(PreNumber);
			return { NeedAddItemCore };
		}
		else
		{
			UXD_ItemCoreBase* NewItemCore = UXD_ObjectFunctionLibrary::DuplicateObject(ItemCore, this);
			NewItemCore->Number = Number;
			ItemCoreList.Add(NewItemCore);
			OnRep_ItemList();
			return { NewItemCore };
		}
	}

	TArray<class UXD_ItemCoreBase*> Res;
	for (int i = 0; i < Number; ++i)
	{
		UXD_ItemCoreBase* NewItemCore = UXD_ObjectFunctionLibrary::DuplicateObject(ItemCore, this);
		Res.Add(NewItemCore);
		NewItemCore->Number = 1;
		ItemCoreList.Add(NewItemCore);
	}
	OnRep_ItemList();
	return Res;
}

template<typename TPredicate>
int32 RemoveItemByPredicate(UXD_InventoryComponentBase* Inventory, int32 &Number, const TPredicate& Predicate)
{
	TArray<class UXD_ItemCoreBase *> &ItemCoreList = Inventory->ItemCoreList;
	int32 RemovedNumber = 0;
	for (int i = 0; i < ItemCoreList.Num(); ++i)
	{
		if (UXD_ItemCoreBase* ItemCore = ItemCoreList[i])
		{
			if (Predicate(ItemCore))
			{
				if (ItemCore->Number - Number <= 0)
				{
					RemovedNumber += ItemCore->Number;
					Number -= ItemCore->Number;

					ItemCoreList.RemoveAt(i--);
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
	Inventory->OnRep_ItemList();
	return RemovedNumber;
}

int32 UXD_InventoryComponentBase::RemoveItemCore(const class UXD_ItemCoreBase* ItemCore, int32 Number /*= 1*/)
{
	return RemoveItemByPredicate(this, Number, [&](UXD_ItemCoreBase* ElementItem) {return ElementItem->IsEqualWithItemCore(ItemCore); });
}

TArray<class UXD_ItemCoreBase*> UXD_InventoryComponentBase::AddItemCoreByType(TSubclassOf<class AXD_ItemBase> Item, int32 Number /*= 1*/)
{
	if (Item)
	{
		return AddItemCore(Item.GetDefaultObject()->GetItemCore(), Number);
	}
	return {};
}

int32 UXD_InventoryComponentBase::RemoveItemCoreByType(TSubclassOf<class AXD_ItemBase> Item, int32 Number)
{
	return RemoveItemByPredicate(this, Number, [&](UXD_ItemCoreBase* ItemCore) {return ItemCore->GetItemClass() == Item; });
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

void UXD_InventoryComponentBase::ThrowItemCore(class UXD_ItemCoreBase* ItemCore, int32 Number /*= 1*/)
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
	if (ItemCore)
	{
		if (ItemCore->CanCompositeInInventory())
		{
			if (auto FindItem = ItemCoreList.FindByPredicate([ItemCore](auto& ElementItem) {return ElementItem->IsEqualWithItemCore(ItemCore); }))
			{
				return (*FindItem)->Number;
			}
		}
		else
		{
			int Number = 0;
			for (UXD_ItemCoreBase* ItemCoreElement : ItemCoreList)
			{
				if (ItemCoreElement->IsEqualWithItemCore(ItemCore))
				{
					Number += 1;
				}
			}
			return Number;
		}
	}
	return 0;
}

int32 UXD_InventoryComponentBase::GetItemNumberByType(TSubclassOf<class AXD_ItemBase> ItemClass)
{
	int32 Number = 0;
	for (UXD_ItemCoreBase* ElementItem : ItemCoreList)
	{
		if (ElementItem->GetItemClass()->IsChildOf(ItemClass))
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
	UXD_ItemCoreBase* const* p_NeedFindItem = ItemCoreList.FindByPredicate([&](UXD_ItemCoreBase* Item) {return Item->GetItemClass()->IsChildOf(ItemType); });
	return p_NeedFindItem ? *p_NeedFindItem : nullptr;
}

TArray<class UXD_ItemCoreBase*> UXD_InventoryComponentBase::FindItemsByType(TSubclassOf<class AXD_ItemBase> ItemType) const
{
	TArray<class UXD_ItemCoreBase*> Items;
	for (UXD_ItemCoreBase* ItemCore : ItemCoreList)
	{
		if (ItemCore->GetItemClass()->IsChildOf(ItemType))
		{
			Items.Add(ItemCore);
		}
	}
	return Items;
}

class UXD_ItemCoreBase* UXD_InventoryComponentBase::FindItemByItemCore(class UXD_ItemCoreBase* ItemCore) const
{
	int32 ItemIndex = ItemCoreList.IndexOfByPredicate([ItemCore](auto& ElementItem) {return ElementItem->IsEqualWithItemCore(ItemCore); });
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

	for (UXD_ItemCoreBase* RemoveItem : TSet<UXD_ItemCoreBase*>(PreItemCoreList).Difference(TSet<UXD_ItemCoreBase*>(ItemCoreList)))
	{
		if (RemoveItem)
		{
			OnRemoveItem.Broadcast(RemoveItem, RemoveItem->Number, 0);
			RemoveItem->WhenRemoveFromInventory(GetOwner(), RemoveItem->Number, 0);
		}
	}

	PreItemCoreList = ItemCoreList;
}
