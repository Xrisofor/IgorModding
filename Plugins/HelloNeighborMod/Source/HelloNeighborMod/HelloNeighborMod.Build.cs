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
				"Json",
				"Steamworks"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"InputCore",
				"Slate",
				"SlateCore",
				"EditorStyle",
				"ToolMenus",
				"DesktopPlatform",
				"Settings",
				"JsonUtilities",
				"SharedSettingsWidgets"
			}
		);
		
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"UnrealEd",
					"PluginBrowser",
					"PropertyEditor",
					"MainFrame",
					"UATHelper",
					"SourceControl"
				}
			);
		}
	}
}