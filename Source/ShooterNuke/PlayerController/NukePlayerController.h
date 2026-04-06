// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NukePlayerController.generated.h"

class ANukeHUD;
class ANukeGameMode;

UCLASS()
class SHOOTERNUKE_API ANukePlayerController : public APlayerController
{
	GENERATED_BODY()
protected:
	void BeginPlay() override;
	void Tick(float deltaTime) override;
	void OnPossess(APawn* pawn) override;
	void ReceivedPlayer() override;
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& outLifetimeProps) const override;

	//Time
	void SetHUDTime();

	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float timeOfClientRequest);

	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float timeOfClientRequest, float timeServerReceivedClientRequest);

	float m_ClientServerDelta = 0.f; //diff b/w client & server time

	UPROPERTY(EditAnywhere, Category = Time, meta = (DisplayName = "Time Sync Frequency"))
	float m_TimeSyncFrequency = 5.f;

	float m_TimeSyncRunningTime = 0.f;

private:
	ANukeHUD* m_NukeHUD = nullptr;
	ANukeGameMode* m_NukeGameMode = nullptr;

	float m_MatchTime = 0.f;
	float m_WarmupTime = 0.f;
	float m_CooldownTime = 0.f;
	float m_LevelStartingTime = 0.f;

	uint32 m_CountDownSecs = 0;

	bool IsCharacterOverlayValid();
	void CheckTimeSync(float deltaTime);

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(FName matchState, float warmupTime, float matchTime, float cooldownTime, float startingTime);

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName m_MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	void HandleMatchState();

public:
	void SetHUDHealth(float health, float maxHealth);
	void SetHUDScore(float scoreAmount);
	void SetHUDDeathCount(uint32 deathCount);
	void SetHUDWeaponAmmo(uint32 ammoCount);
	void SetHUDMatchCountdown(float countdownTime);
	void SetHUDAnnouncementCountdown(float countdownTime);
	void SetHUDCarriedAmmo(uint32 carriedAmmoCount);
	void HideHUDAmmo();
	
	float GetServerTime();

	void OnMatchStateSet(FName& matchState);
};
