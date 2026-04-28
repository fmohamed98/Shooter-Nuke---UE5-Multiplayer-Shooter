// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERNUKE_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileRocket();

protected:
	void OnHit(UPrimitiveComponent* hitComp, AActor* otherActor, UPrimitiveComponent* otherComp, FVector NormalImpulse, const FHitResult& hitResult) override;

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* m_RocketMesh;
};
