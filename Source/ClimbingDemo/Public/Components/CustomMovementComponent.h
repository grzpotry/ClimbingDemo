// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CustomMovementComponent.generated.h"

class UCharacterAnimInstance;

UENUM(BlueprintType)
enum class ECustomMovementMode : uint8
{
	MOVE_Climb = 0 UMETA(DisplayName = "Climb Mode")
};

/**
 * 
 */
UCLASS()
class CLIMBINGDEMO_API UCustomMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	TArray<FHitResult> DoCapsuleTraceMultiByObject(const FVector& Start, const FVector& End, bool bShowDebug = false);
	FHitResult DoLineTraceSingleByObject(const FVector& Start, const FVector& End, bool bShowDebug = false);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	TArray<TEnumAsByte<EObjectTypeQuery>> ClimbableSurfaceTraceTypes;

	TArray<FHitResult> TraceClimbableSurfaces();
	FHitResult TraceFromEyeHeight(float TraceDistance, float TraceStartOffset = 0.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float ClimbCapsuleTraceRadius = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float ClimbCapsuleTraceHalfHeight = 75.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float MaxBreakClimbDeceleration = 75.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float MaxClimbSpeed = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float MaxClimbAcceleration = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	UCharacterAnimInstance * AnimInstance;

	FVector CurrentClimbableSurfaceLocation;
	FVector CurrentClimbableSurfaceNormal;

	void SnapMovementToClimbableSurfaces(float DeltaTime);
	bool TryPlayMontage(UAnimMontage* Montage);

	void UpdateClimbableSurfaceInfo();
	void StartClimbingInternal();

	bool ShouldStopClimbing(FVector currentClimbableSurfaceNormal);

	UFUNCTION()
	void OnAnimMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	virtual void BeginPlay() override;

protected:
	void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	bool IsUpperLedgeReached();
	bool CanClimbDown();
	void PhysClimb(float deltaTime, int32 Iterations);

	virtual float GetMaxSpeed() const override;
	virtual float GetMaxAcceleration() const override;

	virtual FVector ConstrainAnimRootMotionVelocity(const FVector& RootMotionVelocity, const FVector& CurrentVelocity) const override;

public:
	UFUNCTION()
	FVector GetClimbVelocity();
	FORCEINLINE FVector GetCurrentClimbableSurfaceNormal() const { return CurrentClimbableSurfaceNormal; }
	FORCEINLINE FVector GetCurrentClimbableSurfaceLocation() const { return CurrentClimbableSurfaceLocation; }
	FQuat GetClimbRotation(float DeltaTime);
	void StartClimbing();
	void StopClimbing();

	bool IsClimbing() const;
	bool CanClimb();
};
