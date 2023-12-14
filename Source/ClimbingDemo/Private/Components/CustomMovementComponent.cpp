// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CustomMovementComponent.h"

#include "CharacterAnimInstance.h"
#include "CharacterAnimInstance.h"
#include "CharacterAnimInstance.h"
#include "CharacterAnimInstance.h"
#include "CharacterAnimInstance.h"
#include "CharacterAnimInstance.h"
#include "CharacterAnimInstance.h"
#include "CharacterAnimInstance.h"
#include "CharacterAnimInstance.h"
#include "CharacterAnimInstance.h"
#include "CharacterAnimInstance.h"
#include "CharacterAnimInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ClimbingDemo/ClimbingDemoCharacter.h"
#include "ClimbingDemo/DebugHelper.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

void UCustomMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	//Debug::Print(FString::Printf(TEXT("Is falling: %hd"), IsFalling()));
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
		bShowDebug ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None,
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
		bShowDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		OutHit, false);

	return OutHit;
}

TArray<FHitResult> UCustomMovementComponent::TraceClimbableSurfaces()
{
	FVector forwardVector = UpdatedComponent->GetForwardVector();
	FVector offset = forwardVector * 30.0f;
	FVector start = UpdatedComponent->GetComponentLocation() + offset;
	FVector end = start + forwardVector;

	return DoCapsuleTraceMultiByObject(start, end, false);
}

FHitResult UCustomMovementComponent::TraceFromEyeHeight(float TraceDistance, float TraceStartOffset)
{
	const FVector ComponentLocation = UpdatedComponent->GetComponentLocation();
	const FVector EyeHeightOffset = UpdatedComponent->GetUpVector() * (CharacterOwner->BaseEyeHeight + TraceStartOffset);
	const FVector Start = ComponentLocation + EyeHeightOffset;
	const FVector End = Start + UpdatedComponent-> GetForwardVector() * TraceDistance;

	return DoLineTraceSingleByObject(Start, End, true);
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
	CurrentClimbableSurfaceNormal = CurrentClimbableSurfaceNormal.GetSafeNormal();

	//Debug::Print(TEXT("Location: " + CurrentClimbableSurfaceLocation.ToCompactString()), FColor::Cyan, 1);
	//Debug::Print(TEXT("Normal: " + CurrentClimbableSurfaceNormal.ToCompactString()), FColor::Red, 2);;
}

void UCustomMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	if (IsClimbing())
	{
		bOrientRotationToMovement = false;
		CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(48.f);

		Debug::Print(TEXT("OnMovementModeChanged IsClimbing"));
	}

	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode ==  static_cast<uint8>(ECustomMovementMode::MOVE_Climb))
	{
		bOrientRotationToMovement = true;
		CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(96.f);

		UpdatedComponent->SetRelativeRotation(FRotator(0.f, UpdatedComponent->GetComponentRotation().Yaw, 0.f));
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

bool UCustomMovementComponent::IsUpperLedgeReached()
{
	FHitResult eyeTraceHit = TraceFromEyeHeight(50.0);

	if (!eyeTraceHit.HasValidHitObjectHandle())
	{
		FVector start = eyeTraceHit.TraceEnd;
		FVector end = start + 20 * FVector::DownVector;
		FHitResult ledgeSurface = DoLineTraceSingleByObject(start, end, false);

		if (ledgeSurface.bBlockingHit && GetClimbVelocity().Z > 10)
		{
			return true;
		}
	}

	return false;
}

bool UCustomMovementComponent::CanClimbDown()
{
	FVector startOffsetVec = UpdatedComponent->GetForwardVector() * 25;
	FVector start = UpdatedComponent->GetComponentLocation() + startOffsetVec + 70 * FVector::DownVector ;
	FVector end = start + 100 * FVector::DownVector;
	
	if (DoLineTraceSingleByObject(start, end, false).bBlockingHit)
	{
		return false;
	}

	bool isClimbableSurfaceUnderneathCharacter = DoLineTraceSingleByObject(end, end - startOffsetVec, true).bBlockingHit != 0;
	
	return isClimbableSurfaceUnderneathCharacter;
}

void UCustomMovementComponent::PhysClimb(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	 //calculate climbable surface, check if climbing
	 UpdateClimbableSurfaceInfo();

	if (ShouldStopClimbing(GetCurrentClimbableSurfaceNormal()))
	{
		StopClimbing();
		return;
	}
	
	 //boilerplate code from PhysFly
	 RestorePreAdditiveRootMotionVelocity();
	
	 if(!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() )
	 {
	 	//define speed and acc
	 	CalcVelocity(deltaTime, 0.f, true, MaxBreakClimbDeceleration);
	 }
	
	 ApplyRootMotionToVelocity(deltaTime);
	
	 FVector OldLocation = UpdatedComponent->GetComponentLocation();
	 const FVector Adjusted = Velocity * deltaTime;
	 FHitResult Hit(1.f);
	
	 //handle rotation
	 SafeMoveUpdatedComponent(Adjusted, GetClimbRotation(deltaTime), true, Hit);
	
	 if (Hit.Time < 1.f && !AnimInstance->IsAnyMontagePlaying()) //prevent from sliding when playing climb down montage - it results in accelerated fall
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

	SnapMovementToClimbableSurfaces(deltaTime);
		
	if (IsUpperLedgeReached())
	{
		TryPlayMontage(AnimInstance->ClimbOnEdgeMontage);
	}
}

float UCustomMovementComponent::GetMaxSpeed() const
{
	if (IsClimbing())
	{
		return MaxClimbSpeed;
	}
	
	return Super::GetMaxSpeed();
}

float UCustomMovementComponent::GetMaxAcceleration() const
{
	if (IsClimbing())
	{
		return MaxClimbAcceleration;
	}
	return Super::GetMaxAcceleration();
}

FVector UCustomMovementComponent::GetClimbVelocity()
{
	return UKismetMathLibrary::Quat_UnrotateVector(UpdatedComponent->GetComponentQuat(), UpdatedComponent->GetComponentVelocity());
}

void UCustomMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	USkeletalMeshComponent* Mesh = CharacterOwner->GetMesh();

	if (Mesh)
	{
		AnimInstance = static_cast<UCharacterAnimInstance*>(Mesh->GetAnimInstance());

		if (AnimInstance)
		{
			AnimInstance->OnMontageEnded.AddDynamic(this, &UCustomMovementComponent::OnAnimMontageEnded);
			AnimInstance->OnMontageBlendingOut.AddDynamic(this, &UCustomMovementComponent::OnAnimMontageEnded);
		}
	}
}

FQuat UCustomMovementComponent::GetClimbRotation(float DeltaTime)
{
	const FQuat Current = UpdatedComponent->GetComponentQuat();

	if (HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocity())
	{
		return Current;
	}

	const FQuat TargetQuat = FRotationMatrix::MakeFromX(-CurrentClimbableSurfaceNormal).ToQuat();
	return FMath:: QInterpTo(Current, TargetQuat, DeltaTime, 5.0f);
}

void UCustomMovementComponent::SnapMovementToClimbableSurfaces(float DeltaTime)
{
	const FVector Forward = UpdatedComponent->GetForwardVector();
	const FVector Location = UpdatedComponent->GetComponentLocation();

	//we need to calculate appropriate length of new forward so it is long enough to reach exactly climbable surface
	const FVector ProjectedCharacterToSurface = (CurrentClimbableSurfaceLocation - Location).ProjectOnTo(Forward);
	
	const FVector MovementDeltaSnappedToSurface = -CurrentClimbableSurfaceNormal * ProjectedCharacterToSurface.Length();

	DrawDebugLine(GetWorld(), Location, Location + ProjectedCharacterToSurface,FColor::Blue);
	DrawDebugLine(GetWorld(), Location, Location + Forward * 20,FColor::Green);
	DrawDebugLine(GetWorld(), CurrentClimbableSurfaceLocation, CurrentClimbableSurfaceLocation + CurrentClimbableSurfaceNormal * 20,FColor::Red);
	
	UpdatedComponent->MoveComponent(
		MovementDeltaSnappedToSurface * DeltaTime * MaxClimbSpeed,
		UpdatedComponent->GetComponentQuat(),
		true //detect collisions
		);

}

bool UCustomMovementComponent::TryPlayMontage(UAnimMontage* Montage)
{
	if (!AnimInstance || AnimInstance->IsAnyMontagePlaying() || !Montage)
	{
		return false;
	}

	Debug::Print(TEXT("successfully played montage"));
	AnimInstance->Montage_Play(Montage);
	return true;
}

void UCustomMovementComponent::StartClimbing()
{
	if (!IsClimbing() && !IsFalling() && CanClimbDown())
	{
		Debug::Print(TEXT("CanClimbDown"));
		TryPlayMontage(AnimInstance->ClimbFromEdgeMontage);
		//UGameplayStatics::SetGlobalTimeDilation(this, 0.2f);
		return;
	}
	
	if (CanClimb())
	{
		if (IsClimbing())
		{
			Debug::Print(TEXT("Already climbing"));
			return;
		}
		
		TryPlayMontage(AnimInstance->StartClimbMontage);
		return;
	}

	FVector outStartPos;
	FVector outEndPos;
	
	if (CanPerformVaulting(outStartPos, outEndPos))
	{
		Debug::Print(TEXT("Can vault"));
	}
	else
	{
		Debug::Print(TEXT("Can NOT vault"));
	}
}

FVector UCustomMovementComponent::ConstrainAnimRootMotionVelocity(const FVector& RootMotionVelocity,
	const FVector& CurrentVelocity) const
{
	if (IsFalling() && AnimInstance && AnimInstance->IsAnyMontagePlaying())
	{
		bool isPlayingClimbOnEdgeMontage = false;
		
		for (FAnimMontageInstance * playingMontage : AnimInstance->MontageInstances)
		{
			if (playingMontage->Montage == AnimInstance->ClimbOnEdgeMontage)
			{
				isPlayingClimbOnEdgeMontage = true;
				break;
			}
		}

		if (isPlayingClimbOnEdgeMontage)
		{
			return RootMotionVelocity;
		}
	}
	
	return Super::ConstrainAnimRootMotionVelocity(RootMotionVelocity, CurrentVelocity);
}

void UCustomMovementComponent::StartClimbingInternal()
{
	Debug::Print(TEXT("Climbing started"));
	SetMovementMode(EMovementMode::MOVE_Custom, static_cast<uint8>(ECustomMovementMode::MOVE_Climb));
}

bool UCustomMovementComponent::ShouldStopClimbing(FVector currentClimbableSurfaceNormal)
{
	// stop climbing when reaching flat surface (when going up)
	if (FVector::DotProduct(FVector::UpVector, currentClimbableSurfaceNormal) > 0.9 && GetClimbVelocity().Z > 10)
	{
		return true;
	}
	
	FVector downVec = -UpdatedComponent->GetUpVector();
	FVector offset = downVec * 50.0f;
	FVector start = UpdatedComponent->GetComponentLocation();
	FVector end = start + offset;

	// is climbing down on floor
	TArray<FHitResult> results = DoCapsuleTraceMultiByObject(start, end, false);

	for (FHitResult result  : results)
	{
		bool isFloorHit = FVector::Parallel(FVector::UpVector, -result.ImpactNormal) == 1 && GetClimbVelocity().Z < -1;
		
		if (isFloorHit)
		{
			Debug::Print(TEXT("REACHED FLOOR"));
			return true;
		}
	}

	return false;
}

void UCustomMovementComponent::OnAnimMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == AnimInstance->StartClimbMontage || Montage == AnimInstance->ClimbFromEdgeMontage)
	{
		StartClimbingInternal();
		StopMovementImmediately();
	}

	if (Montage == AnimInstance->ClimbOnEdgeMontage)
	{
		SetMovementMode(MOVE_Walking);
	}
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

bool UCustomMovementComponent::CanPerformVaulting(FVector& OutStartPosition, FVector& OutEndPosition)
{
	FVector UpVector = UpdatedComponent->GetUpVector();
	FVector DownVector = -UpdatedComponent->GetUpVector();

	OutStartPosition = FVector::Zero();
	OutEndPosition = FVector::Zero();

	for (int i = 0; i < 5; i++)
	{
		FVector Start = UpdatedComponent->GetComponentLocation() + UpdatedComponent->GetForwardVector() * (100 * (i+1)) + UpVector * 50;
		FVector End = Start + DownVector * 200;
		FHitResult result = DoLineTraceSingleByObject(Start, End, true);

		if (result.IsValidBlockingHit())
		{
			if (OutStartPosition == FVector::Zero())
			{
				OutStartPosition = result.ImpactPoint;
			}
			else
			{
				OutEndPosition = result.ImpactPoint;
			}
		}
	}
	
	return OutStartPosition != FVector::Zero() && OutEndPosition != FVector::Zero();
}
