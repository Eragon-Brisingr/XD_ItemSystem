// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <Engine/EngineTypes.h>
#include <UObject/NoExportTypes.h>
#include "XD_ItemCoreBase.generated.h"

class AXD_ItemBase;
class UXD_InventoryComponentBase;
class UMaterialInterface;

/**
 * 
 */
#if WITH_EDITOR
// 用此作用域生成的道具实例是不会被保存、且编辑器的层级窗不可见
struct XD_ITEMSYSTEM_API FSpawnPreviewItemScope
{
	FSpawnPreviewItemScope() { Counter += 1; }
	~FSpawnPreviewItemScope() { Counter -= 1; }
	static bool IsSpawnPreviewItem() { return Counter != 0; }
private:
	static uint8 Counter;
};
#endif

UCLASS(abstract, BlueprintType, EditInlineNew)
class XD_ITEMSYSTEM_API UXD_ItemCoreBase : public UObject
{
	GENERATED_BODY()
	
public:
	UXD_ItemCoreBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	bool IsSupportedForNetworking() const override;
	void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	UWorld* GetWorld() const override;

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif //WITH_EDITOR
public:
	UPROPERTY(Transient)
	UXD_InventoryComponentBase* OwingInventory;
	
	UPROPERTY(SaveGame, EditDefaultsOnly, BlueprintReadOnly, Category = "物品", meta = (DisplayName = "物品名"))
	FText ItemName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "物品", meta = (DisplayName = "模型", AllowedClasses = "StaticMesh,SkeletalMesh"))
	TSoftObjectPtr<UObject> ItemMesh;
	// TODO：材质复写，考虑缩略图情况
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "物品", meta = (DisplayName = "模型材质覆盖"))
	TMap<FName, TSoftObjectPtr<UMaterialInterface>> MeshMaterialOverrideList;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "物品", ReplicatedUsing = OnRep_Number, meta = (ExposeOnSpawn = "true", DisplayName = "数量", ClampMin = "1"))
	int32 Number = 1;
	UFUNCTION()
	void OnRep_Number(int32 PreNumber);
	
	// 物品合并
public:
	UPROPERTY(EditDefaultsOnly, Category = "物品", meta = (DisplayName = "物品模型可合并"))
	uint8 bCanMergeItem : 1;
	UPROPERTY(EditDefaultsOnly, Category = "物品", meta = (DisplayName = "最小合并数目", ClampMin = "2", EditCondition = bCanMergeItem))
	int32 MinItemMergeNumber = 5;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "物品", meta = (DisplayName = "道具合并时模型", AllowedClasses = "StaticMesh,SkeletalMesh", EditCondition = bCanMergeItem))
	TSoftObjectPtr<UObject> ItemMergeMesh;
	// TODO：材质复写，考虑缩略图情况
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "物品", meta = (DisplayName = "合并模型材质覆盖", EditCondition = bCanMergeItem))
	TMap<FName, TSoftObjectPtr<UMaterialInterface>> MergeMeshMaterialOverrideList;
	
	bool CanMergeItem() const { return bCanMergeItem && !ItemMergeMesh.IsNull(); }
	UFUNCTION(BlueprintCallable, Category = "物品|基础")
	bool IsMergedItem() const { return CanMergeItem() && Number >= MinItemMergeNumber; }
	TSoftObjectPtr<UObject> GetCurrentItemModel() const;

	//生成实体
public:
	UFUNCTION(BlueprintCallable, Category = "物品", meta = (AutoCreateRefTerm = "Location, Rotation"))
	AXD_ItemBase* SpawnItemActorInLevel(ULevel* OuterLevel, int32 ItemNumber = 1, const FVector& Location = FVector::ZeroVector, const FRotator& Rotation = FRotator::ZeroRotator, ESpawnActorCollisionHandlingMethod CollisionHandling = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn) const;
	AXD_ItemBase* SpawnItemActorInLevel(ULevel* OuterLevel, int32 ItemNumber = 1, const FName& Name = NAME_None, EObjectFlags InObjectFlags = RF_NoFlags, const FVector& Location = FVector::ZeroVector, const FRotator& Rotation = FRotator::ZeroRotator, ESpawnActorCollisionHandlingMethod CollisionHandling = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn) const;

	UFUNCTION(BlueprintCallable, Category = "物品", meta = (AutoCreateRefTerm = "Location, Rotation"))
	AXD_ItemBase* SpawnItemActorForOwner(AActor* Owner, APawn* Instigator, int32 ItemNumber = 1, const FVector& Location = FVector::ZeroVector, const FRotator& Rotation = FRotator::ZeroRotator, ESpawnActorCollisionHandlingMethod CollisionHandling = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "物品", meta = (DeterminesOutputType = "self"))
	UXD_ItemCoreBase* DeepDuplicateCore(const UObject* Outer, const FName& Name = NAME_None) const;
	template<typename T>
	static T* DeepDuplicateCore(const T* ItemCore, const UObject* Outer, const FName& Name = NAME_None) 
	{
		static_assert(TIsDerivedFrom<T, UXD_ItemCoreBase>::IsDerived, "T must be derived from UXD_ItemCoreBase");
		return CastChecked<T>(ItemCore->DeepDuplicateCore(Outer, Name));
	}

	UFUNCTION(BlueprintCallable, Category = "物品", meta = (WorldContext = WorldContextObject))
	AXD_ItemBase* SpawnPreviewItemActor(const UObject* WorldContextObject);

	// 获得生成的具体类型
	TSubclassOf<AXD_ItemBase> GetSpawnedItemClass() const { return GetSpawnedItemClass(Number); }
	TSubclassOf<AXD_ItemBase> GetSpawnedItemClass(int32 SpawnedNumber) const;

	virtual TSubclassOf<AXD_ItemBase> GetStaticMeshActor() const;
	virtual TSubclassOf<AXD_ItemBase> GetSkeletalMeshActor() const;
private:
	void SettingSpawnedItem(AXD_ItemBase* Item, int32 Number) const;

public:
	UPROPERTY(EditDefaultsOnly, Category = "物品", meta = (DisplayName = "在背包中可以叠加"))
	uint8 bCanCompositeInInventory : 1;
	UFUNCTION(BlueprintCallable, Category = "物品|基础")
	bool CanCompositeInInventory() const { return bCanCompositeInInventory; }

	//若有增加物品的属性，且该属性可变，需重载
	UFUNCTION(BlueprintPure, Category = "物品|基础")
	virtual bool IsEqualWithItemCore(const UXD_ItemCoreBase* ItemCore) const;
	UFUNCTION(BlueprintNativeEvent, Category = "物品|基础", meta = (DisplayName = "IsEqualWithItemCore"))
	bool RecevieIsEqualWithItemCore(const UXD_ItemCoreBase* ItemCore) const;
	bool RecevieIsEqualWithItemCore_Implementation(const UXD_ItemCoreBase* ItemCore) const { return true; }

	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "物品|基础")
	FText GetItemName() const;
	virtual FText GetItemName_Implementation() const;

	// 道具被丢弃时的行为
	virtual void WhenThrow(AActor* WhoThrowed, int32 ThrowNumber, ULevel* ThrowToLevel);
	virtual void WhenRemoveFromInventory(class AActor* ItemOwner, int32 RemoveNumber, int32 ExistNumber);
};
