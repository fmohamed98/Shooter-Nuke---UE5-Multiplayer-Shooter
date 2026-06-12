// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "RocketMovementComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystemInstance.h"

AProjectileRocket::AProjectileRocket()
{
	m_ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketMesh"));
	m_ProjectileMesh->SetupAttachment(RootComponent);
	m_ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	m_RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	m_RocketMovementComponent->bRotationFollowsVelocity = true;
	m_RocketMovementComponent->SetIsReplicated(true);
}

void AProjectileRocket::OnHit(UPrimitiveComponent* hitComp, AActor* otherActor, UPrimitiveComponent* otherComp, FVector NormalImpulse, const FHitResult& hitResult)
{
	if (otherActor == GetOwner())
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit self"));
		return;
	}
	
	APawn* firingPawn = GetInstigator();
	if (firingPawn != nullptr && HasAuthority())
	{
		AController* firingController = firingPawn->GetController();
		if (firingController != nullptr)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(this, m_Damage, 10.f, GetActorLocation(), 200.f, 500.f, 1.f, UDamageType::StaticClass(), TArray<AActor*>(), this, firingController);
		}
	}

	StartDestroyTimer();

	if (m_ImpactParticles != nullptr)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), m_ImpactParticles, GetActorTransform());
	}

	if (m_ImpactSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, m_ImpactSound, GetActorLocation());
	}

	if (m_ProjectileMesh != nullptr)
	{
		m_ProjectileMesh->SetVisibility(false);
	}

	if (m_CollisionBox != nullptr)
	{
		m_CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (m_TrailSystemComponent != nullptr && m_TrailSystemComponent->GetSystemInstance() != nullptr)
	{
		m_TrailSystemComponent->GetSystemInstance()->Deactivate();
		m_TrailSystemComponent = nullptr;
	}

	if (m_SoundLoopComponent != nullptr && m_SoundLoopComponent->IsPlaying())
	{
		m_SoundLoopComponent->Stop();
		m_SoundLoopComponent = nullptr;
	}
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	if (m_CollisionBox != nullptr && !HasAuthority())
	{
		m_CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit);
	}

	SpawnTrailSystem();

	if (m_SoundLoop != nullptr && m_SoundLoopAttenuation != nullptr)
	{
		m_SoundLoopComponent = UGameplayStatics::SpawnSoundAttached(m_SoundLoop, GetRootComponent(), FName(), GetActorLocation(), EAttachLocation::KeepWorldPosition, false, 1.f, 1.f, 0.f, m_SoundLoopAttenuation, nullptr, false);
	}
}

void AProjectileRocket::Destroyed()
{
}
