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

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& olderFrame, const FFramePackage& youngerFrame, float hitTime)
{
	const float distance = youngerFrame.m_Time - olderFrame.m_Time;
	const float interpFraction = FMath::Clamp((hitTime - olderFrame.m_Time) / distance, 0, 1);

	FFramePackage interpFramePackage;
	interpFramePackage.m_Time = hitTime;

	for (auto& youngerPair : youngerFrame.m_HitBoxInfo)
	{
		const FName& boxInfoName = youngerPair.Key;

		const FBoxInfo& olderBoxInfo = olderFrame.m_HitBoxInfo[boxInfoName];
		const FBoxInfo& youngerBoxInfo = youngerFrame.m_HitBoxInfo[boxInfoName];

		FBoxInfo interpBoxInfo;
		interpBoxInfo.m_Location = FMath::VInterpTo(olderBoxInfo.m_Location, youngerBoxInfo.m_Location, 1.f, interpFraction);
		interpBoxInfo.m_Rotation = FMath::RInterpTo(olderBoxInfo.m_Rotation, youngerBoxInfo.m_Rotation, 1.f, interpFraction);
		interpBoxInfo.m_BoxExtent = youngerBoxInfo.m_BoxExtent;

		interpFramePackage.m_HitBoxInfo.Add(boxInfoName, interpBoxInfo);
	}

	return interpFramePackage;
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

void ULagCompensationComponent::ServerSideRewind(ANukeCharacter* hitCharacter, const FVector_NetQuantize& traceStart, const FVector_NetQuantize& hitLocation, float hitTime)
{
	if (hitCharacter == nullptr || hitCharacter->GetLagCompensationComponent() == nullptr ||
		hitCharacter->GetLagCompensationComponent()->m_FrameHistory.GetHead() == nullptr ||
		hitCharacter->GetLagCompensationComponent()->m_FrameHistory.GetTail() == nullptr)
	{
		return;
	}

	//history of hit character
	const TDoubleLinkedList<FFramePackage>& history = hitCharacter->GetLagCompensationComponent()->m_FrameHistory;
	const float oldestHistoryTime = history.GetTail()->GetValue().m_Time;
	const float newestHistoryTime = history.GetHead()->GetValue().m_Time;
	bool shouldInterpolate = true;

	if (oldestHistoryTime > hitTime)
	{
		//hit character too laggy to do server side rewind
		return;
	}

	FFramePackage frameToCheck;
	if (oldestHistoryTime == hitTime)
	{
		frameToCheck = history.GetTail()->GetValue();
		shouldInterpolate = false;
	}
	else if (newestHistoryTime <= hitTime)
	{
		frameToCheck = history.GetHead()->GetValue();
		shouldInterpolate = false;
	}

	auto younger = history.GetHead();
	auto older = younger;

	while (older && older->GetValue().m_Time > hitTime)
	{
		younger = older;
		older = older->GetNextNode();
	}

	if (older->GetValue().m_Time == hitTime)  //unlikely
	{
		frameToCheck = older->GetValue();
		shouldInterpolate = false;
	}

	if (shouldInterpolate)
	{
		//..
	}
}

