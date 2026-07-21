// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

class ANukePlayerController;
class ANukeCharacter;
class AWeapon;

USTRUCT(BlueprintType)
struct FBoxInfo
{
	GENERATED_BODY()

	FVector m_Location;
	FRotator m_Rotation;
	FVector m_BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	float m_Time;

	TMap<FName, FBoxInfo> m_HitBoxInfo;

	UPROPERTY()
	ANukeCharacter* m_Character;
};


USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	bool m_HitConfirmed;
	bool m_HeadShot;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<ANukeCharacter*, uint32> m_HeadShots;

	UPROPERTY()
	TMap<ANukeCharacter*, uint32> m_BodyShots;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHOOTERNUKE_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULagCompensationComponent();
	friend class ANukeCharacter;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void SaveFramePackage();
	void SaveFramePackage(FFramePackage& packge);
	FFramePackage InterpBetweenFrames(const FFramePackage& olderFrame, const FFramePackage& youngerFrame, float hitTime);
	FFramePackage GetFrameToCheck(ANukeCharacter* hitCharacter, float hitTime);
	void CacheBoxPositions(ANukeCharacter* hitCharacter, FFramePackage& outFramePackage);
	void MoveHitBoxes(ANukeCharacter* hitCharacter, const FFramePackage& package);
	void ResetHitBoxes(ANukeCharacter* hitCharacter, const FFramePackage& package);
	void SetCharacterCollision(ANukeCharacter* hitCharacter, ECollisionEnabled::Type collisionEnabled);

	//HitScan SSR
	FServerSideRewindResult HitScanConfirmHit(const FFramePackage& package, ANukeCharacter* hitCharacter, const FVector_NetQuantize& traceStart, const FVector_NetQuantize& hitLocation);
	FServerSideRewindResult HitScanServerSideRewind(ANukeCharacter* hitCharacter, const FVector_NetQuantize& traceStart, const FVector_NetQuantize& hitLocation, float hitTime);

	//Projectile SSR
	FServerSideRewindResult ProjectileServerSideRewind(ANukeCharacter* hitCharacter, const FVector_NetQuantize& traceStart, const FVector_NetQuantize100& initialVelocity, float hitTime);
	FServerSideRewindResult ProjectileConfirmHit(const FFramePackage& package, ANukeCharacter* hitCharacter, const FVector_NetQuantize& traceStart, const FVector_NetQuantize100& initialVelocity, float hitTime);

	//Shotgun SSR
	FShotgunServerSideRewindResult ShotgunServerSideRewind(const TArray<ANukeCharacter*>& hitCharacters, const FVector_NetQuantize& traceStart, const TArray<FVector_NetQuantize>& hitLocations, float hitTime);
	FShotgunServerSideRewindResult ShotgunConfirmHit(const TArray<FFramePackage>& framePackages, const FVector_NetQuantize& traceStart, const TArray<FVector_NetQuantize>& hitLocations);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ShowFramePackage(const FFramePackage& package, const FColor& color);

	UFUNCTION(Server, Reliable)
	void ServerHitScanScoreRequest(ANukeCharacter* hitCharacter, const FVector_NetQuantize& traceStart, const FVector_NetQuantize& hitLocation, float hitTime, AWeapon* damageCauser);

	UFUNCTION(Server, Reliable)
	void ServerProjectileScoreRequest(ANukeCharacter* hitCharacter, const FVector_NetQuantize& traceStart, const FVector_NetQuantize100& initialVelocity, float hitTime);

	UFUNCTION(Server, Reliable)
	void ServerShotgunScoreRequest(const TArray<ANukeCharacter*>& hitCharacters, const FVector_NetQuantize& traceStart, const TArray<FVector_NetQuantize>& hitLocations, float hitTime);

private:
	UPROPERTY()
	ANukeCharacter* m_Character;

	UPROPERTY()
	ANukePlayerController* m_Controller;

	TDoubleLinkedList<FFramePackage> m_FrameHistory;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Max Record Time"))
	float m_MaxRecordTime = 4.f;
};
