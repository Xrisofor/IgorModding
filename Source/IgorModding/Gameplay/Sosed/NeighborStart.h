// (C) Xrisofor

#pragma once

#include "CoreMinimal.h"
#include "Components/ArrowComponent.h"
#include "Engine/NavigationObjectBase.h"
#include "NeighborStart.generated.h"

UCLASS(Blueprintable, ClassGroup=Common, hidecategories=Collision)
class IGORMODDING_API ANeighborStart : public ANavigationObjectBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<class ASosed> ClassSpawn;
	
public:
	ANeighborStart(const FObjectInitializer& ObjectInitializer);
	
	virtual void BeginPlay() override;

#if WITH_EDITORONLY_DATA
private:
	UPROPERTY()
	class UArrowComponent* ArrowComponent;
#endif

#if WITH_EDITORONLY_DATA
public:
	class UArrowComponent* GetArrowComponent() const;
#endif
};
