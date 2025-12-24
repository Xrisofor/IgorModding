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

	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);
	
	TSharedRef<class SWidget> GenerateModListWidget();
	TArray<FString> GetAllMods();
	void PackageSelectedMod(FString ModName);

private:
	TSharedPtr<class FUICommandList> CreateModCommand;
	TSharedPtr<class FUICommandList> PackageModCommands;
};
