// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"

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
    FVector end = start + (hitTarget - start)*1.25f;
    
    FHitResult hitResult;
    if (UWorld* world = GetWorld())
    {
        world->LineTraceSingleByChannel(hitResult, start, end, ECollisionChannel::ECC_Visibility);
        if (hitResult.bBlockingHit)
        {
            if (HasAuthority())
            {
                UGameplayStatics::ApplyDamage(hitResult.GetActor(), m_Damage, instigatorController, this, UDamageType::StaticClass());
            }
            UGameplayStatics::SpawnEmitterAtLocation(world, m_ImpactParticles, hitResult.ImpactPoint, hitResult.ImpactNormal.Rotation());
        }
    }
}
