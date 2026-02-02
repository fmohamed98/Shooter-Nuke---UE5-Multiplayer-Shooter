// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include <Interfaces/OnlineSessionInterface.h>

#include "Menu.generated.h"
/**
 * 
 */

class UButton;
class UMPGameSessionSubsystem;

UCLASS()
class MULTIPLAYERGAMESESSION_API UMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 numPublicConnections = 4, FString matchType = FString(TEXT("FreeForMoi")), FString m_PathToLobby = FString(TEXT("/Game/ThirdPerson/Maps/Lobby")));
	void MenuTearDown();

	bool Initialize() override;
	void NativeDestruct() override;

	UFUNCTION()
	void OnCreateSession(const bool wasSuccessful);
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& sessionResults, const bool wasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type result);

	UFUNCTION()
	void OnDestroySession(const bool wasSuccessful);
	UFUNCTION()
	void OnStartSession(const bool wasSuccessful);

private:
	UPROPERTY(meta = (BindWidget))
	UButton* m_HostButton;

	UPROPERTY(meta = (BindWidget))
	UButton* m_JoinButton;

    UFUNCTION()
    void OnHostButtonClicked();

	UFUNCTION()
	void OnJoinButtonClicked();

	UMPGameSessionSubsystem* m_MPGameSessionSubSystem = nullptr;

	int32 m_NumPublicConnections{ 4 };
	FString m_MatchType{ TEXT("FreeForMoi") };
	FString m_PathToLobby{ TEXT("/Game/ThirdPerson/Maps/Lobby")};
};
