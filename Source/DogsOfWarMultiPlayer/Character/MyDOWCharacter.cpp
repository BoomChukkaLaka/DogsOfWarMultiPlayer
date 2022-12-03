// Fill out your copyright notice in the Description page of Project Settings.


#include "MyDOWCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "DogsOfWarMultiPlayer/Weapon/Weapon.h"
#include "DogsOfWarMultiPlayer/CharacterComponents/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "MyDOWAnimInstance.h"
#include "DogsOfWarMultiPlayer/DogsOfWarMultiPlayer.h"
#include "DogsOfWarMultiPlayer/PlayerController/DOW_PlayerController.h"
#include "DogsOfWarMultiPlayer/GameMode/DogsOfWar_GameMode.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"

#include "DogsOfWarMultiPlayer/Weapon/WeaponTypes.h"

// Sets default values
AMyDOWCharacter::AMyDOWCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//This ensures players always spawn 
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	//GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f); //check

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	// Set Network update frequency 
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	//charRunSpeed = 1.0f;
	//charIsNotSprinting = true;
}
/*
void AMyDOWCharacter::OnCameraUpdate(const FVector& CameraLocation, const FRotator& CameraRotation)
{
	//USkeletalMeshComponent* DefMesh1P = Cast<USkeletalMeshComponent>(GetClass()->GetDefaultSubobjectByName(TEXT("PawnMesh1P")));
	USkeletalMeshComponent* DefMesh1P = Cast<USkeletalMeshComponent>(GetClass()->GetDefaultSubobjectByName(TEXT("CharacterMesh0")));
	const FMatrix DefMeshLS = FRotationTranslationMatrix(DefMesh1P->GetRelativeRotation(), DefMesh1P->GetRelativeLocation());
	const FMatrix LocalToWorld = ActorToWorld().ToMatrixWithScale();

	// Mesh rotating code expect uniform scale in LocalToWorld matrix

	const FRotator RotCameraPitch(CameraRotation.Pitch, 0.0f, 0.0f);
	const FRotator RotCameraYaw(0.0f, CameraRotation.Yaw, 0.0f);

	const FMatrix LeveledCameraLS = FRotationTranslationMatrix(RotCameraYaw, CameraLocation) * LocalToWorld.Inverse();
	const FMatrix PitchedCameraLS = FRotationMatrix(RotCameraPitch) * LeveledCameraLS;
	const FMatrix MeshRelativeToCamera = DefMeshLS * LeveledCameraLS.Inverse();
	const FMatrix PitchedMesh = MeshRelativeToCamera * PitchedCameraLS;

	//Mesh1P->SetRelativeLocationAndRotation(PitchedMesh.GetOrigin(), PitchedMesh.Rotator());
	GetMesh()->SetRelativeLocationAndRotation(PitchedMesh.GetOrigin(), PitchedMesh.Rotator());
}
*/

void AMyDOWCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AMyDOWCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(AMyDOWCharacter, Health);
	DOREPLIFETIME(AMyDOWCharacter, bCharIsMovingForwards); 
}

void AMyDOWCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void AMyDOWCharacter::Elim()
{
	if (Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->DropWeapon();
	}
	MulticastElim();
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&AMyDOWCharacter::ElimTimerFinished,
		ElimDelay
	);
}

void AMyDOWCharacter::MulticastElim_Implementation()
{
	bElimmed = true;
	PlayDeathMontage();

	// After being killed - Player movement is disabled
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if (DOW_PlayerController)
	{
		DisableInput(DOW_PlayerController);
	}
	// Also disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bool bHideSniperScope = IsLocallyControlled() &&
		Combat &&
		Combat->bAiming &&
		Combat->EquippedWeapon &&
		Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	if (bHideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}
}

void AMyDOWCharacter::ElimTimerFinished()
{
	ADogsOfWar_GameMode* DogsOfWar_GameMode = GetWorld()->GetAuthGameMode<ADogsOfWar_GameMode>();
	if (DogsOfWar_GameMode)
	{
		DogsOfWar_GameMode->RequestRespawn(this, Controller);
	}
}

// Called when the game starts or when spawned
void AMyDOWCharacter::BeginPlay()
{
	Super::BeginPlay();	

	UpdateHUDHealth();
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AMyDOWCharacter::ReceiveDamage);
	}
}

// Called every frame
void AMyDOWCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
	HideCameraIfCharacterClose();
}

// Called to bind functionality to input
void AMyDOWCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMyDOWCharacter::Jump);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMyDOWCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyDOWCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AMyDOWCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMyDOWCharacter::LookUp);

	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &AMyDOWCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("DropEquippedWeapon", IE_Pressed, this, &AMyDOWCharacter::DropEquippedWeaponPressed);
	PlayerInputComponent->BindAction("DropEquippedWeapon", IE_Pressed, this, &AMyDOWCharacter::DropEquippedWeaponReleased);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AMyDOWCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AMyDOWCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AMyDOWCharacter::AimButtonReleased);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AMyDOWCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AMyDOWCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("Reload", IE_Released, this, &AMyDOWCharacter::ReloadButtonPressed);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMyDOWCharacter::SprintButtonPressed);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMyDOWCharacter::SprintButtonReleased);
}

void AMyDOWCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
}
/*
void AMyDOWCharacter::Sprint()
{
	charIsNotSprinting = !charIsNotSprinting;
	if (charIsNotSprinting)
	{
		charRunSpeed = 1.0f;
	}
	else
	{
		charRunSpeed = 2.5;
	}
}
*/

void AMyDOWCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AMyDOWCharacter::PlayDeathMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);
	}
}

void AMyDOWCharacter::PlayReloadMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;

		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("Rifle");
			break;
		}

		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AMyDOWCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AMyDOWCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	UpdateHUDHealth();
	PlayHitReactMontage();

	if (Health == 0.f)
	{
		ADogsOfWar_GameMode* DogsOfWar_GameMode = GetWorld()->GetAuthGameMode<ADogsOfWar_GameMode>();
		if (DogsOfWar_GameMode)
		{
			DOW_PlayerController = DOW_PlayerController == nullptr ? Cast<ADOW_PlayerController>(Controller) : DOW_PlayerController;
			ADOW_PlayerController* AttackerController = Cast<ADOW_PlayerController>(InstigatorController);
			DogsOfWar_GameMode->PlayerEliminated(this, DOW_PlayerController, AttackerController);
		}
	}
}

//bool AMyDOWCharacter::IsAlive() const
//{
//	return Health > 0;
//}

void AMyDOWCharacter::MoveForward(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void AMyDOWCharacter::MoveRight(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void AMyDOWCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AMyDOWCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void AMyDOWCharacter::EquipButtonPressed()
{
	if (Combat)
	{
		if (HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipButtonPressed();
		}
	}
}

void AMyDOWCharacter::DropEquippedWeapon()
{
	//if (EquippedWeapon == nullptr) return false;
	if (Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->DropWeapon();
	}
	//return !EquippedWeapon();

	//Combat->EquippedWeapon->SetWeaponState(EWeaponState::EWS_Dropped);

	//if (EquippedWeapon)
	//{
	//	FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
	//	EquippedWeapon->GetWeaponMesh->DetachFromComponent(DetachmentTransformRules);
	//
	//	EquippedWeapon->SetItemsState(EWeaponState::EWS_Dropped);
	//}
}

void AMyDOWCharacter::DropEquippedWeaponPressed()
{
	DropEquippedWeapon();
	Combat->bCanFire = false;
	//IsWeaponEquipped();
}

void AMyDOWCharacter::DropEquippedWeaponReleased()
{

}

// What happens when RPC call is executed on the server
void AMyDOWCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

void AMyDOWCharacter::CrouchButtonPressed()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void AMyDOWCharacter::ReloadButtonPressed()
{
	if (Combat)
	{
		Combat->Reload();
	}
}

void AMyDOWCharacter::AimButtonPressed()
{
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void AMyDOWCharacter::AimButtonReleased()
{
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

float AMyDOWCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void AMyDOWCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;
	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) // Standing still, not jumping
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true; //changed too true to rotate on the rootbone when character is standing still
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir) // Running, or Jumping
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}

void AMyDOWCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// Map pitch from [270, 360] to [-90, 0]
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.5);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
	//if (!HasAuthority() && IsLocallyControlled())
	//{ 
	//	UE_LOG(LogTemp, Warning, TEXT("AO_Pitch: %f"), AO_Pitch);
	//}
	
}

void AMyDOWCharacter::SimProxiesTurn()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	bRotateRootBone = false;
	float Speed = CalculateSpeed();
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	//Output Proxy yaw value to console
	//UE_LOG(LogTemp, Warning, TEXT("ProxyYaw: %f"), ProxyYaw);

	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

}

void AMyDOWCharacter::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void AMyDOWCharacter::FireButtonPressed()
{
	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void AMyDOWCharacter::FireButtonReleased()
{
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void AMyDOWCharacter::W_ButtonPressed()
{
	bCharIsMovingForwards = true;
}

void AMyDOWCharacter::W_ButtonReleased()
{
	bCharIsMovingForwards = false;
	SprintButtonReleased();
}

void AMyDOWCharacter::SprintButtonPressed()
{
	if (Combat && bCharIsMovingForwards)
	{
		Combat->SetCharIsSprinting(true);
	}
}

void AMyDOWCharacter::SprintButtonReleased()
{
	if (Combat)
	{
		Combat->SetCharIsSprinting(false);
	}
}

//bool AMyDOWCharacter::IsFirstPerson() const
//{
//	return IsAlive() && Controller && Controller->IsLocalPlayerController();
//}

void AMyDOWCharacter::TurnInPlace(float DeltaTime)
{
	//UE_LOG(LogTemp, Warning, TEXT("AO_Yaw: %f"), AO_Yaw);
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void AMyDOWCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled()) return;
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void AMyDOWCharacter::OnRep_Health()
{
	UpdateHUDHealth();
	PlayHitReactMontage();
}

void AMyDOWCharacter::UpdateHUDHealth()
{
	DOW_PlayerController = DOW_PlayerController == nullptr ? Cast<ADOW_PlayerController>(Controller) : DOW_PlayerController;
	if (DOW_PlayerController)
	{
		DOW_PlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void AMyDOWCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

void AMyDOWCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

bool AMyDOWCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool AMyDOWCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

AWeapon* AMyDOWCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector AMyDOWCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();
	return Combat->HitTarget;
}

ECombatState AMyDOWCharacter::GetCombatState() const
{
	if (Combat == nullptr) return ECombatState::ECS_MAX;
	return Combat->CombatState;
}





