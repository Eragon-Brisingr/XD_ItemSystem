// Fill out your copyright notice in the Description page of Project Settings.

#include "Abstract/GameplayItemBase.h"
#include <Engine/World.h>
#include <Net/UnrealNetwork.h>
#include <Engine/ActorChannel.h>
#include <Components/StaticMeshComponent.h>
#include <Components/SkeletalMeshComponent.h>
#include <Engine/AssetManager.h>
#include <Engine/StreamableManager.h>
#if WITH_EDITOR
#include <Misc/UObjectToken.h>
#endif

#include "Abstract/GameplayItemCoreBase.h"
#include "GameplayItemSystemUtility.h"
#include "Logging/TokenizedMessage.h"

#define LOCTEXT_NAMESPACE "物品" 

// Sets default values
AGameplayItemBase::AGameplayItemBase(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bReplicates = true;
}

// Called when the game starts or when spawned
void AGameplayItemBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGameplayItemBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGameplayItemBase::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AGameplayItemBase, ItemCore, COND_InitialOnly);
}

bool AGameplayItemBase::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
	bool IsFailed = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	if (ItemCore)
	{
		IsFailed |= Channel->ReplicateSubobject(ItemCore, *Bunch, *RepFlags);
	}
	return IsFailed;
}

void AGameplayItemBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	ItemInWorldSetting();
}

void AGameplayItemBase::OnConstruction(const FTransform& Transform)
{
	if (ItemCore)
	{
		InitItemMesh();
	}
}

void AGameplayItemBase::OnRep_AttachmentReplication()
{
	SetItemSimulatePhysics(GetAttachmentReplication().AttachParent == nullptr ? true : false);

	Super::OnRep_AttachmentReplication();
}

#if WITH_EDITOR
void AGameplayItemBase::PostLoad()
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

	CheckItemErrors(TEXT("LoadErrors"));
}

void AGameplayItemBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UGameplayItemCoreBase, Number))
	{
		if (ensureAlways(ItemCore->GetSpawnedItemClass() == GetClass()))
		{
			InitItemMesh();
		}
	}
}

void AGameplayItemBase::CheckForErrors()
{
	Super::CheckForErrors();
	CheckItemErrors(TEXT("MapCheck"));
}

void AGameplayItemBase::CheckItemErrors(const FName& LogName)
{
	if (ItemCore == nullptr)
	{
		FMessageLog(LogName).Error()
			->AddToken(FUObjectToken::Create(this))
			->AddToken(FTextToken::Create(LOCTEXT("物品核心为空", "物品核心为空")));
	}
	else
	{
		const UClass* DesiredClass = ItemCore->GetSpawnedItemClass();
		const UClass* CurrentClass = GetClass();
		if (DesiredClass != CurrentClass)
		{
			FMessageLog(LogName).Error()
				->AddToken(FUObjectToken::Create(this))
				->AddToken(FTextToken::Create(LOCTEXT("物品核心需要的实体类型为", "物品核心需要的实体类型为")))
				->AddToken(FUObjectToken::Create(DesiredClass))
				->AddToken(FTextToken::Create(LOCTEXT("当前类型为", "当前类型为")))
				->AddToken(FUObjectToken::Create(CurrentClass));
		}
		if (ItemCore->Number != 1 && ItemCore->Number < ItemCore->GetMinItemMergeNumberValue())
		{
			FMessageLog(LogName).Error()
				->AddToken(FUObjectToken::Create(this))
				->AddToken(FTextToken::Create(FText::Format(LOCTEXT("物品为堆叠类型，但是当前数量[{0}]小于最小堆叠数量[{1}]", "物品为堆叠类型，但是当前数量[{0}]小于最小堆叠数量[{1}]"), ItemCore->Number, ItemCore->GetMinItemMergeNumberValue())));
		}
	}
}
#endif

void AGameplayItemBase::InitStaticMeshComponent(UStaticMeshComponent* StaticMeshComponent)
{
	const FGameplayItemModelData& ModelData = ItemCore->GetCurrentItemModel();
	if (ensureAlways(ModelData.ModelType == EItemModelType::StaticMesh))
	{
		TSoftObjectPtr<UObject> StaticMesh = ModelData.Model;

		bool IsAllLoaded = StaticMesh.IsValid();
		for (const TPair<FName, TSoftObjectPtr<UMaterialInterface>>& Pair : ModelData.MaterialOverride)
		{
			IsAllLoaded &= Pair.Value.IsValid();
		}
		auto WhenAllLoaded = [=]
		{
			if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(ModelData.Model.Get()))
			{
				StaticMeshComponent->SetStaticMesh(StaticMesh);
				for (const TPair<FName, TSoftObjectPtr<UMaterialInterface>>& Pair : ModelData.MaterialOverride)
				{
					StaticMeshComponent->SetMaterialByName(Pair.Key, Pair.Value.Get());
				}
			}
		};
		if (IsAllLoaded)
		{
			WhenAllLoaded();
		}
		else
		{
			FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
			TArray<FSoftObjectPath> ItemsToStream = { StaticMesh.ToSoftObjectPath() };
			for (const TPair<FName, TSoftObjectPtr<UMaterialInterface>>& Pair : ModelData.MaterialOverride)
			{
				ItemsToStream.Add(Pair.Value.ToSoftObjectPath());
			}
			Streamable.RequestAsyncLoad(ItemsToStream, FStreamableDelegate::CreateWeakLambda(this, WhenAllLoaded));
		}
	}
}

void AGameplayItemBase::InitSkeletalMeshComponent(USkeletalMeshComponent* SkeletalMeshComponent)
{
	const FGameplayItemModelData& ModelData = ItemCore->GetCurrentItemModel();
	if (ensureAlways(ModelData.ModelType == EItemModelType::SkeletalMesh))
	{
		TSoftObjectPtr<UObject> SkeletalMesh = ModelData.Model;

		bool IsAllLoaded = SkeletalMesh.IsValid();
		for (const TPair<FName, TSoftObjectPtr<UMaterialInterface>>& Pair : ModelData.MaterialOverride)
		{
			IsAllLoaded &= Pair.Value.IsValid();
		}
		auto WhenAllLoaded = [=]
		{
			if (USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(ModelData.Model.Get()))
			{
				SkeletalMeshComponent->SetSkeletalMesh(SkeletalMesh);
				for (const TPair<FName, TSoftObjectPtr<UMaterialInterface>>& Pair : ModelData.MaterialOverride)
				{
					SkeletalMeshComponent->SetMaterialByName(Pair.Key, Pair.Value.Get());
				}
			}
		};
		if (IsAllLoaded)
		{
			WhenAllLoaded();
		}
		else
		{
			FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
			TArray<FSoftObjectPath> ItemsToStream = { SkeletalMesh.ToSoftObjectPath() };
			for (const TPair<FName, TSoftObjectPtr<UMaterialInterface>>& Pair : ModelData.MaterialOverride)
			{
				ItemsToStream.Add(Pair.Value.ToSoftObjectPath());
			}
			Streamable.RequestAsyncLoad(ItemsToStream, FStreamableDelegate::CreateWeakLambda(this, WhenAllLoaded));
		}
	}
}

void AGameplayItemBase::OnActorChannelOpen(class FInBunch& InBunch, class UNetConnection* Connection)
{
	Super::OnActorChannelOpen(InBunch, Connection);
}

void AGameplayItemBase::OnSerializeNewActor(class FOutBunch& OutBunch)
{
	Super::OnSerializeNewActor(OutBunch);
}

void AGameplayItemBase::WhenItemInWorld()
{
	if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(GetRootComponent()))
	{
		AActor* AttachedParent = GetAttachParentActor();
		SetItemSimulatePhysics(AttachedParent == nullptr ? true : false);

		SetItemCollisionProfileName(GetDefault<UGameplayItemSystemSettings>()->ItemCollisionProfileName);
		Root->BodyInstance.bUseCCD = true;
		Root->SetCanEverAffectNavigation(false);
		SetReplicateMovement(true);
	}
	ReceiveWhenItemInWorld();
}

void AGameplayItemBase::WhenSetItemCollisionProfileName(const FName& CollisionProfileName)
{
	if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(GetRootComponent()))
	{
		Root->SetCollisionProfileName(CollisionProfileName);
	}
	ReceiveWhenSetItemCollisionProfileName(CollisionProfileName);
}

void AGameplayItemBase::WhenSetItemSimulatePhysics(bool bSimulate)
{
	if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(GetRootComponent()))
	{
		Root->SetSimulatePhysics(bSimulate);
	}
	ReceiveWhenSetItemSimulatePhysics(bSimulate);
}

void AGameplayItemBase::OnRep_ItemCore()
{
	if (ensure(ItemCore))
	{
		InitItemMesh();
		if (OnItemCoreValidNative.IsBound())
		{
			OnItemCoreValidNative.Execute();
			OnItemCoreValidNative.Unbind();
		}
	}
}

int32 AGameplayItemBase::GetNumber() const
{
	return ItemCore ? ItemCore->Number : 0;
}

UPrimitiveComponent* AGameplayItemBase::GetRootMeshComponent() const
{
	return CastChecked<UPrimitiveComponent>(GetRootComponent());
}

FText AGameplayItemBase::GetItemName() const
{
	return ItemCore ? ItemCore->GetItemName() : FText::GetEmpty();
}

class UGameplayItemCoreBase* AGameplayItemBase::GetItemCore() const
{
	return ItemCore;
}

const class UGameplayItemCoreBase* AGameplayItemBase::GetItemCoreConst() const
{
	return ItemCore;
}

class UGameplayItemCoreBase* AGameplayItemBase::CreateItemCore(UObject* Outer) const
{
	return UGameplayItemCoreBase::DeepDuplicateCore(ItemCore, Outer);
}

bool AGameplayItemBase::IsEqualWithItem(const AGameplayItemBase* Item) const
{
	return this && Item && GetItemCoreConst()->IsEqualWithItemCore(Item->GetItemCoreConst());
}

bool AGameplayItemBase::IsEqualWithItemCore(const class UGameplayItemCoreBase* CompareItemCore) const
{
	return this && ItemCore && ItemCore->IsEqualWithItemCore(CompareItemCore);
}

AGameplayItem_StaticMesh::AGameplayItem_StaticMesh(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(GET_MEMBER_NAME_CHECKED(AGameplayItem_StaticMesh, StaticMeshComponent));

	SetRootComponent(StaticMeshComponent);
}

AGameplayItem_SkeletalMesh::AGameplayItem_SkeletalMesh(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(GET_MEMBER_NAME_CHECKED(AGameplayItem_SkeletalMesh, SkeletalMeshComponent));

	SetRootComponent(SkeletalMeshComponent);
}

#undef LOCTEXT_NAMESPACE
