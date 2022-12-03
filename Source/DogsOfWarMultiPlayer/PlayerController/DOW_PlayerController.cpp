// Fill out your copyright notice in the Description page of Project Settings.


#include "DOW_PlayerController.h"
#include "DogsOfWarMultiPlayer/HUD/DOWHUD.h"
#include "DogsOfWarMultiPlayer/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "DogsOfWarMultiPlayer/Character/MyDOWCharacter.h"

void ADOW_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	DOWHUD = Cast<ADOWHUD>(GetHUD());
}

void ADOW_PlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AMyDOWCharacter* MyDOWCharacter = Cast<AMyDOWCharacter>(InPawn);
	if (MyDOWCharacter)
	{
		SetHUDHealth(MyDOWCharacter->GetHealth(), MyDOWCharacter->GetMaxHealth());
	}
}

void ADOW_PlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	DOWHUD = DOWHUD == nullptr ? Cast<ADOWHUD>(GetHUD()) : DOWHUD;
	bool bHUDValid = DOWHUD &&
		DOWHUD->CharacterOverlay &&
		DOWHUD->CharacterOverlay->WepAmmoQTY;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		DOWHUD->CharacterOverlay->WepAmmoQTY->SetText(FText::FromString(AmmoText));
	}
}

void ADOW_PlayerController::SetHUDInventory_WepAmmoQTY(int32 Ammo)
{
	DOWHUD = DOWHUD == nullptr ? Cast<ADOWHUD>(GetHUD()) : DOWHUD;
	bool bHUDValid = DOWHUD &&
		DOWHUD->CharacterOverlay &&
		DOWHUD->CharacterOverlay->Inventory_WepAmmoQTY;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		DOWHUD->CharacterOverlay->Inventory_WepAmmoQTY->SetText(FText::FromString(AmmoText));
	}
}

void ADOW_PlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	DOWHUD = DOWHUD == nullptr ? Cast<ADOWHUD>(GetHUD()) : DOWHUD;

	bool bHUDValid = DOWHUD &&
		DOWHUD->CharacterOverlay &&
		DOWHUD->CharacterOverlay->HealthBar &&
		DOWHUD->CharacterOverlay->HealthText;

	// Logging checks
	//if (DOWHUD) UE_LOG(LogTemp, Warning, TEXT, ("DOWHUD valid"));
	//if (DOWHUD && DOWHUD->CharacterOverlay) UE_LOG(LogTemp, Warning, TEXT, ("DOWHUD->CharacterOverlay valid"));

	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		DOWHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		DOWHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}

