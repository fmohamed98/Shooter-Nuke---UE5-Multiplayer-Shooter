// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "ShooterNuke/Character/NukeCharacter.h"
#include "ShooterNuke/PlayerController/NukePlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Engine/SkeletalMeshSocket.h"
#include "BulletShell.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	m_WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(m_WeaponMesh);

	m_WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	m_WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	m_WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	m_AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	m_AreaSphere->SetupAttachment(RootComponent);
	m_AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	m_AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	m_PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	m_PickupWidget->SetupAttachment(RootComponent);
	m_PickupWidget->SetVisibility(false);
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		m_AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		m_AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		m_AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
		m_AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	}

	m_Ammo = m_MagCapacity;
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComponent, int32 otherBodyIndex, bool isFromSweep, const FHitResult& sweepResult)
{
	if (ANukeCharacter* nukeCharacter = Cast<ANukeCharacter>(otherActor))
	{
		nukeCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComponent, int32 otherBodyIndex)
{
	if (ANukeCharacter* nukeCharacter = Cast<ANukeCharacter>(otherActor))
	{
		nukeCharacter->SetOverlappingWeapon(nullptr);
	}
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, m_WeaponState);
	DOREPLIFETIME(AWeapon, m_Ammo);
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();

	if (Owner == nullptr)
	{
		m_OwningCharacterController = nullptr;
		m_OwningCharacter = nullptr;
	}
	else
	{
		SetHUDWeaponAmmo();
	}
}

void AWeapon::ShowPickupWidget(const bool showWidget)
{
	if (m_PickupWidget != nullptr)
	{
		m_PickupWidget->SetVisibility(showWidget);
	}
}

void AWeapon::Fire(const FVector& hitTarget)
{
	if (m_FireAnimation != nullptr)
	{
		m_WeaponMesh->PlayAnimation(m_FireAnimation, false);
	}

	const USkeletalMeshSocket* ammoEjectSocket = m_WeaponMesh->GetSocketByName(FName("AmmoEject"));
	if (ammoEjectSocket == nullptr)
	{
		return;
	}

	FTransform socketTransform = ammoEjectSocket->GetSocketTransform(GetWeaponMesh());

	if (UWorld* world = GetWorld())
	{
		world->SpawnActor<ABulletShell>(m_BulletShellClass, socketTransform.GetLocation(), socketTransform.GetRotation().Rotator());
	}

	SpendRound();
}

void AWeapon::OnRep_WeaponState()
{
	switch (m_WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		m_WeaponMesh->SetSimulatePhysics(false);
		m_WeaponMesh->SetEnableGravity(false);
		m_WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		m_WeaponMesh->SetSimulatePhysics(true);
		m_WeaponMesh->SetEnableGravity(true);
		m_WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	}
}

void AWeapon::OnRep_Ammo()
{
	SetHUDWeaponAmmo();
}

void AWeapon::SpendRound()
{
	if (m_Ammo == 0)
	{
		return;
	}

	m_Ammo--;
	SetHUDWeaponAmmo();
}

void AWeapon::SetHUDWeaponAmmo()
{
	m_OwningCharacter = m_OwningCharacter == nullptr ? Cast<ANukeCharacter>(GetOwner()) : m_OwningCharacter;
	if (m_OwningCharacter == nullptr)
	{
		return;
	}

	m_OwningCharacterController = m_OwningCharacterController == nullptr ? Cast<ANukePlayerController>(m_OwningCharacter->GetController()) : m_OwningCharacterController;
	if (m_OwningCharacterController == nullptr)
	{
		return;
	}

	m_OwningCharacterController->SetHUDWeaponAmmo(m_Ammo);
}

void AWeapon::SetWeaponState(EWeaponState weaponState)
{
	if (m_AreaSphere == nullptr || m_WeaponMesh == nullptr)
	{
		return;
	}

	m_WeaponState = weaponState;
	switch (m_WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		m_AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		m_WeaponMesh->SetSimulatePhysics(false);
		m_WeaponMesh->SetEnableGravity(false);
		m_WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		if (HasAuthority())
		{
			m_AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		m_WeaponMesh->SetSimulatePhysics(true);
		m_WeaponMesh->SetEnableGravity(true);
		m_WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	}
}

void AWeapon::Drop()
{
	if (m_WeaponMesh == nullptr)
	{
		return;
	}

	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules detachRules(EDetachmentRule::KeepWorld, true);
	m_WeaponMesh->DetachFromComponent(detachRules);

	SetOwner(nullptr);
	m_OwningCharacterController = nullptr;
	m_OwningCharacter = nullptr;
}

void AWeapon::AddAmmo(uint32 ammoToAdd)
{
	m_Ammo = FMath::Clamp(m_Ammo + ammoToAdd, 0, m_MagCapacity);
	SetHUDWeaponAmmo();
}
