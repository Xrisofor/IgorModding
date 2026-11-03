// (C) Xrisofor

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "HelloNeighborModSettings.generated.h"

#define LOCTEXT_NAMESPACE "HelloNeighborModSettings"

USTRUCT()
struct FPluginTemplate
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Config, meta=(ToolTip="Unique ID, e.g. EmptyMap"))
	FName TemplateId;
	
	UPROPERTY(EditAnywhere, Config)
	FText TemplateName;

	UPROPERTY(EditAnywhere, Config, meta=(MultiLine=true))
	FText TemplateDescription;
	
	UPROPERTY(EditAnywhere, Config, meta=(ToolTip="Relative to HelloNeighborMod/Templates"))
	FString TemplatePath;
	
	UPROPERTY(EditAnywhere, Config, meta=(ToolTip="Relative to HelloNeighborMod/Resources"))
	FString IconFileName;

	UPROPERTY(EditAnywhere, Config)
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, Config)
	int32 SortOrder = 0;
};

UCLASS(Config=Editor, DefaultConfig, meta = (DisplayName="Hello Neighbor Mod"))
class HELLONEIGHBORMOD_API UHelloNeighborModSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// Base
	UPROPERTY(EditAnywhere, Config, meta=(ToolTip="Version string used in the packaging command line (-basedonreleaseversion)"))
	FString BasedOnReleaseVersion = "1.0";
	
	UPROPERTY(EditAnywhere, Config, meta=(TitleProperty="TemplateName", ToolTip="Defines the available plugin templates in HelloNeighborMod/Templates"))
	TArray<FPluginTemplate> Templates = {
		{ "EmptyMap",
		  LOCTEXT("Tpl_EmptyMap_Name", "Empty Map"),
		  LOCTEXT("Tpl_EmptyMap_Desc", "Creates a new empty map"),
		  TEXT("EmptyMap"), TEXT("EmptyMap128.png"), true, 0 },

		{ "TestFieldMap",
		  LOCTEXT("Tpl_TestField_Name", "Test Field Map"),
		  LOCTEXT("Tpl_TestField_Desc", "Simple map to try new things out"),
		  TEXT("TestFieldMap"), TEXT("TestFieldMap128.png"), true, 10 },

		{ "AISetupMap",
		  LOCTEXT("Tpl_AISetup_Name", "AI Setup Map"),
		  LOCTEXT("Tpl_AISetup_Desc", "Basic map to learn AI setup"),
		  TEXT("AISetupMap"), TEXT("AiSetupMap128.png"), true, 20 },
	};
	
public:
	static UHelloNeighborModSettings* Get() { return GetMutableDefault<UHelloNeighborModSettings>(); }
	
	const FPluginTemplate* FindTemplateById(FName Id) const { return Templates.FindByPredicate([Id](const FPluginTemplate& T) { return T.TemplateId == Id; }); }
	
	virtual FName GetContainerName() const override { return FName("Project"); }
	virtual FName GetCategoryName() const override { return FName("Plugins"); }
	virtual FName GetSectionName() const override { return FName("HelloNeighborMod"); }

#if WITH_EDITOR
	virtual FText GetSectionText() const override { return NSLOCTEXT("HelloNeighborMod", "HelloNeighborModName", "Hello Neighbor Mod"); }
	virtual FText GetSectionDescription() const override { return NSLOCTEXT("HelloNeighborMod", "HelloNeighborModDescription", "Настройки плагина Hello Neighbor Mod"); }
#endif
};

#undef LOCTEXT_NAMESPACE