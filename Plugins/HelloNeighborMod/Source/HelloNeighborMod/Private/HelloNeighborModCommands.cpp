// Copyright Epic Games, Inc. All Rights Reserved.

#include "HelloNeighborModCommands.h"

#define LOCTEXT_NAMESPACE "FHelloNeighborModModule"

void FHelloNeighborModCommands::RegisterCommands()
{
	UI_COMMAND(NewModButton, "Create Mod", "Create a Game Mode", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(PackageModButton, "Package Mod", "Package the Mod", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
