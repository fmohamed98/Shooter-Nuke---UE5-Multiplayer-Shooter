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

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

// Called when the game starts or when spawned
void ANukeCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ANukeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimOffset(DeltaTime);
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

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ANukeCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ANukeCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &ANukeCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ANukeCharacter::LookUp);
}

void ANukeCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ANukeCharacter, m_OverlappingWeapon,COND_OwnerOnly);
}

void ANukeCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (m_CombatComponent != nullptr)
	{
		m_CombatComponent->m_Character = this;
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

void ANukeCharacter::AimOffset(const float deltaTime)
{
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
	if (Controller != nullptr && value != 0.f)
	{
		const FRotator yawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector direction(FRotationMatrix(yawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(direction, value);
	}
}

void ANukeCharacter::MoveRight(const float value)
{
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
	if (m_CombatComponent != nullptr)
	{
		m_CombatComponent->SetAiming(true);
	}
}

void ANukeCharacter::AimButtonReleased()
{
	if (m_CombatComponent != nullptr)
	{
		m_CombatComponent->SetAiming(false);
	}
}




