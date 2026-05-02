// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System;

public class IgorModdingEditorTarget : TargetRules
{
	public IgorModdingEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		ExtraModuleNames.AddRange( new string[] { "IgorModding" } );
		
		bOverrideBuildEnvironment = true;
		
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		
		if (Target.Version.MajorVersion >= 5)
		{
			try
			{
				System.Reflection.PropertyInfo BuildProp = this.GetType().GetProperty("DefaultBuildSettings");
				if (BuildProp != null) BuildProp.SetValue(this, Enum.Parse(BuildProp.PropertyType, "Latest"));
				
				System.Reflection.PropertyInfo IncludeProp = this.GetType().GetProperty("IncludeOrderVersion");
				if (IncludeProp != null) IncludeProp.SetValue(this, Enum.Parse(IncludeProp.PropertyType, "Latest"));
				
				System.Reflection.PropertyInfo CppProp = this.GetType().GetProperty("CppStandard");
				if (CppProp != null) CppProp.SetValue(this, Enum.Parse(CppProp.PropertyType, "Latest"));
			}
			catch (Exception) { }
		}
	}
}