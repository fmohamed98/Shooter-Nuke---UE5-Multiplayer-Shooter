// Fill out your copyright notice in the Description page of Project Settings.


#include "NukePlayerController.h"
#include "ShooterNuke/HUD/NukeHUD.h"
#include "ShooterNuke/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "ShooterNuke/Character/NukeCharacter.h"

void ANukePlayerController::BeginPlay()
{
    Super::BeginPlay();

    m_NukeHUD = Cast<ANukeHUD>(GetHUD());
}

void ANukePlayerController::Tick(float deltaTime)
{
    Super::Tick(deltaTime);

    SetHUDTime();
    CheckTimeSync(deltaTime);
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

void ANukePlayerController::SetHUDTime()
{
    uint32 secondsLeft = FMath::CeilToInt(m_MatchTime - GetServerTime());

    if (secondsLeft != m_CountDownSecs)
    {
        SetHUDMatchCountdown(secondsLeft);
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
