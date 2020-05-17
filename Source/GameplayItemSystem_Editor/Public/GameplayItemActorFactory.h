// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActorFactories/ActorFactory.h"
#include "GameplayItemActorFactory.generated.h"

/**
 * 
 */
UCLASS()
class GAMEPLAYITEMSYSTEM_EDITOR_API UGameplayItemActorFactory : public UActorFactory
{
	GENERATED_BODY()
public:
	UGameplayItemActorFactory(const FObjectInitializer& ObjectInitializer);

protected:
	AActor* GetDefaultActor(const FAssetData& AssetData) override;
	UObject* GetAssetFromActorInstance(AActor* ActorInstance) override;
	bool CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg) override;
	bool PreSpawnActor(UObject* Asset, FTransform& InOutLocation) override;
	AActor* SpawnActor(UObject* Asset, ULevel* InLevel, const FTransform& Transform, EObjectFlags InObjectFlags, const FName Name) override;
	void PostSpawnActor(UObject* Asset, AActor* NewActor) override;
	void PostCreateBlueprint(UObject* Asset, AActor* CDO) override;
};
