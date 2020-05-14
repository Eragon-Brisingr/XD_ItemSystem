﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <GameFramework/Actor.h>
#include "XD_ItemBase.generated.h"

class UXD_ItemCoreBase;

/**
*
*/

UCLASS(abstract, NotBlueprintable, notplaceable)
class XD_ITEMSYSTEM_API AXD_ItemBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AXD_ItemBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
protected:
	// Called when the game starts or when spawned
	void BeginPlay() override;
public:
	// Called every frame
	void Tick(float DeltaTime) override;

	void GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const override;
	bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;
	void PostInitializeComponents() override;
	void OnConstruction(const FTransform& Transform) override;
	void OnRep_AttachmentReplication() override;
#if WITH_EDITOR
	void PostLoad() override;
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif //WITH_EDITOR
	void OnActorChannelOpen(class FInBunch& InBunch, class UNetConnection* Connection) override;
	void OnSerializeNewActor(class FOutBunch& OutBunch) override;
public:
	//初始化模型
	virtual void InitItemMesh() {}

	//物品在世界中的处理
	virtual void WhenItemInWorldSetting();
	virtual void SetItemCollisionProfileName(const FName& CollisionProfileName);
	virtual void SetItemSimulatePhysics(bool bSimulate);
protected:
	void InitStaticMeshComponent(UStaticMeshComponent* StaticMeshComponent);
	void InitSkeletalMeshComponent(USkeletalMeshComponent* SkeletalMeshComponent);
public:
	UPROPERTY(VisibleAnywhere, Instanced, SaveGame, ReplicatedUsing = OnRep_ItemCore, Category = "物品", meta = (DisplayName = "物品核心"))
	UXD_ItemCoreBase* ItemCore;
	UFUNCTION()
	void OnRep_ItemCore();
	// 网络上即使Actor送达了可能ItemCore还没到，所以要用这个
	DECLARE_DELEGATE(FOnRepItemCoreNative);
	FOnRepItemCoreNative OnItemCoreValidNative;

#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient)
	TSubclassOf<UXD_ItemCoreBase> BelongToCoreType;
#endif

	UFUNCTION(BlueprintCallable, Category = "物品")
	int32 GetNumber() const;

	UFUNCTION(BlueprintCallable, Category = "物品")
	UPrimitiveComponent* GetRootMeshComponent() const;
public:
	UFUNCTION(BlueprintPure, Category = "物品", meta = (DisplayName = "Create Item Core", DeterminesOutputType = "ItemClass"))
	static UXD_ItemCoreBase* CreateItemCoreByType(TSubclassOf<AXD_ItemBase> ItemClass, UObject* Outer);

	//获取物品属性
public:
	UFUNCTION(BlueprintPure, Category = "物品|基础")
	FText GetItemName() const;

public:
	//非特殊情况不要去修改ItemCore的值，若要修改请使用CreateItemCore
	UFUNCTION(BlueprintPure, Category = "物品", meta = (DisplayName = "Get Item Core"))
	UXD_ItemCoreBase* GetItemCore() const;

	const UXD_ItemCoreBase* GetItemCoreConst() const;

	UFUNCTION(BlueprintCallable, Category = "物品")
	UXD_ItemCoreBase* CreateItemCore(UObject* Outer) const;

	UFUNCTION(BlueprintPure, Category = "物品|基础")
	bool IsEqualWithItem(const AXD_ItemBase* Item) const;

	UFUNCTION(BlueprintCallable, Category = "物品")
	bool IsEqualWithItemCore(const UXD_ItemCoreBase* CompareItemCore) const;
};

UCLASS()
class XD_ITEMSYSTEM_API AXD_Item_StaticMesh : public AXD_ItemBase
{
	GENERATED_BODY()
public:
	// Sets default values for this actor's properties
	AXD_Item_StaticMesh(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
public:
	UPROPERTY()
	UStaticMeshComponent* StaticMeshComponent;

	void InitItemMesh() override { InitStaticMeshComponent(StaticMeshComponent); }
};

UCLASS()
class XD_ITEMSYSTEM_API AXD_Item_SkeletalMesh : public AXD_ItemBase
{
	GENERATED_BODY()
public:
	// Sets default values for this actor's properties
	AXD_Item_SkeletalMesh(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
public:
	UPROPERTY()
	USkeletalMeshComponent* SkeletalMeshComponent;

	void InitItemMesh() override { InitSkeletalMeshComponent(SkeletalMeshComponent); }
};
