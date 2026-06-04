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

protected:
	FVector TraceEndWithScatter(const FVector traceStart, const FVector& hitTarget);

private:
	UPROPERTY(EditAnywhere, meta = (DisplayName = "Damage"))
	float m_Damage = 20.f;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Impact Particles"))
	UParticleSystem* m_ImpactParticles;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Beam Particles"))
	UParticleSystem* m_BeamParticles;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Muzzle Flash"))
	UParticleSystem* m_MuzzleFlash;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Fire Sound"))
	USoundCue* m_FireSound;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Hit Sound"))
	USoundCue* m_HitSound;

	//Trace end with scatter params
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter", meta = (DisplayName = "Distance to Sphere"))
	float m_DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter", meta = (DisplayName = "Sphere Radius"))
	float m_SphereRadius = 75.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter", meta = (DisplayName = "Use Scatter"))
	bool m_UseScatter = false;
};
