// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERNUKE_API AShotgun : public AHitScanWeapon
{
	GENERATED_BODY()

public:
	void FireShotgun(const TArray<FVector_NetQuantize>& hitTargets);
	void ShotgunTraceEndWithScatter(const FVector& hitTarget, TArray<FVector_NetQuantize>& hitTargets);

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Number of Pellets"), Category = "Weapon Scatter")
	uint32 m_NumberOfPellets = 10;
};
