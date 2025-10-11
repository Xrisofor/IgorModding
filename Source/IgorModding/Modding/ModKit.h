// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "Engine/AssetManager.h"
#include "ModKit.generated.h"

USTRUCT(BlueprintType)
struct FMod
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString Name;

    UPROPERTY(BlueprintReadWrite)
    FString FriendlyName;

    UPROPERTY(BlueprintReadWrite)
    FString Description;

    UPROPERTY(BlueprintReadWrite)
    FString Author;

    UPROPERTY(BlueprintReadWrite)
    bool bIsBetaVersion;

    UPROPERTY(BlueprintReadWrite)
    FSlateBrush IconBrush;
};

USTRUCT(BlueprintType)
struct FModMap
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FAssetData AssetData;

    UPROPERTY(BlueprintReadWrite)
    FString ModName;

    UPROPERTY(BlueprintReadWrite)
    FString ModDescription;

    UPROPERTY(BlueprintReadWrite)
    FString ModAuthor;

    UPROPERTY(BlueprintReadWrite)
    bool bModIsBetaVersion;

    UPROPERTY(BlueprintReadWrite)
    FSlateBrush Brush;
};

USTRUCT(BlueprintType)
struct FModNeighbor
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FAssetData AssetData;

    UPROPERTY(BlueprintReadWrite)
    FString ModName;

    UPROPERTY(BlueprintReadWrite)
    FString ModDescription;

    UPROPERTY(BlueprintReadWrite)
    FString ModAuthor;

    UPROPERTY(BlueprintReadWrite)
    bool bModIsBetaVersion;

    UPROPERTY(BlueprintReadWrite)
    FSlateBrush Brush;

    UPROPERTY()
    TSoftClassPtr<class ASosed> NeighborClass;
};

UCLASS(Blueprintable)
class IGORMODDING_API UModKit : public UObject
{
    GENERATED_BODY()

public:
    UModKit();
    
    UFUNCTION()
    void FindMods();

    UFUNCTION(BlueprintCallable)
    FMod FindMod(const FString& ModName) const;
    
    UFUNCTION(BlueprintPure)
    TArray<FModMap> GetAllModMaps() const { return AllModMaps; }

    UFUNCTION(BlueprintPure)
    TArray<FModNeighbor> GetAllModNeighbors() const { return AllModNeighbors; }

    UFUNCTION(BlueprintPure)
    TArray<FMod> GetAllMods() const { return AllMods; }

    UFUNCTION(BlueprintPure)
    int32 GetNumModMap() const { return NumActiveMap; }

    UFUNCTION(BlueprintPure)
    int32 GetNumModNeighbor() const { return NumActiveNeighbor; }

    UFUNCTION(BlueprintPure)
    bool HasActiveModMap() const { return NumActiveMap >= 0; }

    UFUNCTION(BlueprintPure)
    bool HasActiveModNeighbor() const { return NumActiveNeighbor >= 0; }

    UFUNCTION(BlueprintPure)
    bool HasActiveMods() const { return HasActiveModMap() || HasActiveModNeighbor(); }

    UFUNCTION(BlueprintCallable)
    void SetNumModMap(int32 Num) { NumActiveMap = Num; }

    UFUNCTION(BlueprintCallable)
    void SetNumModNeighbor(int32 Num) { NumActiveNeighbor = Num; }

private:
    TArray<FMod> AllMods;
    TArray<FModMap> AllModMaps;
    TArray<FModNeighbor> AllModNeighbors;

    int32 NumActiveMap;
    int32 NumActiveNeighbor;
};
