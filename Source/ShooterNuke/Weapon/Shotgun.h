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
	void Fire(const FVector& hitTarget) override;

private:
	UPROPERTY(EditAnywhere, meta = (DisplayName = "Number of Pellets"))
	uint32 m_NumberOfPellets = 10;
};
