// (C) Xrisofor

#include "../Modding/ModKit.h"
#include "../Gameplay/Sosed/Sosed.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "AssetRegistryModule.h"
#include "Interfaces/IPluginManager.h"
#include "UObject/SoftObjectPath.h"
#include "Logging/LogMacros.h"
#include "Kismet/KismetRenderingLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogModKit, Log, All);

UModKit::UModKit()
{
    NumActiveMap = -1;
    NumActiveNeighbor = -1;
}

void UModKit::FindMods()
{
    UE_LOG(LogModKit, Display, TEXT("Finding mods..."));

    if (!FPaths::DirectoryExists(FPaths::ProjectModsDir()))
        return;

    TArray<FString> ModFolders;
    IFileManager::Get().FindFiles(ModFolders, *(FPaths::ProjectModsDir() / TEXT("*")), false, true);

    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    TSet<FString> LoadedMods;
    for (const FString& FolderName : ModFolders)
    {
        if (LoadedMods.Contains(FolderName))
        {
            UE_LOG(LogModKit, Warning, TEXT("Find dublicate mod - '%s'"), *FolderName);
            continue;
        }
        LoadedMods.Add(FolderName);

        TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(FolderName);
        if (!Plugin.IsValid())
        {
            UE_LOG(LogModKit, Warning, TEXT("Plugin not found: %s"), *FolderName);
            continue;
        }

        FMod NewMod;
        NewMod.Name = Plugin->GetName();
        NewMod.FriendlyName = Plugin->GetDescriptor().FriendlyName;
        NewMod.Description = Plugin->GetDescriptor().Description;
        NewMod.Author = Plugin->GetDescriptor().CreatedBy;
        NewMod.bIsBetaVersion = Plugin->GetDescriptor().bIsBetaVersion;
        
        FString IconPath = FPaths::Combine(Plugin->GetBaseDir(), TEXT("Resources/Icon128.png"));
        if (FPaths::FileExists(IconPath))
        {
            UTexture2D* IconTexture = UKismetRenderingLibrary::ImportFileAsTexture2D(this, IconPath);
            if (IconTexture)
            {
                IconTexture->AddToRoot();
                NewMod.IconBrush.SetResourceObject(IconTexture);
                NewMod.IconBrush.ImageSize = FVector2D(IconTexture->GetSizeX(), IconTexture->GetSizeY());
            }
        }

        AllMods.Add(NewMod);
        FString PackagePath = "/" + FolderName;
        
        FString PluginContentPath = Plugin->GetMountedAssetPath();
        AssetRegistry.ScanPathsSynchronous({ PluginContentPath }, true);

        TArray<FAssetData> ModAssets;
        AssetRegistry.GetAssetsByPath(FName(*PluginContentPath), ModAssets, true);

        for (const FAssetData& Asset : ModAssets)
        {
            if (!Asset.IsValid())
                continue;
            
            UE_LOG(LogModKit, Display, TEXT("Class PackagePath=%s"), *PackagePath);
            UE_LOG(LogModKit, Display, TEXT("Asset AssetName=%s, AssetClass=%s"), *Asset.AssetName.ToString(), *Asset.AssetClass.ToString());

            FString ClassTextPath = Asset.ObjectPath.ToString();
            if (Asset.AssetClass == UBlueprint::StaticClass()->GetFName())
                ClassTextPath = FString::Printf(TEXT("BlueprintGeneratedClass'%s'"), *(ClassTextPath + TEXT("_C")));

            UE_LOG(LogModKit, Display, TEXT("ClassPath=%s, ClassTextPath=%s"), *Asset.ObjectPath.ToString(), *ClassTextPath);
            
            if (Asset.AssetClass == UWorld::StaticClass()->GetFName())
            {
                FModMap NewMap;
                NewMap.AssetData = Asset;
                
                NewMap.ModName = NewMod.Name;
                NewMap.ModDescription = NewMod.Description;
                NewMap.ModAuthor = NewMod.Author;
                NewMap.bModIsBetaVersion = NewMod.bIsBetaVersion;
                NewMap.Brush = NewMod.IconBrush;
                
                AllModMaps.Add(NewMap);
            }
            else if (Asset.AssetClass == UBlueprint::StaticClass()->GetFName())
            {
                FString GeneratedClassPath;
                if (Asset.GetTagValue("GeneratedClass", GeneratedClassPath))
                {
                    UClass* BlueprintClass = Cast<UClass>(FSoftObjectPath(GeneratedClassPath).TryLoad());
                    if (BlueprintClass && BlueprintClass->IsChildOf(ASosed::StaticClass()))
                    {
                        FModNeighbor NewNeighbor;
                        NewNeighbor.AssetData = Asset;
                        
                        NewNeighbor.NeighborClass = TSoftClassPtr<ASosed>(FSoftObjectPath(GeneratedClassPath));
            
                        NewNeighbor.ModName = NewMod.Name;
                        NewNeighbor.ModDescription = NewMod.Description;
                        NewNeighbor.ModAuthor = NewMod.Author;
                        NewNeighbor.bModIsBetaVersion = NewMod.bIsBetaVersion;
                        NewNeighbor.Brush = NewMod.IconBrush;
            
                        AllModNeighbors.Add(NewNeighbor);
                    }
                }
            }
        }
    }
}

FMod UModKit::FindMod(const FString& ModName) const
{
    for (const FMod& Mod : AllMods)
    {
        if (Mod.Name == ModName)
        {
            return Mod;
        }
    }
    return FMod();
}