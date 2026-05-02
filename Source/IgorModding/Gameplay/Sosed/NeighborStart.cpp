// (C) Xrisofor

#include "../../Gameplay/Sosed/NeighborStart.h"
#include "../../Gameplay/Sosed/Sosed.h"
#include "../../HelloNeighborRebornGameInstance.h"
#include "../../Modding/ModKit.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/ArrowComponent.h"
#include "Engine/Texture2D.h"
#include "Components/CapsuleComponent.h"
#include "Components/BillboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ANeighborStart::ANeighborStart(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GetCapsuleComponent()->InitCapsuleSize(40.0f, 92.0f);
	GetCapsuleComponent()->SetShouldUpdatePhysicsVolume(false);

#if WITH_EDITORONLY_DATA
	ArrowComponent = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("Arrow"));

	if (!IsRunningCommandlet())
	{
		// Structure to hold one-time initialization
		struct FConstructorStatics
		{
			ConstructorHelpers::FObjectFinderOptional<UTexture2D> PlayerStartTextureObject;
			FName ID_PlayerStart;
			FText NAME_PlayerStart;
			FName ID_Navigation;
			FText NAME_Navigation;
			FConstructorStatics()
				: PlayerStartTextureObject(TEXT("/Engine/EditorResources/S_Player"))
				, ID_PlayerStart(TEXT("PlayerStart"))
				, NAME_PlayerStart(NSLOCTEXT("SpriteCategory", "PlayerStart", "Player Start"))
				, ID_Navigation(TEXT("Navigation"))
				, NAME_Navigation(NSLOCTEXT("SpriteCategory", "Navigation", "Navigation"))
			{
			}
		};
		static FConstructorStatics ConstructorStatics;

		if (GetGoodSprite())
		{
			GetGoodSprite()->Sprite = ConstructorStatics.PlayerStartTextureObject.Get();
			GetGoodSprite()->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
			GetGoodSprite()->SpriteInfo.Category = ConstructorStatics.ID_PlayerStart;
			GetGoodSprite()->SpriteInfo.DisplayName = ConstructorStatics.NAME_PlayerStart;
		}
		if (GetBadSprite())
		{
			GetBadSprite()->SetVisibility(false);
		}

		if (ArrowComponent)
		{
			ArrowComponent->ArrowColor = FColor(150, 200, 255);

			ArrowComponent->ArrowSize = 1.0f;
			ArrowComponent->bTreatAsASprite = true;
			ArrowComponent->SpriteInfo.Category = ConstructorStatics.ID_Navigation;
			ArrowComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_Navigation;
			ArrowComponent->SetupAttachment(GetCapsuleComponent());
			ArrowComponent->bIsScreenSizeScaled = true;
		}
	}
#endif // WITH_EDITORONLY_DATA
}

void ANeighborStart::BeginPlay()
{
	Super::BeginPlay();
	TSubclassOf<ASosed> ResolvedClassToSpawn;
	
	UHelloNeighborRebornGameInstance* GameInstance = Cast<UHelloNeighborRebornGameInstance>(GetGameInstance());
	if (GameInstance && GameInstance->GetModKit())
	{
		FModData NewNeighbor;
		if (!GameInstance->GetModKit()->GetActiveModNeighbor(NewNeighbor, ResolvedClassToSpawn))
		{
			if (!ClassSpawn.IsNull())
				ResolvedClassToSpawn = ClassSpawn.LoadSynchronous();
		}
	}
	
	if (ResolvedClassToSpawn)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			ASosed* SpawnedNeighbor = World->SpawnActor<ASosed>(
				ResolvedClassToSpawn,
				GetActorLocation(),
				GetActorRotation(),
				SpawnParams
			);

			if (SpawnedNeighbor)
				SpawnedNeighbor->SpawnDefaultController();
		}
	}
}

#if WITH_EDITORONLY_DATA
UArrowComponent* ANeighborStart::GetArrowComponent() const { return ArrowComponent; }
#endif