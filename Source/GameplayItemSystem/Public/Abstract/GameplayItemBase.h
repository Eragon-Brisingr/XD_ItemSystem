// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <GameFramework/Actor.h>
#include "GameplayItemBase.generated.h"

class UGameplayItemCoreBase;

/**
*
*/

UCLASS(abstract, NotBlueprintable, notplaceable)
class GAMEPLAYITEMSYSTEM_API AGameplayItemBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGameplayItemBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
protected:
	// Called when the game starts or when spawned
	void BeginPlay() override;
public:
	// Called every frame
	void Tick(float DeltaTime) override;

	void GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const override;
	bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;
	void PostInitializeComponents() override;
	void OnConstruction(const FTransform& Transform) override;
	void OnRep_AttachmentReplication() override;
#if WITH_EDITOR
	void PostLoad() override;
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	void CheckForErrors() override;
	void CheckItemErrors(const FName& LogName);
#endif //WITH_EDITOR
	void OnActorChannelOpen(class FInBunch& InBunch, class UNetConnection* Connection) override;
	void OnSerializeNewActor(class FOutBunch& OutBunch) override;
public:
	//初始化模型
	void InitItemMesh() { WhenInitItemMesh(); }
	//物品在世界中的处理
	void ItemInWorldSetting() { WhenItemInWorld(); }
	void SetItemCollisionProfileName(const FName& CollisionProfileName) { WhenSetItemCollisionProfileName(CollisionProfileName); }
	void SetItemSimulatePhysics(bool bSimulate) { WhenSetItemSimulatePhysics(bSimulate); }
protected:
	void InitStaticMeshComponent(UStaticMeshComponent* StaticMeshComponent);
	void InitSkeletalMeshComponent(USkeletalMeshComponent* SkeletalMeshComponent);

	virtual void WhenInitItemMesh() { ReceiveWhenInitItemMesh(); }
	UFUNCTION(BlueprintImplementableEvent, Category = "物品", meta = (DisplayName = "When Init Item Mesh"))
	void ReceiveWhenInitItemMesh();

	virtual void WhenItemInWorld();
	UFUNCTION(BlueprintImplementableEvent, Category = "物品", meta = (DisplayName = "When Item In World"))
	void ReceiveWhenItemInWorld();

	virtual void WhenSetItemCollisionProfileName(const FName& CollisionProfileName);
	UFUNCTION(BlueprintImplementableEvent, Category = "物品", meta = (DisplayName = "When Set Item Collision Profile Name"))
	void ReceiveWhenSetItemCollisionProfileName(const FName& CollisionProfileName);

	virtual void WhenSetItemSimulatePhysics(bool bSimulate);
	UFUNCTION(BlueprintImplementableEvent, Category = "物品", meta = (DisplayName = "When Set Item Simulate Physics"))
	void ReceiveWhenSetItemSimulatePhysics(bool bSimulate);
public:
	UPROPERTY(VisibleAnywhere, Instanced, SaveGame, ReplicatedUsing = OnRep_ItemCore, Category = "物品", meta = (DisplayName = "物品核心"))
	UGameplayItemCoreBase* ItemCore;
	UFUNCTION()
	void OnRep_ItemCore();
	// 网络上即使Actor送达了可能ItemCore还没到，所以要用这个
	DECLARE_DELEGATE(FOnRepItemCoreNative);
	FOnRepItemCoreNative OnItemCoreValidNative;

#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient)
	TSubclassOf<UGameplayItemCoreBase> BelongToCoreType;
#endif

	UFUNCTION(BlueprintCallable, Category = "物品")
	int32 GetNumber() const;

	UFUNCTION(BlueprintCallable, Category = "物品")
	UPrimitiveComponent* GetRootMeshComponent() const;
public:
	UFUNCTION(BlueprintPure, Category = "物品", meta = (DisplayName = "Create Item Core", DeterminesOutputType = "ItemClass"))
	static UGameplayItemCoreBase* CreateItemCoreByType(TSubclassOf<AGameplayItemBase> ItemClass, UObject* Outer);

	//获取物品属性
public:
	UFUNCTION(BlueprintPure, Category = "物品|基础")
	FText GetItemName() const;

public:
	//非特殊情况不要去修改ItemCore的值，若要修改请使用CreateItemCore
	UFUNCTION(BlueprintPure, Category = "物品", meta = (DisplayName = "Get Item Core"))
	UGameplayItemCoreBase* GetItemCore() const;

	const UGameplayItemCoreBase* GetItemCoreConst() const;

	UFUNCTION(BlueprintCallable, Category = "物品")
	UGameplayItemCoreBase* CreateItemCore(UObject* Outer) const;

	UFUNCTION(BlueprintPure, Category = "物品|基础")
	bool IsEqualWithItem(const AGameplayItemBase* Item) const;

	UFUNCTION(BlueprintCallable, Category = "物品")
	bool IsEqualWithItemCore(const UGameplayItemCoreBase* CompareItemCore) const;
};

UCLASS()
class GAMEPLAYITEMSYSTEM_API AGameplayItem_StaticMesh : public AGameplayItemBase
{
	GENERATED_BODY()
public:
	// Sets default values for this actor's properties
	AGameplayItem_StaticMesh(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
public:
	UPROPERTY()
	UStaticMeshComponent* StaticMeshComponent;

	void WhenInitItemMesh() override { InitStaticMeshComponent(StaticMeshComponent); }
};

UCLASS()
class GAMEPLAYITEMSYSTEM_API AGameplayItem_SkeletalMesh : public AGameplayItemBase
{
	GENERATED_BODY()
public:
	// Sets default values for this actor's properties
	AGameplayItem_SkeletalMesh(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
public:
	UPROPERTY()
	USkeletalMeshComponent* SkeletalMeshComponent;

	void WhenInitItemMesh() override { InitSkeletalMeshComponent(SkeletalMeshComponent); }
};
