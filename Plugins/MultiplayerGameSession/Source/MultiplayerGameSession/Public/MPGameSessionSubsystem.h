// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include <Interfaces/OnlineSessionInterface.h>

#include "MPGameSessionSubsystem.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMPOnCreateSessionComplete, bool, wasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMPOnStartSessionComplete, bool, wasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMPOnDestroySessionComplete, bool, wasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMPOnFindSessionComplete, const TArray<FOnlineSessionSearchResult>& sessionResults, bool wasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FMPOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);

UCLASS()
class MULTIPLAYERGAMESESSION_API UMPGameSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UMPGameSessionSubsystem();

	void CreateSession(int32 numPublicConnections, FString matchType);
	void FindSessions(int32 maxSearchResults);
	void JoinSession(const FOnlineSessionSearchResult& sessionSearchResult);
	void DestroySession();
	void StartSession();

	FMPOnCreateSessionComplete m_MPOnCreateSessionComplete;
	FMPOnStartSessionComplete m_MPOnStartSessionComplete;
	FMPOnDestroySessionComplete m_MPOnDestroySessionComplete;
	FMPOnFindSessionComplete m_MPOnFindSessionComplete;
	FMPOnJoinSessionComplete m_MPOnJoinSessionComplete;

protected:
	void OnCreateSessionComplete(FName sessionName, bool wasSuccessful);
	void OnStartSessionComplete(FName sessionName, bool wasSuccessful);
	void OnDestroySessionComplete(FName sessionName, bool wasSuccessful);
	void OnFindSessionComplete(bool wasSuccessful);
	void OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type result);


private:
	void CorrectSessionSettingsForSearchResult();

	IOnlineSessionPtr m_SessionInterface;
	TSharedPtr<FOnlineSessionSettings> m_LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> m_LastSessionSearch;

	FOnCreateSessionCompleteDelegate m_CreateSessionCompleteDelegate;
	FDelegateHandle m_CreateSessionCompleteDelegateHandle;
	FOnFindSessionsCompleteDelegate m_FindSessionCompleteDelegate;
	FDelegateHandle m_FindSessionCompleteDelegateHandle;
	FOnJoinSessionCompleteDelegate m_JoinSessionCompleteDeleegate;
	FDelegateHandle m_JoinSessionCompleteDeleegateHandle;
	FOnStartSessionCompleteDelegate m_StartSessionCompleteDelegate;
	FDelegateHandle m_StartSessionCompleteDelegateHandle;
	FOnDestroySessionCompleteDelegate m_DestroySessionCompleteDelegate;
	FDelegateHandle m_DestroySessionCompleteDelegateHandle;

	bool m_CreateSessionOnDestroy{ false };
	int32 m_LastNumConnections;
	FString m_LastMatchType;
};
