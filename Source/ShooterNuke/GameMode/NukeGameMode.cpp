// Fill out your copyright notice in the Description page of Project Settings.


#include "NukeGameMode.h"
#include "ShooterNuke/Character/NukeCharacter.h"
#include "ShooterNuke/PlayerState/NukePlayerState.h"
#include "ShooterNuke/PlayerController/NukePlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

namespace MatchState
{
    const FName Cooldown = FName("Cooldown");
}

ANukeGameMode::ANukeGameMode()
{
    bDelayedStart = true;
}

void ANukeGameMode::PlayerEliminated(ANukeCharacter* eliminatedCharacter, ANukePlayerController* attackerPlayerController)
{
    if (eliminatedCharacter == nullptr)
    {
        return;
    }

    ANukePlayerController* victimPlayerController = Cast<ANukePlayerController>(eliminatedCharacter->GetController());
    if (victimPlayerController == nullptr || attackerPlayerController == nullptr)
    {
        return;
    }

    ANukePlayerState* attackerPlayerState = Cast<ANukePlayerState>(attackerPlayerController->PlayerState);
    ANukePlayerState* victimPlayerState = Cast<ANukePlayerState>(victimPlayerController->PlayerState);

    if (attackerPlayerState != nullptr && attackerPlayerState != victimPlayerState)
    {
        attackerPlayerState->AddToScore(1.f);
    }

    if (victimPlayerState != nullptr)
    {
        victimPlayerState->IncrementDeathCount();
    }

    eliminatedCharacter->Eliminate();
 }

void ANukeGameMode::RequestRespawn(ANukeCharacter* eliminatedCharacter)
{
    if (eliminatedCharacter == nullptr)
    {
        return;
    }

    AController* eliminatedController = eliminatedCharacter->GetController();

    eliminatedCharacter->Reset();
    eliminatedCharacter->Destroy();

    if (eliminatedController != nullptr)
    {
        TArray<AActor*> playerStarts;
        UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), playerStarts);
        uint32 index = FMath::RandRange(0, playerStarts.Num() - 1);

        RestartPlayerAtPlayerStart(eliminatedController, playerStarts[index]);
    }
}

void ANukeGameMode::Tick(float deltaTime)
{
    Super::Tick(deltaTime);

    if (MatchState == MatchState::WaitingToStart)
    {
        m_CountDownTime = m_WarmupTime - (GetWorld()->GetTimeSeconds() - m_LevelStartingTime);
        if (m_CountDownTime <= 0.f)
        {
            StartMatch();
        }
    }
    else if (MatchState == MatchState::InProgress)
    {
        m_CountDownTime = m_WarmupTime + m_MatchTime - (GetWorld()->GetTimeSeconds() - m_LevelStartingTime);
        if (m_CountDownTime <= 0.f)
        {
            SetMatchState(MatchState::Cooldown);
        }
    }
    else if (MatchState == MatchState::Cooldown)
    {
        m_CountDownTime = m_WarmupTime + m_MatchTime + m_CooldownTime - (GetWorld()->GetTimeSeconds() - m_LevelStartingTime);
        if (m_CountDownTime <= 0.f)
        {
            RestartGame();
        }
    }
}

void ANukeGameMode::BeginPlay()
{
    Super::BeginPlay();

    m_LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void ANukeGameMode::OnMatchStateSet()
{
    Super::OnMatchStateSet();

    for (auto it = GetWorld()->GetPlayerControllerIterator(); it; it++)
    {
        ANukePlayerController* playerController = Cast<ANukePlayerController>(*it);
        if (playerController != nullptr)
        {
            playerController->OnMatchStateSet(MatchState);
        }
    }
}
