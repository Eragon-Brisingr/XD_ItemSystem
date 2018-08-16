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

UCLASS()
class XD_ITEMSYSTEM_API AXD_ItemBase : public AActor, public IXD_SaveGameInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AXD_ItemBase();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const override;

	virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;

	//初始化模型
public:
#if WITH_EDITORONLY_DATA
	bool bMeshInit;
#endif

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void PreInitializeComponents() override;

	void InitRootMesh();

	void BeThrowedSetting();

	//ISaveGameInterface
	virtual void WhenLoad_Implementation() override;
	//End ISaveGameInterface

public:
#if WITH_EDITORONLY_DATA
	//取消勾选则采用蓝图名
	UPROPERTY(EditAnywhere, Category = "物品", meta = (DisplayName = "不采用蓝图名"))
	bool bNotPickBlueprintName = true;
#endif

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif //WITH_EDITOR
public:
public:
	UPROPERTY(EditAnywhere, Instanced, SaveGame, ReplicatedUsing = OnRep_InnerItemCore, Category = "物品", meta = (DisplayName = "物品核心"))
	class UXD_ItemCoreBase* InnerItemCore;

	UFUNCTION()
	void OnRep_InnerItemCore();

	UFUNCTION(BlueprintImplementableEvent, Category = "物品")
	void WhenInnerItemCoreInited();

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category = "物品", meta = (DisplayName = "物品名", EditCondition = "bNotPickBlueprintName"))
	FText ItemName;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category = "物品", meta = (DisplayName = "重量"))
	float Weight;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category = "物品", meta = (DisplayName = "价格"))
	float Price;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category = "物品", meta = (DisplayName = "描述"))
	FText Describe;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "物品", meta = (DisplayName = "模型", AllowedClasses = "StaticMesh, SkeletalMesh"))
	class UObject* Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "物品", meta = (DisplayName = "道具叠加时模型", AllowedClasses = "StaticMesh, SkeletalMesh"))
	class UObject* ItemCompositeMesh;

	UPROPERTY(EditDefaultsOnly, Category = "物品", meta = (DisplayName = "最小叠加数目", ClampMin = "0", EditCondition = "bUseCompositeMesh"))
	int32 MinItemCompositeNumber = 5;

	UFUNCTION(BlueprintCallable, Category = "物品")
	int32 GetNumber() const;

	UFUNCTION(BlueprintCallable, Category = "物品|基础")
	bool CanCompositeItem() const { return ItemCompositeMesh != nullptr; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "物品", meta = (DisplayName = "模型材质覆盖"))
	TMap<FName, UMaterialInterface*> MaterialOverrideList;

	UPROPERTY(BlueprintReadOnly, Category = "物品")
	UPrimitiveComponent* RootMeshComponent;
protected:
	class UObject* GetItemMesh();

	UFUNCTION(BlueprintPure, Category = "物品", meta = (DisplayName = "Create Item Core", DeterminesOutputType = "ItemClass"))
	static UXD_ItemCoreBase* CreateItemCoreByType(TSubclassOf<AXD_ItemBase> ItemClass, UObject* Outer);

	//获取物品属性
public:
	UFUNCTION(BlueprintPure, Category = "物品|基础")
	FText GetItemName() const { return GetItemNameImpl(InnerItemCore); }

	UFUNCTION(BlueprintPure, Category = "物品|基础")
	float GetWeight() const { return GetWeightImpl(InnerItemCore); }

	UFUNCTION(BlueprintPure, Category = "物品|基础")
	float GetPrice() const { return GetPriceImpl(InnerItemCore); }

	UFUNCTION(BlueprintPure, Category = "物品|基础")
	FText GetItemTypeDesc() const { return GetItemTypeDescImpl(InnerItemCore); }

	//物品属性重写
public:
	UFUNCTION(BlueprintNativeEvent, Category = "物品|基础")
	FText GetItemNameImpl(const class UXD_ItemCoreBase* ItemCore) const;
	virtual FText GetItemNameImpl_Implementation(const class UXD_ItemCoreBase* ItemCore) const { return ItemName; }

	UFUNCTION(BlueprintNativeEvent, Category = "物品|基础")
	float GetWeightImpl(const class UXD_ItemCoreBase* ItemCore) const;
	virtual float GetWeightImpl_Implementation(const class UXD_ItemCoreBase* ItemCore) const { return Weight; }

	UFUNCTION(BlueprintNativeEvent, Category = "物品|基础")
	float GetPriceImpl(const class UXD_ItemCoreBase* ItemCore) const;
	virtual float GetPriceImpl_Implementation(const class UXD_ItemCoreBase* ItemCore) const { return Price; }

	UFUNCTION(BlueprintNativeEvent, Category = "物品|基础")
	FText GetItemTypeDescImpl(const class UXD_ItemCoreBase* ItemCore) const;
	virtual FText GetItemTypeDescImpl_Implementation(const class UXD_ItemCoreBase* ItemCore) const;

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

	UFUNCTION(BlueprintNativeEvent, Category = "物品|基础")
	void UseItemImpl(class UXD_ItemCoreBase* ItemCore, class APawn* ItemOwner, EUseItemInput UseItemInput) const;
	virtual void UseItemImpl_Implementation(class UXD_ItemCoreBase* ItemCore, class APawn* ItemOwner, EUseItemInput UseItemInput) const{}

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
