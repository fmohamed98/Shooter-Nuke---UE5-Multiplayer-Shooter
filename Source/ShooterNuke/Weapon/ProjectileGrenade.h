// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileGrenade.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERNUKE_API AProjectileGrenade : public AProjectile
{
	GENERATED_BODY()
public:
	AProjectileGrenade();
	void Destroyed() override;

protected:
	void BeginPlay() override;

	UFUNCTION()
	void OnBounce(const FHitResult& impactResult, const FVector& impactVelocity);

private:
	UPROPERTY(EditAnywhere, meta = (DisplayName = "Bounce Sound"))
	USoundCue* m_BounceSound;
};
