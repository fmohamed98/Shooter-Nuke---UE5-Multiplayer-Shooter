// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "NukeGameMode.generated.h"

class ANukeCharacter;
class ANukePlayerController;

UCLASS()
class SHOOTERNUKE_API ANukeGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ANukeGameMode();

	void PlayerEliminated(ANukeCharacter* eliminatedCharcter, ANukePlayerController* attackerPlayerController);
	void RequestRespawn(ANukeCharacter* eliminatedCharcter);

	void Tick(float DeltaSeconds) override;

	UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "Warmup Time"))
	float m_WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "Match Time"))
	float m_MatchTime = 120.f;

	float m_LevelStartingTime = 0.f;

protected:
	void BeginPlay() override;
	void OnMatchStateSet() override;

private:
	float m_CountDownTime = 0.f;
};
