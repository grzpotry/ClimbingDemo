// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ClimbingDemo : ModuleRules
{
	public ClimbingDemo(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(new string[] { "MotionWarping" });
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput" });
	}
}
