// Fill out your copyright notice in the Description page of Project Settings.

#include "Abstract/XD_ItemBase.h"
#include <Engine/World.h>
#include <Net/UnrealNetwork.h>
#include <Engine/ActorChannel.h>
#include <Components/StaticMeshComponent.h>
#include <Components/SkeletalMeshComponent.h>
#include <Engine/AssetManager.h>
#include <Engine/StreamableManager.h>

#include "Abstract/XD_ItemCoreBase.h"
#include "XD_ItemSystemUtility.h"

#define LOCTEXT_NAMESPACE "物品" 

// Sets default values
AXD_ItemBase::AXD_ItemBase(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bReplicates = true;
}

// Called when the game starts or when spawned
void AXD_ItemBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AXD_ItemBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AXD_ItemBase::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AXD_ItemBase, ItemCore, COND_InitialOnly);
}

bool AXD_ItemBase::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
	bool IsFailed = false;
	if (ItemCore)
	{
		IsFailed |= Channel->ReplicateSubobject(ItemCore, *Bunch, *RepFlags);
	}
	IsFailed |= Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	return IsFailed;
}

void AXD_ItemBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	WhenItemInWorldSetting();
}

void AXD_ItemBase::OnConstruction(const FTransform& Transform)
{
	if (ItemCore)
	{
		InitItemMesh();
	}
}

void AXD_ItemBase::InitStaticMeshComponent(UStaticMeshComponent* StaticMeshComponent)
{
	const FXD_ItemModelData& ModelData = ItemCore->GetCurrentItemModel();
	if (ensureAlways(ModelData.ModelType == EItemModelType::StaticMesh))
	{
		TSoftObjectPtr<UObject> StaticMesh = ModelData.Model;

		FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
		TArray<FSoftObjectPath> ItemsToStream = { StaticMesh.ToSoftObjectPath() };
		for (const TPair<FName, TSoftObjectPtr<UMaterialInterface>>& Pair : ModelData.MaterialOverride)
		{
			ItemsToStream.Add(Pair.Value.ToSoftObjectPath());
		}
		Streamable.RequestAsyncLoad(ItemsToStream, FStreamableDelegate::CreateWeakLambda(this, [=]
		{
			const FXD_ItemModelData& ModelData = ItemCore->GetCurrentItemModel();
			if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(ModelData.Model.Get()))
			{
				StaticMeshComponent->SetStaticMesh(StaticMesh);
				for (const TPair<FName, TSoftObjectPtr<UMaterialInterface>>& Pair : ModelData.MaterialOverride)
				{
					StaticMeshComponent->SetMaterialByName(Pair.Key, Pair.Value.Get());
				}
			}
		}));
	}
}

void AXD_ItemBase::InitSkeletalMeshComponent(USkeletalMeshComponent* SkeletalMeshComponent)
{
	const FXD_ItemModelData& ModelData = ItemCore->GetCurrentItemModel();
	if (ensureAlways(ModelData.ModelType == EItemModelType::SkeletalMesh))
	{
		TSoftObjectPtr<UObject> SkeletalMesh = ModelData.Model;

		FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
		TArray<FSoftObjectPath> ItemsToStream = { SkeletalMesh.ToSoftObjectPath() };
		for (const TPair<FName, TSoftObjectPtr<UMaterialInterface>>& Pair : ModelData.MaterialOverride)
		{
			ItemsToStream.Add(Pair.Value.ToSoftObjectPath());
		}
		Streamable.RequestAsyncLoad(ItemsToStream, FStreamableDelegate::CreateWeakLambda(this, [=]
		{
			const FXD_ItemModelData& ModelData = ItemCore->GetCurrentItemModel();
			if (USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(ModelData.Model.Get()))
			{
				SkeletalMeshComponent->SetSkeletalMesh(SkeletalMesh);
				for (const TPair<FName, TSoftObjectPtr<UMaterialInterface>>& Pair : ModelData.MaterialOverride)
				{
					SkeletalMeshComponent->SetMaterialByName(Pair.Key, Pair.Value.Get());
				}
			}
		}));
	}
}

void AXD_ItemBase::OnActorChannelOpen(class FInBunch& InBunch, class UNetConnection* Connection)
{
	Super::OnActorChannelOpen(InBunch, Connection);
}

void AXD_ItemBase::OnSerializeNewActor(class FOutBunch& OutBunch)
{
	Super::OnSerializeNewActor(OutBunch);
}

void AXD_ItemBase::WhenItemInWorldSetting()
{
	if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(GetRootComponent()))
	{
		AActor* AttachedParent = GetAttachParentActor();
		SetItemSimulatePhysics(AttachedParent == nullptr ? true : false);

		SetItemCollisionProfileName(GetDefault<UXD_ItemSystemSettings>()->ItemCollisionProfileName);
		Root->BodyInstance.bUseCCD = true;
		Root->SetCanEverAffectNavigation(false);
		SetReplicateMovement(true);
	}
}

void AXD_ItemBase::SetItemCollisionProfileName(const FName& CollisionProfileName)
{
	if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(GetRootComponent()))
	{
		Root->SetCollisionProfileName(CollisionProfileName);
	}
}

void AXD_ItemBase::SetItemSimulatePhysics(bool bSimulate)
{
	if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(GetRootComponent()))
	{
		Root->SetSimulatePhysics(bSimulate);
	}
}

void AXD_ItemBase::OnRep_AttachmentReplication()
{
	SetItemSimulatePhysics(GetAttachmentReplication().AttachParent == nullptr ? true : false);

	Super::OnRep_AttachmentReplication();
}

#if WITH_EDITOR
void AXD_ItemBase::PostLoad()
{
	Super::PostLoad();

	if (ItemCore)
	{
		if (ItemCore->CanMergeItem())
		{
			ensure(ItemCore->Number == 1 || ItemCore->Number >= ItemCore->GetMinItemMergeNumberValue());
			if (ItemCore->Number < ItemCore->GetMinItemMergeNumberValue())
			{
				ItemCore->Number = 1;
			}
		}
		else
		{
			ensure(ItemCore->Number == 1);
			ItemCore->Number = 1;
		}
	}
}

void AXD_ItemBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UXD_ItemCoreBase, Number))
	{
		if (ItemCore->GetSpawnedItemClass() == GetClass())
		{
			InitItemMesh();
		}
		else
		{
			// TODO：处理需要更换Actor的情况
			checkNoEntry();
		}
	}
}

#endif

void AXD_ItemBase::OnRep_ItemCore()
{
	if (ensure(ItemCore))
	{
		InitItemMesh();
	}
}

int32 AXD_ItemBase::GetNumber() const
{
	return ItemCore ? ItemCore->Number : 0;
}

UPrimitiveComponent* AXD_ItemBase::GetRootMeshComponent() const
{
	return CastChecked<UPrimitiveComponent>(GetRootComponent());
}

UXD_ItemCoreBase* AXD_ItemBase::CreateItemCoreByType(TSubclassOf<AXD_ItemBase> ItemClass, UObject* Outer)
{
	if (ItemClass)
	{
		return ItemClass.GetDefaultObject()->CreateItemCore(Outer);
	}
	return nullptr;
}

FText AXD_ItemBase::GetItemName() const
{
	return ItemCore ? ItemCore->GetItemName() : FText::GetEmpty();
}

class UXD_ItemCoreBase* AXD_ItemBase::GetItemCore() const
{
	return ItemCore;
}

const class UXD_ItemCoreBase* AXD_ItemBase::GetItemCoreConst() const
{
	return ItemCore;
}

class UXD_ItemCoreBase* AXD_ItemBase::CreateItemCore(UObject* Outer) const
{
	return UXD_ItemCoreBase::DeepDuplicateCore(ItemCore, Outer);
}

bool AXD_ItemBase::IsEqualWithItem(const AXD_ItemBase* Item) const
{
	return this && Item && GetItemCoreConst()->IsEqualWithItemCore(Item->GetItemCoreConst());
}

bool AXD_ItemBase::IsEqualWithItemCore(const class UXD_ItemCoreBase* CompareItemCore) const
{
	return this && ItemCore && ItemCore->IsEqualWithItemCore(CompareItemCore);
}

AXD_Item_StaticMesh::AXD_Item_StaticMesh(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(GET_MEMBER_NAME_CHECKED(AXD_Item_StaticMesh, StaticMeshComponent));

	SetRootComponent(StaticMeshComponent);
}

AXD_Item_SkeletalMesh::AXD_Item_SkeletalMesh(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(GET_MEMBER_NAME_CHECKED(AXD_Item_SkeletalMesh, SkeletalMeshComponent));

	SetRootComponent(SkeletalMeshComponent);
}

#undef LOCTEXT_NAMESPACE
