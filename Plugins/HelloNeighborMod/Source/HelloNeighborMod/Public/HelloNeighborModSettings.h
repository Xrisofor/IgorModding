// (C) Xrisofor

#pragma once

#include "CoreMinimal.h"
#include "ToolMenus.h"
#include "Engine/DeveloperSettings.h"
#include "HelloNeighborModSettings.generated.h"

#define LOCTEXT_NAMESPACE "HelloNeighborMod"

USTRUCT()
struct FPluginTemplate
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, meta=(ToolTip="The display name of this plugin template"))
	FText TemplateName;

	UPROPERTY(EditAnywhere, meta=(ToolTip="A short description of what this template contains"))
	FText TemplateDescription;

	UPROPERTY(EditAnywhere, meta=(ToolTip="Folder path of the template inside HelloNeighborMod/Templates"))
	FString TemplatePath;

	UPROPERTY(EditAnywhere, meta=(ToolTip="Icon filename for this template, located in HelloNeighborMod/Resources"))
	FString IconFileName;
};

USTRUCT()
struct FBuildPlatform
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, meta=(ToolTip="The displayed name in the menu"))
	FText DisplayName;

	UPROPERTY(EditAnywhere, meta=(ToolTip="Hover description"))
	FText Tooltip;

	UPROPERTY(EditAnywhere, meta=(ToolTip="The name of the icon from EditorStyle (for example, Launcher.Platform_Windows)"))
	FName IconName;

	UPROPERTY(EditAnywhere, meta=(ToolTip="ID of the build platform (Win64, Linux, Android)"))
	FString PlatformName;

	UPROPERTY(EditAnywhere, meta=(ToolTip="Additional parameter (for example, ASTC for Android)"))
	FString TargetFlavor;

	FBuildPlatform() {}
	FBuildPlatform(FText InDisplayName, FText InTooltip, FName InIconName, FString InPlatformName, FString InTargetFlavor)
		: DisplayName(InDisplayName), Tooltip(InTooltip), IconName(InIconName), PlatformName(InPlatformName), TargetFlavor(InTargetFlavor)
	{}
};

UCLASS(Config=Editor, DefaultConfig, meta = (DisplayName="Hello Neighbor Mod"))
class HELLONEIGHBORMOD_API UHelloNeighborModSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// Base
	UPROPERTY(EditAnywhere, Config, meta=(ToolTip="Version string used in the packaging command line (-basedonreleaseversion)"))
	FString BasedOnReleaseVersion = "1.0";
	
	UPROPERTY(EditAnywhere, Config, meta=(ToolTip="Defines the available plugin templates in HelloNeighborMod/Templates"))
	TArray<FPluginTemplate> Templates = {
		{ FText::FromString("Empty Map"), FText::FromString("Creates a new empty map"), TEXT("EmptyMap"), TEXT("EmptyMap128.png") },
		{ FText::FromString("Test Field Map"), FText::FromString("Simple map to try new things out"), TEXT("TestFieldMap"), TEXT("TestFieldMap128.png") },
		{ FText::FromString("AI Setup Map"), FText::FromString("Basic map to learn AI setup"), TEXT("AISetupMap"), TEXT("AiSetupMap128.png") }
	};
	
	// Platforms
	UPROPERTY(EditAnywhere, Config, Category="Platforms", meta=(ToolTip="List of supported platforms for the build"))
	TArray<FBuildPlatform> SupportedPlatforms = {
		{ LOCTEXT("Pl_Win64", "Windows (64-bit)"), LOCTEXT("Pl_Win64_T", "Build for Windows"), "Launcher.Platform_Windows", "Win64", "" },
		{ LOCTEXT("Pl_Linux", "Linux"),  LOCTEXT("Pl_Linux_T", "Build for Linux"), "Launcher.Platform_Linux", "Linux", "" },
		{ LOCTEXT("Pl_Android", "Android (ASTC)"), LOCTEXT("Pl_Android_T", "Build for Android"), "Launcher.Platform_Android", "Android", "ASTC" }
	};
	
public:
	static UHelloNeighborModSettings* Get() { return GetMutableDefault<UHelloNeighborModSettings>(); }
	
	const FPluginTemplate* FindTemplateByName(const FText& Name) const { return Templates.FindByPredicate([&](const FPluginTemplate& T) { return T.TemplateName.EqualTo(Name); }); }
	
	virtual FName GetContainerName() const override { return FName("Project"); }
	virtual FName GetCategoryName() const override { return FName("Plugins"); }
	virtual FName GetSectionName() const override { return FName("HelloNeighborMod"); }

#if WITH_EDITOR
	virtual FText GetSectionText() const override { return NSLOCTEXT("HelloNeighborMod", "HelloNeighborModName", "Hello Neighbor Mod"); }
	virtual FText GetSectionDescription() const override { return NSLOCTEXT("HelloNeighborMod", "HelloNeighborModDescription", "Настройки плагина Hello Neighbor Mod"); }
#endif
};

#undef LOCTEXT_NAMESPACE