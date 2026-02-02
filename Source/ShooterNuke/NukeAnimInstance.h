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
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	ANukeCharacter* NukeCharacter;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool IsInAir;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool IsAccelerating;
};
