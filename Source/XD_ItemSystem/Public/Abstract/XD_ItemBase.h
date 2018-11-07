// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "XD_SaveGameInterface.h"
#include <GameplayTagContainer.h>
#include "XD_ItemType.h"
#include "XD_ItemBase.generated.h"

/**
*
*/

UCLASS(Abstract)
class XD_ITEMSYSTEM_API AXD_ItemBase : public AActor, public IXD_SaveGameInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AXD_ItemBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const override;

	virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;

	virtual void PostInitProperties() override;

	virtual void PostInitializeComponents() override;
	//初始化模型
public:
#if WITH_EDITORONLY_DATA

	UPROPERTY()
	UStaticMeshComponent* BlueprintPreviewHelper;
#endif
	//ISaveGameInterface
	virtual void WhenPostLoad_Implementation() override;
	//End ISaveGameInterface

public:
	void InitRootMesh(bool ExecuteSpawnInWorldInit = true);

	//物品在世界中的处理
	virtual void WhenItemInWorldSetting();

	virtual void SetItemCollisionProfileName(const FName& CollisionProfileName);

	virtual void SetItemSimulatePhysics(bool bSimulate);
public:
#if WITH_EDITORONLY_DATA
	//取消勾选则采用蓝图名
	UPROPERTY(EditDefaultsOnly, Category = "物品", meta = (DisplayName = "不采用蓝图名"))
	bool bNotPickBlueprintName = true;
#endif

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif //WITH_EDITOR

public:
	UPROPERTY(EditAnywhere, Instanced, SaveGame, ReplicatedUsing = OnRep_InnerItemCore, Category = "物品", meta = (DisplayName = "物品核心"))
	class UXD_ItemCoreBase* InnerItemCore;

	UFUNCTION()
	virtual void OnRep_InnerItemCore();

	UPROPERTY(SaveGame, EditDefaultsOnly, BlueprintReadOnly, Category = "物品", meta = (DisplayName = "物品名", EditCondition = "bNotPickBlueprintName"))
	FText ItemName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "物品", meta = (DisplayName = "模型", AllowedClasses = "StaticMesh,SkeletalMesh"))
	TSoftObjectPtr<class UObject> ItemMesh;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "物品", meta = (DisplayName = "模型材质覆盖"))
	TMap<FName, TSoftObjectPtr<UMaterialInterface>> MeshMaterialOverrideList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "物品", meta = (DisplayName = "道具叠加时模型", AllowedClasses = "StaticMesh,SkeletalMesh"))
	TSoftObjectPtr<class UObject> ItemCompositeMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "物品", meta = (DisplayName = "模型材质覆盖"))
	TMap<FName, TSoftObjectPtr<UMaterialInterface>> CompositeMeshMaterialOverrideList;

	UPROPERTY(EditDefaultsOnly, Category = "物品", meta = (DisplayName = "最小叠加数目", ClampMin = "2", EditCondition = "bUseCompositeMesh"))
	int32 MinItemCompositeNumber = 5;

	UPROPERTY(EditDefaultsOnly, Category = "物品", meta = (DisplayName = "在背包中可以叠加"))
	uint8 bCanCompositeInInventory : 1;

	UFUNCTION(BlueprintCallable, Category = "物品")
	int32 GetNumber() const;

	bool CanCompositeItem() const { return !ItemCompositeMesh.IsNull(); }

	UFUNCTION(BlueprintCallable, Category = "物品|基础")
	bool IsCompositeItem() const { return CanCompositeItem() && GetNumber() >= MinItemCompositeNumber; }

	UFUNCTION(BlueprintCallable, Category = "物品")
	UPrimitiveComponent* GetRootMeshComponent() const;
protected:
	class UObject* GetItemMeshSync() const;

	void UpdateMaterialsOverrideSync();
public:
	UFUNCTION(BlueprintPure, Category = "物品", meta = (DisplayName = "Create Item Core", DeterminesOutputType = "ItemClass"))
	static UXD_ItemCoreBase* CreateItemCoreByType(TSubclassOf<AXD_ItemBase> ItemClass, UObject* Outer);

	//获取物品属性
public:
	UFUNCTION(BlueprintPure, Category = "物品|基础")
	FText GetItemName() const { return GetItemNameImpl(InnerItemCore); }

	//物品属性重写
public:
	UFUNCTION(BlueprintNativeEvent, Category = "物品|基础")
	FText GetItemNameImpl(const class UXD_ItemCoreBase* ItemCore) const;
	virtual FText GetItemNameImpl_Implementation(const class UXD_ItemCoreBase* ItemCore) const { return ItemName; }

	//非特殊情况不要去修改ItemCore的值，若要修改请使用CreateItemCore
	UFUNCTION(BlueprintPure, Category = "物品", meta = (DisplayName = "Get Item Core"))
	class UXD_ItemCoreBase* GetItemCore_Careful() const;

	const class UXD_ItemCoreBase* GetItemCore() const;

	UFUNCTION(BlueprintCallable, Category = "物品")
	class UXD_ItemCoreBase* CreateItemCore(UObject* Outer) const;

	UFUNCTION(BlueprintPure, Category = "物品|基础")
	bool EqualForItem(const AXD_ItemBase* Item) const;

	UFUNCTION(BlueprintCallable, Category = "物品")
	bool EqualForItemCore(const class UXD_ItemCoreBase* CompareItemCore) const;

	//默认扔出单一物品，若希望扔出一组，可重载
	UFUNCTION(BlueprintNativeEvent, Category = "物品|基础")
	void BeThrowedImpl(AActor* WhoThrowed, UXD_ItemCoreBase* ItemCore, int32 ThrowNumber, ULevel* ThrowToLevel) const;
	virtual void BeThrowedImpl_Implementation(AActor* WhoThrowed, UXD_ItemCoreBase* ItemCore, int32 ThrowNumber, ULevel* ThrowToLevel) const;

	UFUNCTION(BlueprintNativeEvent, Category = "物品|基础")
	void WhenRemoveFromInventory(class AActor* ItemOwner, class UXD_ItemCoreBase* ItemCore, int32 RemoveNumber, int32 ExistNumber) const;
	virtual void WhenRemoveFromInventory_Implementation(class AActor* ItemOwner, class UXD_ItemCoreBase* ItemCore, int32 RemoveNumber, int32 ExistNumber) const {}

	UFUNCTION(BlueprintCallable, Category = "物品|基础", BlueprintNativeEvent)
	void ItemSpecialEvent(const FGameplayTag& EventTag);
	virtual void ItemSpecialEvent_Implementation(const FGameplayTag& EventTag){}
};
