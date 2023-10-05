// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CustomMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ClimbingDemo/ClimbingDemoCharacter.h"
#include "ClimbingDemo/DebugHelper.h"
#include "Components/CapsuleComponent.h"

void UCustomMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

TArray<FHitResult> UCustomMovementComponent::DoCapsuleTraceMultiByObject(const FVector& Start, const FVector& End,
                                                                         bool bShowDebug)
{
	TArray<FHitResult> OutCapsuleTraceHitResults;

	UKismetSystemLibrary::CapsuleTraceMultiForObjects(
		this,
		Start,
		End,
		ClimbCapsuleTraceRadius,
		ClimbCapsuleTraceHalfHeight,
		ClimbableSurfaceTraceTypes,
		false,
		TArray<AActor*>(),
		bShowDebug ? EDrawDebugTrace::Persistent : EDrawDebugTrace::None,
		OutCapsuleTraceHitResults, false);

	return OutCapsuleTraceHitResults;
}

FHitResult UCustomMovementComponent::DoLineTraceSingleByObject(const FVector& Start, const FVector& End, bool bShowDebug)
{
	FHitResult OutHit;

	UKismetSystemLibrary::LineTraceSingleForObjects(
		this,
		Start,
		End,
		ClimbableSurfaceTraceTypes,
		false,
		TArray<AActor*>(),
		bShowDebug ? EDrawDebugTrace::Persistent : EDrawDebugTrace::None,
		OutHit, false);

	return OutHit;
}

TArray<FHitResult> UCustomMovementComponent::TraceClimbableSurfaces()
{
	FVector forwardVector = UpdatedComponent->GetForwardVector();
	FVector offset = forwardVector * 30.0f;
	FVector start = UpdatedComponent->GetComponentLocation() + offset;
	FVector end = start + forwardVector;

	return DoCapsuleTraceMultiByObject(start, end);
}

FHitResult UCustomMovementComponent::TraceFromEyeHeight(float TraceDistance, float TraceStartOffset)
{
	const FVector ComponentLocation = UpdatedComponent->GetComponentLocation();
	const FVector EyeHeightOffset = UpdatedComponent->GetUpVector() * (CharacterOwner->BaseEyeHeight + TraceStartOffset);
	const FVector Start = ComponentLocation + EyeHeightOffset;
	const FVector End = Start + UpdatedComponent-> GetForwardVector() * TraceDistance;

	return DoLineTraceSingleByObject(Start, End);
}

void UCustomMovementComponent::UpdateClimbableSurfaceInfo()
{
	const TArray<FHitResult> climbableSurfaces = TraceClimbableSurfaces();

	CurrentClimbableSurfaceLocation = FVector::ZeroVector;
	CurrentClimbableSurfaceNormal = FVector::ZeroVector;

	if (climbableSurfaces.IsEmpty())
	{
		return;
	}

	for (const FHitResult& ClimbableSurface : climbableSurfaces)
	{
		CurrentClimbableSurfaceLocation += ClimbableSurface.ImpactPoint;
		CurrentClimbableSurfaceNormal += ClimbableSurface.ImpactNormal;
	}

	CurrentClimbableSurfaceLocation /= climbableSurfaces.Num();
	CurrentClimbableSurfaceNormal = CurrentClimbableSurfaceNormal.GetSafeNormal(); // ?

	Debug::Print(TEXT("Location: " + CurrentClimbableSurfaceLocation.ToCompactString()), FColor::Cyan, 1);
	Debug::Print(TEXT("Normal: " + CurrentClimbableSurfaceNormal.ToCompactString()), FColor::Red, 2);;
}

void UCustomMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	if (IsClimbing())
	{
		bOrientRotationToMovement = false;
		CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(48.f);

		Debug::PrintWithStack(TEXT("OnMovementModeChanged IsClimbing"));
	}

	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode ==  static_cast<uint8>(ECustomMovementMode::MOVE_Climb))
	{
		bOrientRotationToMovement = true;
		CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(96.f);
		StopMovementImmediately();

		Debug::Print(TEXT("OnMovementModeChanged stopped climbing"));
	}
	
	UCharacterMovementComponent::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
}

void UCustomMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	if (IsClimbing())
	{
		PhysClimb(deltaTime, Iterations);
	}
	Super::PhysCustom(deltaTime, Iterations);
}

void UCustomMovementComponent::PhysClimb(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	//calculate climbable surface, check if climbing
	UpdateClimbableSurfaceInfo();


	//boilerplate code from PhysFly
	RestorePreAdditiveRootMotionVelocity();

	if( !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() )
	{
		//define speed and acc
		CalcVelocity(deltaTime, 0.f, true, MaxBreakClimbDeceleration);
	}

	ApplyRootMotionToVelocity(deltaTime);

	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	const FVector Adjusted = Velocity * deltaTime;
	FHitResult Hit(1.f);

	//handle rotation
	SafeMoveUpdatedComponent(Adjusted, UpdatedComponent->GetComponentQuat(), true, Hit);

	if (Hit.Time < 1.f)
	{
		//adjust and try again
		HandleImpact(Hit, deltaTime, Adjusted);
		SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
	}

	//snap movement to climbable surfaces
	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() )
	{
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime;
	}
}

void UCustomMovementComponent::StartClimbing()
{
	if (!CanClimb())
	{
		Debug::Print(TEXT("Can't climb"));
		return;
	}

	if (IsClimbing())
	{
		Debug::Print(TEXT("Already climbing"));
		return;
	}

	Debug::Print(TEXT("Climbing started"));
	SetMovementMode(EMovementMode::MOVE_Custom, static_cast<uint8>(ECustomMovementMode::MOVE_Climb));
}

void UCustomMovementComponent::StopClimbing()
{
	Debug::Print(TEXT("StopClimbing"));
	SetMovementMode(EMovementMode::MOVE_Falling);
}

bool UCustomMovementComponent::IsClimbing() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == static_cast<uint8>(ECustomMovementMode::MOVE_Climb);
}

bool UCustomMovementComponent::CanClimb()
{
	if (IsFalling())
	{
		return false;
	}

	if (TraceClimbableSurfaces().IsEmpty() || !TraceFromEyeHeight(100.f).bBlockingHit)
	{
		return false;
	}

	return true;
}
