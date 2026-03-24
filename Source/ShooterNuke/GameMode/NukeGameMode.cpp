// Fill out your copyright notice in the Description page of Project Settings.


#include "NukeGameMode.h"
#include "ShooterNuke/Character/NukeCharacter.h"
#include "ShooterNuke/PlayerState/NukePlayerState.h"
#include "ShooterNuke/PlayerController/NukePlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

void ANukeGameMode::PlayerEliminated(ANukeCharacter* eliminatedCharacter, ANukePlayerController* attackerPlayerController)
{
    if (eliminatedCharacter == nullptr)
    {
        return;
    }

    ANukePlayerController* victimPlayerController = Cast<ANukePlayerController>(eliminatedCharacter->GetController());
    if (victimPlayerController == nullptr || attackerPlayerController == nullptr)
    {
        return;
    }

    ANukePlayerState* attackerPlayerState = Cast<ANukePlayerState>(attackerPlayerController->PlayerState);
    ANukePlayerState* victimPlayerState = Cast<ANukePlayerState>(victimPlayerController->PlayerState);

    if (attackerPlayerState != nullptr && attackerPlayerState != victimPlayerState)
    {
        attackerPlayerState->AddToScore(1.f);
    }

    if (victimPlayerState != nullptr)
    {
        victimPlayerState->IncrementDeathCount();
    }

    eliminatedCharacter->Eliminate();
 }

void ANukeGameMode::RequestRespawn(ANukeCharacter* eliminatedCharacter)
{
    if (eliminatedCharacter == nullptr)
    {
        return;
    }

    AController* eliminatedController = eliminatedCharacter->GetController();

    eliminatedCharacter->Reset();
    eliminatedCharacter->Destroy();

    if (eliminatedController != nullptr)
    {
        TArray<AActor*> playerStarts;
        UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), playerStarts);
        uint32 index = FMath::RandRange(0, playerStarts.Num() - 1);

        RestartPlayerAtPlayerStart(eliminatedController, playerStarts[index]);
    }
}
