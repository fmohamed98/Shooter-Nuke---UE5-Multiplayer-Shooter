// Fill out your copyright notice in the Description page of Project Settings.


#include "NukeAnimInstance.h"
#include "NukeCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UNukeAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    m_NukeCharacter = Cast<ANukeCharacter>(TryGetPawnOwner());
}

void UNukeAnimInstance::NativeUpdateAnimation(float deltaTime)
{
    Super::NativeUpdateAnimation(deltaTime);

    if (m_NukeCharacter == nullptr)
    {
        return;
    }

    FVector velocity = m_NukeCharacter->GetVelocity();
    velocity.Z = 0.f;
    m_Speed = velocity.Size();

    m_IsInAir = m_NukeCharacter->GetCharacterMovement()->IsFalling();
    m_IsAccelerating = m_NukeCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f;
    m_IsWeaponEquipped = m_NukeCharacter->IsWeaponEquipped();
    m_IsCrouched = m_NukeCharacter->bIsCrouched;
}