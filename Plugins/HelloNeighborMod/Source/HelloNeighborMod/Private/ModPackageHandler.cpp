// (C) Xrisofor

#include "ModPackageHandler.h"
#include "HelloNeighborModSettings.h"
#include "Interfaces/IPluginManager.h"
#include "IUATHelperModule.h"
#include "Async/Async.h"
#include "HAL/PlatformFileManager.h"

#include "Runtime/Launch/Resources/Version.h"
#if ENGINE_MAJOR_VERSION >= 5
    #include "Styling/AppStyle.h"
    #define DEFAULT_EDITOR_STYLE FAppStyle
#else
    #include "EditorStyleSet.h"
    #define DEFAULT_EDITOR_STYLE FEditorStyle
#endif

FString FModPackageHandler::GetStageFolderName(FString Platform, FString Flavor)
{
    // (Win32, Win64, HoloLens, Mac, XboxOne, PS4, IOS, Android, HTML5, Linux, LinuxAArch64, AllDesktop, TVOS, Switch, Lumin)

#if ENGINE_MAJOR_VERSION >= 5
    if (Platform == "Win64") return TEXT("Windows");
    if (Platform == "Linux") return TEXT("Linux");
#else
    if (Platform == "Win64") return TEXT("WindowsNoEditor");
    if (Platform == "Linux") return TEXT("LinuxNoEditor");
#endif
    
    if (Platform == "Android") return Flavor.IsEmpty() ? TEXT("Android") : FString::Printf(TEXT("Android_%s"), *Flavor);
    return Platform;
}

void FModPackageHandler::PackageMod(FString ModName, FString TargetPlatform, FString CookFlavor, FOnPackageComplete OnComplete)
{
	if (!IPluginManager::Get().FindPlugin(ModName).IsValid()) return;

    const FString ProjectPath = FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath());
    const FString ProjectName = FPaths::GetBaseFilename(ProjectPath);
    const FString ArchivePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("ModArchive") / ModName);

#if ENGINE_MAJOR_VERSION >= 5
    const FString EditorExe = TEXT("UnrealEditor-Cmd.exe");
#else
    const FString EditorExe = TEXT("UE4Editor-Cmd.exe");
#endif

    FString StageFolder = GetStageFolderName(TargetPlatform, CookFlavor);

    FString PlatformArgs = FString::Printf(TEXT("-platform=%s -targetplatform=%s"), *TargetPlatform, *TargetPlatform);
    if (!CookFlavor.IsEmpty()) PlatformArgs += FString::Printf(TEXT(" -cookflavor=%s"), *CookFlavor);

    const FString CommandLine = FString::Printf(
        TEXT("BuildCookRun -project=\"%s\" -noP4 -clientconfig=Development -nocompile -nocompileeditor -installed -ue4exe=%s -utf8output %s -cook -unversionedcookedcontent -pak -dlcname=\"%s\" -DLCIncludeEngineContent -basedonreleaseversion=%s -stage -archive -archivedirectory=\"%s\""),
        *ProjectPath, *EditorExe, *PlatformArgs, *ModName, *UHelloNeighborModSettings::Get()->BasedOnReleaseVersion, *ArchivePath
    );
    
    FText TaskName = FText::Format(NSLOCTEXT("HelloNeighborMod", "PkgTask", "{0} ({1})"), FText::FromString(ModName), FText::FromString(TargetPlatform));
    FText ShortName = NSLOCTEXT("HelloNeighborMod", "PkgShort", "Mod Packaging");
    FText Description = NSLOCTEXT("HelloNeighborMod", "PkgShort", "Mod Task");
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

#if ENGINE_MAJOR_VERSION >= 5
    IUATHelperModule::Get().CreateUatTask(
        CommandLine, 
        TaskName, 
        ShortName, 
        Description, 
        Icon, 
        nullptr,
        ResultCallback
    );
#else
    IUATHelperModule::Get().CreateUatTask(
        CommandLine, 
        TaskName, 
        ShortName, 
        Description, 
        Icon, 
        ResultCallback
    );
#endif
}

bool FModPackageHandler::IsModPackaged(FString ModName, FString Platform)
{
    IPlatformFile& PF = FPlatformFileManager::Get().GetPlatformFile();
    
    FString Flavor = "";
    if (const UHelloNeighborModSettings* Settings = UHelloNeighborModSettings::Get())
    {
        for (const auto& P : Settings->SupportedPlatforms)
            if (P.PlatformName == Platform) { Flavor = P.TargetFlavor; break; }
    }

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
