// (C) Xrisofor

#include "../../Gameplay/Sosed/Sosed.h"

// Sets default values
ASosed::ASosed()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASosed::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASosed::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASosed::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

