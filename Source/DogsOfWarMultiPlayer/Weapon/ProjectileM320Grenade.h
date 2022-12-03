// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileM320Grenade.generated.h"

/**
 * 
 */
UCLASS()
class DOGSOFWARMULTIPLAYER_API AProjectileM320Grenade : public AProjectile
{
	GENERATED_BODY()
public:
	AProjectileM320Grenade();
	virtual void Destroyed() override;
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);
private:

	UPROPERTY(EditAnywhere)
	USoundCue* BounceSound;
};
