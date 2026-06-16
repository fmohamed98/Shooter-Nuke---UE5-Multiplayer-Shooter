// Fill out your copyright notice in the Description page of Project Settings.


#include "NukePlayerController.h"
#include "ShooterNuke/HUD/NukeHUD.h"
#include "ShooterNuke/HUD/CharacterOverlay.h"
#include "ShooterNuke/HUD/Announcement.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "ShooterNuke/Character/NukeCharacter.h"
#include "ShooterNuke/GameMode/NukeGameMode.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "ShooterNuke/NukeComponents/CombatComponent.h"
#include "ShooterNuke/GameState/NukeGameState.h"
#include "ShooterNuke/PlayerState/NukePlayerState.h"

void ANukePlayerController::BeginPlay()
{
    Super::BeginPlay();

    m_NukeHUD = Cast<ANukeHUD>(GetHUD());

    ServerCheckMatchState();
}

void ANukePlayerController::Tick(float deltaTime)
{
    Super::Tick(deltaTime);

    SetHUDTime();
    CheckTimeSync(deltaTime);
    CheckPing(deltaTime);
}

void ANukePlayerController::CheckTimeSync(float deltaTime)
{
    m_TimeSyncRunningTime += deltaTime;
    if (IsLocalController() && m_TimeSyncRunningTime >= m_TimeSyncFrequency)
    {
        ServerRequestServerTime(GetWorld()->GetTimeSeconds());
        m_TimeSyncRunningTime = 0.f;
    }
}

void ANukePlayerController::CheckPing(float deltaTime)
{
    m_HighPingRunningTime += deltaTime;
    if (m_HighPingRunningTime >= m_CheckPingFrequency)
    {
        if (PlayerState == nullptr)
        {
            PlayerState = GetPlayerState<APlayerState>();
        }

        if (PlayerState != nullptr && PlayerState->GetPingInMilliseconds() > m_HighPingThreshold)
        {
            HighPingWarning();
            m_PingAnimationRunningTime = 0.f;
        }
        m_HighPingRunningTime = 0.f;
    }

    if (IsCharacterOverlayValid())
    {
        UCharacterOverlay* characterOverlay = m_NukeHUD->m_CharacterOverlay;
        if (characterOverlay->m_HighPingAnimation && characterOverlay->IsAnimationPlaying(characterOverlay->m_HighPingAnimation))
        {
            m_PingAnimationRunningTime += deltaTime;
            if (m_PingAnimationRunningTime >= m_HighPingDuration)
            {
                StopHighPingWarning();
            }
        }
    }
}

void ANukePlayerController::ClientJoinMidGame_Implementation(FName matchState, float warmupTime, float matchTime, float cooldownTime, float startingTime)
{
    m_MatchState = matchState;
    m_WarmupTime = warmupTime;
    m_MatchTime = matchTime;
    m_CooldownTime = cooldownTime;
    m_LevelStartingTime = startingTime;

    OnMatchStateSet(m_MatchState);
}

void ANukePlayerController::ServerCheckMatchState_Implementation()
{
    ANukeGameMode* nukeGameMode = Cast<ANukeGameMode>(UGameplayStatics::GetGameMode(this));
    if (nukeGameMode == nullptr)
    {
        return;
    }

    m_WarmupTime = nukeGameMode->m_WarmupTime;
    m_MatchTime = nukeGameMode->m_MatchTime;
    m_CooldownTime = nukeGameMode->m_CooldownTime;
    m_LevelStartingTime = nukeGameMode->m_LevelStartingTime;

    m_MatchState = nukeGameMode->GetMatchState();
    ClientJoinMidGame(m_MatchState, m_WarmupTime, m_MatchTime, m_CooldownTime, m_LevelStartingTime);
}

void ANukePlayerController::OnPossess(APawn* pawn)
{
    Super::OnPossess(pawn);

    ANukeCharacter* nukeCharacter = Cast<ANukeCharacter>(pawn);
    if (nukeCharacter != nullptr)
    {
        SetHUDHealth(nukeCharacter->GetHealth(), nukeCharacter->GetMaxHealth());
    }
}

void ANukePlayerController::ReceivedPlayer()
{
    Super::ReceivedPlayer();

    if (IsLocalController())
    {
        ServerRequestServerTime(GetWorld()->GetTimeSeconds());
    }
}

void ANukePlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ANukePlayerController, m_MatchState);
}

void ANukePlayerController::SetHUDTime()
{
    float timeLeft = 0.f;
    if (m_MatchState == MatchState::WaitingToStart)
    {
        timeLeft = m_WarmupTime - (GetServerTime() - m_LevelStartingTime);
    }
    else if (m_MatchState == MatchState::InProgress)
    {
        timeLeft = m_WarmupTime + m_MatchTime - (GetServerTime() - m_LevelStartingTime);
    }
    else if (m_MatchState == MatchState::Cooldown)
    {
        timeLeft = m_WarmupTime + m_MatchTime + m_CooldownTime - (GetServerTime() - m_LevelStartingTime);
    }
    uint32 secondsLeft = FMath::CeilToInt(timeLeft);


    if (HasAuthority())
    {
        m_NukeGameMode = m_NukeGameMode == nullptr ? Cast<ANukeGameMode>(UGameplayStatics::GetGameMode(this)) : m_NukeGameMode;
        if (m_NukeGameMode != nullptr)
        {
            secondsLeft = FMath::CeilToInt(m_NukeGameMode->GetCountDownTime());
        }
    }

    if (secondsLeft != m_CountDownSecs)
    {
        if (m_MatchState == MatchState::WaitingToStart || m_MatchState == MatchState::Cooldown)
        {
            SetHUDAnnouncementCountdown(timeLeft);
        }
        else if (m_MatchState == MatchState::InProgress)
        {
            SetHUDMatchCountdown(timeLeft);
        }
    }

    m_CountDownSecs = secondsLeft;
}

void ANukePlayerController::ClientReportServerTime_Implementation(float timeOfClientRequest, float timeServerReceivedClientRequest)
{
    float roundTripTime = GetWorld()->GetTimeSeconds() - timeOfClientRequest;
    float currentServerTime = timeServerReceivedClientRequest + roundTripTime * .5f;

    m_ClientServerDelta = currentServerTime - GetWorld()->GetTimeSeconds();
}

void ANukePlayerController::ServerRequestServerTime_Implementation(float timeOfClientRequest)
{
    ClientReportServerTime(timeOfClientRequest, GetWorld()->GetTimeSeconds());
}

bool ANukePlayerController::IsCharacterOverlayValid()
{
    m_NukeHUD = m_NukeHUD == nullptr ? Cast<ANukeHUD>(GetHUD()) : m_NukeHUD;
    if (m_NukeHUD == nullptr)
    {
        return false;
    }

    UCharacterOverlay* characterOverlay = m_NukeHUD->m_CharacterOverlay;
    if (characterOverlay == nullptr)
    {
        return false;
    }

    return true;
}

bool ANukePlayerController::IsAnnouncementValid()
{
    m_NukeHUD = m_NukeHUD == nullptr ? Cast<ANukeHUD>(GetHUD()) : m_NukeHUD;
    if (m_NukeHUD == nullptr)
    {
        return false;
    }

    UAnnouncement* announcement = m_NukeHUD->m_Announcement;
    if (announcement == nullptr)
    {
        return false;
    }

    if (announcement->m_InfoText == nullptr ||
        announcement->m_AnnouncementText == nullptr ||
        announcement->m_WarmupTime == nullptr)
    {
        return false;
    }

    return true;
}

void ANukePlayerController::SetHUDHealth(float health, float maxHealth)
{
    if (!IsCharacterOverlayValid())
    {
        return;
    }

    UCharacterOverlay* characterOverlay = m_NukeHUD->m_CharacterOverlay;
    if (characterOverlay->m_HealthBar && characterOverlay->m_HealthText)
    {
        const float healthPercent = health / maxHealth;
        characterOverlay->m_HealthBar->SetPercent(healthPercent);

        FText healthText = FText::Format(
            FText::FromString("{0}/{1}"),
            FText::AsNumber(FMath::CeilToInt(health)),
            FText::AsNumber(FMath::CeilToInt(maxHealth))
        );
        characterOverlay->m_HealthText->SetText(healthText);
    }
}

void ANukePlayerController::SetHUDScore(float scoreAmount)
{
    if (!IsCharacterOverlayValid())
    {
        return;
    }

    UCharacterOverlay* characterOverlay = m_NukeHUD->m_CharacterOverlay;
    if (characterOverlay->m_ScoreAmount)
    {
        FText scoreText = FText::AsNumber(FMath::FloorToInt(scoreAmount));
        characterOverlay->m_ScoreAmount->SetText(scoreText);
    }
}

void ANukePlayerController::SetHUDDeathCount(uint32 deathCount)
{
    if (!IsCharacterOverlayValid())
    {
        return;
    }

    UCharacterOverlay* characterOverlay = m_NukeHUD->m_CharacterOverlay;
    if (characterOverlay->m_DeathCount)
    {
        FText deathsText = FText::AsNumber(deathCount);
        characterOverlay->m_DeathCount->SetText(deathsText);
    }
}

void ANukePlayerController::SetHUDWeaponAmmo(uint32 ammoCount)
{
    if (!IsCharacterOverlayValid())
    {
        return;
    }

    UCharacterOverlay* characterOverlay = m_NukeHUD->m_CharacterOverlay;
    if (characterOverlay->m_WeaponAmmoCount)
    {
        FText ammoText = FText::AsNumber(ammoCount);
        characterOverlay->m_WeaponAmmoCount->SetText(ammoText);
        characterOverlay->m_WeaponAmmoCount->SetVisibility(ESlateVisibility::Visible);
        characterOverlay->m_WeaponAmmoText->SetVisibility(ESlateVisibility::Visible);
    }
}

void ANukePlayerController::SetHUDMatchCountdown(float countdownTime)
{
    if (!IsCharacterOverlayValid())
    {
        return;
    }

    UCharacterOverlay* characterOverlay = m_NukeHUD->m_CharacterOverlay;
    if (characterOverlay->m_MatchCountDownText)
    {
        if (countdownTime < 0.f)
        {
            characterOverlay->m_MatchCountDownText->SetText(FText());
            return;
        }

        uint32 minutes = FMath::FloorToInt(countdownTime / 60.f);
        uint32 seconds = countdownTime - minutes * 60;

        FText countdownText = FText::Format(
            FText::FromString("{0}:{1}"),
            FText::AsNumber(minutes),
            FText::AsNumber(seconds)
        );

        characterOverlay->m_MatchCountDownText->SetText(countdownText);
    }
}

void ANukePlayerController::SetHUDAnnouncementCountdown(float countdownTime)
{
    m_NukeHUD = m_NukeHUD == nullptr ? Cast<ANukeHUD>(GetHUD()) : m_NukeHUD;
    if (m_NukeHUD == nullptr)
    {
        return;
    }

    UAnnouncement* announcement = m_NukeHUD->m_Announcement;
    if (announcement && announcement->m_WarmupTime)
    {
        if (countdownTime < 0.f)
        {
            announcement->m_WarmupTime->SetText(FText());
            return;
        }

        uint32 minutes = FMath::FloorToInt(countdownTime / 60.f);
        uint32 seconds = countdownTime - minutes * 60;

        FText countdownText = FText::Format(
            FText::FromString("{0}:{1}"),
            FText::AsNumber(minutes),
            FText::AsNumber(seconds)
        );

        announcement->m_WarmupTime->SetText(countdownText);
    }
}

void ANukePlayerController::SetHUDCarriedAmmo(uint32 carriedAmmoCount)
{
    if (!IsCharacterOverlayValid())
    {
        return;
    }

    UCharacterOverlay* characterOverlay = m_NukeHUD->m_CharacterOverlay;
    if (characterOverlay->m_CarriedAmmoCount)
    {
        FText carriedAmmoText = FText::AsNumber(carriedAmmoCount);
        characterOverlay->m_CarriedAmmoCount->SetText(carriedAmmoText);
        characterOverlay->m_CarriedAmmoCount->SetVisibility(ESlateVisibility::Visible);
    }
}

void ANukePlayerController::HideHUDAmmo()
{
    if (!IsCharacterOverlayValid())
    {
        return;
    }

    UCharacterOverlay* characterOverlay = m_NukeHUD->m_CharacterOverlay;
    if (characterOverlay->m_WeaponAmmoCount 
        && characterOverlay->m_WeaponAmmoText
        && characterOverlay->m_CarriedAmmoCount)
    {
        characterOverlay->m_WeaponAmmoCount->SetVisibility(ESlateVisibility::Hidden);
        characterOverlay->m_WeaponAmmoText->SetVisibility(ESlateVisibility::Hidden);
        characterOverlay->m_CarriedAmmoCount->SetVisibility(ESlateVisibility::Hidden);
    }
}

float ANukePlayerController::GetServerTime()
{
    if (HasAuthority())
    {
        return GetWorld()->GetTimeSeconds();
    }
    else
    {
        return GetWorld()->GetTimeSeconds() + m_ClientServerDelta;
    }
}

void ANukePlayerController::OnMatchStateSet(FName& matchState)
{
    m_MatchState = matchState;

    HandleMatchState();
}

void ANukePlayerController::HighPingWarning()
{
    if (!IsCharacterOverlayValid())
    {
        return;
    }

    UCharacterOverlay* characterOverlay = m_NukeHUD->m_CharacterOverlay;
    if (characterOverlay->m_HighPingImage && characterOverlay->m_HighPingAnimation)
    {
        characterOverlay->m_HighPingImage->SetOpacity(1.f);
        characterOverlay->PlayAnimation(characterOverlay->m_HighPingAnimation, 0.f, 5);
    }
}

void ANukePlayerController::StopHighPingWarning()
{
    if (!IsCharacterOverlayValid())
    {
        return;
    }

    UCharacterOverlay* characterOverlay = m_NukeHUD->m_CharacterOverlay;
    if (characterOverlay->m_HighPingImage && characterOverlay->m_HighPingAnimation)
    {
        characterOverlay->m_HighPingImage->SetOpacity(0.f);
        if (characterOverlay->IsAnimationPlaying(characterOverlay->m_HighPingAnimation))
        {
            characterOverlay->StopAnimation(characterOverlay->m_HighPingAnimation);
        }
    }
}

void ANukePlayerController::OnRep_MatchState()
{
    HandleMatchState();
}

void ANukePlayerController::HandleMatchState()
{
    // Must run before the HUD check: on the server, non-host PlayerControllers have no HUD,
    // so IsCharacterOverlayValid() returns false and would skip this, leaving m_DisableGameplay
    // as false on the server. The server would then replicate false back to the client, overriding
    // the client's local true and leaving the character's rotation unlocked during cooldown.
    if (m_MatchState == MatchState::Cooldown)
    {
        ANukeCharacter* nukeCharacter = Cast<ANukeCharacter>(GetPawn());
        if (nukeCharacter != nullptr && nukeCharacter->GetCombatComponent())
        {
            nukeCharacter->m_DisableGameplay = true;
            nukeCharacter->GetCombatComponent()->FireButtonPressed(false);
            nukeCharacter->GetCombatComponent()->SetAiming(false);
        }
    }

    if (!IsCharacterOverlayValid() || !IsAnnouncementValid())
    {
        return;
    }

    if (m_MatchState == MatchState::InProgress)
    {
        m_NukeHUD->m_Announcement->SetVisibility(ESlateVisibility::Hidden);
        m_NukeHUD->m_CharacterOverlay->SetVisibility(ESlateVisibility::Visible);
    }
    else if (m_MatchState == MatchState::Cooldown)
    {
        m_NukeHUD->m_Announcement->SetVisibility(ESlateVisibility::Visible);
        FText cooldownText = FText::FromString("New Match Starts In :");
        m_NukeHUD->m_Announcement->m_AnnouncementText->SetText(cooldownText);
        m_NukeHUD->m_CharacterOverlay->SetVisibility(ESlateVisibility::Hidden);

        ANukeGameState* nukeGameState = Cast<ANukeGameState>(UGameplayStatics::GetGameState(this));
        ANukePlayerState* nukePlayerState = GetPlayerState<ANukePlayerState>();
        if (nukeGameState!= nullptr && nukePlayerState != nullptr)
        {
            TArray<ANukePlayerState*>& topScoringPlayers = nukeGameState->m_TopScoringPlayers;
            FString infoString;

            if (topScoringPlayers.Num() == 0)
            {
                infoString = "There is no winner";
            }
            else if (topScoringPlayers.Num() == 1 && topScoringPlayers[0] == nukePlayerState)
            {
                infoString = "You are the winner!";
            }
            else if (topScoringPlayers.Num() == 1)
            {
                infoString = FString::Printf(
                    TEXT("Winner : %s"),
                    *topScoringPlayers[0]->GetPlayerName()
                );
            }
            else if (topScoringPlayers.Num() > 1)
            {
                infoString = "Players tied for the win:\n";

                for (const auto& tiedPlayer : topScoringPlayers)
                {
                    if (tiedPlayer == nullptr)
                    {
                        continue;
                    }

                    infoString += tiedPlayer->GetPlayerName();
                    infoString += "\n";
                }
            }

            m_NukeHUD->m_Announcement->m_InfoText->SetText(FText::FromString(infoString));
        }
    }
}
