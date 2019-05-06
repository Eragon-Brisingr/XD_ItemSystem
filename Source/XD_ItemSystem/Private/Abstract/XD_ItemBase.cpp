﻿// Fill out your copyright notice in the Description page of Project Settings.

#include "XD_ItemBase.h"
#include "XD_ItemCoreBase.h"
#include "XD_ObjectFunctionLibrary.h"
#include <UnrealNetwork.h>
#include <Engine/ActorChannel.h>
#include "XD_ActorFunctionLibrary.h"
#include "XD_ItemSystemUtility.h"

#define LOCTEXT_NAMESPACE "物品" 

// Sets default values
AXD_ItemBase::AXD_ItemBase(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer),
	ItemName(LOCTEXT("物品", "物品")),
	bCanCompositeInInventory(true)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bReplicates = true;

	InnerItemCore = CreateDefaultSubobject<UXD_ItemCoreBase>(GET_MEMBER_NAME_CHECKED(AXD_ItemBase, InnerItemCore));

#if WITH_EDITOR
	BlueprintPreviewHelper = CreateEditorOnlyDefaultSubobject<UStaticMeshComponent>(TEXT("蓝图预览图用"), true);
	if (BlueprintPreviewHelper)
	{
		BlueprintPreviewHelper->bIsEditorOnly = true;
		BlueprintPreviewHelper->SetStaticMesh(ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'")).Object);
		BlueprintPreviewHelper->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		BlueprintPreviewHelper->SetWorldScale3D(FVector(0.0001f, 0.0001f, 0.0001f));
	}
#endif
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

#if WITH_EDITOR
	if (InnerItemCore)
	{
		InnerItemCore->ItemClass = GetClass();
	}
#endif
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

void AXD_ItemBase::PostInitProperties()
{
	Super::PostInitProperties();
#if WITH_EDITOR
	if (BlueprintPreviewHelper)
	{
		BlueprintPreviewHelper->DestroyComponent();
	}
#endif
	if (InnerItemCore)
	{
		InnerItemCore->ItemClass = GetClass();
	}
	InitRootMesh(false);
}

void AXD_ItemBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	//删除读取的物品携带的默认根骨骼
	for (UActorComponent* ActorComponent : GetComponents())
	{
		if (ActorComponent->GetFName() == USceneComponent::GetDefaultSceneRootVariableName())
		{
			ActorComponent->DestroyComponent();
			break;
		}
	}
	//在PostInitProperties后才执行WhenItemInWorldSetting，防止[Attempting to move a fully simulated skeletal mesh]的警告
	WhenItemInWorldSetting();
}

void AXD_ItemBase::InitRootMesh(bool ExecuteSpawnInWorldInit)
{
	bool NeedReinit = true;

	if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(GetItemMeshSync()))
	{
		if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(GetRootComponent()))
		{
			if (StaticMeshComponent->GetStaticMesh() != StaticMesh)
			{
				StaticMeshComponent->SetStaticMesh(StaticMesh);
			}
			else
			{
				NeedReinit = false;
			}
		}
		else
		{
			StaticMeshComponent = UXD_ActorFunctionLibrary::AddComponent<UStaticMeshComponent>(this, TEXT("静态网格体模型组件"));
			StaticMeshComponent->BodyInstance.bNotifyRigidBodyCollision = true;
			StaticMeshComponent->SetWorldTransform(GetActorTransform());

			if (USceneComponent* TheRootComponent = GetRootComponent())
			{
				TArray<USceneComponent*> ChildrenCompnents = TheRootComponent->GetAttachChildren();
				for (USceneComponent* SceneComponent : ChildrenCompnents)
				{
					SceneComponent->AttachToComponent(StaticMeshComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true), SceneComponent->GetAttachSocketName());
				}
				TheRootComponent->DestroyComponent();
			}
			SetRootComponent(StaticMeshComponent);

			StaticMeshComponent->SetStaticMesh(StaticMesh);
		}
	}
	else if (USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(GetItemMeshSync()))
	{
		if (USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(GetRootComponent()))
		{
			if (SkeletalMeshComponent->SkeletalMesh != SkeletalMesh)
			{
				SkeletalMeshComponent->SetSkeletalMesh(SkeletalMesh);
			}
			else
			{
				NeedReinit = false;
			}
		}
		else
		{
			SkeletalMeshComponent = UXD_ActorFunctionLibrary::AddComponent<USkeletalMeshComponent>(this, TEXT("骨骼体模型组件"));
			SkeletalMeshComponent->BodyInstance.bNotifyRigidBodyCollision = true;
			SkeletalMeshComponent->SetWorldTransform(GetActorTransform());
			if (USceneComponent* TheRootComponent = GetRootComponent())
			{
				TArray<USceneComponent*> ChildrenCompnents = TheRootComponent->GetAttachChildren();
				for (USceneComponent* SceneComponent : ChildrenCompnents)
				{
					SceneComponent->AttachToComponent(SkeletalMeshComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true), SceneComponent->GetAttachSocketName());
				}
				TheRootComponent->DestroyComponent();
			}

			SetRootComponent(SkeletalMeshComponent);

			SkeletalMeshComponent->SetSkeletalMesh(SkeletalMesh);
		}
	}

	if (NeedReinit && ExecuteSpawnInWorldInit)
	{
		WhenItemInWorldSetting();
	}
}

void AXD_ItemBase::WhenPostLoad_Implementation()
{
	InitRootMesh();
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

	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AXD_ItemBase, ItemMesh))
	{
		MeshMaterialOverrideList.Empty();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(AXD_ItemBase, bNotPickBlueprintName))
	{
		if (bNotPickBlueprintName == false)
		{
			ItemName = FText::FromString(GetClass()->GetName().Left(GetClass()->GetName().Len() - 2));
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(AXD_ItemBase, MeshMaterialOverrideList)
				|| PropertyName == GET_MEMBER_NAME_CHECKED(AXD_ItemBase, CompositeMeshMaterialOverrideList))
	{
		UpdateMaterialsOverrideSync();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(AXD_ItemBase, InnerItemCore))
	{
		if (InnerItemCore)
		{
			InnerItemCore->ItemClass = GetClass();
		}
		else
		{
			InnerItemCore = NewObject<UXD_ItemCoreBase>(this, GET_MEMBER_NAME_CHECKED(AXD_ItemBase, InnerItemCore));
		}
	}
}

#endif

void AXD_ItemBase::OnRep_InnerItemCore()
{
	if (InnerItemCore)
	{
		//假设ItemCore的Number网络初始化在InnerItemCore前
		InitRootMesh();
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

class UObject* AXD_ItemBase::GetItemMeshSync() const
{
	return IsCompositeItem() ? ItemCompositeMesh.LoadSynchronous() : ItemMesh.LoadSynchronous();
}

void AXD_ItemBase::UpdateMaterialsOverrideSync()
{
	if (IsCompositeItem())
	{
		for (auto& MaterialOverride : CompositeMeshMaterialOverrideList)
		{
			GetRootMeshComponent()->SetMaterialByName(MaterialOverride.Key, MaterialOverride.Value.LoadSynchronous());
		}
	}
	else
	{
		for (auto& MaterialOverride : MeshMaterialOverrideList)
		{
			GetRootMeshComponent()->SetMaterialByName(MaterialOverride.Key, MaterialOverride.Value.LoadSynchronous());
		}
	}
}

UXD_ItemCoreBase* AXD_ItemBase::CreateItemCoreByType(TSubclassOf<AXD_ItemBase> ItemClass, UObject* Outer)
{
	if (ItemClass)
	{
		return ItemClass.GetDefaultObject()->CreateItemCore(Outer);
	}
	return nullptr;
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
	return UXD_ObjectFunctionLibrary::DuplicateObject(InnerItemCore, Outer);
}

bool AXD_ItemBase::IsEqualWithItem(const AXD_ItemBase* Item) const
{
	return this && Item && GetItemCore()->IsEqualWithItemCore(Item->GetItemCore());
}

bool AXD_ItemBase::IsEqualWithItemCore(const class UXD_ItemCoreBase* CompareItemCore) const
{
	return this && InnerItemCore && InnerItemCore->IsEqualWithItemCore(CompareItemCore);
}

void AXD_ItemBase::BeThrowedImpl_Implementation(AActor* WhoThrowed, UXD_ItemCoreBase* ItemCore, int32 ThrowNumber, ULevel* ThrowToLevel) const
{
	if (WhoThrowed)
	{
		FVector ThrowLocation = WhoThrowed->GetActorLocation() + WhoThrowed->GetActorRotation().RotateVector(FVector(100.f, 0.f, 0.f));
		FRotator ThrowRotation = WhoThrowed->GetActorRotation();
		if (ThrowNumber > MinItemCompositeNumber && CanCompositeItem())
		{
			FActorSpawnParameters ActorSpawnParameters;
			ActorSpawnParameters.OverrideLevel = ThrowToLevel;
			ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			ActorSpawnParameters.bDeferConstruction = true;
			AXD_ItemBase* ItemActor = ThrowToLevel->GetWorld()->SpawnActor<AXD_ItemBase>(ItemCore->ItemClass, ThrowLocation, ThrowRotation, ActorSpawnParameters);
			ItemActor->InnerItemCore = UXD_ObjectFunctionLibrary::DuplicateObject(ItemCore, ItemActor);
			ItemActor->InnerItemCore->Number = ThrowNumber;
			ItemActor->FinishSpawning(FTransform(ThrowRotation, ThrowLocation));
		}
		else
		{
			for (int i = 0; i < ThrowNumber; ++i)
			{
				AXD_ItemBase* SpawnedItem = ItemCore->SpawnItemActorInLevel(ThrowToLevel, 1, ThrowLocation, ThrowRotation);
				SpawnedItem->WhenItemInWorldSetting();
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
