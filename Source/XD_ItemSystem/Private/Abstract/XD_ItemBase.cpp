// Fill out your copyright notice in the Description page of Project Settings.

#include "XD_ItemBase.h"
#include "XD_ItemCoreBase.h"
#include "XD_ObjectFunctionLibrary.h"
#include <UnrealNetwork.h>
#include <Engine/ActorChannel.h>
#include "XD_ActorFunctionLibrary.h"
#include "XD_ItemSystemUtility.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"

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

	DOREPLIFETIME_CONDITION(AXD_ItemBase, InnerItemCore, COND_InitialOnly);
	DOREPLIFETIME(AXD_ItemBase, bItemSimulatePhysics);
}

bool AXD_ItemBase::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
	bool IsFailed = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	if (InnerItemCore)
	{
		IsFailed |= Channel->ReplicateSubobject(InnerItemCore, *Bunch, *RepFlags);
	}

	return IsFailed;
}

void AXD_ItemBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	//在PostInitProperties后才执行WhenItemInWorldSetting，防止[Attempting to move a fully simulated skeletal mesh]的警告
	WhenItemInWorldSetting();
}

void AXD_ItemBase::OnConstruction(const FTransform& Transform)
{
	if (InnerItemCore)
	{
		InitItemMesh();
	}
}

void AXD_ItemBase::WhenPostLoad_Implementation()
{
	InitItemMesh();
}

void AXD_ItemBase::WhenItemInWorldSetting()
{
	if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(GetRootComponent()))
	{
		if (GetAttachParentActor() == nullptr)
		{
			SetItemSimulatePhysics(true);
		}
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
	bItemSimulatePhysics = bSimulate;
	OnRep_ItemSimulatePhysics();
}

void AXD_ItemBase::OnRep_ItemSimulatePhysics()
{
	if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(RootComponent))
	{
		Root->SetSimulatePhysics(bItemSimulatePhysics);
	}
}

#if WITH_EDITOR

void AXD_ItemBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UXD_ItemCoreBase, Number))
	{
		if (InnerItemCore->GetSpawnedItemClass() == GetClass())
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

void AXD_ItemBase::OnRep_InnerItemCore()
{
	if (InnerItemCore)
	{
		//假设ItemCore的Number网络初始化在InnerItemCore前
		InitItemMesh();
	}
}

int32 AXD_ItemBase::GetNumber() const
{
	return InnerItemCore ? InnerItemCore->Number : 0;
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
	return InnerItemCore ? InnerItemCore->GetItemName() : FText::GetEmpty();
}

class UXD_ItemCoreBase* AXD_ItemBase::GetItemCore_Careful() const
{
	return InnerItemCore;
}

const class UXD_ItemCoreBase* AXD_ItemBase::GetItemCore() const
{
	return InnerItemCore;
}

class UXD_ItemCoreBase* AXD_ItemBase::CreateItemCore(UObject* Outer) const
{
	return UXD_ItemCoreBase::DeepDuplicateCore(InnerItemCore, Outer);
}

bool AXD_ItemBase::IsEqualWithItem(const AXD_ItemBase* Item) const
{
	return this && Item && GetItemCore()->IsEqualWithItemCore(Item->GetItemCore());
}

bool AXD_ItemBase::IsEqualWithItemCore(const class UXD_ItemCoreBase* CompareItemCore) const
{
	return this && InnerItemCore && InnerItemCore->IsEqualWithItemCore(CompareItemCore);
}

AXD_Item_StaticMesh::AXD_Item_StaticMesh(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(GET_MEMBER_NAME_CHECKED(AXD_Item_StaticMesh, StaticMeshComponent));

	SetRootComponent(StaticMeshComponent);
}

void AXD_Item_StaticMesh::InitItemMesh()
{
	TSoftObjectPtr<UObject> StaticMesh = InnerItemCore->IsMergedItem() ? InnerItemCore->ItemMergeMesh : InnerItemCore->ItemMesh;
	// TODO：异步加载
	UStaticMesh* ItemMesh = CastChecked<UStaticMesh>(StaticMesh.LoadSynchronous());
	StaticMeshComponent->SetStaticMesh(ItemMesh);
}

AXD_Item_SkeletalMesh::AXD_Item_SkeletalMesh(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(GET_MEMBER_NAME_CHECKED(AXD_Item_SkeletalMesh, SkeletalMeshComponent));

	SetRootComponent(SkeletalMeshComponent);
}

void AXD_Item_SkeletalMesh::InitItemMesh()
{	
	TSoftObjectPtr<UObject> SkeletalMesh = InnerItemCore->IsMergedItem() ? InnerItemCore->ItemMergeMesh : InnerItemCore->ItemMesh;
	// TODO：异步加载
	USkeletalMesh* ItemMesh = CastChecked<USkeletalMesh>(SkeletalMesh.LoadSynchronous());
	SkeletalMeshComponent->SetSkeletalMesh(ItemMesh);
}

#undef LOCTEXT_NAMESPACE
