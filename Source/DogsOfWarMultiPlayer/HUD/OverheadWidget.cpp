// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UOverheadWidget::SetDisplayText(FString TextToDisplay)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}

void UOverheadWidget::ShowPlayerInfo(APawn* InPawn)
{
	FString Text = FString::Printf(TEXT("%s\n%s"), *GetPlayerName(InPawn), *GetPlayerNetRole(InPawn));
	SetDisplayText(Text);
}

FString UOverheadWidget::GetPlayerNetRole(APawn* InPawn)
{
	// For Testing Roles - Displays above Character head
	ENetRole LocalRole = InPawn->GetLocalRole();
	FString Role;
	switch (LocalRole)
	{
		case ENetRole::ROLE_Authority:
			Role = FString("Authority");
			break;
		case ENetRole::ROLE_AutonomousProxy:
			Role = FString("Autonomous Proxy");
			break;
		case ENetRole::ROLE_SimulatedProxy:
			Role = FString("Simulated Proxy");
			break;
		case ENetRole::ROLE_None:
			Role = FString("NONE");
			break;
	}
	//FString LocalRoleString = FString::Printf(TEXT("Local Role: %s"), *Role);
	//SetDisplayText(LocalRoleString);
	return FString::Printf(TEXT("Local Role: %s"), *Role);
}

FString UOverheadWidget::GetPlayerName(APawn* InPawn)
{
	auto State = InPawn->GetPlayerState();
	FString Name = State == nullptr ? FString(TEXT("???")) : State->GetPlayerName();
	return FString::Printf(TEXT("Name: %s"), *Name);
}

void UOverheadWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	RemoveFromParent();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}
