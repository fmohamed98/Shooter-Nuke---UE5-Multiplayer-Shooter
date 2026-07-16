// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterNuke/NukeTypes/TurningInPlace.h"
#include "ShooterNuke/NukeTypes/CombatState.h"
#include "ShooterNuke/Interfaces/InteractWithCrosshairInterface.h"
#include "Components/TimelineComponent.h"

#include "NukeCharacter.generated.h"

class UAnimMontage;
class USpringArmComponent;
class UCameraComponent;
class UBoxComponent;
class UCombatComponent;
class ULagCompensationComponent;
class USoundCue;
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
	void ReloadButtonPressed();
	void AimOffset(const float deltaTime);

	void UpdateHUDHealth();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& outLifetimeProps) const override;
	void PostInitializeComponents() override;
	void Destroyed() override;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", DisplayName = "Combat Component"))
	UCombatComponent* m_CombatComponent;

	UPROPERTY(VisibleAnywhere, meta = (DisplayName = "Lag Compensation Component"))
	ULagCompensationComponent* m_LagCompensationComponent;

	UPROPERTY(EditAnywhere, Category = Combat, meta = (DisplayName = "Fire Weapon Montage"))
	UAnimMontage* m_FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat, meta = (DisplayName = "Reload Montage"))
	UAnimMontage* m_ReloadMontage;

	UPROPERTY(EditAnywhere, Category = Combat, meta = (DisplayName = "Hit Reaction Montage"))
	UAnimMontage* m_HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat, meta = (DisplayName = "Elimination Montage"))
	UAnimMontage* m_ElimMontage;

	ANukePlayerController* m_NukePlayerController = nullptr;

public:
	//Hit boxes for server-side rewind
	UPROPERTY(EditAnywhere, meta = (DisplayName = "Head"))
	UBoxComponent* m_HeadBox;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Spine_02"))
	UBoxComponent* m_Spine02;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Spine_03"))
	UBoxComponent* m_Spine03;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Pelvis"))
	UBoxComponent* m_Pelvis;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Upperarm_l"))
	UBoxComponent* m_UpperarmL;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Upperarm_r"))
	UBoxComponent* m_UpperarmR;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Lowerarm_l"))
	UBoxComponent* m_LowerarmL;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Lowerarm_r"))
	UBoxComponent* m_LowerarmR;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Hand_l"))
	UBoxComponent* m_HandL;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Hand_r"))
	UBoxComponent* m_HandR;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Backpack"))
	UBoxComponent* m_Backpack;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Blanket"))
	UBoxComponent* m_Blanket;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Thigh_l"))
	UBoxComponent* m_ThighL;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Thigh_r"))
	UBoxComponent* m_ThighR;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Calf_l"))
	UBoxComponent* m_CalfL;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Calf_r"))
	UBoxComponent* m_CalfR;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Foot_l"))
	UBoxComponent* m_FootL;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Foot_r"))
	UBoxComponent* m_FootR;

	UPROPERTY()
	TMap<FName, UBoxComponent*> m_HitCollisionBoxes;

	bool IsWeaponEquipped() const;
	bool IsAiming() const;
	void SetOverlappingWeapon(AWeapon* weapon);
	void PlayFireMontage();
	void PlayReloadMontage();
	void PlayHitReactMontage();
	void PlayElimMontage();

	UFUNCTION()
	void ReceiveDamage(AActor* damagedActor, float damage, const UDamageType* damageType, AController* instigatorController, AActor* damageCauser);

	void Eliminate();

	UFUNCTION(NetMulticast, Reliable)
	void MultiCastEliminate();

	FORCEINLINE UCameraComponent* GetFollowCamera() { return m_FollowCamera; }
	FORCEINLINE float GetAimOffsetYaw() const { return m_AimOffsetYaw; }
	FORCEINLINE float GetAimOffsetPitch() const { return m_AimOffsetPitch; }
	FORCEINLINE float GetHealth() const { return m_Health; }
	FORCEINLINE float GetMaxHealth() const { return m_MaxHealth; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return m_TurningInPlace; }
	FORCEINLINE bool IsEliminated() const { return m_IsEliminated; }
	FORCEINLINE UCombatComponent* GetCombatComponent() const { return m_CombatComponent; }
	FORCEINLINE ULagCompensationComponent* GetLagCompensationComponent() const { return m_LagCompensationComponent; }
	ECombatState GetCombatState() const;
	AWeapon* GetEquippedWeapon() const;

	UPROPERTY(Replicated)
	bool m_DisableGameplay = false;

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool showScope);

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

	UFUNCTION()
	void OnRep_Health();

	//Elimination
	bool m_IsEliminated = false;

	FTimerHandle m_ElimTimer;

	UPROPERTY(EditDefaultsOnly)
	float m_ElimDelay = 3.f;

	void ElimTimerFinished();

	//Dissolve effect
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* m_DissolveTimelineComponent;

	FOnTimelineFloat m_DissolveTrack;

	UFUNCTION()
	void UpdateDissolveMaterial(float dissolveValue);
	void StartDissolve();

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Dissolve Curve"))
	UCurveFloat* m_DissolveCurve;

	UPROPERTY(VisibleAnywhere, Category = Elimination, meta = (DisplayName = "Dynamic Dissolve Material Instance"))
	UMaterialInstanceDynamic* m_DynamicDissolveMaterialInstance;

	UPROPERTY(EditAnywhere, Category = Elimination, meta = (DisplayName = "Dissolve Material Instance"))
	UMaterialInstance* m_DissolveMaterialInstance;

	//ElimBot effect
	UPROPERTY(EditAnywhere, Category = Elimination, meta = (DisplayName = "Elimination Bot Effect"))
	UParticleSystem* m_ElimBotEffect;

	UPROPERTY(VisibleAnywhere, Category = Elimination, meta = (DisplayName = "Elimination Bot Component"))
	UParticleSystemComponent* m_ElimBotComponent;

	UPROPERTY(EditAnywhere, Category = Elimination, meta = (DisplayName = "Elimination Bot SoundCue"))
	USoundCue* m_ElimBotSound;
};
