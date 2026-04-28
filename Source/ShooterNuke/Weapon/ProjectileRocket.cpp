// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"

AProjectileRocket::AProjectileRocket()
{
	m_RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketMesh"));
	m_RocketMesh->SetupAttachment(RootComponent);
	m_RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AProjectileRocket::OnHit(UPrimitiveComponent* hitComp, AActor* otherActor, UPrimitiveComponent* otherComp, FVector NormalImpulse, const FHitResult& hitResult)
{
	APawn* firingPawn = GetInstigator();
	if (firingPawn != nullptr)
	{
		AController* firingController = firingPawn->GetController();
		if (firingController != nullptr)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(this, m_Damage, 10.f, GetActorLocation(), 200.f, 500.f, 1.f, UDamageType::StaticClass(), TArray<AActor*>(), this, firingController);
		}
	}

	Super::OnHit(hitComp, otherActor, otherComp, NormalImpulse, hitResult);
}