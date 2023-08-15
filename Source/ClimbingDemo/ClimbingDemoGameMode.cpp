// Copyright Epic Games, Inc. All Rights Reserved.

#include "ClimbingDemoGameMode.h"
#include "ClimbingDemoCharacter.h"
#include "UObject/ConstructorHelpers.h"

AClimbingDemoGameMode::AClimbingDemoGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
