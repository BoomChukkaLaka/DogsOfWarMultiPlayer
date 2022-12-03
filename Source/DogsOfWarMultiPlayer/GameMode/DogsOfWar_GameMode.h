// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "DogsOfWar_GameMode.generated.h"

/**
 * 
 */
UCLASS()
class DOGSOFWARMULTIPLAYER_API ADogsOfWar_GameMode : public AGameMode
{
	GENERATED_BODY()
public:
	virtual void PlayerEliminated(class AMyDOWCharacter* ElimmedCharacter, class ADOW_PlayerController* VictimController, ADOW_PlayerController* AttackerController);
	virtual void RequestRespawn(class ACharacter* ElimmedCharacter, AController* ElimmedController);
};
