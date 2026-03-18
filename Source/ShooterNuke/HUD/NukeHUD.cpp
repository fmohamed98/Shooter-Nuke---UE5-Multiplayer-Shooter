// Fill out your copyright notice in the Description page of Project Settings.


#include "NukeHUD.h"
#include "CharacterOverlay.h"

void ANukeHUD::DrawHUD()
{
    Super::DrawHUD();

    FVector2D viewPortSize;
    if (GEngine)
    {
        GEngine->GameViewport->GetViewportSize(viewPortSize);
        const FVector2D viewPortCenter(viewPortSize.X / 2.f, viewPortSize.Y / 2.f);

        const float spreadScaled = m_CrosshairSpreadMax * m_HUDPackage.m_CrossHairSpread;

        DrawCrossHair(m_HUDPackage.m_CrosshairCenter, viewPortCenter, FVector2D(0.f,0.f), m_HUDPackage.m_CrossHairColour);
        DrawCrossHair(m_HUDPackage.m_CrosshairLeft, viewPortCenter, FVector2D(-spreadScaled, 0.f), m_HUDPackage.m_CrossHairColour);
        DrawCrossHair(m_HUDPackage.m_CrosshairRight, viewPortCenter, FVector2D(spreadScaled, 0.f), m_HUDPackage.m_CrossHairColour);
        DrawCrossHair(m_HUDPackage.m_CrosshairTop, viewPortCenter, FVector2D(0.f, -spreadScaled), m_HUDPackage.m_CrossHairColour);
        DrawCrossHair(m_HUDPackage.m_CrosshairBottom, viewPortCenter, FVector2D(0.f, spreadScaled), m_HUDPackage.m_CrossHairColour);
    }
}

void ANukeHUD::BeginPlay()
{
    Super::BeginPlay();

    AddCharacterOverlay();
}

void ANukeHUD::AddCharacterOverlay()
{
    APlayerController* playerController = GetOwningPlayerController();
    if(playerController != nullptr && m_CharacterOverlayClass)
    {
        m_CharacterOverlay = CreateWidget<UCharacterOverlay>(playerController, m_CharacterOverlayClass);
        m_CharacterOverlay->AddToViewport();
    }
}

void ANukeHUD::DrawCrossHair(UTexture2D* texture, FVector2D viewPortCenter, FVector2D spread, FLinearColor crossHairColour)
{
    if (texture == nullptr)
    {
        return;
    }

    const float texWidth = texture->GetSizeX();
    const float texHeight = texture->GetSizeY();

    const FVector2D texDrawPoint(viewPortCenter.X - texWidth / 2.f + spread.X,
                                viewPortCenter.Y - texHeight / 2.f + spread.Y);

    DrawTexture(texture, texDrawPoint.X, texDrawPoint.Y, texWidth, texHeight, 0.f,0.f,1.f,1.f,crossHairColour);
}
