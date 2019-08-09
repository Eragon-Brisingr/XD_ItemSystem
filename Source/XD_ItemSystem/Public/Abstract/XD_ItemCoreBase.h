// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <Engine/EngineTypes.h>
#include "UObject/NoExportTypes.h"
#include "XD_ItemType.h"
#include "XD_ItemCoreBase.generated.h"

class AXD_ItemBase;
class UXD_InventoryComponentBase;

/**
 * 
 */

UCLASS(Blueprintable, BlueprintType, EditInlineNew, collapsecategories)
class XD_ITEMSYSTEM_API UXD_ItemCoreBase : public UObject
{
	GENERATED_BODY()
	
public:
	UXD_ItemCoreBase();

	bool IsSupportedForNetworking() const override;

	void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	void PostInitProperties() override;

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif //WITH_EDITOR
public:
	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "物品", Replicated, meta = (ExposeOnSpawn = "true", DisplayName = "物品类型"))
	TSubclassOf<AXD_ItemBase> ItemClass;

	FORCEINLINE TSubclassOf<AXD_ItemBase> GetItemClass() const { return ItemClass; }
	
	UPROPERTY(BlueprintReadOnly, Category = "物品")
	UXD_InventoryComponentBase* OwingInventory;
	
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "物品", ReplicatedUsing = OnRep_Number, meta = (ExposeOnSpawn = "true", DisplayName = "数量", ClampMin = "1"))
	int32 Number = 1;

	UFUNCTION()
	void OnRep_Number(int32 PreNumber);

	template<typename ItemActorType = AXD_ItemBase>
	const ItemActorType* GetItemDefaultActor() const { return GetItemClass()->GetDefaultObject<ItemActorType>(); }
	
	UFUNCTION(BlueprintCallable, Category = "物品|基础")
	AActor* GetOwner() const;
	//生成实体
public:
	UFUNCTION(BlueprintCallable, Category = "物品", meta = (AutoCreateRefTerm = "Location, Rotation"))
	AXD_ItemBase* SpawnItemActorInLevel(ULevel* OuterLevel, int32 ItemNumber = 1, const FVector& Location = FVector::ZeroVector, const FRotator& Rotation = FRotator::ZeroRotator, ESpawnActorCollisionHandlingMethod CollisionHandling = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn) const;

	UFUNCTION(BlueprintCallable, Category = "物品", meta = (AutoCreateRefTerm = "Location, Rotation"))
	AXD_ItemBase* SpawnItemActorForOwner(AActor* Owner, APawn* Instigator, int32 ItemNumber = 1, const FVector& Location = FVector::ZeroVector, const FRotator& Rotation = FRotator::ZeroRotator, ESpawnActorCollisionHandlingMethod CollisionHandling = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "物品", meta = (DeterminesOutputType = "self"))
	UXD_ItemCoreBase* DeepDuplicateCore(const UObject* Outer) const;
	template<typename T>
	static T* DeepDuplicateCore(const T* ItemCore, const UObject* Outer) 
	{
		static_assert(TIsDerivedFrom<T, UXD_ItemCoreBase>::IsDerived, "T must be derived from UXD_ItemCoreBase");
		return CastChecked<T>(ItemCore->DeepDuplicateCore(Outer));
	}

	UFUNCTION(BlueprintCallable, Category = "物品", meta = (WorldContext = WorldContextObject))
	AXD_ItemBase* SpawnPreviewItemActor(const UObject* WorldContextObject);
private:
	void SettingSpawnedItem(AXD_ItemBase* Item, int32 Number) const;

public:
	UFUNCTION(BlueprintCallable, Category = "物品|基础")
	bool CanCompositeInInventory() const;

	//若有增加物品的属性，且该属性可变，需重载
	UFUNCTION(BlueprintPure, Category = "物品|基础")
	virtual bool IsEqualWithItemCore(const UXD_ItemCoreBase* ItemCore) const;
	UFUNCTION(BlueprintNativeEvent, Category = "物品|基础", meta = (DisplayName = "IsEqualWithItemCore"))
	bool RecevieIsEqualWithItemCore(const UXD_ItemCoreBase* ItemCore) const;
	bool RecevieIsEqualWithItemCore_Implementation(const UXD_ItemCoreBase* ItemCore) const { return true; }

	UFUNCTION(BlueprintPure, Category = "物品|基础")
	FText GetItemName() const;

	UFUNCTION(BlueprintCallable, Category = "物品|基础")
	void WhenThrow(AActor* WhoThrowed, int32 RemoveNumber, ULevel* ThrowLevel);

	void WhenRemoveFromInventory(class AActor* ItemOwner, int32 RemoveNumber, int32 ExistNumber);
};
