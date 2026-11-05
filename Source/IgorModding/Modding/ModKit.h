// (C) Xrisofor

#pragma once

#include "CoreMinimal.h"
#include "AssetData.h"
#include "Styling/SlateBrush.h"
#include "Engine/Blueprint.h"
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
    bool bIsBetaVersion = false;

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

UENUM(BlueprintType)
enum class EModScanResult : uint8 {
    Success,
    Failed
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnModsScanCompleted, EModScanResult, Result);

UCLASS()
class IGORMODDING_API UModKit : public UObject
{
    GENERATED_BODY()

public:
    // Выполняет поиск всех модов, которые находятся в папке Mods
    // Searches for all mods that are in the Mods folder
    UFUNCTION(BlueprintCallable)
    void FindMods();

    // Выполняет поиск мода по названию, а не по FriendlyName
    // Searches for a mod by name, not by FriendlyName
    UFUNCTION(BlueprintCallable)
    FMod FindMod(const FString& ModName) const;

    // Возвращает путь к папке mods, Android и iOS возвращают путь к папке сохранения, в которой находится Mods
    // Returns the path to the mods folder, Android and iOS return the path to the save folder where Mods is located
    UFUNCTION(BlueprintCallable)
    FString GetModsDirectory() const;

    // Возвращает все найденные моды.
    // Returns all discovered mods.
    UFUNCTION(BlueprintPure)
    TArray<FMod> GetAllMods() const { return AllMods; }
    
    // Делегат завершения поиска модов
    // Delegate for mod scan completion
    UPROPERTY(BlueprintAssignable)
    FOnModsScanCompleted OnModsScanCompleted;
    
#pragma region Maps
    // Возвращает все найденные карты всех модов.
    // Returns all maps found across all mods.
    UFUNCTION(BlueprintPure)
    TArray<FModData> GetAllModMaps() const { return AllModMaps; }
    
    // Возвращает все найденные карты указанного мода
    // Returns all maps found in the specified mod
    UFUNCTION(BlueprintPure)
    TArray<FModData> GetModMaps(const FString& ModName) const
    {
        if (AllModMaps.Num() == 0)
            return TArray<FModData>();
        
        TArray<FModData> TempMaps;
        for (FModData Maps : AllModMaps)
        {
            if (Maps.Name == ModName)
                TempMaps.Add(Maps);
        }
        
        return TempMaps;
    };
    
    // Возвращает данные и имя активной карты, если она установлена
    // Returns the active map data and name if one is set
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

    // Проверяет, установлена ли активная карта и валидны ли её данные
    // Checks whether an active map is set and its data is valid
    UFUNCTION(BlueprintPure)
    bool HasActiveModMap() const { return ActiveMap.IsSet() && ActiveMap->IsValid(); }

    // Устанавливает активную карту или сбрасывает её, если данные невалидны
    // Sets the active map or resets it if the data is invalid
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
    // Возвращает всех найденных соседей всех модов.
    // Returns all neighbors found across all mods.
    UFUNCTION(BlueprintPure)
    TArray<FModData> GetAllModNeighbors() const { return AllModNeighbors; }

    // Возвращает всех найденных соседей указанного мода
    // Returns all neighbors found in the specified mod
    UFUNCTION(BlueprintPure)
    TArray<FModData> GetModNeighbors(const FString& ModName) const
    {
        if (AllModNeighbors.Num() == 0)
            return TArray<FModData>();
        
        TArray<FModData> TempNeighbors;
        for (FModData Neighbors : AllModNeighbors)
        {
            if (Neighbors.Name == ModName)
                TempNeighbors.Add(Neighbors);
        }
        
        return TempNeighbors;
    };
    
    // Возвращает данные активного соседа и его класс, если он установлен
    // Returns the active neighbor's data and class if one is set
    UFUNCTION(BlueprintPure)
    bool GetActiveModNeighbor(FModData& Mod, TSubclassOf<ASosed>& Class) const
    {
        if (ActiveNeighbor.IsSet())
        {
            Mod = ActiveNeighbor.GetValue();

            FString GeneratedClassPath;
            Mod.AssetData.GetTagValue(FBlueprintTags::GeneratedClassPath, GeneratedClassPath);

            if (!GeneratedClassPath.IsEmpty())
            {
                FSoftObjectPath SoftPath(GeneratedClassPath);
                TSoftClassPtr<ASosed> ClassPtr(SoftPath);
                Class = ClassPtr.LoadSynchronous();
            }
        
            return true;
        }
        return false;
    }

    // Проверяет, установлен ли активный сосед и валидны ли его данные
    // Checks whether an active neighbor is set and its data is valid
    UFUNCTION(BlueprintPure)
    bool HasActiveModNeighbor() const { return ActiveNeighbor.IsSet() && ActiveNeighbor->IsValid(); }

    // Устанавливает активного соседа или сбрасывает его, если данные невалидны
    // Sets the active neighbor or resets it if the data is invalid
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
    bool bIsScanning = false;
    TArray<FString> ModFoldersQueue;
    void ProcessNextMod();
    void FinishScan(EModScanResult Result);
    
    UPROPERTY() TArray<FMod> AllMods;
    UPROPERTY() TArray<FModData> AllModMaps;
    UPROPERTY() TArray<FModData> AllModNeighbors;

    TOptional<FModData> ActiveMap;
    TOptional<FModData> ActiveNeighbor;
};
