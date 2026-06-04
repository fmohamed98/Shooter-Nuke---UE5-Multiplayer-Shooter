// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "ShooterNuke/Character/NukeCharacter.h"


void AShotgun::Fire(const FVector& hitTarget)
{
    AWeapon::Fire(hitTarget);

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

    TMap<ANukeCharacter*,uint32> hitMap;
    for (uint32 i = 0; i < m_NumberOfPellets; i++)
    {
        FHitResult fireHit;
        WeaponTraceHit(start, hitTarget, fireHit);

        ANukeCharacter* nukeCharacter = Cast<ANukeCharacter>(fireHit.GetActor());
        if (nukeCharacter != nullptr && HasAuthority())
        {
            hitMap.FindOrAdd(nukeCharacter)++;
        }

        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), m_ImpactParticles, fireHit.ImpactPoint, fireHit.ImpactNormal.Rotation());
        UGameplayStatics::PlaySoundAtLocation(this, m_HitSound, fireHit.ImpactPoint);
    }

    for (auto& hitPair : hitMap)
    {
        if (HasAuthority())
        {
            UGameplayStatics::ApplyDamage(hitPair.Key, m_Damage * hitPair.Value, instigatorController, this, UDamageType::StaticClass());
        }
    }
}
