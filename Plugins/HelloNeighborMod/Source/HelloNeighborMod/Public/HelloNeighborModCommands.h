// (C) Xrisofor

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "HelloNeighborModStyle.h"

class HELLONEIGHBORMOD_API FHelloNeighborModCommands : public TCommands<FHelloNeighborModCommands>
{
public:
	FHelloNeighborModCommands()
		: TCommands<FHelloNeighborModCommands>(TEXT("HelloNeighborMod"), NSLOCTEXT("Contexts", "HelloNeighborMod", "HelloNeighborMod Plugin"), NAME_None, FHelloNeighborModStyle::GetStyleSetName())
	{
	}
	
	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> NewModButton;
	TSharedPtr<FUICommandInfo> PackageModButton;
};