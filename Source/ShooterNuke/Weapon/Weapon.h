// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class USphereComponent;

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Inital State"),
	EWS_Equipped UMETA(DisplayName = "Equipped State"),
	EWS_Dropped UMETA(DisplayName = "Dropped State"),

	EWS_MAX UMETA(DisplayName = "DefaultMax")
};

UCLASS()
class SHOOTERNUKE_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties", meta = (DisplayName = "Weapon Mesh"))
	USkeletalMeshComponent* m_WeaponMesh;
	
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties", meta = (DisplayName = "Area Sphere"))
	USphereComponent* m_AreaSphere;

	UPROPERTY(VisibleAnywhere, meta = (DisplayName = "Weapon State"))
	EWeaponState m_WeaponState;
};
