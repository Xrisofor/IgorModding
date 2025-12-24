// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HelloNeighborMod : ModuleRules
{
	public HelloNeighborMod(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"DeveloperSettings",
				"Json"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"InputCore",
				"UnrealEd",
				"ToolMenus",
				"Slate",
				"SlateCore",
				"EditorStyle",
				"PluginBrowser",
				"UATHelper",
				"DesktopPlatform",
				"Settings",
				"JsonUtilities"
			}
		);
	}
}