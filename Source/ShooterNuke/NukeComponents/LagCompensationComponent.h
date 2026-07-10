// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

class ANukePlayerController;

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
	void SaveFramePackage(FFramePackage& packge);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void ShowFramePackage(const FFramePackage& package, const FColor& color);

private:
	UPROPERTY()
	ANukeCharacter* m_Character;

	UPROPERTY()
	ANukePlayerController* m_Controller;

	TDoubleLinkedList<FFramePackage> m_FrameHistory;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Max Record Time"))
	float m_MaxRecordTime = 4.f;
};
