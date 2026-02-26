// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UBoxComponent;
class UProjectileMovementComponent;

UCLASS()
class SHOOTERNUKE_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere, meta = (DisplayName = "Collision Box"))
	UBoxComponent* m_CollisionBox;

	UPROPERTY(VisibleAnywhere, meta = (DisplayName = "Projectile Movement Component"))
	UProjectileMovementComponent* m_ProjectileMovementComponent;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Tracer"))
	UParticleSystem* m_Tracer;
};
