// Fill out your copyright notice in the Description page of Project Settings.


#include "NukeGameMode.h"
#include "ShooterNuke/Character/NukeCharacter.h"

void ANukeGameMode::PlayerEliminated(ANukeCharacter* eliminatedCharacter, ANukePlayerController* attackerPlayerController)
{
    if (eliminatedCharacter != nullptr)
    {
        eliminatedCharacter->MultiCastEliminate();
    }
}
