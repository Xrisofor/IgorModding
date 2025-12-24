// (C) Xrisofor

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "HelloNeighborModSettings.generated.h"

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

UCLASS(Config=Editor, DefaultConfig, meta = (DisplayName="Hello Neighbor Mod"))
class HELLONEIGHBORMOD_API UHelloNeighborModSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetContainerName() const override { return FName("Project"); }
	virtual FName GetCategoryName() const override { return FName("Plugins"); }
	virtual FName GetSectionName() const override { return FName("HelloNeighborMod"); }

#if WITH_EDITOR
	virtual FText GetSectionText() const override { return NSLOCTEXT("HelloNeighborMod", "HelloNeighborModName", "Hello Neighbor Mod"); }
	virtual FText GetSectionDescription() const override { return NSLOCTEXT("HelloNeighborMod", "HelloNeighborModDescription", "Настройки плагина Hello Neighbor Mod"); }
#endif
	
public:
	UPROPERTY(EditAnywhere, Config, meta=(ToolTip="Version string used in the packaging command line (-basedonreleaseversion)"))
	FString BasedOnReleaseVersion = "1.0";
	
	UPROPERTY(EditAnywhere, Config, meta=(ToolTip="Defines the available plugin templates in HelloNeighborMod/Templates"))
	TArray<FPluginTemplate> Templates = {
		{ FText::FromString("Empty Map"), FText::FromString("Creates a new empty map"), TEXT("EmptyMap"), TEXT("EmptyMap128.png") },
		{ FText::FromString("Test Field Map"), FText::FromString("Simple map to try new things out"), TEXT("TestFieldMap"), TEXT("TestFieldMap128.png") },
		{ FText::FromString("AI Setup Map"), FText::FromString("Basic map to learn AI setup"), TEXT("AISetupMap"), TEXT("AiSetupMap128.png") }
	};
};