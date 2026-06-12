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
#include "Sound/SoundCue.h"

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
	
	if (m_EquippedWeapon->m_EquipSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, m_EquippedWeapon->m_EquipSound, m_EquippedWeapon->GetActorLocation());
	}

	if (m_CarriedAmmoMap.Contains(m_EquippedWeapon->GetWeaponType()))
	{
		m_CarriedAmmo = m_CarriedAmmoMap[m_EquippedWeapon->GetWeaponType()];
	}

	m_PlayerController = m_PlayerController == nullptr ? Cast<ANukePlayerController>(m_Character->Controller) : m_PlayerController;
	if(m_PlayerController != nullptr)
	{
		m_PlayerController->SetHUDCarriedAmmo(m_CarriedAmmo);
	}

	m_Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	m_Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (m_EquippedWeapon == nullptr)
	{
		// No weapon equipped � hide ammo on client
		if (m_PlayerController == nullptr && m_Character != nullptr)
		{
			m_PlayerController = Cast<ANukePlayerController>(m_Character->GetController());
		}
		if (m_PlayerController != nullptr)
		{
			m_PlayerController->HideHUDAmmo();
		}
		return;
	}

	if (m_EquippedWeapon->m_EquipSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, m_EquippedWeapon->m_EquipSound, m_EquippedWeapon->GetActorLocation());
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

	m_PlayerController = m_PlayerController == nullptr ? Cast<ANukePlayerController>(m_Character->Controller) : m_PlayerController;
	if (m_PlayerController != nullptr)
	{
		m_PlayerController->SetHUDCarriedAmmo(m_CarriedAmmo);
	}

	if (m_EquippedWeapon->IsEmpty())
	{
		Reload();
	}
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

	if (m_Character->HasAuthority())
	{
		InitCarriedAmmo();
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
	DOREPLIFETIME_CONDITION(UCombatComponent, m_CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, m_CombatState);
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

	if (m_Character != nullptr && m_CombatState == ECombatState::ECS_Unoccupied)
	{
		m_Character->PlayFireMontage();
		m_EquippedWeapon->Fire(traceHitTarget);
	}
}

void UCombatComponent::Reload()
{
	if (m_CarriedAmmo > 0 && m_CombatState != ECombatState::ECS_Reloading)
	{
		ServerReload();
	}
}

void UCombatComponent::HandleReload()
{
	if (m_Character == nullptr)
	{
		return;
	}

	m_Character->PlayReloadMontage();
}

uint32 UCombatComponent::GetAmountToReload()
{
	if (m_EquippedWeapon == nullptr)
	{
		return 0;
	}

	checkf(m_EquippedWeapon->GetAmmoCount() <= m_EquippedWeapon->GetMagCapacity(), TEXT("Ammo greater than Mag Capacity"));
	uint32 roomInMag = m_EquippedWeapon->GetMagCapacity() - m_EquippedWeapon->GetAmmoCount();

	if (m_CarriedAmmoMap.Contains(m_EquippedWeapon->GetWeaponType()))
	{
		uint32 carriedAmmo = m_CarriedAmmoMap[m_EquippedWeapon->GetWeaponType()];
		return FMath::Min(roomInMag, carriedAmmo);
	}

	return 0;
}

void UCombatComponent::FinishReload()
{
	if (m_Character != nullptr && m_Character->HasAuthority())
	{
		m_CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}

	if (m_IsFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::ServerReload_Implementation()
{
	m_CombatState = ECombatState::ECS_Reloading;
	HandleReload();
}

void UCombatComponent::OnRep_CombatState()
{
	switch (m_CombatState)
	{
	case ECombatState::ECS_Unoccupied:
		if (m_IsFireButtonPressed)
		{
			Fire();
		}
		break;
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;
	default:
		break;
	}
}

void UCombatComponent::UpdateAmmoValues()
{
	if (m_EquippedWeapon == nullptr)
	{
		return;
	}

	uint32 reloadAmount = GetAmountToReload();
	if (m_CarriedAmmoMap.Contains(m_EquippedWeapon->GetWeaponType()))
	{
		m_CarriedAmmoMap[m_EquippedWeapon->GetWeaponType()] -= reloadAmount;
		m_CarriedAmmo = m_CarriedAmmoMap[m_EquippedWeapon->GetWeaponType()];
	}

	m_PlayerController = m_PlayerController == nullptr ? Cast<ANukePlayerController>(m_Character->Controller) : m_PlayerController;
	if (m_PlayerController != nullptr)
	{
		m_PlayerController->SetHUDCarriedAmmo(m_CarriedAmmo);
	}

	m_EquippedWeapon->AddAmmo(reloadAmount);
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

	m_PlayerController = m_PlayerController == nullptr ? Cast<ANukePlayerController>(m_Character->Controller) : m_PlayerController;
	m_HUD = m_HUD == nullptr ? Cast<ANukeHUD>(m_PlayerController->GetHUD()) : m_HUD;

	if (m_HUD == nullptr)
	{
		return;
	}

	if (m_EquippedWeapon == nullptr)
	{
		m_HudPackage = FHUDPackage();
		m_HUD->SetHUDPackage(m_HudPackage);
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
	if (m_Character == nullptr || m_EquippedWeapon == nullptr)
	{
		return;
	}

	m_IsAiming = isAiming;
	ServerSetAiming(m_IsAiming);

	m_Character->GetCharacterMovement()->MaxWalkSpeed = isAiming ? m_AimWalkSpeed : m_BaseWalkSpeed;

	if (m_Character->IsLocallyControlled() && m_EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		m_Character->ShowSniperScopeWidget(isAiming);
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

bool UCombatComponent::CanFire()
{
	if (m_EquippedWeapon == nullptr)
	{
		return false;
	}

	return m_CanFire && 
		(m_EquippedWeapon->GetAmmoCount() > 0) &&
		(m_CombatState == ECombatState::ECS_Unoccupied);
}

void UCombatComponent::Fire()
{
	if (CanFire())
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

	if (m_EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

void UCombatComponent::InitCarriedAmmo()
{
	m_CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, m_StartingARAmmo); 
	m_CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, m_StartingRocketAmmo); 
	m_CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, m_StartingPistolAmmo);
	m_CarriedAmmoMap.Emplace(EWeaponType::EWT_SubMachineGun, m_StartingSMGAmmo);
	m_CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, m_StartingShotgunAmmo);
	m_CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, m_StartingSniperAmmo);
	m_CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, m_StartingGrenadeAmmo);
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	m_PlayerController = m_PlayerController == nullptr ? Cast<ANukePlayerController>(m_Character->Controller) : m_PlayerController;
	if (m_PlayerController != nullptr)
	{
		m_PlayerController->SetHUDCarriedAmmo(m_CarriedAmmo);
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

