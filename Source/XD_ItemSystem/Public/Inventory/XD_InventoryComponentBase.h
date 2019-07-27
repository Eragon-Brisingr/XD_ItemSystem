// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "XD_SaveGameInterface.h"
#include "XD_ItemType.h"
#include "XD_InventoryComponentBase.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XD_ITEMSYSTEM_API UXD_InventoryComponentBase : public UActorComponent, public IXD_SaveGameInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UXD_InventoryComponentBase();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const override;

	virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;

	//IXD_SaveGameInterface
	virtual void WhenGameInit_Implementation() override;
	virtual void WhenPostLoad_Implementation() override;
	//IXD_SaveGameInterface

#if WITH_EDITOR
	TSubclassOf<class AXD_ItemBase> InitItemsType;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UPROPERTY(EditAnywhere, Category = "配置|常用", meta = (DisplayName = "初始道具"))
	TArray<FXD_Item> InitItems;

//回调事件
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAddItem, class UXD_ItemCoreBase*, ItemCore, int32, AddNumber, int32, ExistNumber);
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "背包")
	FOnAddItem OnAddItem;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRemoveItem, class UXD_ItemCoreBase*, ItemCore, int32, RemoveNumber, int32, ExistNumber);
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "背包")
	FOnRemoveItem OnRemoveItem;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnGetItemFromOther, class AActor*, Instigator, class UXD_ItemCoreBase*, ItemCore, int32, ItemNumber, bool, IsBuy);
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "背包")
	FOnGetItemFromOther OnGetItemFromOther;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnRemoveItemByOther, class AActor*, Instigator, class UXD_ItemCoreBase*, ItemCore, int32, ItemNumber, bool, IsBuy);
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "背包")
	FOnRemoveItemByOther OnRemoveItemByOther;

	UPROPERTY()
	TArray<class UXD_ItemCoreBase*> PreItemCoreList;
	UPROPERTY(VisibleAnywhere, Instanced, BlueprintReadOnly, Category = "背包", ReplicatedUsing = OnRep_ItemList, SaveGame)
	TArray<class UXD_ItemCoreBase*> ItemCoreList;

	UFUNCTION()
	void OnRep_ItemList();
public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "背包")
	TArray<class UXD_ItemCoreBase*> AddItemCore(const class UXD_ItemCoreBase* ItemCore, int32 Number = 1);

	//返回值是移除的道具数目
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "背包")
	int32 RemoveItemCore(const class UXD_ItemCoreBase* ItemCore, int32 Number = 1);

	void AddItemArray(const TArray<FXD_Item>& Items);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "背包")
	TArray<class UXD_ItemCoreBase*> AddItemCoreByType(TSubclassOf<class AXD_ItemBase> Item, int32 Number = 1);

	//返回值是移除的道具数目
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "背包")
	int32 RemoveItemCoreByType(TSubclassOf<class AXD_ItemBase> Item, int32 Number = 1);

	//考虑网络的Owner
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "背包")
	void GetItemFromOther(UXD_InventoryComponentBase* OtherInventory, class UXD_ItemCoreBase* ItemCore, int32 Number = 1);

	ULevel* GetThrowedLevel();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "背包")
	void ThrowItemCore(class UXD_ItemCoreBase* ItemCore, int32 Number = 1);

	UFUNCTION(BlueprintPure, Category = "背包")
	int32 GetItemNumber(class AXD_ItemBase* Item);

	UFUNCTION(BlueprintPure, Category = "背包")
	int32 GetItemNumberByCore(const class UXD_ItemCoreBase* ItemCore);

	UFUNCTION(BlueprintPure, Category = "背包")
	int32 GetItemNumberByType(TSubclassOf<class AXD_ItemBase> ItemClass);

	UFUNCTION(BlueprintCallable, Category = "背包")
	void ClearItem();

	UFUNCTION(BlueprintCallable, Category = "背包")
	class UXD_ItemCoreBase* FindItemByType(TSubclassOf<class AXD_ItemBase> ItemType) const;

	UFUNCTION(BlueprintCallable, Category = "背包")
	TArray<class UXD_ItemCoreBase*> FindItemsByType(TSubclassOf<class AXD_ItemBase> ItemType) const;

	UFUNCTION(BlueprintCallable, Category = "背包")
	class UXD_ItemCoreBase* FindItemByItemCore(class UXD_ItemCoreBase* ItemCore) const;
};
