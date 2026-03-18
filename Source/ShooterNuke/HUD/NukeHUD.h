// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "NukeHUD.generated.h"

class UTexture2D;
class UCharacterOverlay;

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
public:
	UTexture2D* m_CrosshairCenter;
	UTexture2D* m_CrosshairLeft;
	UTexture2D* m_CrosshairRight;
	UTexture2D* m_CrosshairTop;
	UTexture2D* m_CrosshairBottom;

	float m_CrossHairSpread;
	FLinearColor m_CrossHairColour;
};

/**
 * 
 */
UCLASS()
class SHOOTERNUKE_API ANukeHUD : public AHUD
{
	GENERATED_BODY()
public:
	void DrawHUD() override;

	FORCEINLINE void SetHUDPackage(const FHUDPackage& package) { m_HUDPackage = package; }

	UCharacterOverlay* m_CharacterOverlay;

protected:
	void BeginPlay() override;

	void AddCharacterOverlay();
private:
	FHUDPackage m_HUDPackage;

	UPROPERTY(EditAnywhere, Category = "Player Stats", meta = (DisplayName = "Character Overlay Class"))
	TSubclassOf<UUserWidget> m_CharacterOverlayClass;

	float m_CrosshairSpreadMax = 16.f;

	void DrawCrossHair(UTexture2D* texture, FVector2D viewPortCenter, FVector2D spread, FLinearColor crossHairColour);
};
