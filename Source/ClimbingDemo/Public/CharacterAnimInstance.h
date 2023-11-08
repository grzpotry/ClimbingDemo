// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ClimbingDemo/ClimbingDemoCharacter.h"
#include "CharacterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class CLIMBINGDEMO_API UCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Reference", meta = (AllowPrivateAccess = "true"))
	float GroundSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Reference", meta = (AllowPrivateAccess = "true"))
	float AirSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Reference", meta = (AllowPrivateAccess = "true"))
	bool IsFalling;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeInitializeAnimation() override;

	UPROPERTY()
	AClimbingDemoCharacter * Character;

	UPROPERTY()
	UCustomMovementComponent * CustomMovement;

	void UpdateGroundSpeed();
};
