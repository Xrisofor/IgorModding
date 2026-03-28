// (C) Xrisofor

#pragma once

#include "CoreMinimal.h"
#include "ModMetadataObject.h"
#include "Modules/ModuleManager.h"

static const FName HelloNeighborCreateNewModTabName("NewGameMod");

class FHelloNeighborModModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
private:
	void RegisterMenus();
	void CreateButtonClicked() { FGlobalTabmanager::Get()->TryInvokeTab(HelloNeighborCreateNewModTabName); };

	TArray<FString> GetAllMods();
	static FString GetSteamAppIdPath()
	{
		static const FString Path = FPaths::Combine(FPlatformProcess::BaseDir(), TEXT("steam_appid.txt"));
		return Path;
	}
	
	TSharedRef<class SWidget> GenerateModMenu(bool bIsPublishMenu);
	void FillPlatformSubMenu(class FMenuBuilder& MenuBuilder, FString ModName, bool bIsPublishMenu);

	TSharedRef<class SDockTab> OnSpawnCreateTab(const class FSpawnTabArgs& SpawnTabArgs);
	void OpenModEditor(TSharedRef<class IPlugin> Plugin);
	FReply OnEditModFinished(UModMetadataObject* ModMetadataObject, TSharedRef<class IPlugin> Plugin, TSharedPtr<SWindow> PropertiesWindow);
};