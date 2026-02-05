// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "ShooterNuke/Weapon/Weapon.h"
#include "ShooterNuke/Character/NukeCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
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
}

// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, m_EquippedWeapon);
}

