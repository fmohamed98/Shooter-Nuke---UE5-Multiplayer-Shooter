// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterNuke/NukeTypes/TurningInPlace.h"

#include "NukeCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UCombatComponent;
class AWeapon;

UCLASS()
class SHOOTERNUKE_API ANukeCharacter : public ACharacter
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
	void AimOffset(const float deltaTime);

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

public:	
	bool IsWeaponEquipped() const;
	bool IsAiming() const;
	void SetOverlappingWeapon(AWeapon* weapon);

	FORCEINLINE float GetAimOffsetYaw() const { return m_AimOffsetYaw; }
	FORCEINLINE float GetAimOffsetPitch() const { return m_AimOffsetPitch; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return m_TurningInPlace; }

private:
	float m_InterpAimOffsetYaw = 0.f;
	float m_AimOffsetYaw = 0.f;
	float m_AimOffsetPitch = 0.f;
	FRotator m_StartingAimRotation{ 0.f,0.f,0.f };

	ETurningInPlace m_TurningInPlace{ ETurningInPlace::ETIP_NotTurning };
	void TurnInPlace(float deltaTime);
};
