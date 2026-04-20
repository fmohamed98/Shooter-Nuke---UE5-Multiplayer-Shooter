// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "NukeGameState.generated.h"

class ANukePlayerState;

UCLASS()
class SHOOTERNUKE_API ANukeGameState : public AGameState
{
	GENERATED_BODY()
public:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScore(ANukePlayerState* scoringPlayerState);

	UPROPERTY(Replicated)
	TArray<ANukePlayerState*> m_TopScoringPlayers;
	
private:
	float m_TopScore = 0.f;
};
