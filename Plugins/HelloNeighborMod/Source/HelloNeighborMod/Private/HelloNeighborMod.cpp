// (C) Xrisofor

#include "HelloNeighborMod.h"

#include "HelloNeighborModStyle.h"
#include "HelloNeighborModCommands.h"
#include "HelloNeighborModSettings.h"
#include "ModPluginWizardDefinition.h"

#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Docking/SDockTab.h"

#include "EditorStyleSet.h"
#include "ToolMenus.h"
#include "IPluginBrowser.h"
#include "IUATHelperModule.h"

#include "Framework/Docking/TabManager.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FHelloNeighborModModule"

void FHelloNeighborModModule::StartupModule()
{
	FHelloNeighborModStyle::Initialize();
	FHelloNeighborModStyle::ReloadTextures();

	FHelloNeighborModCommands::Register();

	CreateModCommand = MakeShareable(new FUICommandList);
	PackageModCommands = MakeShareable(new FUICommandList);

	CreateModCommand->MapAction(
		FHelloNeighborModCommands::Get().NewModButton,
		FExecuteAction::CreateRaw(this, &FHelloNeighborModModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FHelloNeighborModModule::RegisterMenus));

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(HelloNeighborCreateNewModTabName,
		FOnSpawnTab::CreateRaw(this, &FHelloNeighborModModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("CreateNewModTabTitle", "New Game Mod"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FHelloNeighborModModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(HelloNeighborCreateNewModTabName);

	FHelloNeighborModCommands::Unregister();
	FHelloNeighborModStyle::Shutdown();
}

void FHelloNeighborModModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);
	UToolMenu* Toolbar = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
	FToolMenuSection& Section = Toolbar->FindOrAddSection("PlayToolBar");

	// Create Mod button
	FToolMenuEntry CreateModEntry = FToolMenuEntry::InitToolBarButton(FHelloNeighborModCommands::Get().NewModButton);
	CreateModEntry.SetCommandList(CreateModCommand);
	CreateModEntry.Icon = FSlateIcon(FHelloNeighborModStyle::Get().GetStyleSetName(), "HelloNeighborMod.CreateNewMod");
	Section.AddEntry(CreateModEntry);

	// Package Mod Dropdown
	FToolMenuEntry PackageModEntry = FToolMenuEntry::InitComboButton(
		"PackageModDropdown",
		FUIAction(),
		FOnGetContent::CreateRaw(this, &FHelloNeighborModModule::GenerateModListWidget),
		LOCTEXT("PackageMod", "Package Mod"),
		LOCTEXT("PackageModTooltip", "Select a mod to package")
	);
	PackageModEntry.Icon = FSlateIcon(FHelloNeighborModStyle::Get().GetStyleSetName(), "HelloNeighborMod.PackageMod");
	Section.AddEntry(PackageModEntry);
}

TSharedRef<SWidget> FHelloNeighborModModule::GenerateModListWidget()
{
	FMenuBuilder MenuBuilder(true, PackageModCommands);

	TArray<FString> AllMods = GetAllMods();

	for (const FString& ModName : AllMods)
	{
		FUIAction Action(FExecuteAction::CreateRaw(this, &FHelloNeighborModModule::PackageSelectedMod, ModName));
		FSlateIcon ModIcon(FHelloNeighborModStyle::Get().GetStyleSetName(), "HelloNeighborMod.ModFolder");

		MenuBuilder.AddMenuEntry(
			FText::FromString(ModName), 
			FText::FromString("Package this mod"), 
			ModIcon, 
			Action
		);
	}

	return MenuBuilder.MakeWidget();
}

TArray<FString> FHelloNeighborModModule::GetAllMods()
{
	TArray<FString> AllMods;
	
	TArray<TSharedRef<IPlugin>> Plugins = IPluginManager::Get().GetDiscoveredPlugins();

	for (TSharedRef<IPlugin> Plugin : Plugins)
	{
		FString PluginBaseDir = Plugin->GetBaseDir();
		
		if (!PluginBaseDir.StartsWith(FPaths::ProjectModsDir()))
			continue;

		AllMods.Add(Plugin->GetName());
	}

	return AllMods;
}

void FHelloNeighborModModule::PackageSelectedMod(FString ModName)
{
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(ModName);
	if (!Plugin.IsValid())
		return;

	UHelloNeighborModSettings* ModSettings = GetMutableDefault<UHelloNeighborModSettings>();
	if (!ModSettings)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to retrieve Hello Neighbor Mod Settings"));
		return;
	}

	FString BasedOnReleaseVersion = ModSettings->BasedOnReleaseVersion;

	FString ProjectFullPath = FPaths::GetProjectFilePath();
	FPaths::MakeStandardFilename(ProjectFullPath);

	FString ProjectName = FPaths::GetBaseFilename(ProjectFullPath);

	FString CommandLine = FString::Printf(
		TEXT("BuildCookRun -project=\"%s\" -noP4 -clientconfig=Development -serverconfig=Development -nocompile -nocompileeditor -installed -ue4exe=UE4Editor-Cmd.exe -utf8output -platform=Win64 -targetplatform=Win64 -ini:Game:[/Script/UnrealEd.ProjectPackagingSettings]:BlueprintNativizationMethod=Disabled -cook -map= -unversionedcookedcontent -pak -dlcname=\"%s\" -DLCIncludeEngineContent -basedonreleaseversion=%s -stage"),
		*ProjectFullPath,
		*ModName,
		*BasedOnReleaseVersion);

#if PLATFORM_WINDOWS
	FText PlatformName = LOCTEXT("PlatformName_Windows", "Windows");
#elif PLATFORM_MAC
	FText PlatformName = LOCTEXT("PlatformName_Mac", "Mac");
#elif PLATFORM_LINUX
	FText PlatformName = LOCTEXT("PlatformName_Linux", "Linux");
#else
	FText PlatformName = LOCTEXT("PlatformName_Other", "Other OS");
#endif

	IUATHelperModule::UatTaskResultCallack UatTask = [ModName, ProjectName](FString Result, double TaskTime)
	{
		const bool bSuccess = Result.Equals(TEXT("Completed"), ESearchCase::IgnoreCase);
		if (!bSuccess)
		{
			UE_LOG(LogTemp, Error, TEXT("Packaging mod %s failed (Result: %s)"), *ModName, *Result);
			return;
		}

		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		FString ProjectRoot = FPaths::ConvertRelativePathToFull(FPaths::GetPath(FPaths::GetProjectFilePath()));
		FString ModRoot = FPaths::Combine(ProjectRoot, TEXT("Mods"), ModName);
		FString SourcePath = FPaths::Combine(ModRoot, TEXT("Saved/StagedBuilds/WindowsNoEditor"), ProjectName, TEXT("Mods"), ModName);
		FString ResourcesPath = FPaths::Combine(ModRoot, TEXT("Resources"));
		FString TargetPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("ModPackage"), ModName);

		if (!PlatformFile.DirectoryExists(*TargetPath))
			PlatformFile.CreateDirectoryTree(*TargetPath);
		
		if (PlatformFile.DirectoryExists(*ResourcesPath))
		{
			FString TargetResources = FPaths::Combine(TargetPath, TEXT("Resources"));
			if (!PlatformFile.DirectoryExists(*TargetResources))
				PlatformFile.CreateDirectoryTree(*TargetResources);

			if (PlatformFile.CopyDirectoryTree(*TargetResources, *ResourcesPath, true))
			{
				UE_LOG(LogTemp, Display, TEXT("Resources copied: %s -> %s"), *ResourcesPath, *TargetResources);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to copy Resources: %s -> %s"), *ResourcesPath, *TargetResources);
			}
		}
		
		if (PlatformFile.DirectoryExists(*SourcePath))
		{
			UE_LOG(LogTemp, Display, TEXT("Copying built mod from %s to %s"), *SourcePath, *TargetPath);

			if (PlatformFile.CopyDirectoryTree(*TargetPath, *SourcePath, true))
			{
				UE_LOG(LogTemp, Display, TEXT("Built mod copied successfully"));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to copy built mod from %s -> %s"), *SourcePath, *TargetPath);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Source directory not found: %s"), *SourcePath);
		}

		UE_LOG(LogTemp, Display, TEXT("Mod %s packaged successfully."), *ModName);
	};

	IUATHelperModule::Get().CreateUatTask(
		CommandLine,
		PlatformName,
		LOCTEXT("PackagePluginTaskName", "Packaging Plugin"),
		LOCTEXT("PackagePluginTaskShortName", "Package Plugin Task"),
		FEditorStyle::GetBrush(TEXT("MainFrame.CookContent")),
		UatTask
	);
}

TSharedRef<SDockTab> FHelloNeighborModModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	TSharedPtr<IPluginWizardDefinition> WizardDefinition = MakeShareable(new FModPluginWizardDefinition());
	return IPluginBrowser::Get().SpawnPluginCreatorTab(SpawnTabArgs, WizardDefinition);
}
#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FHelloNeighborModModule, HelloNeighborMod)
