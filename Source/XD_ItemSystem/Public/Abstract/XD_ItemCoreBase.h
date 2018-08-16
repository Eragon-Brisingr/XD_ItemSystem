// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "XD_ItemType.h"
#include "XD_ItemCoreBase.generated.h"

/**
 * 
 */

UCLASS(Blueprintable, BlueprintType, EditInlineNew, collapsecategories)
class XD_ITEMSYSTEM_API UXD_ItemCoreBase : public UObject
{
	GENERATED_BODY()
	
public:
	virtual bool IsSupportedForNetworking() const override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const;

	virtual void PostInitProperties() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif //WITH_EDITOR
public:
	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "物品", Replicated, meta = (ExposeOnSpawn = "true", DisplayName = "物品类型"))
	TSubclassOf<class AXD_ItemBase> ItemClass;
	
	UPROPERTY(BlueprintReadOnly, Category = "物品")
	class UXD_InventoryComponentBase* OwingInventory;
	
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "物品", ReplicatedUsing = OnRep_Number, meta = (ExposeOnSpawn = "true", DisplayName = "数量", ClampMin = "1"))
	int32 Number = 1;

	UFUNCTION()
	void OnRep_Number(int32 PreNumber);

	UFUNCTION()
	class AXD_ItemBase* GetItemDefaultActor() const;

	template<typename ItemActorType>
	ItemActorType* GetItemDefalutActorImpl() const
	{
		return ItemClass->GetDefaultObject<ItemActorType>();
	}
	
	UFUNCTION(BlueprintCallable, Category = "物品|基础")
	AActor* GetOwner() const;
	//生成实体
public:
	UFUNCTION(BlueprintCallable, Category = "物品", meta = (AutoCreateRefTerm = "Location, Rotation"))
	class AXD_ItemBase* SpawnItemActorInLevel(ULevel* OuterLevel, const FVector& Location = FVector::ZeroVector, const FRotator& Rotation = FRotator::ZeroRotator, ESpawnActorCollisionHandlingMethod CollisionHandling = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

	UFUNCTION(BlueprintCallable, Category = "物品", meta = (AutoCreateRefTerm = "Location, Rotation"))
	class AXD_ItemBase* SpawnItemActorForOwner(AActor* Owner, APawn* Instigator, const FVector& Location = FVector::ZeroVector, const FRotator& Rotation = FRotator::ZeroRotator, ESpawnActorCollisionHandlingMethod CollisionHandling = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
private:
	class AXD_ItemBase* SettingSpawnedItem(class AXD_ItemBase* Item) const;

public:
	//若有增加物品的属性，且该属性可变，需重载，若希望物品永不叠加，则返回false
	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "物品|基础")
	bool EqualForItemCore(const UXD_ItemCoreBase* ItemCore) const;
	bool EqualForItemCore_Implementation(const UXD_ItemCoreBase* ItemCore) const;

	UFUNCTION(BlueprintCallable, Category = "物品|基础")
	void UseItem(class APawn* ItemOwner, EUseItemInput UseItemInput);

	UFUNCTION(BlueprintPure, Category = "物品|基础")
	FText GetItemName() const;

	UFUNCTION(BlueprintPure, Category = "物品|基础")
	float GetWeight() const;

	UFUNCTION(BlueprintPure, Category = "物品|基础")
	float GetPrice() const;

	UFUNCTION(BlueprintPure, Category = "物品|基础")
	FText GetItemTypeDesc() const;

	UFUNCTION(BlueprintPure, Category = "物品|基础")
	float GetTradePrice(class APawn* Role, class UXD_InventoryComponentBase* Trader, bool IsBuy) const;

	UFUNCTION(BlueprintPure, Category = "物品|基础")
	FText GetDescribe() const;

	UFUNCTION(BlueprintCallable, Category = "物品|基础")
	void BeThrowed(AActor* WhoThrowed, int32 RemoveNumber, ULevel* ThrowLevel);

	void WhenRemoveFromInventory(class AActor* ItemOwner, int32 RemoveNumber, int32 ExistNumber);
};
