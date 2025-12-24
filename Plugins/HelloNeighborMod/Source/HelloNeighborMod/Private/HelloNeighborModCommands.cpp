// (C) Xrisofor

#include "HelloNeighborModCommands.h"

#define LOCTEXT_NAMESPACE "FHelloNeighborModModule"

void FHelloNeighborModCommands::RegisterCommands()
{
	UI_COMMAND(NewModButton, "Create Mod", "Create a Game Mode", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(PackageModButton, "Package Mod", "Package the Mod", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
