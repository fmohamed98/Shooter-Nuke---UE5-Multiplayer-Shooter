// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterNuke/NukeTypes/TurningInPlace.h"
#include "ShooterNuke/Interfaces/InteractWithCrosshairInterface.h"

#include "NukeCharacter.generated.h"

class UAnimMontage;
class USpringArmComponent;
class UCameraComponent;
class UCombatComponent;
class AWeapon;
class ANukePlayerController;

UCLASS()
class SHOOTERNUKE_API ANukeCharacter : public ACharacter, public IInteractWithCrosshairInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ANukeCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void Jump() override;
	void MoveForward(const float value);
	void MoveRight(const float value);
	void Turn(const float value);
	void LookUp(const float value);

	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void FireButtonPressed();
	void FireButtonReleased();
	void AimOffset(const float deltaTime);

	void UpdateHUDHealth();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& outLifetimeProps) const override;
	void PostInitializeComponents() override;

private:
	UPROPERTY(VisibleAnywhere, Category = Camera, meta = (DisplayName = "Camera Boom"))
	USpringArmComponent* m_CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera, meta = (DisplayName = "Follow Camera"))
	UCameraComponent* m_FollowCamera;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* m_OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* lastWeapon); //Rep_Notify

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	UPROPERTY(VisibleAnywhere, meta = (DisplayName = "Combat Component"))
	UCombatComponent* m_CombatComponent;

	UPROPERTY(EditAnywhere, Category = Combat, meta = (DisplayName = "Fire Weapon Montage"))
	UAnimMontage* m_FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat, meta = (DisplayName = "Hit Reaction Montage"))
	UAnimMontage* m_HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat, meta = (DisplayName = "Elimination Montage"))
	UAnimMontage* m_ElimMontage;

	ANukePlayerController* m_NukePlayerController = nullptr;
public:	
	bool IsWeaponEquipped() const;
	bool IsAiming() const;
	void SetOverlappingWeapon(AWeapon* weapon);
	void PlayFireMontage();
	void PlayHitReactMontage();
	void PlayElimMontage();

	UFUNCTION()
	void ReceiveDamage(AActor* damagedActor, float damage, const UDamageType* damageType, AController* instigatorController, AActor* damageCauser);

	UFUNCTION(NetMulticast, Reliable)
	void MultiCastEliminate();

	FORCEINLINE UCameraComponent* GetFollowCamera() { return m_FollowCamera; }
	FORCEINLINE float GetAimOffsetYaw() const { return m_AimOffsetYaw; }
	FORCEINLINE float GetAimOffsetPitch() const { return m_AimOffsetPitch; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return m_TurningInPlace; }
	FORCEINLINE bool IsEliminated() const { return m_IsEliminated; }

private:
	float m_InterpAimOffsetYaw = 0.f;
	float m_AimOffsetYaw = 0.f;
	float m_AimOffsetPitch = 0.f;
	FRotator m_StartingAimRotation{ 0.f,0.f,0.f };

	ETurningInPlace m_TurningInPlace{ ETurningInPlace::ETIP_NotTurning };
	void TurnInPlace(float deltaTime);

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Camera  Threshold"))
	float m_CameraThreshold = 200.f;

	void HideCameraIfCharacterClose();

	//Player Health
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float m_MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float m_Health = 100.f;

	bool m_IsEliminated = false;

	UFUNCTION()
	void OnRep_Health();
};
