// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UBoxComponent;
class UProjectileMovementComponent;
class USoundCue;

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

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Damage"))
	float m_Damage = 5.f;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Collision Box"))
	UBoxComponent* m_CollisionBox;

	UPROPERTY(VisibleAnywhere, meta = (DisplayName = "Projectile Movement Component"))
	UProjectileMovementComponent* m_ProjectileMovementComponent;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	void Destroyed() override;

private:
	UPROPERTY(EditAnywhere, meta = (DisplayName = "Tracer"))
	UParticleSystem* m_Tracer;
};
