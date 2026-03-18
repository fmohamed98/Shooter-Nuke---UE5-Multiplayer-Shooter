// Fill out your copyright notice in the Description page of Project Settings.


#include "NukePlayerController.h"
#include "ShooterNuke/HUD/NukeHUD.h"
#include "ShooterNuke/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void ANukePlayerController::BeginPlay()
{
    Super::BeginPlay();

    m_NukeHUD = Cast<ANukeHUD>(GetHUD());
}

void ANukePlayerController::SetHUDHealth(float health, float maxHealth)
{
    if (m_NukeHUD == nullptr)
    {
        return;
    }

    UCharacterOverlay* characterOverlay = m_NukeHUD->m_CharacterOverlay;
    if (characterOverlay == nullptr)
    {
        return;
    }

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
