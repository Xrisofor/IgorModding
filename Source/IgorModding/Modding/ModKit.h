// (C) Xrisofor

#pragma once

#include "CoreMinimal.h"
#include "../Gameplay/Sosed/Sosed.h"
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
    FString Version;

    UPROPERTY(BlueprintReadWrite)
    bool bIsBetaVersion;

    UPROPERTY(BlueprintReadWrite)
    FSlateBrush IconBrush;
};

USTRUCT(BlueprintType)
struct FModData : public FMod
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FAssetData AssetData;

    bool IsValid() const { return AssetData.IsValid(); }
};

UCLASS()
class IGORMODDING_API UModKit : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION()
    void FindMods();

    UFUNCTION(BlueprintCallable)
    FMod FindMod(const FString& ModName) const;

    UFUNCTION(BlueprintCallable)
    FString GetModsDirectory() const;

    UFUNCTION(BlueprintPure)
    TArray<FMod> GetAllMods() const { return AllMods; }

#pragma region Maps
    UFUNCTION(BlueprintPure)
    TArray<FModData> GetAllModMaps() const { return AllModMaps; }
    
    UFUNCTION(BlueprintPure)
    bool GetActiveModMap(FModData& Mod, FName& MapName) const
    {
        if (ActiveMap.IsSet())
        {
            Mod = ActiveMap.GetValue();
            MapName = ActiveMap->AssetData.AssetName;
            return true;
        }
        return false;
    };

    UFUNCTION(BlueprintPure)
    bool HasActiveModMap() const { return ActiveMap.IsSet() && ActiveMap->IsValid(); }

    UFUNCTION(BlueprintCallable)
    void SetActiveModMap(const FModData& Mod)
    {
        if (!Mod.IsValid())
        {
            ActiveMap.Reset();
            return;
        }

        ActiveMap = Mod;
    }
#pragma endregion

#pragma region Neighbors
    UFUNCTION(BlueprintPure)
    TArray<FModData> GetAllModNeighbors() const { return AllModNeighbors; }

    UFUNCTION(BlueprintPure)
    bool GetActiveModNeighbor(FModData& Mod, TSubclassOf<ASosed>& Class) const
    {
        if (ActiveNeighbor.IsSet())
        {
            Mod = ActiveNeighbor.GetValue();

            FString GeneratedClassPath;
            if (Mod.AssetData.GetTagValue(FBlueprintTags::GeneratedClassPath, GeneratedClassPath))
            {
                FSoftObjectPath SoftPath(GeneratedClassPath);
                TSoftClassPtr<ASosed> ClassPtr(SoftPath);
                Class = ClassPtr.LoadSynchronous();
            }
            
            return true;
        }
        return false;
    };

    UFUNCTION(BlueprintPure)
    bool HasActiveModNeighbor() const { return ActiveNeighbor.IsSet() && ActiveNeighbor->IsValid(); }

    UFUNCTION(BlueprintCallable)
    void SetActiveModNeighbor(const FModData& Mod)
    {
        if (!Mod.IsValid())
        {
            ActiveNeighbor.Reset();
            return;
        }

        ActiveNeighbor = Mod;
    }
#pragma endregion

private:
    TArray<FString> ModFoldersQueue;
    void ProcessNextMod();
    
    TArray<FMod> AllMods;
    TArray<FModData> AllModMaps;
    TArray<FModData> AllModNeighbors;

    TOptional<FModData> ActiveMap;
    TOptional<FModData> ActiveNeighbor;
};
