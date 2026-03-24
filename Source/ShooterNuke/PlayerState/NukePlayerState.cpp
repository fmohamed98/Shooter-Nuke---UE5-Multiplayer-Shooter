// Fill out your copyright notice in the Description page of Project Settings.


#include "NukePlayerState.h"
#include "ShooterNuke/Character/NukeCharacter.h"
#include "ShooterNuke/PlayerController/NukePlayerController.h"
#include "Net/UnrealNetwork.h"

void ANukePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ANukePlayerState, m_DeathCount);
}

ANukePlayerController* ANukePlayerState::GetValidPlayerController()
{
    ANukeCharacter* nukeCharacter = Cast<ANukeCharacter>(GetPawn());
    if (nukeCharacter == nullptr)
    {
        return nullptr;
    }

    ANukePlayerController* nukePlayerController = Cast<ANukePlayerController>(nukeCharacter->GetController());
    return nukePlayerController;
}

void ANukePlayerState::AddToScore(float scoreAmount)
{
    SetScore(GetScore() + scoreAmount);

    if (ANukePlayerController* nukePlayerController = GetValidPlayerController())
    {
        nukePlayerController->SetHUDScore(GetScore());
    }
}

void ANukePlayerState::OnRep_Score()
{
    Super::OnRep_Score();

    if (ANukePlayerController* nukePlayerController = GetValidPlayerController())
    {
        nukePlayerController->SetHUDScore(GetScore());
    }
}

void ANukePlayerState::IncrementDeathCount()
{
    m_DeathCount++;

    if (ANukePlayerController* nukePlayerController = GetValidPlayerController())
    {
        nukePlayerController->SetHUDDeathCount(m_DeathCount);
    }
}

void ANukePlayerState::OnRep_DeathCount()
{
    if (ANukePlayerController* nukePlayerController = GetValidPlayerController())
    {
        nukePlayerController->SetHUDDeathCount(m_DeathCount);
    }
}
