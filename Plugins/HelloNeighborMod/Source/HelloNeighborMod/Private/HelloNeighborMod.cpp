// (C) Xrisofor

#include "HelloNeighborMod.h"

#include "HelloNeighborModStyle.h"
#include "HelloNeighborModSettings.h"
#include "ModPluginWizardDefinition.h"
#include "ModMetadataObject.h"
#include "ModPackageHandler.h"
#include "ModPublishHandler.h"

#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "Modules/ModuleManager.h"

#include "IPluginBrowser.h"
#include "ISourceControlOperation.h"
#include "SourceControlOperations.h"
#include "ISourceControlProvider.h"
#include "ISourceControlModule.h"

#include "ToolMenus.h"
#include "Framework/Application/SlateApplication.h"
#include "Interfaces/IPluginManager.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/MessageDialog.h"

#include "Widgets/SWindow.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Images/SImage.h"

#include "Runtime/Launch/Resources/Version.h"
#if ENGINE_MAJOR_VERSION >= 5
	#include "Styling/AppStyle.h"
	#define DEFAULT_EDITOR_STYLE FAppStyle
	#define GET_EDITOR_STYLE_NAME FAppStyle::GetAppStyleSetName()
#else
	#include "EditorStyleSet.h"
	#define DEFAULT_EDITOR_STYLE FEditorStyle
	#define GET_EDITOR_STYLE_NAME FEditorStyle::GetStyleSetName()
#endif

#define LOCTEXT_NAMESPACE "FHelloNeighborModModule"

#pragma region Startup & Shutdown Module | Register Menus
void FHelloNeighborModModule::StartupModule()
{
	FHelloNeighborModStyle::Initialize();

	FString AppId;
	if (GConfig->GetString(TEXT("OnlineSubsystemSteam"), TEXT("SteamDevAppId"), AppId, GEngineIni) && !AppId.IsEmpty())
		FFileHelper::SaveStringToFile(AppId, *GetSteamAppIdPath());

	SteamAPI_Init();
	SteamAPI_RunCallbacks();

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FHelloNeighborModModule::RegisterMenus));

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(HelloNeighborCreateNewModTabName,
		FOnSpawnTab::CreateRaw(this, &FHelloNeighborModModule::OnSpawnCreateTab))
		.SetDisplayName(LOCTEXT("CreateNewModTabTitle", "New Game Mod"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(
		"ModMetadataObject",
		FOnGetDetailCustomizationInstance::CreateStatic(&FModPluginMetadataCustomization::MakeInstance)
	);
	PropertyModule.NotifyCustomizationModuleChanged();
}

void FHelloNeighborModModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(HelloNeighborCreateNewModTabName);
	FHelloNeighborModStyle::Shutdown();

	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout("ModMetadataObject");
	}

	SteamAPI_RunCallbacks();
	SteamAPI_Shutdown();
	
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FString Path = GetSteamAppIdPath();
    
	if (PlatformFile.FileExists(*Path))
		PlatformFile.DeleteFile(*Path);
}

void FHelloNeighborModModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);
	
	FName ToolbarName = (ENGINE_MAJOR_VERSION >= 5) 
			? "LevelEditor.LevelEditorToolBar.PlayToolBar" 
			: "LevelEditor.LevelEditorToolBar";

	UToolMenu* Toolbar = UToolMenus::Get()->ExtendMenu(ToolbarName);
	if (Toolbar)
	{
		FToolMenuSection& Section = Toolbar->FindOrAddSection("HelloNeighborMod");
		
		FToolMenuEntry CreateModEntry = FToolMenuEntry::InitToolBarButton(
			"CreateModButton",
			FUIAction(FExecuteAction::CreateRaw(this, &FHelloNeighborModModule::CreateButtonClicked)),
			LOCTEXT("CreateModLabel", "Create Mod"),
			LOCTEXT("CreateModTooltip", "Create a new mod using templates"),
			FSlateIcon(FHelloNeighborModStyle::Get().GetStyleSetName(), "HelloNeighborMod.CreateNewMod")
		);
		
#if ENGINE_MAJOR_VERSION >= 5
		CreateModEntry.StyleNameOverride = "CalloutToolbar";
#endif
		Section.AddEntry(CreateModEntry);
		
		FToolMenuEntry PackageEntry = FToolMenuEntry::InitComboButton(
			"PackageMenu", 
			FUIAction(),
			FOnGetContent::CreateRaw(this, &FHelloNeighborModModule::GenerateModMenu, false),
			LOCTEXT("PackageMod", "Package Mod"),
			LOCTEXT("PackageModTooltip", "Select a mod and platform to package"),
			FSlateIcon(FHelloNeighborModStyle::Get().GetStyleSetName(), "HelloNeighborMod.PackageMod")
		);

#if ENGINE_MAJOR_VERSION >= 5
		CreateModEntry.StyleNameOverride = "CalloutToolbar";
#endif
		Section.AddEntry(PackageEntry);
		
		if (SteamAPI_Init() && SteamUser()->BLoggedOn())
		{
			FToolMenuEntry PublishEntry = FToolMenuEntry::InitComboButton(
				"PublicationMenu", 
				FUIAction(),
				FOnGetContent::CreateRaw(this, &FHelloNeighborModModule::GenerateModMenu, true),
				LOCTEXT("PublishMod", "Upload Mod"),
				LOCTEXT("PublishModTooltip", "Upload your mods to the Workshop"),
				FSlateIcon(FHelloNeighborModStyle::Get().GetStyleSetName(), "HelloNeighborMod.PublicationMod")
			);

#if ENGINE_MAJOR_VERSION >= 5
			CreateModEntry.StyleNameOverride = "CalloutToolbar";
#endif
			Section.AddEntry(PublishEntry);
		}
	}
	
	UToolMenus::Get()->RefreshAllWidgets();
}
#pragma endregion 

#pragma region Generate Mod Menu & Fill Platform Sub Menu
TSharedRef<SWidget> FHelloNeighborModModule::GenerateModMenu(bool bIsPublishMenu)
{
	FMenuBuilder MenuBuilder(true, nullptr);
	TArray<FString> AllMods = GetAllMods();

	if (AllMods.Num() == 0)
	{
		MenuBuilder.AddMenuEntry(LOCTEXT("NoModsFound", "No mods found"), FText::GetEmpty(), FSlateIcon(), FUIAction());
		return MenuBuilder.MakeWidget();
	}

	for (const FString& ModName : AllMods)
	{
		MenuBuilder.AddSubMenu(
			FText::FromString(ModName),
			FText::Format(LOCTEXT("ModSubMenuT", "Options for {0}"), FText::FromString(ModName)),
			FNewMenuDelegate::CreateRaw(this, &FHelloNeighborModModule::FillPlatformSubMenu, ModName, bIsPublishMenu),
			false,
			FSlateIcon(FHelloNeighborModStyle::Get().GetStyleSetName(), "HelloNeighborMod.ModFolder")
		);
	}

	return MenuBuilder.MakeWidget();
}

void FHelloNeighborModModule::FillPlatformSubMenu(FMenuBuilder& MenuBuilder, FString ModName, bool bIsPublishMenu)
{
	const UHelloNeighborModSettings* Settings = GetDefault<UHelloNeighborModSettings>();
	
	for (const FBuildPlatform& Platform : Settings->SupportedPlatforms)
	{
		if (bIsPublishMenu && Platform.PlatformName.Contains(TEXT("Android"))) continue;

		FText Label = Platform.DisplayName;
		FText Tooltip = Platform.Tooltip;

		if (bIsPublishMenu)
		{
			bool bIsPackaged = FModPackageHandler::IsModPackaged(ModName, Platform.PlatformName);
			
			Label = bIsPackaged ? 
				FText::Format(LOCTEXT("UploadReady", "Upload {0}"), Platform.DisplayName) : 
				FText::Format(LOCTEXT("BuildAndUpload", "Build & Upload {0}"), Platform.DisplayName);
			
			Tooltip = bIsPackaged ? 
				LOCTEXT("UploadTip", "Files found for this platform. Ready to upload.") : 
				LOCTEXT("BuildUploadTip", "No files found for this platform. Will build then upload.");
		}

		MenuBuilder.AddMenuEntry(
			Label, Tooltip,
			FSlateIcon(GET_EDITOR_STYLE_NAME, Platform.IconName),
			FUIAction(FExecuteAction::CreateLambda([ModName, Platform, bIsPublishMenu]() {
				if (bIsPublishMenu)
					FModPublishHandler::Get().PublishMod(ModName, Platform.PlatformName);
				else
					FModPackageHandler::PackageMod(ModName, Platform.PlatformName, Platform.TargetFlavor);
			}))
		);
	}

	MenuBuilder.AddMenuSeparator();

	MenuBuilder.AddMenuEntry(
		LOCTEXT("EditMod", "Edit Mod Properties..."),
		LOCTEXT("EditModTip", "Change name, description, author, etc."),
		FSlateIcon(FHelloNeighborModStyle::GetStyleSetName(), "HelloNeighborMod.CreateNewMod"),
		FUIAction(FExecuteAction::CreateLambda([this, ModName]() {
			TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(ModName);
			if (Plugin.IsValid())
			{
				this->OpenModEditor(Plugin.ToSharedRef());
			}
		}))
	);
}
#pragma endregion

#pragma region Get All Mods | On Spawn Create Tab | Open Mod Editor
TArray<FString> FHelloNeighborModModule::GetAllMods()
{
	TArray<FString> AllMods;
	TArray<TSharedRef<IPlugin>> Plugins = IPluginManager::Get().GetDiscoveredPlugins();
	for (TSharedRef<IPlugin> Plugin : Plugins)
	{
		if (Plugin->GetBaseDir().StartsWith(FPaths::ProjectModsDir()))
			AllMods.Add(Plugin->GetName());
	}
	return AllMods;
}

TSharedRef<SDockTab> FHelloNeighborModModule::OnSpawnCreateTab(const FSpawnTabArgs& SpawnTabArgs)
{
	TSharedPtr<IPluginWizardDefinition> WizardDefinition = MakeShareable(new FModPluginWizardDefinition());
	return IPluginBrowser::Get().SpawnPluginCreatorTab(SpawnTabArgs, WizardDefinition);
}

void FHelloNeighborModModule::OpenModEditor(TSharedRef<IPlugin> Plugin)
{
    UModMetadataObject* MetadataObject = NewObject<UModMetadataObject>();
	MetadataObject->TargetIconPath = Plugin->GetBaseDir() / TEXT("Resources/Icon128.png");
    MetadataObject->PopulateFromDescriptor(Plugin->GetDescriptor());
    MetadataObject->AddToRoot();

	FPropertyEditorModule& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	TSharedRef<IDetailsView> PropertyView = EditModule.CreateDetailView(FDetailsViewArgs(false, false, false, FDetailsViewArgs::ActorsUseNameArea, true));
	PropertyView->SetObject(MetadataObject, true);

    TSharedPtr<SWindow> PropertiesWindow = SNew(SWindow)
        .SupportsMaximize(false)
        .SupportsMinimize(false)
        .SizingRule(ESizingRule::FixedSize)
        .ClientSize(FVector2D(700.0f, 575.0f))
        .Title(LOCTEXT("PluginMetadataTitle", "Mod Properties"));

    PropertiesWindow->SetContent(
		SNew(SBorder)
			.Padding(FMargin(8.0f, 8.0f))
			[
				SNew(SVerticalBox)

				+SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(5.0f, 10.0f, 5.0f, 5.0f))
				[
					SNew(STextBlock)
					.Font(DEFAULT_EDITOR_STYLE::GetFontStyle("DetailsView.CategoryFontStyle"))
					.Text(FText::FromString(Plugin->GetName()))
				]

				+ SVerticalBox::Slot()
				.Padding(5)
				[
					PropertyView
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(5)
				.HAlign(HAlign_Right)
				[
					SNew(SButton)
					.ContentPadding(FMargin(20.0f, 2.0f))
					.Text(LOCTEXT("OkButtonLabel", "Ok"))
					.OnClicked(FOnClicked::CreateRaw(this, &FHelloNeighborModModule::OnEditModFinished, MetadataObject, Plugin, PropertiesWindow))
				]
			]
    );

    FSlateApplication::Get().AddModalWindow(PropertiesWindow.ToSharedRef(), FGlobalTabmanager::Get()->GetRootWindow());
}

FReply FHelloNeighborModModule::OnEditModFinished(UModMetadataObject* MetadataObject, TSharedRef<IPlugin> Plugin, TSharedPtr<SWindow> PropertiesWindow)
{
    FPluginDescriptor OldDescriptor = Plugin->GetDescriptor();
    FPluginDescriptor NewDescriptor = OldDescriptor;
	
    MetadataObject->CopyIntoDescriptor(NewDescriptor);
    MetadataObject->RemoveFromRoot();
	
    if (PropertiesWindow.IsValid())
        PropertiesWindow->RequestDestroyWindow();
	
    FString OldText;
    OldDescriptor.Write(OldText);
    FString NewText;
    NewDescriptor.Write(NewText);

    if (OldText.Compare(NewText, ESearchCase::CaseSensitive) != 0)
    {
        FString DescriptorFileName = Plugin->GetDescriptorFileName();
    	
        if (ISourceControlModule::Get().IsEnabled())
        {
            ISourceControlProvider& SourceControlProvider = ISourceControlModule::Get().GetProvider();
            TSharedPtr<ISourceControlState, ESPMode::ThreadSafe> SourceControlState = SourceControlProvider.GetState(DescriptorFileName, EStateCacheUsage::ForceUpdate);
            
            if (SourceControlState.IsValid() && SourceControlState->CanCheckout())
            {
                SourceControlProvider.Execute(ISourceControlOperation::Create<FCheckOut>(), DescriptorFileName);
            }
        }
    	
        FText FailReason;
        if (!Plugin->UpdateDescriptor(NewDescriptor, FailReason))
        {
            FMessageDialog::Open(EAppMsgType::Ok, FailReason);
        }
    }

    return FReply::Handled();
}
#pragma endregion

#undef LOCTEXT_NAMESPACE
IMPLEMENT_MODULE(FHelloNeighborModModule, HelloNeighborMod)