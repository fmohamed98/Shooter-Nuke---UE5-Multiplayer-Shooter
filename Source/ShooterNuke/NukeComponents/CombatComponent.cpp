// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "ShooterNuke/Weapon/Weapon.h"
#include "ShooterNuke/Character/NukeCharacter.h"
#include "ShooterNuke/PlayerController/NukePlayerController.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h" 
#include "Camera/CameraComponent.h"

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

	if (m_EquippedWeapon != nullptr)
	{
		m_EquippedWeapon->Drop();
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
	m_EquippedWeapon->SetHUDWeaponAmmo();
	m_Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	m_Character->bUseControllerRotationYaw = true;
}

// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (m_Character == nullptr)
	{
		return;
	}
	m_Character->GetCharacterMovement()->MaxWalkSpeed = m_BaseWalkSpeed;
	
	if (UCameraComponent* followCamera = m_Character->GetFollowCamera())
	{
		m_DefaultFOV = followCamera->FieldOfView;
		m_CurrentFOV = m_DefaultFOV;
	}
}


// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (m_Character != nullptr && m_Character->IsLocallyControlled())
	{
		FHitResult hitResult;
		TraceUnderCrossHairs(hitResult);
		m_HitTarget = hitResult.ImpactPoint;

		SetHUDCrossHairs(DeltaTime);
		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, m_EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, m_IsAiming);
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (m_EquippedWeapon == nullptr || m_Character == nullptr)
	{
		return;
	}

	m_EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	USkeletalMeshComponent* nukeMesh = m_Character->GetMesh();
	if (nukeMesh == nullptr)
	{
		return;
	}

	if (const USkeletalMeshSocket* gunSocket = nukeMesh->GetSocketByName(FName("GunSocket")))
	{
		gunSocket->AttachActor(m_EquippedWeapon, nukeMesh);
	}
	m_Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	m_Character->bUseControllerRotationYaw = true;
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
	if (m_Character == nullptr)
	{
		return;
	}

	FVector2D viewPortSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(viewPortSize);
	}

	FVector2D crossHairLocation(viewPortSize.X / 2.f, viewPortSize.Y / 2.f);
	FVector crossHairWorldPosition;
	FVector crossHairWorldDirection;

	bool isScreentoWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this,0),
		crossHairLocation,
		crossHairWorldPosition,
		crossHairWorldDirection);

	if (isScreentoWorld)
	{
		FVector start = crossHairWorldPosition;
		float distanceToCharacter = (m_Character->GetActorLocation() - start).Size();
		start += crossHairWorldDirection * (distanceToCharacter + 100.f);

		FVector end = start + crossHairWorldDirection * TRACE_LENGTH;
		GetWorld()->LineTraceSingleByChannel(traceHitResult, start, end, ECollisionChannel::ECC_Visibility);
	}

	if (traceHitResult.GetActor() != nullptr && traceHitResult.GetActor()->Implements<UInteractWithCrosshairInterface>())
	{
		m_HudPackage.m_CrossHairColour = FLinearColor::Red;
	}
	else
	{
		m_HudPackage.m_CrossHairColour = FLinearColor::White;
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

	m_HudPackage.m_CrosshairBottom = m_EquippedWeapon->m_CrosshairBottom;
	m_HudPackage.m_CrosshairCenter = m_EquippedWeapon->m_CrosshairCenter;
	m_HudPackage.m_CrosshairLeft = m_EquippedWeapon->m_CrosshairLeft;
	m_HudPackage.m_CrosshairRight = m_EquippedWeapon->m_CrosshairRight;
	m_HudPackage.m_CrosshairTop = m_EquippedWeapon->m_CrosshairTop;

	FVector2d walkSpeedRange(0.f, m_Character->GetCharacterMovement()->MaxWalkSpeed);
	FVector2D velocityMultiplierRange(0.f, 1.f);

	FVector velocity = m_Character->GetVelocity();
	velocity.Z = 0.f;
	
	m_CrossHairVelocityFactor = FMath::GetMappedRangeValueClamped(walkSpeedRange, velocityMultiplierRange, velocity.Size());

	if (m_Character->GetCharacterMovement()->IsFalling())
	{
		m_CrossHairAirFactor = FMath::FInterpTo(m_CrossHairAirFactor, 2.25f, deltaTime, 2.25f);
	}
	else
	{
		m_CrossHairAirFactor = FMath::FInterpTo(m_CrossHairAirFactor, 0.f, deltaTime, 30.f);
	}

	if (m_IsAiming)
	{
		m_CrossHairAimFactor = FMath::FInterpTo(m_CrossHairAimFactor, 0.58f, deltaTime, 30.f);
	}
	else
	{
		m_CrossHairAimFactor = FMath::FInterpTo(m_CrossHairAimFactor, 0.f, deltaTime, 30.f);
	}

	m_CrossHairShootFactor = FMath::FInterpTo(m_CrossHairShootFactor, 0.f, deltaTime, 40.f);
	
	m_HudPackage.m_CrossHairSpread = 0.5f +
									m_CrossHairVelocityFactor +
									m_CrossHairAirFactor - 
									m_CrossHairAimFactor +
									m_CrossHairShootFactor;

	m_HUD->SetHUDPackage(m_HudPackage);
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
	if (m_EquippedWeapon == nullptr)
	{
		return;
	}

	m_IsFireButtonPressed = isPressed;

	if (m_IsFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::Fire()
{
	if (m_CanFire)
	{
		m_CanFire = false;
        ServerFire(m_HitTarget);
        m_CrossHairShootFactor = .75f;

        StartFireTimer();
	}
}

void UCombatComponent::FireTimerFinished()
{
	if (m_EquippedWeapon == nullptr)
	{
		return;
	}

	m_CanFire = true;
	if (m_IsFireButtonPressed && m_EquippedWeapon->m_IsAutomatic)
	{
		Fire();
	}
}

void UCombatComponent::StartFireTimer()
{
	if (m_EquippedWeapon == nullptr || m_Character == nullptr)
	{
		return;
	}

	m_Character->GetWorldTimerManager().SetTimer(m_FireTimer, this, &UCombatComponent::FireTimerFinished, m_EquippedWeapon->m_FireDelay);
}

void UCombatComponent::InterpFOV(const float deltaTime)
{
	if (m_EquippedWeapon == nullptr || m_Character == nullptr)
	{
		return;
	}

	UCameraComponent* followCamera = m_Character->GetFollowCamera();
	if (followCamera == nullptr)
	{
		return;
	}

	const float targetFOV = m_IsAiming? m_EquippedWeapon->GetZoomedFOV() : m_DefaultFOV;
	const float interpSpeed = m_IsAiming? m_EquippedWeapon->GetZoomInterpSpeed() : m_UnZoomInterpSpeed;

	m_CurrentFOV = FMath::FInterpTo(m_CurrentFOV, targetFOV, deltaTime, interpSpeed);

	followCamera->SetFieldOfView(m_CurrentFOV);
}

