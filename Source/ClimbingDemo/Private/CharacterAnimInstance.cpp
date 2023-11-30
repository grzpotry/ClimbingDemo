// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAnimInstance.h"

#include <string>

#include "ClimbingDemo/DebugHelper.h"
#include "Components/CustomMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"


void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	UpdateGroundSpeed();
}

void UCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	Character = Cast<AClimbingDemoCharacter>(TryGetPawnOwner());

	if (Character)
	{
		CustomMovement = Character->GetCustomMovementComponent();
	}
}

void UCharacterAnimInstance::UpdateGroundSpeed()
{
	if (Character)
	{
		CustomMovement = Character->GetCustomMovementComponent();
		GroundSpeed = UKismetMathLibrary::VSizeXY(Character->GetVelocity());
		//Debug::Print(FString::Printf(TEXT("GroundSpeed: %f"), GroundSpeed));
		AirSpeed = Character->GetVelocity().Z;

		if (CustomMovement)
		{
			bIsClimbing = CustomMovement->IsClimbing();
			bIsFalling = CustomMovement->IsFalling();
			ClimbVelocity = CustomMovement->GetClimbVelocity();
		}

	}
}
