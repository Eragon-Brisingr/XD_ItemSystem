// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <Components/ActorComponent.h>
#include "XD_SaveGameInterface.h"
#include "XD_InventoryComponentBase.generated.h"

class UXD_ItemCoreBase;
class AXD_ItemBase;
class AActor;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XD_ITEMSYSTEM_API UXD_InventoryComponentBase : public UActorComponent, public IXD_SaveGameInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UXD_InventoryComponentBase();

protected:
	// Called when the game starts
	void BeginPlay() override;

public:
	// Called every frame
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const override;

	bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;

	//IXD_SaveGameInterface
	void WhenPostLoad_Implementation() override;	
	void WhenGameInit_Implementation() override;
	//IXD_SaveGameInterface

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UPROPERTY(EditAnywhere, Category = "背包|配置", meta = (DisplayName = "初始道具", ConfigUseItem = true), Instanced)
	TArray<UXD_ItemCoreBase*> InitItems;

//回调事件
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAddItem, UXD_ItemCoreBase*, ItemCore, int32, AddNumber, int32, ExistNumber);
	UPROPERTY(BlueprintAssignable, Category = "背包")
	FOnAddItem OnAddItem;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRemoveItem, UXD_ItemCoreBase*, ItemCore, int32, RemoveNumber, int32, ExistNumber);
	UPROPERTY(BlueprintAssignable, Category = "背包")
	FOnRemoveItem OnRemoveItem;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnGetItemFromOther, AActor*, Instigator, UXD_ItemCoreBase*, ItemCore, int32, ItemNumber, bool, IsBuy);
	UPROPERTY(BlueprintAssignable, Category = "背包")
	FOnGetItemFromOther OnGetItemFromOther;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnRemoveItemByOther, AActor*, Instigator, UXD_ItemCoreBase*, ItemCore, int32, ItemNumber, bool, IsBuy);
	UPROPERTY(BlueprintAssignable, Category = "背包")
	FOnRemoveItemByOther OnRemoveItemByOther;

	UPROPERTY()
	TArray<UXD_ItemCoreBase*> PreItemCoreList;
	UPROPERTY(VisibleAnywhere, Instanced, BlueprintReadOnly, Category = "背包", ReplicatedUsing = OnRep_ItemList, SaveGame)
	TArray<UXD_ItemCoreBase*> ItemCoreList;

	UFUNCTION()
	void OnRep_ItemList();
public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "背包")
	TArray<UXD_ItemCoreBase*> AddItemCore(const UXD_ItemCoreBase* ItemCore, int32 Number = 1);

	//返回值是移除的道具数目
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "背包")
	int32 RemoveItemCore(const UXD_ItemCoreBase* ItemCore, int32 Number = 1);

	void AddItemArray(const TArray<UXD_ItemCoreBase*>& Items);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "背包", meta = (AllowAbstract = false))
	TArray<UXD_ItemCoreBase*> AddItemCoreByType(TSubclassOf<UXD_ItemCoreBase> Item, int32 Number = 1);

	//返回值是移除的道具数目
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "背包")
	int32 RemoveItemCoreByType(TSubclassOf<UXD_ItemCoreBase> Item, int32 Number = 1);

	//考虑网络的Owner
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "背包")
	void GetItemFromOther(UXD_InventoryComponentBase* OtherInventory, UXD_ItemCoreBase* ItemCore, int32 Number = 1);

	ULevel* GetThrowedLevel();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "背包")
	void ThrowItemCore(UXD_ItemCoreBase* ItemCore, int32 Number = 1);

	UFUNCTION(BlueprintCallable, Category = "背包")
	void ClearItem();
	
	// 功能函数
public:
	UFUNCTION(BlueprintCallable, Category = "背包")
	int32 GetItemNumber(const AXD_ItemBase* Item) const;

	UFUNCTION(BlueprintCallable, Category = "背包")
	int32 GetItemNumberByCore(const UXD_ItemCoreBase* ItemCore) const;

	UFUNCTION(BlueprintCallable, Category = "背包")
	int32 GetItemNumberByType(TSubclassOf<UXD_ItemCoreBase> ItemClass) const;

	UFUNCTION(BlueprintCallable, Category = "背包")
	bool ContainItem(const AXD_ItemBase* Item) const;

	UFUNCTION(BlueprintCallable, Category = "背包")
	bool ContainItemByCore(const UXD_ItemCoreBase* ItemCore) const;

	UFUNCTION(BlueprintCallable, Category = "背包")
	bool ContainItemByType(TSubclassOf<UXD_ItemCoreBase> ItemClass) const;

	UFUNCTION(BlueprintCallable, Category = "背包", meta = (DeterminesOutputType = "ItemType"))
	UXD_ItemCoreBase* FindItemByType(TSubclassOf<UXD_ItemCoreBase> ItemType) const;

	UFUNCTION(BlueprintCallable, Category = "背包", meta = (DeterminesOutputType = "ItemType"))
	TArray<UXD_ItemCoreBase*> FindItemsByType(TSubclassOf<UXD_ItemCoreBase> ItemType) const;

	UFUNCTION(BlueprintCallable, Category = "背包")
	UXD_ItemCoreBase* FindItemByItemCore(UXD_ItemCoreBase* ItemCore) const;
};
