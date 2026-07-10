// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"
#include "ShooterNuke/Character/NukeCharacter.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
ULagCompensationComponent::ULagCompensationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

}


// Called when the game starts
void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();

	m_Character = m_Character == nullptr ? Cast<ANukeCharacter>(GetOwner()) : m_Character;
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& package)
{
	if (m_Character == nullptr)
	{
		return;
	}

	package.m_Time = GetWorld()->GetTimeSeconds();
	for (auto& boxPair : m_Character->m_HitCollisionBoxes)
	{
		FBoxInfo boxInfo;
		boxInfo.m_Location = boxPair.Value->GetComponentLocation();
		boxInfo.m_Rotation = boxPair.Value->GetComponentRotation();
		boxInfo.m_BoxExtent = boxPair.Value->GetScaledBoxExtent();
		package.m_HitBoxInfo.Add(boxPair.Key, boxInfo);
	}
}


// Called every frame
void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (m_FrameHistory.Num() > 1)
	{
		float historyLength = m_FrameHistory.GetHead()->GetValue().m_Time - m_FrameHistory.GetTail()->GetValue().m_Time;

		while (historyLength > m_MaxRecordTime)
		{
			m_FrameHistory.RemoveNode(m_FrameHistory.GetTail());

			historyLength = m_FrameHistory.GetHead()->GetValue().m_Time - m_FrameHistory.GetTail()->GetValue().m_Time;
		}
	}

	FFramePackage thisFramePackage;
	SaveFramePackage(thisFramePackage);
	m_FrameHistory.AddHead(thisFramePackage);

	ShowFramePackage(thisFramePackage, FColor::Red);
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& package, const FColor& color)
{
	for (auto& boxInfo : package.m_HitBoxInfo)
	{
		DrawDebugBox(GetWorld(), boxInfo.Value.m_Location, boxInfo.Value.m_BoxExtent, FQuat(boxInfo.Value.m_Rotation), color, false, m_MaxRecordTime);
	}

}

