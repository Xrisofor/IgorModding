// (C) Xrisofor

#include "ModKit.h"
#include "../Gameplay/Sosed/Sosed.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "AssetRegistryModule.h"
#include "Interfaces/IPluginManager.h"
#include "UObject/SoftObjectPath.h"
#include "Logging/LogMacros.h"
#include "Kismet/KismetRenderingLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogModKit, Log, All);

void UModKit::FindMods()
{
    UE_LOG(LogModKit, Display, TEXT("Finding mods..."));

    FString TargetDir = GetModsDirectory();
    if (!IFileManager::Get().DirectoryExists(*TargetDir))
    {
        IFileManager::Get().MakeDirectory(*TargetDir, true);
        return;
    }

    AllMods.Empty();
    AllModMaps.Empty();
    AllModNeighbors.Empty();

    IFileManager::Get().FindFiles(ModFoldersQueue, *(TargetDir / TEXT("*")), false, true);
    
    if (ModFoldersQueue.Num() > 0)
        ProcessNextMod();
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

FString UModKit::GetModsDirectory() const
{
#if PLATFORM_ANDROID || PLATFORM_IOS
    return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Mods"));
#else
    return FPaths::ProjectModsDir();
#endif
}

void UModKit::ProcessNextMod()
{
    if (ModFoldersQueue.Num() == 0) 
        return;

    FString FolderName = ModFoldersQueue.Pop();
    TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(FolderName);

    if (!Plugin.IsValid())
    {
        GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UModKit::ProcessNextMod);
        return;
    }
    
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
    
    FString PluginContentPath = Plugin->GetMountedAssetPath();
    AssetRegistry.ScanPathsSynchronous({ PluginContentPath }, true);
    
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
        else
        {
            NewMod.IconBrush = FSlateNoResource(); 
            NewMod.IconBrush.ImageSize = FVector2D(128.f, 128.f);
        }
    }
    
    AllMods.Add(NewMod);
    
    FARFilter Filter;
    Filter.PackagePaths.Add(FName(*PluginContentPath));
    Filter.bRecursivePaths = true;
    Filter.ClassNames.Add(UWorld::StaticClass()->GetFName());
    Filter.ClassNames.Add(UBlueprint::StaticClass()->GetFName());

    TArray<FAssetData> ModAssets;
    AssetRegistry.GetAssets(Filter, ModAssets);

    for (const FAssetData& Asset : ModAssets)
    {
        UE_LOG(LogModKit, Display, TEXT("Class PackagePath=%s"), *Asset.PackagePath.ToString());
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
            NewMap.Brush = NewMod.IconBrush;
            AllModMaps.Add(NewMap);
        }
        else if (Asset.AssetClass == UBlueprint::StaticClass()->GetFName())
        {
            FString ParentClassPath;
            if (Asset.GetTagValue(FBlueprintTags::NativeParentClassPath, ParentClassPath))
            {
                if (ParentClassPath.Contains(TEXT("Sosed")))
                {
                    FModNeighbor NewNeighbor;
                    NewNeighbor.AssetData = Asset;
                    
                    FString GeneratedClassPath;
                    if (Asset.GetTagValue(FBlueprintTags::GeneratedClassPath, GeneratedClassPath))
                        NewNeighbor.NeighborClass = TSoftClassPtr<ASosed>(FSoftObjectPath(GeneratedClassPath));

                    NewNeighbor.ModName = NewMod.Name;
                    NewNeighbor.Brush = NewMod.IconBrush;
                    AllModNeighbors.Add(NewNeighbor);
                }
            }
        }
    }
    
    GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UModKit::ProcessNextMod);
}