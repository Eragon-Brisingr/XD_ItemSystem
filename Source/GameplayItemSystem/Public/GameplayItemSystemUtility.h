// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <UObject/NoExportTypes.h>
#include "GameplayItemSystemUtility.generated.h"

/**
 * 
 */
GAMEPLAYITEMSYSTEM_API DECLARE_LOG_CATEGORY_EXTERN(GameplayItemSystem_Log, Log, All);
#define ItemSystem_Display_Log(Format, ...) UE_LOG(GameplayItemSystem_Log, Log, TEXT(Format), ##__VA_ARGS__)
#define ItemSystem_Warning_LOG(Format, ...) UE_LOG(GameplayItemSystem_Log, Warning, TEXT(Format), ##__VA_ARGS__)
#define ItemSystem_Error_Log(Format, ...) UE_LOG(GameplayItemSystem_Log, Error, TEXT(Format), ##__VA_ARGS__)

UCLASS(Config = "GameplayItemSystemSetting", defaultconfig)
class GAMEPLAYITEMSYSTEM_API UGameplayItemSystemSettings : public UObject
{
	GENERATED_BODY()
public:
	UGameplayItemSystemSettings();

	UPROPERTY(EditAnywhere, Category = "调试", Config)
	FName ItemCollisionProfileName = TEXT("PhysicsActor");
};

