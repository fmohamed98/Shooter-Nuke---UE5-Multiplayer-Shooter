// Fill out your copyright notice in the Description page of Project Settings.


#include "NukeAnimInstance.h"
#include "NukeCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UNukeAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    NukeCharacter = Cast<ANukeCharacter>(TryGetPawnOwner());
}

void UNukeAnimInstance::NativeUpdateAnimation(float deltaTime)
{
    Super::NativeUpdateAnimation(deltaTime);

    if (NukeCharacter == nullptr)
    {
        return;
    }

    FVector velocity = NukeCharacter->GetVelocity();
    velocity.Z = 0.f;
    Speed = velocity.Size();

    IsInAir = NukeCharacter->GetCharacterMovement()->IsFalling();

    IsAccelerating = NukeCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f;
}