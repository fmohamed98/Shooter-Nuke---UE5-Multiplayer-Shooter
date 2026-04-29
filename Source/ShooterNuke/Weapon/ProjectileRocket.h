// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;

UCLASS()
class SHOOTERNUKE_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileRocket();

protected:
	void OnHit(UPrimitiveComponent* hitComp, AActor* otherActor, UPrimitiveComponent* otherComp, FVector NormalImpulse, const FHitResult& hitResult) override;
	void BeginPlay() override;
	void Destroyed() override;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Trail System"))
	UNiagaraSystem* m_TrailSystem;

	UNiagaraComponent* m_TrailSystemComponent = nullptr;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Sound Loop"))
	USoundCue* m_SoundLoop;

	UAudioComponent* m_SoundLoopComponent = nullptr;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Sound Loop Attenuation"))
	USoundAttenuation* m_SoundLoopAttenuation;

private:
	UPROPERTY(VisibleAnywhere, meta = (DisplayName = "Trail System"))
	UStaticMeshComponent* m_RocketMesh;

	FTimerHandle m_DestroyTimer;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Destroy Time"))
	float m_DestroyTime = 3.f;

	void DestroyTimerFinished();
};
