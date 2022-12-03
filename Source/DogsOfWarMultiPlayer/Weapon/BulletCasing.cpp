// Fill out your copyright notice in the Description page of Project Settings.


#include "BulletCasing.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

// Sets default values
ABulletCasing::ABulletCasing()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	BulletCasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletCasingMesh"));
	SetRootComponent(BulletCasingMesh);
	BulletCasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	BulletCasingMesh->SetSimulatePhysics(true);
	BulletCasingMesh->SetEnableGravity(true);
	BulletCasingMesh->SetNotifyRigidBodyCollision(true);
	ShellEjectionImpulse = 10.f;
}

// Called when the game starts or when spawned
void ABulletCasing::BeginPlay()
{
	Super::BeginPlay();
	
	BulletCasingMesh->OnComponentHit.AddDynamic(this, &ABulletCasing::OnHit);
	BulletCasingMesh->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);
}

void ABulletCasing::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (ShellSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
	}
	Destroy();
}


