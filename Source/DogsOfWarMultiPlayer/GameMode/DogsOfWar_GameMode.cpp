// Fill out your copyright notice in the Description page of Project Settings.


#include "DogsOfWar_GameMode.h"
#include "DogsOfWarMultiPlayer/Character/MyDOWCharacter.h"
#include "DogsOfWarMultiPlayer/PlayerController/DOW_PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

void ADogsOfWar_GameMode::PlayerEliminated(class AMyDOWCharacter* ElimmedCharacter, class ADOW_PlayerController* VictimController, ADOW_PlayerController* AttackerController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
	}
}

void ADogsOfWar_GameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		// Array of player starts in world
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		//Will respawn character after dying at one of the player starts on map
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}
