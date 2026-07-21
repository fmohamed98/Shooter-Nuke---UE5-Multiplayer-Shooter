// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "ShooterNuke/Character/NukeCharacter.h"
#include "ShooterNuke/PlayerController/NukePlayerController.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "ShooterNuke/NukeComponents/LagCompensationComponent.h"

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
    ANukeCharacter* ownerCharacter = Cast<ANukeCharacter>(GetOwner());
    if (ownerCharacter == nullptr)
    {
        return;
    }

    ANukePlayerController* ownerController = Cast<ANukePlayerController>(ownerCharacter->Controller);
    if (ownerController == nullptr)
    {
        return;
    }

    if (ownerCharacter->HasAuthority() && !m_UseServerSideRewind)
    {
        UGameplayStatics::ApplyDamage(otherActor, m_Damage, ownerCharacter->GetController(), this, UDamageType::StaticClass());
        Super::OnHit(hitComp, otherActor, otherComp, normalImpulse, hitResult);
        return;
    }
    
    ANukeCharacter* hitCharacter = Cast<ANukeCharacter>(otherActor);
    if (m_UseServerSideRewind && ownerCharacter->IsLocallyControlled() && ownerCharacter->GetLagCompensationComponent())
    {
        ownerCharacter->GetLagCompensationComponent()->ServerProjectileScoreRequest(hitCharacter, m_TraceStart, m_InitialVelocity, ownerController->GetServerTime() - ownerController->m_SingleTripTime);
    }    
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
