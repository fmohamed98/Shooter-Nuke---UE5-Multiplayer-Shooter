// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"

AProjectileBullet::AProjectileBullet()
{
    m_ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
    m_ProjectileMovementComponent->bRotationFollowsVelocity = true;
    m_ProjectileMovementComponent->SetIsReplicated(true);
    m_ProjectileMovementComponent->InitialSpeed = m_InitialSpeed;
    m_ProjectileMovementComponent->MaxSpeed = m_InitialSpeed;
}


#ifdef WITH_EDITOR
void AProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& event)
{
    Super::PostEditChangeProperty(event);

    FName propertyName = event.Property != nullptr ? event.Property->GetFName() : NAME_None;

    if (propertyName == GET_MEMBER_NAME_CHECKED(AProjectileBullet, m_InitialSpeed))
    {
        if (m_ProjectileMovementComponent != nullptr)
        {
            m_ProjectileMovementComponent->InitialSpeed = m_InitialSpeed;
            m_ProjectileMovementComponent->MaxSpeed = m_InitialSpeed;
        }
    }
    
}
#endif // WITH_EDITOR

void AProjectileBullet::OnHit(UPrimitiveComponent* hitComp, AActor* otherActor, UPrimitiveComponent* otherComp, FVector normalImpulse, const FHitResult& hitResult)
{
    ACharacter* ownerCharacter = Cast<ACharacter>(GetOwner());
    if (ownerCharacter != nullptr)
    {
        UGameplayStatics::ApplyDamage(otherActor, m_Damage, ownerCharacter->GetController(), this, UDamageType::StaticClass());
    }
    
    Super::OnHit(hitComp, otherActor, otherComp, normalImpulse, hitResult);
}

void AProjectileBullet::BeginPlay()
{
    Super::BeginPlay();

    FPredictProjectilePathParams pathParams;
    pathParams.bTraceWithChannel = true;
    pathParams.bTraceWithCollision = true;
    pathParams.DrawDebugTime = 5.f;
    pathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
    pathParams.LaunchVelocity = GetActorForwardVector() * m_InitialSpeed;
    pathParams.MaxSimTime = 4.f;
    pathParams.ProjectileRadius = 5.f;
    pathParams.SimFrequency = 30.f;
    pathParams.StartLocation = GetActorLocation();
    pathParams.TraceChannel = ECC_Visibility;
    pathParams.ActorsToIgnore.Add(this);

    FPredictProjectilePathResult pathResult;

    UGameplayStatics::PredictProjectilePath(this, pathParams, pathResult);
}
