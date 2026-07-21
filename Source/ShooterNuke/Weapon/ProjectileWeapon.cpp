// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& hitTarget)
{
    Super::Fire(hitTarget);

    UWorld* world = GetWorld();
    if (world == nullptr)
    {
        return;
    }

    APawn* instigatorPawn = Cast<APawn>(GetOwner());
    if (instigatorPawn == nullptr)
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
    spawnParams.Instigator = instigatorPawn;

    AProjectile* spawnedProjectile = nullptr;
    if (m_UseServerSideRewind)
    {
        if (instigatorPawn->HasAuthority()) //server
        {
            if (instigatorPawn->IsLocallyControlled()) //server, host - use replicated projectile
            {
                spawnedProjectile = world->SpawnActor<AProjectile>(m_ProjectileClass, socketTransform.GetLocation(), toTarget.Rotation(), spawnParams);
                spawnedProjectile->m_UseServerSideRewind = false;
                spawnedProjectile->m_Damage = m_Damage;
            }
            else //server, not locally controlled - spawn non-replicated projectile, SSR
            {
                spawnedProjectile = world->SpawnActor<AProjectile>(m_SSRProjectileClass, socketTransform.GetLocation(), toTarget.Rotation(), spawnParams);
                spawnedProjectile->m_UseServerSideRewind = true;
            }
        }
        else
        {
            if (instigatorPawn->IsLocallyControlled()) //client, locally controlled - use non-replicated projectile, use SSR
            {
                spawnedProjectile = world->SpawnActor<AProjectile>(m_SSRProjectileClass, socketTransform.GetLocation(), toTarget.Rotation(), spawnParams);
                spawnedProjectile->m_UseServerSideRewind = true;
                spawnedProjectile->m_TraceStart = socketTransform.GetLocation();
                spawnedProjectile->m_InitialVelocity = spawnedProjectile->GetActorForwardVector() * spawnedProjectile->m_InitialSpeed;
                spawnedProjectile->m_Damage = m_Damage;
            }
            else //client, not locally controlled - spawn non-replicated projectile, no SSR
            {
                spawnedProjectile = world->SpawnActor<AProjectile>(m_ProjectileClass, socketTransform.GetLocation(), toTarget.Rotation(), spawnParams);
                spawnedProjectile->m_UseServerSideRewind = false;
            }
        }
    }
    else //weapon not using SSR
    {
        if (instigatorPawn->HasAuthority())
        {
            spawnedProjectile = world->SpawnActor<AProjectile>(m_ProjectileClass, socketTransform.GetLocation(), toTarget.Rotation(), spawnParams);
            spawnedProjectile->m_UseServerSideRewind = false;
            spawnedProjectile->m_Damage = m_Damage;
        }
    }
}
