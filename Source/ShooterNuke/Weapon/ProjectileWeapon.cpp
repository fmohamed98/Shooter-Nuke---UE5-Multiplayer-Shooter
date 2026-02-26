// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& hitTarget)
{
    Super::Fire(hitTarget);

    if (!HasAuthority())
    {
        return;
    }

    const USkeletalMeshSocket* muzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
    if (muzzleFlashSocket == nullptr)
    {
        return;
    }

    FTransform socketTransform = muzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
    FVector toTarget = hitTarget - socketTransform.GetLocation();

    FActorSpawnParameters spawnParams;
    spawnParams.Owner = GetOwner();
    spawnParams.Instigator = Cast<APawn>(GetOwner());

    if (UWorld* world = GetWorld())
    {
        world->SpawnActor<AProjectile>(m_ProjectileClass, socketTransform.GetLocation(), toTarget.Rotation(), spawnParams);
    }

}
