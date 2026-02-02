// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* newPlayer)
{
    Super::PostLogin(newPlayer);

    uint32 numPlayers = GameState.Get()->PlayerArray.Num();
    if (numPlayers >= 2)
    {
        if (UWorld* world = GetWorld())
        {
            world->ServerTravel(FString("/Game/Maps/NukeMap?listen"));
        }
    }
}
