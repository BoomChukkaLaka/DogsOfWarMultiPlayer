// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DogsOfWarMultiPlayer/HUD/DOWHUD.h"
#include "DogsOfWarMultiPlayer/Weapon/WeaponTypes.h"
#include "DogsOfWarMultiPlayer/Types/CombatState.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80000.f


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DOGSOFWARMULTIPLAYER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatComponent();
	friend class AMyDOWCharacter;
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(class AWeapon* WeaponToEquip);
	void Reload();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon, VisibleAnywhere)
	AWeapon* EquippedWeapon;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	void SetCharIsSprinting(bool bIsSprinting); //check

	UFUNCTION(Server, Reliable)
	void ServerSetCharIsSprinting(bool bIsSprinting);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void FireButtonPressed(bool bPressed);

	void Fire();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReloadFunction();
	int32 AmountToReload();

private:
	UPROPERTY()
	class AMyDOWCharacter* Character;

	UPROPERTY()
	class ADOW_PlayerController* Controller;

	UPROPERTY()
	class ADOWHUD* HUD;

	//UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon, EditAnywhere)
	//AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(Replicated)
	bool bSprinting;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	UPROPERTY(EditAnywhere)
	float SprintSpeed;

	bool bFireButtonPressed;

	/* HUD and Aiming crosshairs */

	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	FVector HitTarget;

	FHUDPackage HUDPackage;

	/* Aiming and FOV */

	// Field of view when not aiming; set to the camera's base FOV in BeginPlay
	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.f;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	/*  Automatic fire */

	FTimerHandle FireTimer;
	bool bCanFire = true;

	void StartFireTimer();
	void FireTimerFinished();

	bool CanFire();

	UPROPERTY(ReplicatedUsing = OnRep_Inventory_WepAmmoQTY)
	int32 Inventory_WepAmmoQTY;

	UFUNCTION()
	void OnRep_Inventory_WepAmmoQTY();

	TMap<EWeaponType, int32> Inventory_WepAmmoQTYMap;

	UPROPERTY(EditAnywhere)
	int32 StartingAssaultRifleAmmo = 30;

	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingSniperAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeLauncherAmmo = 0;

	void InitilizeInventory_WepAmmoQTY();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmoValues();

public:	
		
};
