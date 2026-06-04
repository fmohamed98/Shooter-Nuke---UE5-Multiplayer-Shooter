// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

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

    for (uint32 i = 0; i < m_NumberOfPellets; i++)
    {
        FVector end = TraceEndWithScatter(start, hitTarget);
    }
}
