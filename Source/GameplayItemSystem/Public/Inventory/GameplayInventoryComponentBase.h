// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <Components/ActorComponent.h>
#include "GameplayInventoryComponentBase.generated.h"

class UGameplayItemCoreBase;
class AGameplayItemBase;
class AActor;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMEPLAYITEMSYSTEM_API UGameplayInventoryComponentBase : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGameplayInventoryComponentBase();

protected:
	// Called when the game starts
	void BeginPlay() override;

public:
	// Called every frame
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const override;

	bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
//回调事件
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAddItem, UGameplayItemCoreBase*, ItemCore, int32, AddNumber, int32, ExistNumber);
	UPROPERTY(BlueprintAssignable, Category = "背包")
	FOnAddItem OnAddItem;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRemoveItem, UGameplayItemCoreBase*, ItemCore, int32, RemoveNumber, int32, ExistNumber);
	UPROPERTY(BlueprintAssignable, Category = "背包")
	FOnRemoveItem OnRemoveItem;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnGetItemFromOther, AActor*, Instigator, UGameplayItemCoreBase*, ItemCore, int32, ItemNumber, bool, IsBuy);
	UPROPERTY(BlueprintAssignable, Category = "背包")
	FOnGetItemFromOther OnGetItemFromOther;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnRemoveItemByOther, AActor*, Instigator, UGameplayItemCoreBase*, ItemCore, int32, ItemNumber, bool, IsBuy);
	UPROPERTY(BlueprintAssignable, Category = "背包")
	FOnRemoveItemByOther OnRemoveItemByOther;

	UPROPERTY(Transient)
	TArray<UGameplayItemCoreBase*> PreItemCoreList;
	UPROPERTY(VisibleAnywhere, Instanced, BlueprintReadOnly, Category = "背包", ReplicatedUsing = OnRep_ItemList, SaveGame)
	TArray<UGameplayItemCoreBase*> ItemCoreList;

	UFUNCTION()
	void OnRep_ItemList();
protected:
	template<typename TPredicate>
	friend int32 RemoveItemByPredicate(UGameplayInventoryComponentBase* Inventory, int32& Number, const TPredicate& Predicate);

	void WhenItemCoreAdded(UGameplayItemCoreBase* AddedItemCore);
	void WhenItemCoreRemoved(UGameplayItemCoreBase* RemovedItemCore);
public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "背包")
	TArray<UGameplayItemCoreBase*> AddItemCore(const UGameplayItemCoreBase* ItemCore, int32 Number = 1);

	//返回值是移除的道具数目
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "背包")
	int32 RemoveItemCore(const UGameplayItemCoreBase* ItemCore, int32 Number = 1);

	void AddItemArray(const TArray<UGameplayItemCoreBase*>& Items);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "背包", meta = (AllowAbstract = false))
	TArray<UGameplayItemCoreBase*> AddItemCoreByType(TSubclassOf<UGameplayItemCoreBase> Item, int32 Number = 1);

	//返回值是移除的道具数目
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "背包")
	int32 RemoveItemCoreByType(TSubclassOf<UGameplayItemCoreBase> Item, int32 Number = 1);

	//考虑网络的Owner
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "背包")
	void GetItemFromOther(UGameplayInventoryComponentBase* OtherInventory, UGameplayItemCoreBase* ItemCore, int32 Number = 1);

	ULevel* GetThrowedLevel();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "背包")
	void ThrowItemCore(UGameplayItemCoreBase* ItemCore, int32 Number = 1);

	UFUNCTION(BlueprintCallable, Category = "背包")
	void ClearItem();
	// 功能函数
public:
	UFUNCTION(BlueprintCallable, Category = "背包")
	int32 GetItemNumber(const AGameplayItemBase* Item) const;

	UFUNCTION(BlueprintCallable, Category = "背包")
	int32 GetItemNumberByCore(const UGameplayItemCoreBase* ItemCore) const;

	UFUNCTION(BlueprintCallable, Category = "背包")
	int32 GetItemNumberByType(TSubclassOf<UGameplayItemCoreBase> ItemClass) const;

	UFUNCTION(BlueprintCallable, Category = "背包")
	bool ContainItem(const AGameplayItemBase* Item) const;

	UFUNCTION(BlueprintCallable, Category = "背包")
	bool ContainItemByCore(const UGameplayItemCoreBase* ItemCore) const;

	UFUNCTION(BlueprintCallable, Category = "背包")
	bool ContainItemByType(TSubclassOf<UGameplayItemCoreBase> ItemClass) const;

	UFUNCTION(BlueprintCallable, Category = "背包", meta = (DeterminesOutputType = "ItemType"))
	UGameplayItemCoreBase* FindItemByType(TSubclassOf<UGameplayItemCoreBase> ItemType) const;

	UFUNCTION(BlueprintCallable, Category = "背包", meta = (DeterminesOutputType = "ItemType"))
	TArray<UGameplayItemCoreBase*> FindItemsByType(TSubclassOf<UGameplayItemCoreBase> ItemType) const;

	UFUNCTION(BlueprintCallable, Category = "背包")
	UGameplayItemCoreBase* FindItemByItemCore(UGameplayItemCoreBase* ItemCore) const;
};
