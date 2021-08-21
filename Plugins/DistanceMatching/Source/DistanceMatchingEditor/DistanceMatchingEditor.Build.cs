// Copyright Roman Merkushin. All Rights Reserved.

using UnrealBuildTool;

public class DistanceMatchingEditor : ModuleRules
{
	public DistanceMatchingEditor(ReadOnlyTargetRules target)
		: base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new [] { "Core" });
		PrivateDependencyModuleNames.AddRange(
			new []
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"AnimGraph",
				"AnimationModifiers",
				"DistanceMatching",
				"UnrealEd",
				"BlueprintGraph"
			}
		);
	}
}
