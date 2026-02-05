// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "ShooterNuke/Character/NukeCharacter.h"
#include "Net/UnrealNetwork.h"

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
}

void AWeapon::ShowPickupWidget(const bool showWidget)
{
	if (m_PickupWidget != nullptr)
	{
		m_PickupWidget->SetVisibility(showWidget);
	}
}

void AWeapon::OnRep_WeaponState()
{
	switch (m_WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		break;
	}
}

void AWeapon::SetWeaponState(EWeaponState weaponState)
{
	m_WeaponState = weaponState;

	switch (m_WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		if (m_AreaSphere != nullptr)
		{
			m_AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		break;
	}
}

