// Fill out your copyright notice in the Description page of Project Settings.


#include "NukeGameState.h"
#include "Net/UnrealNetwork.h"
#include "ShooterNuke/PlayerState/NukePlayerState.h"

void ANukeGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ANukeGameState, m_TopScoringPlayers);
}

void ANukeGameState::UpdateTopScore(ANukePlayerState* scoringPlayerState)
{
    if (scoringPlayerState == nullptr)
    {
        return;
    }

    if (m_TopScoringPlayers.Num() == 0)
    {
        m_TopScoringPlayers.Add(scoringPlayerState);
        m_TopScore = scoringPlayerState->GetScore();
    }
    else if (scoringPlayerState->GetScore() == m_TopScore)
    {
        m_TopScoringPlayers.AddUnique(scoringPlayerState);
    }
    else if (scoringPlayerState->GetScore() > m_TopScore)
    {
        m_TopScoringPlayers.Empty();
        m_TopScoringPlayers.Add(scoringPlayerState);
        m_TopScore = scoringPlayerState->GetScore();
    }
}