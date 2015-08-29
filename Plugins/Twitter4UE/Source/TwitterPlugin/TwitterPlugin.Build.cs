// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

namespace UnrealBuildTool.Rules
{
	public class TwitterPlugin : ModuleRules
	{
		public TwitterPlugin(TargetInfo Target)
		{
			PublicIncludePaths.AddRange(
				new string[] {
					// ... add public include paths required here ...
				}
				);

			PrivateIncludePaths.AddRange(
				new string[] {
					"Twitter4UE/Private",
					"Twitter4UE/Private/liboauthcpp",
					// ... add other private include paths required here ...
				}
				);

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
                    "HTTP",
					"CoreUObject",
	    			"Engine",
                    "Json",

					// ... add other public dependencies that you statically link with here ...
				}
				);
		}
	}
}