// (C) Xrisofor

#include "HelloNeighborRebornGameInstance.h"
#include "Modding/ModKit.h"

void UHelloNeighborRebornGameInstance::Init()
{
	Super::Init();

	if (!IsValid(ModKit))
	{
		ModKit = NewObject<UModKit>(this);
		ModKit->FindMods();
	}
}

class UModKit* UHelloNeighborRebornGameInstance::GetModKit() const
{
	return ModKit;
}
