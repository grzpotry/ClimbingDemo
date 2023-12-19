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
	bool bIsFalling;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Reference", meta = (AllowPrivateAccess = "true"))
	bool bIsClimbing;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Reference", meta = (AllowPrivateAccess = "true"))
	FVector ClimbVelocity;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeInitializeAnimation() override;

	UPROPERTY()
	AClimbingDemoCharacter * Character;

	UPROPERTY()
	UCustomMovementComponent * CustomMovement;

	void UpdateGroundSpeed();

public:
	UPROPERTY(EditDefaultsOnly, Category= "Reference", meta = (AllowPrivateAccess = "true"))
	UAnimMontage * StartClimbMontage;

	UPROPERTY(EditDefaultsOnly, Category= "Reference", meta = (AllowPrivateAccess = "true"))
	UAnimMontage * ClimbOnEdgeMontage;

	UPROPERTY(EditDefaultsOnly, Category= "Reference", meta = (AllowPrivateAccess = "true"))
	UAnimMontage * ClimbFromEdgeMontage;

	UPROPERTY(EditDefaultsOnly, Category= "Reference", meta = (AllowPrivateAccess = "true"))
	UAnimMontage * VaultMontage;
};
