// Copyright Epic Games, Inc. All Rights Reserved.

#include "CapStoneGameMode.h"
#include "CapStoneCharacter.h"
#include "UObject/ConstructorHelpers.h"

ACapStoneGameMode::ACapStoneGameMode()
{
	// set default pawn class to our Blueprinted character
	// static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Learning/Character/BP_RLCharacter1.BP_RLCharacter1"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
