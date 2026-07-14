// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"
#include "ShooterNuke/Character/NukeCharacter.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "ShooterNuke/Weapon/Weapon.h"

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

void ULagCompensationComponent::SaveFramePackage()
{
	if (m_Character == nullptr || !m_Character->HasAuthority())
	{
		return;
	}

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

FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& package, ANukeCharacter* hitCharacter, const FVector_NetQuantize& traceStart, const FVector_NetQuantize& hitLocation)
{
	if (hitCharacter == nullptr)
	{
		return FServerSideRewindResult();
	}

	FFramePackage currentFramePackage;
	CacheBoxPositions(hitCharacter, currentFramePackage);
	MoveHitBoxes(hitCharacter, package);
	SetCharacterCollision(hitCharacter, ECollisionEnabled::NoCollision);

	//checking headshot first
	UBoxComponent* headBox = hitCharacter->m_HitCollisionBoxes[FName("head")];
	headBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	headBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	FHitResult confirmHitResult;
	const FVector traceEnd = traceStart + (hitLocation - traceStart)*1.25f;

	if (UWorld* world = GetWorld())
	{
		world->LineTraceSingleByChannel(confirmHitResult, traceStart, traceEnd, ECollisionChannel::ECC_Visibility);
		if (confirmHitResult.bBlockingHit) // hit head return early
		{
			ResetHitBoxes(hitCharacter, currentFramePackage);
			SetCharacterCollision(hitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{true, true};
		}
		else
		{
			for (auto& hitBoxPair : hitCharacter->m_HitCollisionBoxes)
			{
				if (hitBoxPair.Value != nullptr)
				{
					hitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					hitBoxPair.Value->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
				}
			}

			world->LineTraceSingleByChannel(confirmHitResult, traceStart, traceEnd, ECollisionChannel::ECC_Visibility);
			if (confirmHitResult.bBlockingHit) // hit other than head
			{
				ResetHitBoxes(hitCharacter, currentFramePackage);
				SetCharacterCollision(hitCharacter, ECollisionEnabled::QueryAndPhysics);
				return FServerSideRewindResult{ true, false };
			}
		}
	}

	ResetHitBoxes(hitCharacter, currentFramePackage);
	SetCharacterCollision(hitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{ false, false };
}

void ULagCompensationComponent::CacheBoxPositions(ANukeCharacter* hitCharacter, FFramePackage& outFramePackage)
{
	if (hitCharacter == nullptr)
	{
		return;
	}

	for (auto& hitBoxPair : hitCharacter->m_HitCollisionBoxes)
	{
		if (hitBoxPair.Value == nullptr)
		{
			continue;
		}

		FBoxInfo boxInfo;
		boxInfo.m_BoxExtent = hitBoxPair.Value->GetScaledBoxExtent();
		boxInfo.m_Location = hitBoxPair.Value->GetComponentLocation();
		boxInfo.m_Rotation = hitBoxPair.Value->GetComponentRotation();
		outFramePackage.m_HitBoxInfo.Add(hitBoxPair.Key, boxInfo);
	}
}

void ULagCompensationComponent::MoveHitBoxes(ANukeCharacter* hitCharacter, const FFramePackage& package)
{
    if (hitCharacter == nullptr)
    {
        return;
    }

    for (auto& hitBoxPair : hitCharacter->m_HitCollisionBoxes)
    {
        if (hitBoxPair.Value == nullptr)
        {
            continue;
        }

        hitBoxPair.Value->SetWorldLocation(package.m_HitBoxInfo[hitBoxPair.Key].m_Location);
        hitBoxPair.Value->SetWorldRotation(package.m_HitBoxInfo[hitBoxPair.Key].m_Rotation);
        hitBoxPair.Value->SetBoxExtent(package.m_HitBoxInfo[hitBoxPair.Key].m_BoxExtent);
    }
}

void ULagCompensationComponent::ResetHitBoxes(ANukeCharacter* hitCharacter, const FFramePackage& package)
{
	if (hitCharacter == nullptr)
	{
		return;
	}

	for (auto& hitBoxPair : hitCharacter->m_HitCollisionBoxes)
	{
		if (hitBoxPair.Value == nullptr)
		{
			continue;
		}

		hitBoxPair.Value->SetWorldLocation(package.m_HitBoxInfo[hitBoxPair.Key].m_Location);
		hitBoxPair.Value->SetWorldRotation(package.m_HitBoxInfo[hitBoxPair.Key].m_Rotation);
		hitBoxPair.Value->SetBoxExtent(package.m_HitBoxInfo[hitBoxPair.Key].m_BoxExtent);
		hitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ULagCompensationComponent::SetCharacterCollision(ANukeCharacter* hitCharacter, ECollisionEnabled::Type collisionEnabled)
{
	if (hitCharacter != nullptr && hitCharacter->GetMesh() != nullptr)
	{
		hitCharacter->GetMesh()->SetCollisionEnabled(collisionEnabled);
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
	
	SaveFramePackage();
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& package, const FColor& color)
{
	for (auto& boxInfo : package.m_HitBoxInfo)
	{
		DrawDebugBox(GetWorld(), boxInfo.Value.m_Location, boxInfo.Value.m_BoxExtent, FQuat(boxInfo.Value.m_Rotation), color, false, m_MaxRecordTime);
	}
}

FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(ANukeCharacter* hitCharacter, const FVector_NetQuantize& traceStart, const FVector_NetQuantize& hitLocation, float hitTime)
{
	if (hitCharacter == nullptr || hitCharacter->GetLagCompensationComponent() == nullptr ||
		hitCharacter->GetLagCompensationComponent()->m_FrameHistory.GetHead() == nullptr ||
		hitCharacter->GetLagCompensationComponent()->m_FrameHistory.GetTail() == nullptr)
	{
		return FServerSideRewindResult();
	}

	//history of hit character
	const TDoubleLinkedList<FFramePackage>& history = hitCharacter->GetLagCompensationComponent()->m_FrameHistory;
	const float oldestHistoryTime = history.GetTail()->GetValue().m_Time;
	const float newestHistoryTime = history.GetHead()->GetValue().m_Time;
	bool shouldInterpolate = true;

	if (oldestHistoryTime > hitTime)
	{
		//hit character too laggy to do server side rewind
		return FServerSideRewindResult();;
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
		frameToCheck = InterpBetweenFrames(older->GetValue(), younger->GetValue(), hitTime);
	}

	return ConfirmHit(frameToCheck, hitCharacter, traceStart, hitLocation);
}

void ULagCompensationComponent::ServerScoreRequest_Implementation(ANukeCharacter* hitCharacter, const FVector_NetQuantize& traceStart, const FVector_NetQuantize& hitLocation, float hitTime, AWeapon* damageCauser)
{
	if (hitCharacter == nullptr || m_Character == nullptr)
	{
		return;
	}

	FServerSideRewindResult ssrResult = ServerSideRewind(hitCharacter, traceStart, hitLocation, hitTime);

	if (ssrResult.m_HitConfirmed)
	{
		UGameplayStatics::ApplyDamage(hitCharacter, damageCauser->GetDamage(), m_Character->Controller, damageCauser, UDamageType::StaticClass());
	}
}

