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

void ANukePlayerController::OnPossess(APawn* pawn)
{
    Super::OnPossess(pawn);

    ANukeCharacter* nukeCharacter = Cast<ANukeCharacter>(pawn);
    if (nukeCharacter != nullptr)
    {
        SetHUDHealth(nukeCharacter->GetHealth(), nukeCharacter->GetMaxHealth());
    }
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

void ANukePlayerController::SetHUDWeaponAmmoCount(uint32 ammoCount)
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
    }
}
