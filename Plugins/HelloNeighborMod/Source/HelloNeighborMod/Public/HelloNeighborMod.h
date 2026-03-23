// (C) Xrisofor

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

static const FName HelloNeighborCreateNewModTabName("NewGameMod");

class HELLONEIGHBORMOD_API FHelloNeighborModModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	void PluginButtonClicked() { FGlobalTabmanager::Get()->TryInvokeTab(HelloNeighborCreateNewModTabName); };
	
private:
	void RegisterMenus();
	TArray<FString> GetAllMods();
	
	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

	TSharedRef<class SWidget> GenerateModListWidget();
	void GeneratePlatformMenu(FMenuBuilder& MenuBuilder, FString ModName);
	void PackageSelectedMod(FString ModName, FString TargetPlatform, FString CookFlavor);
private:
	TSharedPtr<class FUICommandList> CreateModCommand;
	TSharedPtr<class FUICommandList> PackageModCommands;
};
