// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

void AProjectileBullet::OnHit(UPrimitiveComponent* hitComp, AActor* otherActor, UPrimitiveComponent* otherComp, FVector normalImpulse, const FHitResult& hitResult)
{
    ACharacter* ownerCharacter = Cast<ACharacter>(GetOwner());
    if (ownerCharacter != nullptr)
    {
        UGameplayStatics::ApplyDamage(otherActor, m_Damage, ownerCharacter->GetController(), this, UDamageType::StaticClass());
    }
    
    Super::OnHit(hitComp, otherActor, otherComp, normalImpulse, hitResult);
}
