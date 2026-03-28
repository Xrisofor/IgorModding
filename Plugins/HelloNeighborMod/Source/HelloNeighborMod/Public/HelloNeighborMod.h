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
	TSharedRef<class SDockTab> OnSpawnCreateTab(const class FSpawnTabArgs& SpawnTabArgs);
	void OpenModEditor(TSharedRef<class IPlugin> Plugin);
	
	TSharedRef<class SWidget> GenerateModMenu(bool bIsPublishMenu);
	void FillPlatformSubMenu(class FMenuBuilder& MenuBuilder, FString ModName, bool bIsPublishMenu);

private:
	FReply OnEditModFinished(UModMetadataObject* ModMetadataObject, TSharedRef<class IPlugin> Plugin, TSharedPtr<SWindow> PropertiesWindow);
};