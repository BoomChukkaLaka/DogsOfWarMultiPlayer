// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
//#include "MyPlayerCameraManager.generated.h"

/**
 * 
 
UCLASS()
class DOGSOFWARMULTIPLAYER_API AMyPlayerCameraManager : public APlayerCameraManager
{
	// Need to be changed to GENERATED_UCLASS_BODY as macro includes constructor definition, new “GENERATED_BODY” macro doesn’t.
	GENERATED_UCLASS_BODY()
public:

	/** normal FOV */
	//float NormalFOV;

	/** targeting FOV */
	//float TargetingFOV;

	/** After updating camera, inform pawn to update 1p mesh to match camera's location&rotation */
	//virtual void UpdateCamera(float DeltaTime) override;
//};

