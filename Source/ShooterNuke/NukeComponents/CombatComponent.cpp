// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "ShooterNuke/Weapon/Weapon.h"
#include "ShooterNuke/Character/NukeCharacter.h"
#include "ShooterNuke/PlayerController/NukePlayerController.h"
#include "ShooterNuke/HUD/NukeHUD.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h" 

constexpr float TRACE_LENGTH = 80000.f;

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	m_BaseWalkSpeed = 600.0f;
	m_AimWalkSpeed = 450.0f;
}

void UCombatComponent::EquipWeapon(AWeapon* weapon)
{
	if (m_Character == nullptr || weapon == nullptr)
	{
		return;
	}

	USkeletalMeshComponent* nukeMesh = m_Character->GetMesh();
	if (nukeMesh == nullptr)
	{
		return;
	}

	m_EquippedWeapon = weapon;
	m_EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	if (const USkeletalMeshSocket* gunSocket = nukeMesh->GetSocketByName(FName("GunSocket")))
	{
		gunSocket->AttachActor(m_EquippedWeapon, nukeMesh);
	}

	m_EquippedWeapon->SetOwner(m_Character);

	m_Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	m_Character->bUseControllerRotationYaw = true;
}

// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (m_Character != nullptr)
	{
		m_Character->GetCharacterMovement()->MaxWalkSpeed = m_BaseWalkSpeed;
	}
}


// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SetHUDCrossHairs(DeltaTime);
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, m_EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, m_IsAiming);
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (m_EquippedWeapon != nullptr && m_Character != nullptr)
	{
		m_Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		m_Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::ServerSetAiming_Implementation(const bool isAiming)
{
	m_IsAiming = isAiming;
	if (m_Character != nullptr)
	{
		m_Character->GetCharacterMovement()->MaxWalkSpeed = isAiming ? m_AimWalkSpeed : m_BaseWalkSpeed;
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& traceHitTarget)
{
	MultiCastFire(traceHitTarget);
}

void UCombatComponent::MultiCastFire_Implementation(const FVector_NetQuantize& traceHitTarget)
{
	if (m_EquippedWeapon == nullptr)
	{
		return;
	}

	if (m_Character != nullptr)
	{
		m_Character->PlayFireMontage();
		m_EquippedWeapon->Fire(traceHitTarget);
	}
}

void UCombatComponent::TraceUnderCrossHairs(FHitResult& traceHitResult)
{
	FVector2D viewPortSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(viewPortSize);
	}

	FVector2D crossHairLocarion(viewPortSize.X / 2.f, viewPortSize.Y / 2.f);
	FVector crossHairWorldPosition;
	FVector crossHairWorldDirection;

	bool isScreentoWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this,0),
		crossHairLocarion,
		crossHairWorldPosition,
		crossHairWorldDirection);

	if (isScreentoWorld)
	{
		FVector start = crossHairWorldPosition;
		FVector end = start + crossHairWorldDirection * TRACE_LENGTH;
		GetWorld()->LineTraceSingleByChannel(traceHitResult, start, end, ECollisionChannel::ECC_Visibility);
	}
}

void UCombatComponent::SetHUDCrossHairs(const float deltaTime)
{
	if (m_Character == nullptr || m_Character->Controller == nullptr)
	{
		return;
	}

	if (m_EquippedWeapon == nullptr)
	{
		return;
	}

	m_PlayerController = m_PlayerController == nullptr ? Cast<ANukePlayerController>(m_Character->Controller) : m_PlayerController;
	m_HUD = m_HUD == nullptr ? Cast<ANukeHUD>(m_PlayerController->GetHUD()) : m_HUD;

	if (m_HUD == nullptr)
	{
		return;
	}

	FHUDPackage hudPackage;
	hudPackage.m_CrosshairBottom = m_EquippedWeapon->m_CrosshairBottom;
	hudPackage.m_CrosshairCenter = m_EquippedWeapon->m_CrosshairCenter;
	hudPackage.m_CrosshairLeft = m_EquippedWeapon->m_CrosshairLeft;
	hudPackage.m_CrosshairRight = m_EquippedWeapon->m_CrosshairRight;
	hudPackage.m_CrosshairTop = m_EquippedWeapon->m_CrosshairTop;

	FVector2d walkSpeedRange(0.f, m_Character->GetCharacterMovement()->MaxWalkSpeed);
	FVector2D velocityMultiplierRange(0.f, 1.f);

	FVector velocity = m_Character->GetVelocity();
	velocity.Z = 0.f;
	
	float crossHairVelocityFactor = FMath::GetMappedRangeValueClamped(walkSpeedRange, velocityMultiplierRange, velocity.Size());

	if (m_Character->GetCharacterMovement()->IsFalling())
	{
		m_CrossHairAirFactor = FMath::FInterpTo(m_CrossHairAirFactor, 2.25f, deltaTime, 2.25f);
	}
	else
	{
		m_CrossHairAirFactor = FMath::FInterpTo(m_CrossHairAirFactor, 0.f, deltaTime, 30.f);
	}

	hudPackage.m_CrossHairSpread = crossHairVelocityFactor + m_CrossHairAirFactor;

	m_HUD->SetHUDPackage(hudPackage);
}

void UCombatComponent::SetAiming(const bool isAiming)
{
	m_IsAiming = isAiming;
	ServerSetAiming(m_IsAiming);

	if (m_Character != nullptr)
	{
		m_Character->GetCharacterMovement()->MaxWalkSpeed = isAiming ? m_AimWalkSpeed : m_BaseWalkSpeed;
	}
}

void UCombatComponent::FireButtonPressed(const bool isPressed)
{
	m_IsFireButtonPressed = isPressed;

	if (m_IsFireButtonPressed)
	{
		FHitResult hitResult;
		TraceUnderCrossHairs(hitResult);
		ServerFire(hitResult.ImpactPoint);
	}
}

