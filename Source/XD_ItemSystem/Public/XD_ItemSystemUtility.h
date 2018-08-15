﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "XD_ItemSystemUtility.generated.h"

/**
 * 
 */
DECLARE_LOG_CATEGORY_EXTERN(XD_ItemSystem_Log, Log, All);
#define ItemSystem_Display_Log(Format, ...) UE_LOG(XD_ItemSystem_Log, Log, TEXT(Format), ##__VA_ARGS__)
#define ItemSystem_Warning_LOG(Format, ...) UE_LOG(XD_ItemSystem_Log, Warning, TEXT(Format), ##__VA_ARGS__)
#define ItemSystem_Error_Log(Format, ...) UE_LOG(XD_ItemSystem_Log, Error, TEXT(Format), ##__VA_ARGS__)
