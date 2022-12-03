// Fill out your copyright notice in the Description page of Project Settings.


#include "MyDOWAnimInstance.h"
#include "MyDOWCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DogsOfWarMultiPlayer/TurningInPlace.h"	// unsure if need check?
#include "DogsOfWarMultiPlayer/Weapon/Weapon.h"
#include "DogsOfWarMultiPlayer/Types/CombatState.h"

void UMyDOWAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	MyDOWCharacter = Cast<AMyDOWCharacter>(TryGetPawnOwner());
}

void UMyDOWAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (MyDOWCharacter == nullptr)
	{
		MyDOWCharacter = Cast<AMyDOWCharacter>(TryGetPawnOwner());
	}
	if (MyDOWCharacter == nullptr) return;

	FVector Velocity = MyDOWCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = MyDOWCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = MyDOWCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bWeaponEquipped = MyDOWCharacter->IsWeaponEquipped();
	EquippedWeapon = MyDOWCharacter->GetEquippedWeapon();
	bIsCrouched = MyDOWCharacter->bIsCrouched;
	bAiming = MyDOWCharacter->IsAiming();
	TurningInPlace = MyDOWCharacter->GetTurningInPlace();
	bRotateRootBone = MyDOWCharacter->ShouldRotateRootBone();
	bElimmed = MyDOWCharacter->IsElimmed();

	// Offset Yaw for Strafing
	FRotator AimRotation = MyDOWCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(MyDOWCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = MyDOWCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = MyDOWCharacter->GetAO_Yaw();
	AO_Pitch = MyDOWCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && MyDOWCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		MyDOWCharacter->GetMesh()->TransformToBoneSpace(FName("cc_base_r_hand"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		// The Fname you need to get from the actual bone name from the right hand skeletal bone - Need to match the skeleton so it can find it.
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));
		
		//FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("cc_base_r_hand"), ERelativeTransformSpace::RTS_World);
		//RightHandRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - MyDOWCharacter->GetHitTarget()));		

		FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
		
		//Drawing debug lines from gun direction
		//DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation() + MuzzleX * 1000.f, FColor::Red);
		//DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MyDOWCharacter->GetHitTarget(), FColor::Orange);
		// Debug gun direction lines end

		/*
		if (MyDOWCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			//FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("cc_base_r_hand"), ERelativeTransformSpace::RTS_World);

			//RightHandRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - MyDOWCharacter->GetHitTarget() )); //RightHandRotation.Yaw -= 90.f
			//RightHandRotation.Yaw -= 90.f;
			//RightHandRotation.Pitch -= 90.f;
			// FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - MyDOWCharacter->GetHitTarget()));
			//FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - MyDOWCharacter->GetHitTarget());
			//RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);
		}
		*/
	}


	// For getting the values in editor for direction movement etc -180 - 180 which direction etc
	// if (!MyDOWCharacter->HasAuthority() && !MyDOWCharacter->IsLocallyControlled())
	// {
	//	UE_LOG(LogTemp, Warning, TEXT("AimRotation Yaw %f: "), AimRotation.Yaw);
	//	UE_LOG(LogTemp, Warning, TEXT("MovementRotation Yaw %f: "), MovementRotation.Yaw);
	// }

	bUseFabrikIK = MyDOWCharacter->GetCombatState() != ECombatState::ECS_Reloading;
	bUseAimOffsets = MyDOWCharacter->GetCombatState() != ECombatState::ECS_Reloading;
	bTransformRightHand = MyDOWCharacter->GetCombatState() != ECombatState::ECS_Reloading;
}
