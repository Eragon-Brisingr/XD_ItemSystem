// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "XD_SaveGameInterface.h"
#include <GameplayTagContainer.h>
#include "XD_ItemBase.generated.h"

/**
*
*/

UCLASS(abstract)
class XD_ITEMSYSTEM_API AXD_ItemBase : public AActor, public IXD_SaveGameInterface
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
	void PostInitProperties() override;
	void PostInitializeComponents() override;
	void OnConstruction(const FTransform& Transform) override;
public:
	//ISaveGameInterface
	void WhenPostLoad_Implementation() override;
	//End ISaveGameInterface

public:
	//初始化模型
	virtual void InitItemMesh() {}
protected:
	void InitStaticMeshComponent(UStaticMeshComponent* StaticMeshComponent);
	void InitSkeletalMeshComponent(USkeletalMeshComponent* SkeletalMeshComponent);
public:
	//物品在世界中的处理
	virtual void WhenItemInWorldSetting();

	virtual void SetItemCollisionProfileName(const FName& CollisionProfileName);

	virtual void SetItemSimulatePhysics(bool bSimulate);

	UPROPERTY(ReplicatedUsing = "OnRep_ItemSimulatePhysics")
	uint8 bItemSimulatePhysics : 1;
	UFUNCTION()
	void OnRep_ItemSimulatePhysics();
public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif //WITH_EDITOR

public:
	UPROPERTY(VisibleAnywhere, Instanced, SaveGame, ReplicatedUsing = OnRep_ItemCore, Category = "物品", meta = (DisplayName = "物品核心"))
	class UXD_ItemCoreBase* ItemCore;

	UFUNCTION()
	virtual void OnRep_ItemCore();
	// 网络上即使Actor送达了可能ItemCore还没到，所以要用这个
	DECLARE_DELEGATE(FOnRepItemCoreNative);
	FOnRepItemCoreNative OnItemCoreValidNative;

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
	class UXD_ItemCoreBase* GetItemCore() const;

	const class UXD_ItemCoreBase* GetItemCoreConst() const;

	UFUNCTION(BlueprintCallable, Category = "物品")
	class UXD_ItemCoreBase* CreateItemCore(UObject* Outer) const;

	UFUNCTION(BlueprintPure, Category = "物品|基础")
	bool IsEqualWithItem(const AXD_ItemBase* Item) const;

	UFUNCTION(BlueprintCallable, Category = "物品")
	bool IsEqualWithItemCore(const class UXD_ItemCoreBase* CompareItemCore) const;

	//TODO：物品特殊行为
	UFUNCTION(BlueprintCallable, Category = "物品|基础", BlueprintNativeEvent)
	void ItemSpecialEvent(const FGameplayTag& EventTag);
	virtual void ItemSpecialEvent_Implementation(const FGameplayTag& EventTag){}
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
