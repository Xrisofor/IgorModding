// (C) Xrisofor

#include "ModKit.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Interfaces/IPluginManager.h"
#include "Logging/LogMacros.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Engine/World.h"
#include "Engine/Texture2D.h"
#include "TimerManager.h"
#include "Brushes/SlateNoResource.h"
#include "Engine/BlueprintGeneratedClass.h"

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
        if (GetWorld())
            GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UModKit::ProcessNextMod);
        return;
    }
    
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
    
    FString PluginContentPath = Plugin->GetMountedAssetPath();
    if (PluginContentPath.EndsWith(TEXT("/")))
        PluginContentPath.LeftChopInline(1);
    AssetRegistry.ScanPathsSynchronous({ PluginContentPath }, true);
    
    FMod NewMod;
    NewMod.Name = Plugin->GetName();
    NewMod.FriendlyName = Plugin->GetDescriptor().FriendlyName;
    NewMod.Description = Plugin->GetDescriptor().Description;
    NewMod.Author = Plugin->GetDescriptor().CreatedBy;
    NewMod.Version = Plugin->GetDescriptor().VersionName;
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

#if ENGINE_MAJOR_VERSION >= 5
    Filter.ClassPaths.Add(UWorld::StaticClass()->GetClassPathName());
    Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
    Filter.ClassPaths.Add(UBlueprintGeneratedClass::StaticClass()->GetClassPathName());
#else
    Filter.ClassNames.Add(UWorld::StaticClass()->GetFName());
    Filter.ClassNames.Add(UBlueprint::StaticClass()->GetFName());
    Filter.ClassNames.Add(UBlueprintGeneratedClass::StaticClass()->GetFName());
#endif

    TArray<FAssetData> ModAssets;
    AssetRegistry.GetAssets(Filter, ModAssets);

    static const FName NAME_Blueprint = FName(TEXT("Blueprint"));
    static const FName NAME_BlueprintGeneratedClass = FName(TEXT("BlueprintGeneratedClass"));
    static const FName NAME_World = FName(TEXT("World"));
    
    for (const FAssetData& Asset : ModAssets)
    {
#if ENGINE_MAJOR_VERSION >= 5
        FString CurrentObjectPath = Asset.GetSoftObjectPath().ToString();
        FName CurrentAssetClass = Asset.AssetClassPath.GetAssetName();
#else
        FString CurrentObjectPath = Asset.ObjectPath.ToString();
        FName CurrentAssetClass = Asset.AssetClass;
#endif
        
        UE_LOG(LogModKit, Display, TEXT("Class PackagePath=%s"), *Asset.PackagePath.ToString());
        UE_LOG(LogModKit, Display, TEXT("Asset AssetName=%s, AssetClass=%s"), *Asset.AssetName.ToString(), *CurrentAssetClass.ToString());

        FString ClassTextPath = CurrentObjectPath;
        if (CurrentAssetClass == UBlueprint::StaticClass()->GetFName())
            ClassTextPath = FString::Printf(TEXT("BlueprintGeneratedClass'%s'"), *(CurrentObjectPath + TEXT("_C")));
        
        UE_LOG(LogModKit, Display, TEXT("ClassPath=%s, ClassTextPath=%s"), *CurrentObjectPath, *ClassTextPath);
        
        if (CurrentAssetClass == NAME_World)
        {
            FModData NewMap;
            static_cast<FMod&>(NewMap) = NewMod; 
            NewMap.AssetData = Asset;
            AllModMaps.Add(NewMap);
        }
        else if (CurrentAssetClass == NAME_Blueprint || CurrentAssetClass == NAME_BlueprintGeneratedClass)
        {
            FString ParentClassPath;
            bool bFoundParent = Asset.GetTagValue(FBlueprintTags::NativeParentClassPath, ParentClassPath) || 
                                Asset.GetTagValue(FBlueprintTags::ParentClassPath, ParentClassPath);

#if ENGINE_MAJOR_VERSION >= 5
                if (!bFoundParent || ParentClassPath.IsEmpty())
                {
                    FSoftObjectPath SoftPath(CurrentObjectPath);
                    TSoftClassPtr<UObject> SoftClass(SoftPath);
                    UClass* LoadedClass = SoftClass.LoadSynchronous();
        
                    if (LoadedClass && LoadedClass->IsChildOf(ASosed::StaticClass()))
                    {
                        FModData NewNeighbor;
                        static_cast<FMod&>(NewNeighbor) = NewMod;
                        NewNeighbor.AssetData = Asset;
                        AllModNeighbors.Add(NewNeighbor);
                    }
                }
#else
                if (ParentClassPath.Contains(TEXT("Sosed")))
                {
                    FModData NewNeighbor;
                    static_cast<FMod&>(NewNeighbor) = NewMod; 
                    NewNeighbor.AssetData = Asset;
                    AllModNeighbors.Add(NewNeighbor);
                }
#endif
        }
    }
    
    if (GetWorld())
        GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UModKit::ProcessNextMod);
}