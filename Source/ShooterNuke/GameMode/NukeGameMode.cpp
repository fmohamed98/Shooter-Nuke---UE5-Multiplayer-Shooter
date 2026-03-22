// Fill out your copyright notice in the Description page of Project Settings.


#include "NukeGameMode.h"
#include "ShooterNuke/Character/NukeCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

void ANukeGameMode::PlayerEliminated(ANukeCharacter* eliminatedCharacter, ANukePlayerController* attackerPlayerController)
{
    if (eliminatedCharacter != nullptr)
    {
        eliminatedCharacter->Eliminate();
    }
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
