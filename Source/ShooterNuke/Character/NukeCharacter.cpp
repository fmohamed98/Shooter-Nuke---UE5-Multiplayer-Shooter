// Fill out your copyright notice in the Description page of Project Settings.


#include "NukeCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "ShooterNuke/Weapon/Weapon.h"
#include "ShooterNuke/NukeComponents/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Animation/AnimInstance.h"
#include "ShooterNuke/ShooterNuke.h"
#include "ShooterNuke/PlayerController/NukePlayerController.h"
#include "ShooterNuke/GameMode/NukeGameMode.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
ANukeCharacter::ANukeCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	m_CameraBoom->SetupAttachment(GetMesh());
	m_CameraBoom->TargetArmLength = 600.f;
	m_CameraBoom->bUsePawnControlRotation = true;

	m_FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	m_FollowCamera->SetupAttachment(m_CameraBoom, USpringArmComponent::SocketName);
	m_FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	m_CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	m_CombatComponent->SetIsReplicated(true);

	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	m_DissolveTimelineComponent = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));
}

// Called when the game starts or when spawned
void ANukeCharacter::BeginPlay()
{
	Super::BeginPlay();

	UpdateHUDHealth();

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ANukeCharacter::ReceiveDamage);
	}
}

// Called every frame
void ANukeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimOffset(DeltaTime);

	HideCameraIfCharacterClose();
}

// Called to bind functionality to input
void ANukeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ANukeCharacter::Jump);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ANukeCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ANukeCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ANukeCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ANukeCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ANukeCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ANukeCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ANukeCharacter::ReloadButtonPressed);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ANukeCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ANukeCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &ANukeCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ANukeCharacter::LookUp);
}

void ANukeCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ANukeCharacter, m_OverlappingWeapon,COND_OwnerOnly);
	DOREPLIFETIME(ANukeCharacter, m_Health);
	DOREPLIFETIME(ANukeCharacter, m_DisableGameplay);
}

void ANukeCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (m_CombatComponent != nullptr)
	{
		m_CombatComponent->m_Character = this;
	}
}

void ANukeCharacter::Destroyed()
{
	Super::Destroyed();

	if (m_ElimBotComponent != nullptr)
	{
		m_ElimBotComponent->DestroyComponent();
	}

	if (m_CombatComponent != nullptr && m_CombatComponent->m_EquippedWeapon != nullptr)
	{
		m_CombatComponent->m_EquippedWeapon->Destroy();
	}
}

void ANukeCharacter::OnRep_OverlappingWeapon(AWeapon* lastWeapon)
{
	if (m_OverlappingWeapon != nullptr)
	{
		m_OverlappingWeapon->ShowPickupWidget(true);
	}

	else if (lastWeapon != nullptr) //m_OverlappingWeapon has been set to nullptr
	{
		lastWeapon->ShowPickupWidget(false);
	}
}

void ANukeCharacter::ServerEquipButtonPressed_Implementation()
{
	if (m_CombatComponent != nullptr)
	{
		m_CombatComponent->EquipWeapon(m_OverlappingWeapon);
	}
}

bool ANukeCharacter::IsWeaponEquipped() const
{
	return (m_CombatComponent != nullptr) && (m_CombatComponent->m_EquippedWeapon != nullptr);
}

bool ANukeCharacter::IsAiming() const
{
	return (m_CombatComponent != nullptr) && (m_CombatComponent->m_IsAiming);
}

void ANukeCharacter::SetOverlappingWeapon(AWeapon* weapon)
{
	if (m_OverlappingWeapon != nullptr && IsLocallyControlled())
	{
		m_OverlappingWeapon->ShowPickupWidget(false);
	}

	m_OverlappingWeapon = weapon;
	if (m_OverlappingWeapon != nullptr && IsLocallyControlled())
	{
		m_OverlappingWeapon->ShowPickupWidget(true);
	}
}

void ANukeCharacter::PlayFireMontage()
{
	if (m_CombatComponent == nullptr || m_CombatComponent->m_EquippedWeapon == nullptr)
	{
		return;
	}

	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance != nullptr && m_FireWeaponMontage != nullptr)
	{
		animInstance->Montage_Play(m_FireWeaponMontage);
		FName sectionName;
		sectionName = IsAiming() ? FName("RifleIronSights") : FName("RifleHip");
		animInstance->Montage_JumpToSection(sectionName);
	}
}

void ANukeCharacter::PlayReloadMontage()
{
	if (m_CombatComponent == nullptr || m_CombatComponent->m_EquippedWeapon == nullptr)
	{
		return;
	}

	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance != nullptr && m_ReloadMontage != nullptr)
	{
		animInstance->Montage_Play(m_ReloadMontage);
		FName sectionName;

		switch (m_CombatComponent->m_EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_RocketLauncher:
		case EWeaponType::EWT_AssaultRifle:
		case EWeaponType::EWT_Pistol:
		case EWeaponType::EWT_SubMachineGun:
		case EWeaponType::EWT_Shotgun:
		case EWeaponType::EWT_SniperRifle:
			sectionName = FName("Rifle");
			break;
		}
		animInstance->Montage_JumpToSection(sectionName);
	}
}

void ANukeCharacter::PlayHitReactMontage()
{
	if (m_CombatComponent == nullptr || m_CombatComponent->m_EquippedWeapon == nullptr)
	{
		return;
	}

	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance != nullptr && m_HitReactMontage != nullptr)
	{
		animInstance->Montage_Play(m_HitReactMontage);
		FName sectionName("FromFront");
		animInstance->Montage_JumpToSection(sectionName);
	}
}

void ANukeCharacter::PlayElimMontage()
{
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance != nullptr && m_ElimMontage != nullptr)
	{
		animInstance->Montage_Play(m_ElimMontage);
	}
}

void ANukeCharacter::ReceiveDamage(AActor* damagedActor, float damage, const UDamageType* damageType, AController* instigatorController, AActor* damageCauser)
{
	m_Health = FMath::Clamp(m_Health - damage, 0.f, m_MaxHealth);
	
	UpdateHUDHealth();
	PlayHitReactMontage();

	ANukeGameMode* nukeGameMode = Cast<ANukeGameMode>(GetWorld()->GetAuthGameMode());
	if (m_Health == 0.f && nukeGameMode != nullptr)
	{
		ANukePlayerController* attackerController = Cast<ANukePlayerController>(instigatorController);
		nukeGameMode->PlayerEliminated(this, attackerController);
	}
}

void ANukeCharacter::Eliminate()
{
	if (m_CombatComponent != nullptr && m_CombatComponent->m_EquippedWeapon != nullptr)
	{
		m_CombatComponent->m_EquippedWeapon->Drop();
		m_CombatComponent->m_EquippedWeapon = nullptr;  // triggers OnRep_EquippedWeapon → hides ammo on client
	}

	m_NukePlayerController = m_NukePlayerController == nullptr ? Cast<ANukePlayerController>(Controller) : m_NukePlayerController;
	if (m_NukePlayerController != nullptr)
	{
		m_NukePlayerController->HideHUDAmmo();
	}

	MultiCastEliminate();

	GetWorldTimerManager().SetTimer(m_ElimTimer,this, &ANukeCharacter::ElimTimerFinished, m_ElimDelay);
}

void ANukeCharacter::MultiCastEliminate_Implementation()
{
	m_IsEliminated = true;
	PlayElimMontage();

	if (m_DissolveMaterialInstance != nullptr)
	{
		m_DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(m_DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, m_DynamicDissolveMaterialInstance);

		m_DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
	}
	StartDissolve();

	GetCharacterMovement()->DisableMovement(); //Movement
	GetCharacterMovement()->StopMovementImmediately(); //Rotation
	m_DisableGameplay = true;


	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (m_ElimBotEffect != nullptr)
	{
		FVector elimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		m_ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), m_ElimBotEffect, elimBotSpawnPoint, GetActorRotation());
	}

	if (m_ElimBotSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, m_ElimBotSound, GetActorLocation(), GetActorRotation());
	}

	bool hideSniperScope = IsLocallyControlled() && 
	m_CombatComponent && 
	m_CombatComponent->m_EquippedWeapon &&
	m_CombatComponent->m_EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	if (hideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}
}

void ANukeCharacter::ElimTimerFinished()
{
	ANukeGameMode* nukeGameMode = Cast<ANukeGameMode>(GetWorld()->GetAuthGameMode());
	if (nukeGameMode != nullptr)
	{
		nukeGameMode->RequestRespawn(this);
	}
}

void ANukeCharacter::UpdateDissolveMaterial(float dissolveValue)
{
	if (m_DynamicDissolveMaterialInstance != nullptr)
	{
		m_DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), dissolveValue);
	}
}

void ANukeCharacter::StartDissolve()
{
	m_DissolveTrack.BindDynamic(this, &ANukeCharacter::UpdateDissolveMaterial);

	if (m_DissolveTimelineComponent != nullptr)
	{
		m_DissolveTimelineComponent->AddInterpFloat(m_DissolveCurve, m_DissolveTrack);
		m_DissolveTimelineComponent->Play();
	}
}

void ANukeCharacter::OnRep_Health()
{
	UpdateHUDHealth();
	PlayHitReactMontage();
}

void ANukeCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled() || m_FollowCamera == nullptr)
	{
		return;
	}

	const bool hideCharacter = (m_FollowCamera->GetComponentLocation() - GetActorLocation()).Size() <  m_CameraThreshold;

	GetMesh()->SetVisibility(!hideCharacter);

	if (m_CombatComponent != nullptr && m_CombatComponent->m_EquippedWeapon != nullptr)
	{
		if (USkeletalMeshComponent* weaponMesh = m_CombatComponent->m_EquippedWeapon->GetWeaponMesh())
		{
			weaponMesh->bOwnerNoSee = hideCharacter;
		}
	}
}

void ANukeCharacter::AimOffset(const float deltaTime)
{
	if (m_DisableGameplay)
	{
		bUseControllerRotationYaw = false;
		m_TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	if (m_CombatComponent == nullptr || m_CombatComponent->m_EquippedWeapon == nullptr)
	{
		return;
	}

	FVector velocity = GetVelocity();
	velocity.Z = 0.f;
	float speed = velocity.Size();

	bool isInAir = GetCharacterMovement()->IsFalling();

	if (speed == 0.f && !isInAir)
	{
		FRotator currentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator deltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(currentAimRotation, m_StartingAimRotation);

		m_AimOffsetYaw = deltaAimRotation.Yaw;
		TurnInPlace(deltaTime);
	}
	else
	{
		m_StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		m_AimOffsetYaw = 0.f;
		m_TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	m_AimOffsetPitch = GetBaseAimRotation().Pitch;
	if (m_AimOffsetPitch > 90.f && !IsLocallyControlled())
	{
		FVector2D inRange(270.f, 360.f);
		FVector2D outRange(-90.f, 0.f);

		m_AimOffsetPitch = FMath::GetMappedRangeValueClamped(inRange, outRange, m_AimOffsetPitch);
	}
}

void ANukeCharacter::UpdateHUDHealth()
{
	m_NukePlayerController = m_NukePlayerController == nullptr ? Cast<ANukePlayerController>(Controller) : m_NukePlayerController;
	if (m_NukePlayerController != nullptr)
	{
		m_NukePlayerController->SetHUDHealth(m_Health, m_MaxHealth);
	}
}

ECombatState ANukeCharacter::GetCombatState() const
{
	if (m_CombatComponent == nullptr)
	{
		return ECombatState::ECS_MAX;
	}

	return m_CombatComponent->m_CombatState;
}

void ANukeCharacter::TurnInPlace(float deltaTime)
{
	if (m_AimOffsetYaw > 90.f)
	{
		m_TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (m_AimOffsetYaw < -90.f)
	{
		m_TurningInPlace = ETurningInPlace::ETIP_Left;
	}

	if (m_TurningInPlace == ETurningInPlace::ETIP_NotTurning)
	{
		m_InterpAimOffsetYaw = m_AimOffsetYaw;
	}
	else
	{
		m_InterpAimOffsetYaw = FMath::FInterpTo(m_InterpAimOffsetYaw, 0.f, deltaTime, 4.f);
		m_AimOffsetYaw = m_InterpAimOffsetYaw;
		if (FMath::Abs(m_AimOffsetYaw) < 15.f)
		{
			m_TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			m_StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void ANukeCharacter::Jump()
{
	if (m_DisableGameplay)
	{
		return;
	}

	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void ANukeCharacter::MoveForward(const float value)
{
	if (m_DisableGameplay)
	{
		return;
	}

	if (Controller != nullptr && value != 0.f)
	{
		const FRotator yawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector direction(FRotationMatrix(yawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(direction, value);
	}
}

void ANukeCharacter::MoveRight(const float value)
{
	if (m_DisableGameplay)
	{
		return;
	}

	if (Controller != nullptr && value != 0.f)
	{
		const FRotator yawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector direction(FRotationMatrix(yawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(direction, value);
	}
}

void ANukeCharacter::Turn(const float value)
{
	AddControllerYawInput(value);
}

void ANukeCharacter::LookUp(const float value)
{
	AddControllerPitchInput(value);
}

void ANukeCharacter::EquipButtonPressed()
{
	if (m_DisableGameplay)
	{
		return;
	}

	if (m_CombatComponent == nullptr)
	{
		return;
	}

	if (HasAuthority())
	{
		m_CombatComponent->EquipWeapon(m_OverlappingWeapon);
	}
	else
	{
		ServerEquipButtonPressed();
	}
}

void ANukeCharacter::CrouchButtonPressed()
{
	if (m_DisableGameplay)
	{
		return;
	}

	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ANukeCharacter::AimButtonPressed()
{
	if (m_DisableGameplay)
	{
		return;
	}

	if (m_CombatComponent != nullptr)
	{
		m_CombatComponent->SetAiming(true);
	}
}

void ANukeCharacter::AimButtonReleased()
{
	if (m_DisableGameplay)
	{
		return;
	}

	if (m_CombatComponent != nullptr)
	{
		m_CombatComponent->SetAiming(false);
	}
}

void ANukeCharacter::FireButtonPressed()
{
	if (m_DisableGameplay)
	{
		return;
	}

	if (m_CombatComponent != nullptr)
	{
		m_CombatComponent->FireButtonPressed(true);
	}
}

void ANukeCharacter::FireButtonReleased()
{
	if (m_DisableGameplay)
	{
		return;
	}

	if (m_CombatComponent != nullptr)
	{
		m_CombatComponent->FireButtonPressed(false);
	}
}

void ANukeCharacter::ReloadButtonPressed()
{
	if (m_DisableGameplay)
	{
		return;
	}

	if (m_CombatComponent != nullptr)
	{
		m_CombatComponent->Reload();
	}
}




