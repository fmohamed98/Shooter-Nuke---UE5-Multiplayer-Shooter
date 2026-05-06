// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

class UParticleSystem;

UCLASS()
class SHOOTERNUKE_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	void Fire(const FVector& hitTarget) override;

private:
	UPROPERTY(EditAnywhere, meta = (DisplayName = "Damage"))
	float m_Damage = 20.f;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Impact Particles"))
	UParticleSystem* m_ImpactParticles;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Beam Particles"))
	UParticleSystem* m_BeamParticles;
};
