// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DOW_PlayerController.generated.h"

/**
 * 
 */
UCLASS()
class DOGSOFWARMULTIPLAYER_API ADOW_PlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	void SetHUDHealth(float Health, float MaxHealth);
	virtual void OnPossess(APawn* InPawn) override;
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDInventory_WepAmmoQTY(int32 Ammo);
	

protected:
	virtual void BeginPlay() override;

private:
	class ADOWHUD* DOWHUD;
};
