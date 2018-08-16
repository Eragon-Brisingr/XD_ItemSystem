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

	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

	virtual void WhenLoad_Implementation() override;
public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "背包")
	void AddItemArray(const TArray<FItem>& Items);

	//考虑网络的Owner
	void GetItemFromOther(class APawn* Instigator, UXD_InventoryComponentBase* OtherInventory, class UXD_ItemCoreBase* ItemCore, int32 Number = 1);

//新实现
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAddItem, class UXD_ItemCoreBase*, ItemCore, int32, AddNumber, int32, ExistNumber);
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "背包")
	FOnAddItem OnAddItem;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRemoveItem, class UXD_ItemCoreBase*, ItemCore, int32, RemoveNumber, int32, ExistNumber);
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "背包")
	FOnRemoveItem OnRemoveItem;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnGetItemFromOther, class APawn*, Instigator, class UXD_ItemCoreBase*, ItemCore, int32, ItemNumber, bool, IsBuy);
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "背包")
	FOnGetItemFromOther OnGetItemFromOther;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnRemoveItemByOther, class APawn*, Instigator, class UXD_ItemCoreBase*, ItemCore, int32, ItemNumber, bool, IsBuy);
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "背包")
	FOnRemoveItemByOther OnRemoveItemByOther;

	UPROPERTY()
	TArray<class UXD_ItemCoreBase*> PreItemCoreList;
	UPROPERTY(VisibleAnywhere, Instanced, BlueprintReadOnly, Category = "背包", ReplicatedUsing = OnRep_ItemList, SaveGame)
	TArray<class UXD_ItemCoreBase*> ItemCoreList;

	UFUNCTION()
	void OnRep_ItemList();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "背包")
	class UXD_ItemCoreBase* AddItemCore(const class UXD_ItemCoreBase* ItemCore, int32 Number = 1);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "背包")
	int32 RemoveItemCore(const class UXD_ItemCoreBase* ItemCore, int32 Number = 1);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "背包")
	class UXD_ItemCoreBase* AddItemCoreByType(TSubclassOf<class AXD_ItemBase> Item, int32 Number = 1);

	ULevel* GetThrowedLevel();

	UFUNCTION(BlueprintCallable, Category = "背包", Reliable, WithValidation, Server)
	void ThrowItemCore(class UXD_ItemCoreBase* ItemCore, int32 Number = 1);
	virtual void ThrowItemCore_Implementation(class UXD_ItemCoreBase* ItemCore, int32 Number = 1);
	bool ThrowItemCore_Validate(class UXD_ItemCoreBase* ItemCore, int32 Number) { return true; }

	UFUNCTION(BlueprintPure, Category = "背包")
	int32 GetItemNumber(class AXD_ItemBase* Item);

	UFUNCTION(BlueprintPure, Category = "背包")
	int32 GetItemNumberByCore(const class UXD_ItemCoreBase* ItemCore);

	UFUNCTION(BlueprintPure, Category = "背包")
	int32 GetItemNumberByClass(TSubclassOf<class AXD_ItemBase> ItemClass);

	UFUNCTION(BlueprintCallable, Category = "背包")
	void ClearItem();

	UFUNCTION(BlueprintCallable, Category = "背包")
	class UXD_ItemCoreBase* FindItemByType(TSubclassOf<class AXD_ItemBase> ItemType) const;

	UFUNCTION(BlueprintCallable, Category = "背包")
	TArray<class UXD_ItemCoreBase*> FindItemsByType(TSubclassOf<class AXD_ItemBase> ItemType) const;

	UFUNCTION(BlueprintCallable, Category = "背包")
	class UXD_ItemCoreBase* FindItemByItemCore(class UXD_ItemCoreBase* ItemCore) const;
};
