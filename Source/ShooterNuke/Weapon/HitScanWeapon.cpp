// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "WeaponTypes.h"
#include "ShooterNuke/NukeComponents/LagCompensationComponent.h"
#include "ShooterNuke/Character/NukeCharacter.h"
#include "ShooterNuke/PlayerController/NukePlayerController.h"

void AHitScanWeapon::Fire(const FVector& hitTarget)
{
    Super::Fire(hitTarget);

    APawn* ownerPawn = Cast<APawn>(GetOwner());
    if (ownerPawn == nullptr)
    {
        return;
    }
    AController* instigatorController = ownerPawn->GetController();

    const USkeletalMeshSocket* muzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
    if (muzzleFlashSocket == nullptr)
    {
        return;
    }

    FTransform socketTransform = muzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
    FVector start = socketTransform.GetLocation();

    FHitResult hitResult;
    WeaponTraceHit(start, hitTarget, hitResult);

    if (HasAuthority() && !m_UseServerSideRewind)
    {
        UGameplayStatics::ApplyDamage(hitResult.GetActor(), m_Damage, instigatorController, this, UDamageType::StaticClass());
    }

    if(!HasAuthority() && m_UseServerSideRewind)
    {
        m_OwningCharacter = m_OwningCharacter == nullptr ? Cast<ANukeCharacter>(ownerPawn) : m_OwningCharacter;
        m_OwningCharacterController = m_OwningCharacterController == nullptr ? Cast<ANukePlayerController>(instigatorController) : m_OwningCharacterController;

        if (m_OwningCharacter != nullptr && m_OwningCharacterController != nullptr && m_OwningCharacter->GetLagCompensationComponent() && m_OwningCharacter->IsLocallyControlled())
        {
            m_OwningCharacter->GetLagCompensationComponent()->ServerScoreRequest(Cast<ANukeCharacter>(hitResult.GetActor()), start, hitTarget, m_OwningCharacterController->GetServerTime() - m_OwningCharacterController->m_SingleTripTime, this);
        }
    }

    UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), m_ImpactParticles, hitResult.ImpactPoint, hitResult.ImpactNormal.Rotation());
    UGameplayStatics::PlaySoundAtLocation(this, m_HitSound, hitResult.ImpactPoint);

    //if weapon firing animation is not there
    UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), m_MuzzleFlash, socketTransform);
    UGameplayStatics::PlaySoundAtLocation(this, m_FireSound, GetActorLocation());
}

FVector AHitScanWeapon::TraceEndWithScatter(const FVector& hitTarget)
{
    const USkeletalMeshSocket* muzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
    if (muzzleFlashSocket == nullptr)
    {
        return FVector();
    }

    FTransform socketTransform = muzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
    FVector traceStart = socketTransform.GetLocation();

    FVector toTargetNormalized = (hitTarget - traceStart).GetSafeNormal();
    FVector sphereCenter = traceStart + toTargetNormalized * m_DistanceToSphere;
    FVector randVec = UKismetMathLibrary::RandomUnitVector()*FMath::FRandRange(0.f, m_SphereRadius);
    FVector endLoc = sphereCenter + randVec;
    FVector toEndLoc = endLoc - traceStart;
    FVector traceEnd = traceStart + toEndLoc * TRACE_LENGTH / toEndLoc.Size();

    /*DrawDebugSphere(GetWorld(), sphereCenter, m_SphereRadius,12, FColor::Red, true);
    DrawDebugSphere(GetWorld(), endLoc, 4.f, 12, FColor::Orange, true);
    DrawDebugLine(GetWorld(), traceStart, traceEnd, FColor::Cyan,true);*/

    return traceEnd;
}

void AHitScanWeapon::WeaponTraceHit(const FVector& traceStart, const FVector& hitTarget, FHitResult& outHit)
{
    UWorld* world = GetWorld();
    if (world == nullptr)
    {
        return;
    }

    FVector end = traceStart + (hitTarget - traceStart) * 1.25f;

    world->LineTraceSingleByChannel(outHit, traceStart, end, ECollisionChannel::ECC_Visibility);
    FVector beamEnd = end;
    if (outHit.bBlockingHit)
    {
        beamEnd = outHit.ImpactPoint;
    }

    DrawDebugSphere(GetWorld(), beamEnd, 16.f, 12, FColor::Orange, true);

    UParticleSystemComponent* beam = UGameplayStatics::SpawnEmitterAtLocation(world, m_BeamParticles, traceStart, FRotator::ZeroRotator, true);
    if (beam != nullptr)
    {
        beam->SetVectorParameter(FName("Target"), beamEnd);
    }
}
