// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "ShooterNuke/Character/NukeCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "ShooterNuke/NukeComponents/LagCompensationComponent.h"
#include "ShooterNuke/PlayerController/NukePlayerController.h"

void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& hitTargets)
{
    AWeapon::Fire(FVector());

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

    const FTransform socketTransform = muzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
    const FVector start = socketTransform.GetLocation();

    TMap<ANukeCharacter*, uint32> hitMap;
    for (auto& hitTarget : hitTargets)
    {
        FHitResult hitResult;
        WeaponTraceHit(start, hitTarget, hitResult);

        if (ANukeCharacter* nukeCharacter = Cast<ANukeCharacter>(hitResult.GetActor()))
        {
            hitMap.FindOrAdd(nukeCharacter)++;
        }

        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), m_ImpactParticles, hitResult.ImpactPoint, hitResult.ImpactNormal.Rotation());
        UGameplayStatics::PlaySoundAtLocation(this, m_HitSound, hitResult.ImpactPoint);
    }

    TArray<ANukeCharacter*> hitCharacters;
    for (auto& hitPair : hitMap)
    {
        bool causeAuthDamage = !m_UseServerSideRewind || ownerPawn->IsLocallyControlled();
        if (HasAuthority() && causeAuthDamage)
        {
            UGameplayStatics::ApplyDamage(hitPair.Key, m_Damage * hitPair.Value, instigatorController, this, UDamageType::StaticClass());
        }

        hitCharacters.Add(hitPair.Key);
    }

    if (!HasAuthority() && m_UseServerSideRewind)
    {
        m_OwningCharacter = m_OwningCharacter == nullptr ? Cast<ANukeCharacter>(ownerPawn) : m_OwningCharacter;
        m_OwningCharacterController = m_OwningCharacterController == nullptr ? Cast<ANukePlayerController>(instigatorController) : m_OwningCharacterController;

        if (m_OwningCharacter != nullptr && m_OwningCharacterController != nullptr && m_OwningCharacter->GetLagCompensationComponent() && m_OwningCharacter->IsLocallyControlled())
        {
            m_OwningCharacter->GetLagCompensationComponent()->ServerShotgunScoreRequest(hitCharacters, start, hitTargets, m_OwningCharacterController->GetServerTime() - m_OwningCharacterController->m_SingleTripTime);
        }
    }
}

void AShotgun::ShotgunTraceEndWithScatter(const FVector& hitTarget, TArray<FVector_NetQuantize>& hitTargets)
{
    const USkeletalMeshSocket* muzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
    if (muzzleFlashSocket == nullptr)
    {
        return;
    }

    const FTransform socketTransform = muzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
    const FVector traceStart = socketTransform.GetLocation();

    const FVector toTargetNormalized = (hitTarget - traceStart).GetSafeNormal();
    const FVector sphereCenter = traceStart + toTargetNormalized * m_DistanceToSphere;

    for (uint32 i = 0; i < m_NumberOfPellets; i++)
    {
        const FVector randVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, m_SphereRadius);
        const FVector endLoc = sphereCenter + randVec;
        const FVector toEndLoc = endLoc - traceStart;
        const FVector traceEnd = traceStart + toEndLoc * TRACE_LENGTH / toEndLoc.Size();

        hitTargets.Add(TraceEndWithScatter(hitTarget));
    }
}
