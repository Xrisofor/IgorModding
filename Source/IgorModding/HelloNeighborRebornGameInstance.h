// (C) Xrisofor

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "HelloNeighborRebornGameInstance.generated.h"

UCLASS()
class IGORMODDING_API UHelloNeighborRebornGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	UFUNCTION(BlueprintPure)
	class UModKit* GetModKit() const;

protected:
	UPROPERTY()
	class UModKit* ModKit;
};
