// Fill out your copyright notice in the Description page of Project Settings.


#include "NukeCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

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
}

// Called to bind functionality to input
void ANukeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ANukeCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ANukeCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &ANukeCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ANukeCharacter::LookUp);
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


