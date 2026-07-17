// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "ProjectileWeapon.generated.h"

class AProjectile;

UCLASS()
class SHOOTERNUKE_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere, meta = (DisplayName = "Projectile Class"));
	TSubclassOf<AProjectile> m_ProjectileClass;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Server-side Rewind Projectile Class"));
	TSubclassOf<AProjectile> m_SSRProjectileClass;

public:
	void Fire(const FVector& hitTarget) override;
};
