// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileGrenade.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "GameFramework/ProjectileMovementComponent.h"

AProjectileGrenade::AProjectileGrenade()
{
	m_ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grenade Mesh"));
	m_ProjectileMesh->SetupAttachment(RootComponent);
	m_ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	m_ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	m_ProjectileMovementComponent->bRotationFollowsVelocity = true;
	m_ProjectileMovementComponent->SetIsReplicated(true);
	m_ProjectileMovementComponent->bShouldBounce = true;
}

void AProjectileGrenade::Destroyed()
{
	ExplodeDamage();

	Super::Destroyed();
}

void AProjectileGrenade::BeginPlay()
{
	AActor::BeginPlay();

	SpawnTrailSystem();
	StartDestroyTimer();

	if (m_ProjectileMovementComponent != nullptr)
	{
		m_ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AProjectileGrenade::OnBounce);
	}
}

void AProjectileGrenade::OnBounce(const FHitResult& impactResult, const FVector& impactVelocity)
{
	if (m_BounceSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, m_BounceSound, GetActorLocation());
	}
}
