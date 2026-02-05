// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "NukeAnimInstance.generated.h"

class ANukeCharacter;

/**
 * 
 */
UCLASS()
class SHOOTERNUKE_API UNukeAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	void NativeInitializeAnimation() override;
	void NativeUpdateAnimation(float deltaTime) override;

private:
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true", DisplayName = "Nuke Character"))
	ANukeCharacter* m_NukeCharacter;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true", DisplayName = "Speed"))
	float m_Speed;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true", DisplayName = "Is in Air"))
	bool m_IsInAir;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true", DisplayName = "Is Accelerating"))
	bool m_IsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true", DisplayName = "Is Weapon Equipped"))
	bool m_IsWeaponEquipped;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true", DisplayName = "Is Crouching"))
	bool m_IsCrouched;
};
