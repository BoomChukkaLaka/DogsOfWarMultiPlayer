// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class DogsOfWarMultiPlayerEditorTarget : TargetRules
{
	public DogsOfWarMultiPlayerEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.Add("DogsOfWarMultiPlayer");
	}
}
