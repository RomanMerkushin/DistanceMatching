// Copyright Roman Merkushin. All Rights Reserved.

using UnrealBuildTool;

public class DistanceMatching : ModuleRules
{
	public DistanceMatching(ReadOnlyTargetRules target)
		: base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[] { "Core" });
		PrivateDependencyModuleNames.AddRange(new[] { "CoreUObject", "Engine" });
	}
}
