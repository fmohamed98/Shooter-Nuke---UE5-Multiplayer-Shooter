// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "NukePlayerState.generated.h"

class ANukePlayerController;

UCLASS()
class SHOOTERNUKE_API ANukePlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	void OnRep_Score() override;
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& outLifetimeProps) const override;

	UFUNCTION()
	void OnRep_DeathCount();

	void AddToScore(float scoreAmount);
	void IncrementDeathCount();

private:
	ANukePlayerController* GetValidPlayerController();

	UPROPERTY(ReplicatedUsing = OnRep_DeathCount)
	uint32 m_DeathCount = 0;
};
