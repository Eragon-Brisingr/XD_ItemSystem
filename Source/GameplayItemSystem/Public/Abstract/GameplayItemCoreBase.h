// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <Engine/EngineTypes.h>
#include <UObject/NoExportTypes.h>
#include "GameplayItemCoreBase.generated.h"

class AGameplayItemBase;
class UGameplayInventoryComponentBase;
class UMaterialInterface;

/**
 * 
 */
#if WITH_EDITOR
// 用此作用域生成的道具实例是不会被保存、且编辑器的层级窗不可见
struct GAMEPLAYITEMSYSTEM_API FSpawnPreviewItemScope
{
	FSpawnPreviewItemScope() { Counter += 1; }
	~FSpawnPreviewItemScope() { Counter -= 1; }
	static bool IsSpawnPreviewItem() { return Counter != 0; }
private:
	static uint8 Counter;
};
#endif

UENUM()
enum class EItemModelType : uint8
{
	StaticMesh,
	SkeletalMesh,
	Actor,
	None
};

USTRUCT()
struct GAMEPLAYITEMSYSTEM_API FGameplayItemModelData
{
	GENERATED_BODY()
public:
	FGameplayItemModelData() = default;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "模型", AllowedClasses = "StaticMesh,SkeletalMesh,GameplayItemEntityBlueprint"))
	TSoftObjectPtr<UObject> Model;
	UPROPERTY(VisibleAnywhere, meta = (DisplayName = "模型类型"))
	EItemModelType ModelType = EItemModelType::None;
	// TODO：考虑缩略图情况
	UPROPERTY(EditAnywhere, meta = (DisplayName = "模型材质覆盖"))
	TMap<FName, TSoftObjectPtr<UMaterialInterface>> MaterialOverride;
#if WITH_EDITOR
	void UpdateModelType();
#endif
};

USTRUCT(BlueprintType, BlueprintInternalUseOnly)
struct GAMEPLAYITEMSYSTEM_API FGameplayItemCoreSparseData
{
	GENERATED_BODY()
public:
	FGameplayItemCoreSparseData();
	
#if WITH_EDITORONLY_DATA
	// HACK：不明原因导致SparseData的第一个SoftObject的编辑器赋值会Crash，占位
	UPROPERTY(EditDefaultsOnly, Category = "物品", meta = (NoGetter), AdvancedDisplay)
	TSoftObjectPtr<UObject> HACK_SOFTOBJECT_SLOT;
#endif

	UPROPERTY(EditDefaultsOnly, Category = "物品", meta = (DisplayName = "物品名"))
	FText ItemNameValue;
	UPROPERTY(EditDefaultsOnly, Category = "物品", meta = (DisplayName = "模型", GetByRef))
	FGameplayItemModelData ItemModelValue;
	
	UPROPERTY(EditDefaultsOnly, Category = "物品", meta = (DisplayName = "在背包中可以叠加"))
	uint8 bCanCompositeInInventoryValue : 1;
	UPROPERTY(EditDefaultsOnly, Category = "物品", meta = (DisplayName = "物品模型可合并"))
	uint8 bCanMergeItemValue : 1;
	UPROPERTY(EditDefaultsOnly, Category = "物品", meta = (DisplayName = "最小合并数目", ClampMin = "2"))
	int32 MinItemMergeNumberValue = 5;
	UPROPERTY(EditDefaultsOnly, Category = "物品", meta = (DisplayName = "合并时模型", GetByRef))
	FGameplayItemModelData MergeItemModelValue;
};

UCLASS(abstract, BlueprintType, EditInlineNew, SparseClassDataTypes = GameplayItemCoreSparseData)
class GAMEPLAYITEMSYSTEM_API UGameplayItemCoreBase : public UObject
{
	GENERATED_BODY()
	
public:
	UGameplayItemCoreBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	bool IsSupportedForNetworking() const override;
	void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	UWorld* GetWorld() const override;

#if WITH_EDITOR
	void PostLoad() override;
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif //WITH_EDITOR
public:
	// TODO：可添加最大叠加上限概念，现在把物品数量配置的上限设置为了1000防止配出太大的数量
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category = "物品", ReplicatedUsing = OnRep_Number, meta = (DisplayName = "数量", ClampMin = "1", ClampMax = "1000"))
	int32 Number = 1;
	UFUNCTION()
	void OnRep_Number(int32 PreNumber);
	
	UFUNCTION(BlueprintPure, Category = "物品|基础")
	FText GetItemName() const { return ReceiveGetItemName(); }
	UFUNCTION(BlueprintNativeEvent, Category = "物品|基础", meta = (DisplayName = "Get Item Name"))
	FText ReceiveGetItemName() const;
	FText ReceiveGetItemName_Implementation() const { return GetItemNameValue(); }

	// 物品合并
	bool CanMergeItem() const { return GetCanMergeItemValue() && !GetMergeItemModelValue().Model.IsNull(); }
	UFUNCTION(BlueprintCallable, Category = "物品|基础")
	bool IsMergedItem() const { return CanMergeItem() && Number >= GetMinItemMergeNumberValue(); }
	virtual const FGameplayItemModelData& GetCurrentItemModel() const;
	
	//生成实体
public:
	UFUNCTION(BlueprintCallable, Category = "物品")
	AGameplayItemBase* SpawnItemActorInLevel(ULevel* OuterLevel, FVector Location, FRotator Rotation, int32 ItemNumber = 1, ESpawnActorCollisionHandlingMethod CollisionHandling = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn) const { return SpawnItemActorInLevel(OuterLevel,Location, Rotation, ItemNumber, NAME_None, RF_NoFlags, CollisionHandling); }
	AGameplayItemBase* SpawnItemActorInLevel(ULevel* OuterLevel, const FVector& Location = FVector::ZeroVector, const FRotator& Rotation = FRotator::ZeroRotator, int32 ItemNumber = 1, const FName& Name = NAME_None, EObjectFlags InObjectFlags = RF_NoFlags, ESpawnActorCollisionHandlingMethod CollisionHandling = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn) const;

	UFUNCTION(BlueprintCallable, Category = "物品", meta = (AutoCreateRefTerm = "Location, Rotation"))
	AGameplayItemBase* SpawnItemActorForOwner(AActor* Owner, APawn* Instigator, const FVector& Location = FVector::ZeroVector, const FRotator& Rotation = FRotator::ZeroRotator, int32 ItemNumber = 1, ESpawnActorCollisionHandlingMethod CollisionHandling = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn) const;
	
	UFUNCTION(BlueprintCallable, Category = "物品", meta = (WorldContext = WorldContextObject))
	AGameplayItemBase* SpawnPreviewItemActor(const UObject* WorldContextObject);
private:
	void SettingSpawnedItem(AGameplayItemBase* Item, int32 Number) const;
public:
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "物品", meta = (DeterminesOutputType = "self"))
	UGameplayItemCoreBase* DeepDuplicateCore(const UObject* Outer, const FName& Name = NAME_None) const;
	template<typename T>
	static T* DeepDuplicateCore(const T* ItemCore, const UObject* Outer, const FName& Name = NAME_None) 
	{
		static_assert(TIsDerivedFrom<T, UGameplayItemCoreBase>::IsDerived, "T must be derived from UGameplayItemCoreBase");
		return CastChecked<T>(ItemCore->DeepDuplicateCore(Outer, Name));
	}

	// 获得生成的具体类型
	TSubclassOf<AGameplayItemBase> GetSpawnedItemClass() const { return GetSpawnedItemClass(Number); }
	TSubclassOf<AGameplayItemBase> GetSpawnedItemClass(int32 SpawnedNumber) const;

	// 返回所属的ItemEntity
	virtual TSubclassOf<AGameplayItemBase> GetBelongToEntityType() const;
protected:
	virtual TSubclassOf<AGameplayItemBase> GetStaticMeshEntityType() const;
	virtual TSubclassOf<AGameplayItemBase> GetSkeletalMeshEntityType() const;

	// 背包操作
public:
	UPROPERTY(Transient)
	UGameplayInventoryComponentBase* OwingInventory;
	
	UFUNCTION(BlueprintCallable, Category = "物品|基础")
	bool CanCompositeInInventory() const { return GetCanCompositeInInventoryValue(); }

	//若有增加物品的属性，且该属性可变，需重载
	UFUNCTION(BlueprintPure, Category = "物品|基础")
	virtual bool IsEqualWithItemCore(const UGameplayItemCoreBase* ItemCore) const;
	UFUNCTION(BlueprintNativeEvent, Category = "物品|基础", meta = (DisplayName = "IsEqualWithItemCore"))
	bool RecevieIsEqualWithItemCore(const UGameplayItemCoreBase* ItemCore) const;
	bool RecevieIsEqualWithItemCore_Implementation(const UGameplayItemCoreBase* ItemCore) const { return true; }
	
	virtual void WhenThrow(AActor* WhoThrowed, int32 ThrowNumber, ULevel* ThrowToLevel);
	virtual void WhenRemoveFromInventory(class AActor* ItemOwner, int32 RemoveNumber, int32 ExistNumber) {}
};
