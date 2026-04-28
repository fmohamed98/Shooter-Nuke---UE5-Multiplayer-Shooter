// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShooterNuke/HUD/NukeHUD.h"
#include "ShooterNuke/Weapon/WeaponTypes.h"
#include "ShooterNuke/NukeTypes/CombatState.h"

#include "CombatComponent.generated.h"

class AWeapon;
class ANukeCharacter;
class ANukePlayerController;
class ANukeHUD;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHOOTERNUKE_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatComponent();
	friend ANukeCharacter;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& outLifetimeProps) const override;

private:
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* m_EquippedWeapon;

	UFUNCTION()
	void OnRep_EquippedWeapon();

	ANukeCharacter* m_Character = nullptr;
	ANukeHUD* m_HUD = nullptr;
	ANukePlayerController* m_PlayerController = nullptr;

	FHUDPackage m_HudPackage;
	FVector m_HitTarget;

	bool m_IsFireButtonPressed = false;

	UPROPERTY(Replicated)
	bool m_IsAiming;

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(const bool isAiming);

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& traceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MultiCastFire(const FVector_NetQuantize& traceHitTarget);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void TraceUnderCrossHairs(FHitResult& hitResult);
	void SetHUDCrossHairs(const float deltaTime);
public:
	void SetAiming(const bool isAiming);
	void FireButtonPressed(const bool isPressed);

	bool CanFire();
	void Fire();
	void EquipWeapon(AWeapon* weapon);
	void Reload();
	void HandleReload();
	uint32 GetAmountToReload();

	UFUNCTION(BlueprintCallable)
	void FinishReload();

private:
	UPROPERTY(EditAnywhere, meta = (DisplayName = "Base Walk Speed"))
	float m_BaseWalkSpeed;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Aim Walk Speed"))
	float m_AimWalkSpeed;

	float m_CrossHairVelocityFactor = 0.f;
	float m_CrossHairAirFactor = 0.f;
	float m_CrossHairAimFactor = 0.f;
	float m_CrossHairShootFactor = 0.f;

	//Aiming and FOV
	float m_DefaultFOV;
	float m_CurrentFOV;

	UPROPERTY(EditAnywhere, Category = Combat, meta = (DisplayName = "Zoomed FOV"))
	float m_ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = Combat, meta = (DisplayName = "UnZoom Interpolation Speed"))
	float m_UnZoomInterpSpeed = 20.f;

	void InterpFOV(const float deltaTime);

	//Automatic fire
	FTimerHandle m_FireTimer;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Assault Rifle Staring Ammo "))
	uint32 m_StartingARAmmo = 30;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Assault Rifle Staring Ammo "))
	uint32 m_StartingRocketAmmo = 0;

	void InitCarriedAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	uint32 m_CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	TMap<EWeaponType, int32>m_CarriedAmmoMap;

	bool m_CanFire = true;

	void StartFireTimer();
	void FireTimerFinished();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState m_CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmoValues();
};
