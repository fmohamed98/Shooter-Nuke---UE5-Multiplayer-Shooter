// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class USphereComponent;
class UWidgetComponent;
class UAnimationAsset;
class ABulletShell;

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

	UFUNCTION()
	void OnSphereOverlap(
		UPrimitiveComponent* overlappedComponent,
		AActor* otherActor,
		UPrimitiveComponent* otherComponent,
		int32 otherBodyIndex,
		bool isFromSweep,
		const FHitResult& sweepResult
	);

	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* overlappedComponent,
		AActor* otherActor,
		UPrimitiveComponent* otherComponent,
		int32 otherBodyIndex
	);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void ShowPickupWidget(const bool showWidget);
	virtual void Fire(const FVector& hitTarget);

	void Drop();

private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties", meta = (DisplayName = "Weapon Mesh"))
	USkeletalMeshComponent* m_WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties", meta = (DisplayName = "Area Sphere"))
	USphereComponent* m_AreaSphere;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_WeaponState, meta = (DisplayName = "Weapon State"))
	EWeaponState m_WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties", meta = (DisplayName = "Pickup Widget"))
	UWidgetComponent* m_PickupWidget;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties", meta = (DisplayName = "Fire Animation"))
	UAnimationAsset* m_FireAnimation;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties", meta = (DisplayName = "Bullet Shell"))
	TSubclassOf<ABulletShell> m_BulletShellClass;

public:
	//Textures
	UPROPERTY(EditAnywhere, Category = "Crosshairs", meta = (DisplayName = "Crosshair Center"))
	UTexture2D* m_CrosshairCenter;

	UPROPERTY(EditAnywhere, Category = "Crosshairs", meta = (DisplayName = "Crosshair Left"))
	UTexture2D* m_CrosshairLeft;

	UPROPERTY(EditAnywhere, Category = "Crosshairs", meta = (DisplayName = "Crosshair Right"))
	UTexture2D* m_CrosshairRight;

	UPROPERTY(EditAnywhere, Category = "Crosshairs", meta = (DisplayName = "Crosshair Top"))
	UTexture2D* m_CrosshairTop;

	UPROPERTY(EditAnywhere, Category = "Crosshairs", meta = (DisplayName = "Crosshair Bottom"))
	UTexture2D* m_CrosshairBottom;

	UPROPERTY(EditAnywhere, Category = Combat, meta = (DisplayName = "Fire Delay"))
	float m_FireDelay = .15f;

	UPROPERTY(EditAnywhere, Category = Combat, meta = (DisplayName = "Automatic"))
	bool m_IsAutomatic = false;

	void SetWeaponState(EWeaponState weaponState);
	FORCEINLINE USphereComponent* GetAreaSphere() const { return m_AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return m_WeaponMesh; }
	FORCEINLINE float GetZoomedFOV() const { return m_ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return m_ZoomInterpSpeed; }

private:
	UPROPERTY(EditAnywhere, meta = (DisplayName = "Zoomed FOV"))
	float m_ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Zoom Interpolation Speed"))
	float m_ZoomInterpSpeed = 20.f;
};
