// (C) Xrisofor

#include "ModPackageHandler.h"
#include "HelloNeighborModSettings.h"
#include "Interfaces/IPluginManager.h"
#include "PlatformInfo.h"
#include "EditorStyleSet.h"
#include "IUATHelperModule.h"
#include "Async/Async.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/ConfigCacheIni.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#define DEFAULT_EDITOR_STYLE FEditorStyle

FString FModPackageHandler::GetStageFolderName(FString Platform, FString Flavor)
{
    const FName FlavorName = Flavor.IsEmpty() ? NAME_None : FName(*Flavor);

    for (const PlatformInfo::FPlatformInfo& Info : PlatformInfo::GetPlatformInfoArray())
    {
        if (Info.PlatformType != EBuildTargetType::Game) continue;
        if (Info.UBTTargetId.ToString() != Platform) continue;
        if (Info.PlatformFlavor != FlavorName) continue;

        return Info.TargetPlatformName.ToString();
    }

    return Platform;
}

FString FModPackageHandler::GetCookFlavor(const FString& Platform)
{
    if (Platform != TEXT("Android"))
        return FString();
    
    const TCHAR* Section = TEXT("/Script/AndroidRuntimeSettings.AndroidRuntimeSettings");
    auto IsEnabled = [Section](const TCHAR* Key)
    {
        bool bValue = false;
        GConfig->GetBool(Section, Key, bValue, GEngineIni);
        return bValue;
    };
    
    if (IsEnabled(TEXT("bMultiTargetFormat_ASTC"))) return TEXT("ASTC");
    if (IsEnabled(TEXT("bMultiTargetFormat_ETC2"))) return TEXT("ETC2");
    if (IsEnabled(TEXT("bMultiTargetFormat_DXT")))  return TEXT("DXT");

    return TEXT("ETC2");
}

void FModPackageHandler::PackageMod(FString ModName, FString TargetPlatform, FString CookFlavor, FOnPackageComplete OnComplete)
{
    // Find Plugin
    if (!IPluginManager::Get().FindPlugin(ModName).IsValid())
    {
        OnComplete.ExecuteIfBound(false, ModName, TargetPlatform);
        return;
    }
    
    // Find Cook Flavor (Example Android - ASTC)
    if (CookFlavor.IsEmpty())
        CookFlavor = GetCookFlavor(TargetPlatform);

    const FString ProjectPath = FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath());
    const FString ProjectName = FPaths::GetBaseFilename(ProjectPath);
    const FString ArchivePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("ModArchive") / ModName);
    
    // UE Editor CMD
    const FString EditorExe = TEXT("UE4Editor-Cmd.exe");
    FString StageFolder = GetStageFolderName(TargetPlatform, CookFlavor);
    
    // Has a release been created?
    const FString ReleasePath = FPaths::Combine(FPaths::ProjectDir(), TEXT("Releases"), UHelloNeighborModSettings::Get()->BasedOnReleaseVersion, StageFolder);
    if (!FPaths::DirectoryExists(ReleasePath))
    {
        FNotificationInfo Info(FText::Format(
            NSLOCTEXT("HelloNeighborMod", "PkgNoRelease", "Base release not found: {0}"),
            FText::FromString(ReleasePath)));
        Info.ExpireDuration = 8.0f;
        TSharedPtr<SNotificationItem> Item = FSlateNotificationManager::Get().AddNotification(Info);
        if (Item.IsValid()) Item->SetCompletionState(SNotificationItem::CS_Fail);

        OnComplete.ExecuteIfBound(false, ModName, StageFolder);
        return;
    }
    
    FString PlatformArgs = FString::Printf(TEXT("-platform=%s -targetplatform=%s"), *TargetPlatform, *TargetPlatform);
    if (!CookFlavor.IsEmpty()) PlatformArgs += FString::Printf(TEXT(" -cookflavor=%s"), *CookFlavor);

    // -dlcname - plugin to cook as DLC (output goes to Mods/<ModName>)
    // -basedonreleaseversion - only assets that differ from the release are packed
    // -DLCIncludeEngineContent - include engine assets the mod uses that are missing in the release
    // -unversionedcookedcontent - must match the base release, otherwise the pak will not load in the game
    // -nocompile/-nocompileeditor - C++ is already built, the mod contains content only
    const FString CommandLine = FString::Printf(
        TEXT("BuildCookRun -project=\"%s\" -noP4 -clientconfig=Development -nocompile -nocompileeditor -installed -ue4exe=%s -utf8output %s -cook -unversionedcookedcontent -pak -dlcname=\"%s\" -DLCIncludeEngineContent -basedonreleaseversion=%s -stage -archive -archivedirectory=\"%s\""),
        *ProjectPath, *EditorExe, *PlatformArgs, *ModName, *UHelloNeighborModSettings::Get()->BasedOnReleaseVersion, *ArchivePath
    );
    
    FText TaskName = FText::Format(NSLOCTEXT("HelloNeighborMod", "PkgTask", "{0} ({1})"), FText::FromString(ModName), FText::FromString(TargetPlatform));
    FText ShortName = NSLOCTEXT("HelloNeighborMod", "PkgShort", "Mod Packaging");
    FText Description = NSLOCTEXT("HelloNeighborMod", "PkgDesc", "Mod Task");
    const FSlateBrush* Icon = DEFAULT_EDITOR_STYLE::GetBrush(TEXT("MainFrame.CookContent"));
    
    auto ResultCallback = [ModName, ProjectName, StageFolder, OnComplete](FString Result, double)
    {
        AsyncTask(ENamedThreads::GameThread, [Result, ModName, ProjectName, StageFolder, OnComplete]()
        {
            bool bSuccess = Result.Equals(TEXT("Completed"), ESearchCase::IgnoreCase) || Result.Contains(TEXT("Success"));
            if (bSuccess)
                MovePackagedFiles(ModName, ProjectName, StageFolder, OnComplete);
            else
                OnComplete.ExecuteIfBound(false, ModName, StageFolder);
        });
    };

    IUATHelperModule::Get().CreateUatTask(
        CommandLine, 
        TaskName, 
        ShortName, 
        Description, 
        Icon, 
        ResultCallback
    );
}

bool FModPackageHandler::IsModPackaged(FString ModName, FString Platform)
{
    IPlatformFile& PF = FPlatformFileManager::Get().GetPlatformFile();
    
    FString Flavor = GetCookFlavor(Platform);
    FString StageFolder = GetStageFolderName(Platform, Flavor);
    FString CheckPath = FPaths::Combine(GetPackagePath(ModName), TEXT("Content"), TEXT("Paks"), StageFolder);
	
    return PF.DirectoryExists(*CheckPath);
}

void FModPackageHandler::MovePackagedFiles(FString ModName, FString ProjectName, FString StageFolder, FOnPackageComplete OnComplete)
{
    IPlatformFile& PF = FPlatformFileManager::Get().GetPlatformFile();
    const FString SavedDir = FPaths::ProjectSavedDir();
    const FString ArchiveRoot = FPaths::Combine(SavedDir, TEXT("ModArchive"), ModName);
    const FString TargetPath = GetPackagePath(ModName);
    const FString DeepSourcePath = FPaths::Combine(ArchiveRoot, StageFolder, ProjectName, TEXT("Mods"), ModName);
    
    if (TSharedPtr<IPlugin> ModPlugin = IPluginManager::Get().FindPlugin(ModName))
    {
        const FString Resources = FPaths::Combine(ModPlugin->GetBaseDir(), TEXT("Resources"));
        if (PF.DirectoryExists(*Resources))
        {
            FString DestRes = FPaths::Combine(TargetPath, TEXT("Resources"));
            PF.CreateDirectoryTree(*DestRes);
            PF.CopyDirectoryTree(*DestRes, *Resources, true);
        }
    }
    
    if (PF.DirectoryExists(*DeepSourcePath))
    {
        PF.CreateDirectoryTree(*TargetPath);
        if (PF.CopyDirectoryTree(*TargetPath, *DeepSourcePath, true))
        {
            PF.DeleteDirectoryRecursively(*ArchiveRoot);
            
            if (TSharedPtr<IPlugin> ModPlugin = IPluginManager::Get().FindPlugin(ModName))
            {
                FString ModSavedFolder = FPaths::Combine(ModPlugin->GetBaseDir(), TEXT("Saved"));
                if (PF.DirectoryExists(*ModSavedFolder))
                    PF.DeleteDirectoryRecursively(*ModSavedFolder);
            }
			
            OnComplete.ExecuteIfBound(true, ModName, StageFolder);
            return;
        }
    }
    OnComplete.ExecuteIfBound(false, ModName, StageFolder);
}
