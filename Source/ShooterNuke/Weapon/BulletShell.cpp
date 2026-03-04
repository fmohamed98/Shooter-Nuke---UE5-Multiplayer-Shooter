// Fill out your copyright notice in the Description page of Project Settings.


#include "BulletShell.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

// Sets default values
ABulletShell::ABulletShell()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	m_BulletShellMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletShellMesh"));
	SetRootComponent(m_BulletShellMesh);

	m_BulletShellMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	m_BulletShellMesh->SetSimulatePhysics(true);
	m_BulletShellMesh->SetEnableGravity(true);
	m_BulletShellMesh->SetNotifyRigidBodyCollision(true);

	m_ShellEjectionImpulse = 1000.f;
}

// Called when the game starts or when spawned
void ABulletShell::BeginPlay()
{
	Super::BeginPlay();
	
	m_BulletShellMesh->OnComponentHit.AddDynamic(this, &ABulletShell::OnHit);
	m_BulletShellMesh->AddImpulse(GetActorForwardVector() * m_ShellEjectionImpulse);
}

// Called every frame
void ABulletShell::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABulletShell::OnHit(UPrimitiveComponent* hitComp, AActor* otherActor, UPrimitiveComponent* otherComp, FVector NormalImpulse, const FHitResult& hitResult)
{
	UGameplayStatics::PlaySoundAtLocation(this, m_ShellSound, GetActorLocation());
	Destroy();
}

