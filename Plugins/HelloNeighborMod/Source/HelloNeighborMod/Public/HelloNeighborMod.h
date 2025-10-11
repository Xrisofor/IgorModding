// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

class FHelloNeighborModModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	void PluginButtonClicked();
	
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
