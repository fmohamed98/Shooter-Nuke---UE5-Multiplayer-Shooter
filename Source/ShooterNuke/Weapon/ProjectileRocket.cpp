// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystemInstance.h"
#include "Sound/SoundCue.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"

AProjectileRocket::AProjectileRocket()
{
	m_RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketMesh"));
	m_RocketMesh->SetupAttachment(RootComponent);
	m_RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AProjectileRocket::OnHit(UPrimitiveComponent* hitComp, AActor* otherActor, UPrimitiveComponent* otherComp, FVector NormalImpulse, const FHitResult& hitResult)
{
	APawn* firingPawn = GetInstigator();
	if (firingPawn != nullptr && HasAuthority())
	{
		AController* firingController = firingPawn->GetController();
		if (firingController != nullptr)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(this, m_Damage, 10.f, GetActorLocation(), 200.f, 500.f, 1.f, UDamageType::StaticClass(), TArray<AActor*>(), this, firingController);
		}
	}

	GetWorldTimerManager().SetTimer(m_DestroyTimer, this, &AProjectileRocket::DestroyTimerFinished, m_DestroyTime);

	if (m_ImpactParticles != nullptr)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), m_ImpactParticles, GetActorTransform());
	}

	if (m_ImpactSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, m_ImpactSound, GetActorLocation());
	}

	if (m_RocketMesh != nullptr)
	{
		m_RocketMesh->SetVisibility(false);
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

	if (m_TrailSystem != nullptr)
	{
		m_TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(m_TrailSystem, GetRootComponent(), FName(), GetActorLocation(), GetActorRotation(), EAttachLocation::KeepWorldPosition, false);
	}

	if (m_SoundLoop != nullptr && m_SoundLoopAttenuation != nullptr)
	{
		m_SoundLoopComponent = UGameplayStatics::SpawnSoundAttached(m_SoundLoop, GetRootComponent(), FName(), GetActorLocation(), EAttachLocation::KeepWorldPosition, false, 1.f, 1.f, 0.f, m_SoundLoopAttenuation, nullptr, false);
	}
}

void AProjectileRocket::Destroyed()
{
}

void AProjectileRocket::DestroyTimerFinished()
{
	Destroy();
}
