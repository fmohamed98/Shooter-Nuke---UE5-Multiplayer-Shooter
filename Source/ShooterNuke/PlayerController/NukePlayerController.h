// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NukePlayerController.generated.h"

class ANukeHUD;
/**
 * 
 */
UCLASS()
class SHOOTERNUKE_API ANukePlayerController : public APlayerController
{
	GENERATED_BODY()
protected:
	void BeginPlay() override;
	void OnPossess(APawn* pawn) override;
private:
	ANukeHUD* m_NukeHUD;

	bool IsCharacterOverlayValid();

public:
	void SetHUDHealth(float health, float maxHealth);
	void SetHUDScore(float scoreAmount);
	void SetHUDDeathCount(uint32 deathCount);
	void SetHUDWeaponAmmo(uint32 ammoCount);
	void HideHUDWeaponAmmo();
};
