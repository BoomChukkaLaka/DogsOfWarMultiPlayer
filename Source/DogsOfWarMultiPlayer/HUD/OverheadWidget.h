// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"


UCLASS()
class DOGSOFWARMULTIPLAYER_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DisplayText;

	
	void SetDisplayText(FString TextToDisplay);
	FString GetPlayerNetRole(APawn* InPawn);
	FString GetPlayerName(APawn* InPawn);
	UFUNCTION(BlueprintCallable)
	void ShowPlayerInfo(APawn* InPawn);

protected:
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;
};
