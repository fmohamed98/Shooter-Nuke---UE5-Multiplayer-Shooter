// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

class  UProgressBar;
class UTextBlock;
class UImage;
/**
 * 
 */
UCLASS()
class SHOOTERNUKE_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta=(BindWidget))
	UProgressBar* m_HealthBar;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* m_HealthText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* m_ScoreAmount;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* m_DeathCount;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* m_WeaponAmmoCount;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* m_WeaponAmmoText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* m_CarriedAmmoCount;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* m_MatchCountDownText;

	UPROPERTY(meta=(BindWidget))
	UImage* m_HighPingImage;

	UPROPERTY(meta=(BindWidgetAnim), Transient)
	UWidgetAnimation* m_HighPingAnimation;
};
