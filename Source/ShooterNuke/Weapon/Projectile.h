// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UBoxComponent;
class UProjectileMovementComponent;
class USoundCue;
class UNiagaraSystem;
class UNiagaraComponent;

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

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* hitComp, AActor* otherActor, UPrimitiveComponent* otherComp, FVector NormalImpulse, const FHitResult& hitResult);

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Impact Particles"))
	UParticleSystem* m_ImpactParticles;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Impact Sound"))
	USoundCue* m_ImpactSound;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Trail System"))
	UNiagaraSystem* m_TrailSystem;

	UPROPERTY()
	UNiagaraComponent* m_TrailSystemComponent = nullptr;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Collision Box"))
	UBoxComponent* m_CollisionBox;

	UPROPERTY(VisibleAnywhere, meta = (DisplayName = "Projectile Movement Component"))
	UProjectileMovementComponent* m_ProjectileMovementComponent;

	UPROPERTY(VisibleAnywhere, meta = (DisplayName = "Projectile Mesh"))
	UStaticMeshComponent* m_ProjectileMesh;


	UPROPERTY(EditAnywhere, meta = (DisplayName = "Damage Inner Radius"))
	float m_DamageInnerRadius = 200.f;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Damage Outer Radius"))
	float m_DamageOuterRadius = 500.f;

	void SpawnTrailSystem();

	void StartDestroyTimer();
	void DestroyTimerFinished();
	void ExplodeDamage();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	void Destroyed() override;

	//SSR
	bool m_UseServerSideRewind = false;
	FVector_NetQuantize m_TraceStart;
	FVector_NetQuantize100 m_InitialVelocity;

	float m_Damage = 5.f;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Initial Speed"))
	float m_InitialSpeed = 15000.f;

private:
	UPROPERTY(EditAnywhere, meta = (DisplayName = "Tracer"))
	UParticleSystem* m_Tracer;

	FTimerHandle m_DestroyTimer;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Destroy Time"))
	float m_DestroyTime = 3.f;

};
