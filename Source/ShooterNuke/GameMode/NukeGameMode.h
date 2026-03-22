// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "NukeGameMode.generated.h"

class ANukeCharacter;
class ANukePlayerController;

UCLASS()
class SHOOTERNUKE_API ANukeGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	void PlayerEliminated(ANukeCharacter* eliminatedCharcter, ANukePlayerController* attackerPlayerController);
	void RequestRespawn(ANukeCharacter* eliminatedCharcter);
};
