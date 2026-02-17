// Fill out your copyright notice in the Description page of Project Settings.


#include "NukeAnimInstance.h"
#include "NukeCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

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
    m_IsAiming = m_NukeCharacter->IsAiming();

    //YawOffset
    FRotator aimRotation = m_NukeCharacter->GetBaseAimRotation();
    FRotator movementRotation = UKismetMathLibrary::MakeRotFromX(velocity);
    FRotator deltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(movementRotation, aimRotation);
    m_DeltaRotationSmooth = UKismetMathLibrary::RInterpTo(m_DeltaRotationSmooth, deltaRotation, deltaTime, 6.0f);
    m_YawOffSet = m_DeltaRotationSmooth.Yaw;

    //Lean
    m_CharacterRotationLastFrame = m_CharacterRotation;
    m_CharacterRotation = m_NukeCharacter->GetActorRotation();
    const FRotator delta = UKismetMathLibrary::NormalizedDeltaRotator(m_CharacterRotation, m_CharacterRotationLastFrame);
    const float target = delta.Yaw / deltaTime;
    const float interp = FMath::FInterpTo(m_Lean, target, deltaTime, 6.0f);
    m_Lean = FMath::Clamp(interp, -90.f, 90.f);

    m_AimOffsetYaw = m_NukeCharacter->GetAimOffsetYaw();
    m_AimOffsetPitch = m_NukeCharacter->GetAimOffsetPitch();

    m_TurningInPlace = m_NukeCharacter->GetTurningInPlace();
}