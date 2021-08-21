// Copyright Roman Merkushin. All Rights Reserved.

using UnrealBuildTool;

public class DistanceMatching : ModuleRules
{
	public DistanceMatching(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[] { "Core" });
		PrivateDependencyModuleNames.AddRange(new[] { "CoreUObject", "Engine" });
	}
}
