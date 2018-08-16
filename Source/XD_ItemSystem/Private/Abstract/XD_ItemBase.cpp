// Fill out your copyright notice in the Description page of Project Settings.

#include "XD_ItemBase.h"
#include "XD_ItemCoreBase.h"
#include "XD_ObjectFunctionLibrary.h"
#include <UnrealNetwork.h>
#include <Engine/ActorChannel.h>

#define LOCTEXT_NAMESPACE "物品" 

// Sets default values
AXD_ItemBase::AXD_ItemBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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

void AXD_ItemBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITOR
	if (bMeshInit == false)
	{
		InitRootMesh();
		bMeshInit = true;
	}
#endif
}

void AXD_ItemBase::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	InitRootMesh();
}

void AXD_ItemBase::InitRootMesh()
{
	if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(GetItemMesh()))
	{
		if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(GetRootComponent()))
		{
			StaticMeshComponent->SetStaticMesh(StaticMesh);
			RootMeshComponent = StaticMeshComponent;
		}
		else
		{
			StaticMeshComponent = NewObject<UStaticMeshComponent>(this, TEXT("静态网格体模型组件"));
			AddOwnedComponent(StaticMeshComponent);
			StaticMeshComponent->RegisterComponent();
			StaticMeshComponent->BodyInstance.bNotifyRigidBodyCollision = true;
			StaticMeshComponent->SetWorldTransform(GetActorTransform());

			GetRootComponent()->DestroyComponent();
			SetRootComponent(StaticMeshComponent);
			RootMeshComponent = StaticMeshComponent;

			StaticMeshComponent->SetStaticMesh(StaticMesh);
		}
	}
	else if (USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(GetItemMesh()))
	{
		if (USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(GetRootComponent()))
		{
			SkeletalMeshComponent->SetSkeletalMesh(SkeletalMesh);
			RootMeshComponent = SkeletalMeshComponent;
		}
		else
		{
			SkeletalMeshComponent = NewObject<USkeletalMeshComponent>(this, TEXT("骨骼体模型组件"));
			AddOwnedComponent(SkeletalMeshComponent);
			SkeletalMeshComponent->RegisterComponent();
			SkeletalMeshComponent->BodyInstance.bNotifyRigidBodyCollision = true;
			SkeletalMeshComponent->SetWorldTransform(GetActorTransform());

			auto AttachChildren = GetRootComponent()->GetAttachChildren();
			for (USceneComponent* SceneComponent : AttachChildren)
			{
				SceneComponent->AttachToComponent(SkeletalMeshComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true), SceneComponent->GetAttachSocketName());
			}

			GetRootComponent()->DestroyComponent();
			SetRootComponent(SkeletalMeshComponent);
			RootMeshComponent = SkeletalMeshComponent;

			SkeletalMeshComponent->SetSkeletalMesh(SkeletalMesh);
		}
	}

	for (auto& MaterialOverride : MaterialOverrideList)
	{
		RootMeshComponent->SetMaterialByName(MaterialOverride.Key, MaterialOverride.Value);
	}

	BeThrowedSetting();
}

void AXD_ItemBase::BeThrowedSetting()
{
	if (auto Root = Cast<UPrimitiveComponent>(GetRootComponent()))
	{
		Root->SetSimulatePhysics(true);
		Root->SetCollisionProfileName(TEXT("Item"));
		Root->BodyInstance.bUseCCD = true;
		Root->SetCanEverAffectNavigation(false);
		SetReplicateMovement(true);
	}
}

void AXD_ItemBase::WhenLoad_Implementation()
{
	InitRootMesh();
}

void AXD_ItemBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AXD_ItemBase, Mesh))
	{
		MaterialOverrideList.Empty();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(AXD_ItemBase, bNotPickBlueprintName))
	{
		if (bNotPickBlueprintName == false)
		{
			ItemName = FText::FromString(GetClass()->GetName().Left(GetClass()->GetName().Len() - 2));
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(AXD_ItemBase, MaterialOverrideList))
	{
		if (RootMeshComponent)
		{
			for (auto& MaterialOverride : MaterialOverrideList)
			{
				RootMeshComponent->SetMaterialByName(MaterialOverride.Key, MaterialOverride.Value);
			}
		}
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

void AXD_ItemBase::OnRep_InnerItemCore()
{
	if (InnerItemCore)
	{
		if (InnerItemCore->Number >= MinItemCompositeNumber)
		{
			//假设ItemCore的Number网络初始化在InnerItemCore前
			InitRootMesh();
		}
		WhenInnerItemCoreInited();
	}
}

int32 AXD_ItemBase::GetNumber() const
{
	return InnerItemCore ? InnerItemCore->Number : 0;
}

class UObject* AXD_ItemBase::GetItemMesh()
{
	return GetNumber() >= MinItemCompositeNumber ? ItemCompositeMesh : Mesh;
}

UXD_ItemCoreBase* AXD_ItemBase::CreateItemCoreByType(TSubclassOf<AXD_ItemBase> ItemClass, UObject* Outer)
{
	if (ItemClass)
	{
		return ItemClass.GetDefaultObject()->CreateItemCore(Outer);
	}
	return nullptr;
}

FText AXD_ItemBase::GetItemTypeDescImpl_Implementation(const class UXD_ItemCoreBase* ItemCore) const
{
	return LOCTEXT("AXD_ItemBase ItemTypeDesc", "继承物品");
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

bool AXD_ItemBase::EqualForItem(const AXD_ItemBase* Item) const
{
	return this && Item && GetItemCore()->EqualForItemCore(Item->GetItemCore());
}

bool AXD_ItemBase::EqualForItemCore(const class UXD_ItemCoreBase* CompareItemCore) const
{
	return this && InnerItemCore && InnerItemCore->EqualForItemCore(CompareItemCore);
}

void AXD_ItemBase::BeThrowedImpl_Implementation(AActor* WhoThrowed, UXD_ItemCoreBase* ItemCore, int32 ThrowNumber, ULevel* ThrowToLevel) const
{
	if (WhoThrowed)
	{
		FVector ThrowLocation = WhoThrowed->GetActorLocation() + WhoThrowed->GetActorRotation().RotateVector(FVector(100.f, 0.f, 0.f));
		FRotator ThrowRotation = WhoThrowed->GetActorRotation();
		if (ThrowNumber < MinItemCompositeNumber || ItemCompositeMesh == nullptr)
		{
			for (int i = 0; i < ThrowNumber; ++i)
			{
				AXD_ItemBase* SpawnedItem = ItemCore->SpawnItemActorInLevel(ThrowToLevel, ThrowLocation, ThrowRotation);
				SpawnedItem->BeThrowedSetting();
			}
		}
		else
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
	}
}

#undef LOCTEXT_NAMESPACE
