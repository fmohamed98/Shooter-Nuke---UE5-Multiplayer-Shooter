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
	void Tick(float deltaTime) override;
	void OnPossess(APawn* pawn) override;
	void ReceivedPlayer() override;
	//Time
	void SetHUDTime();

	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float timeOfClientRequest);

	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float timeOfClientRequest, float timeServerReceivedClientRequest);

	float m_ClientServerDelta = 0.f; //diff b/w client & server time

	UPROPERTY(EditAnywhere, Category = Time)
	float m_TimeSyncFrequency = 5.f;

	float m_TimeSyncRunningTime = 0.f;

private:
	ANukeHUD* m_NukeHUD;

	float m_MatchTime = 120.f;

	uint32 m_CountDownSecs = 0;

	bool IsCharacterOverlayValid();
	void CheckTimeSync(float deltaTime);

public:
	void SetHUDHealth(float health, float maxHealth);
	void SetHUDScore(float scoreAmount);
	void SetHUDDeathCount(uint32 deathCount);
	void SetHUDWeaponAmmo(uint32 ammoCount);
	void SetHUDMatchCountdown(float countdownTime);
	void SetHUDCarriedAmmo(uint32 carriedAmmoCount);
	void HideHUDAmmo();
	
	float GetServerTime();
};
