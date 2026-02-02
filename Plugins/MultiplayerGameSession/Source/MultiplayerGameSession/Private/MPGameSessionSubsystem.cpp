// Fill out your copyright notice in the Description page of Project Settings.


#include "MPGameSessionSubsystem.h"

#include <OnlineSubsystem.h>
#include <OnlineSessionSettings.h>
#include <Online\OnlineSessionNames.h>

UMPGameSessionSubsystem::UMPGameSessionSubsystem():
    m_CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
    m_FindSessionCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionComplete)),
    m_JoinSessionCompleteDeleegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
    m_StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete)),
    m_DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete))
{
    if (IOnlineSubsystem* subSystem = IOnlineSubsystem::Get())
    {
        m_SessionInterface = subSystem->GetSessionInterface();
    }
}

void UMPGameSessionSubsystem::CreateSession(int32 numPublicConnections, FString matchType)
{
    if (!m_SessionInterface.IsValid())
    {
        return;
    }

    if (auto existingSession = m_SessionInterface->GetNamedSession(NAME_GameSession))
    {
        m_CreateSessionOnDestroy = true;
        m_LastNumConnections = numPublicConnections;
        m_LastMatchType = matchType;

        DestroySession();
    }

    m_CreateSessionCompleteDelegateHandle = m_SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(m_CreateSessionCompleteDelegate);

    m_LastSessionSettings = MakeShareable(new FOnlineSessionSettings);
    m_LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
    m_LastSessionSettings->bAllowJoinInProgress = true;
    m_LastSessionSettings->bAllowJoinViaPresence = true;
    m_LastSessionSettings->bShouldAdvertise = true;
    m_LastSessionSettings->NumPublicConnections = numPublicConnections;
    m_LastSessionSettings->bUsesPresence = true;
    m_LastSessionSettings->bUseLobbiesIfAvailable = true;
    m_LastSessionSettings->BuildUniqueId = 1;
    m_LastSessionSettings->Set(FName("MatchType"), matchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

    if (const UWorld* world = GetWorld())
    {
        if (const ULocalPlayer* localPlayer = world->GetFirstLocalPlayerFromController())
        {
            if (!m_SessionInterface->CreateSession(*localPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *m_LastSessionSettings))
            {
                m_SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(m_CreateSessionCompleteDelegateHandle);
                m_MPOnCreateSessionComplete.Broadcast(false);
            }
        }
    }
}

void UMPGameSessionSubsystem::FindSessions(int32 maxSearchResults)
{
    if (!m_SessionInterface.IsValid())
    {
        return;
    }

    m_FindSessionCompleteDelegateHandle = m_SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(m_FindSessionCompleteDelegate);

    m_LastSessionSearch = MakeShareable(new FOnlineSessionSearch);
    m_LastSessionSearch->MaxSearchResults = maxSearchResults;
    m_LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";;
    m_LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

    if (const UWorld* world = GetWorld())
    {
        if (const ULocalPlayer* localPlayer = world->GetFirstLocalPlayerFromController())
        {
            if (!m_SessionInterface->FindSessions(*localPlayer->GetPreferredUniqueNetId(), m_LastSessionSearch.ToSharedRef()))
            {
                m_SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(m_FindSessionCompleteDelegateHandle);
                m_MPOnFindSessionComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
            }
        }
    }
}

void UMPGameSessionSubsystem::JoinSession(const FOnlineSessionSearchResult& sessionSearchResult)
{
    if (!m_SessionInterface.IsValid())
    {
        return;
    }

    m_JoinSessionCompleteDeleegateHandle =  m_SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(m_JoinSessionCompleteDeleegate);

    if (const UWorld* world = GetWorld())
    {
        if (const ULocalPlayer* localPlayer = world->GetFirstLocalPlayerFromController())
        {
            if (!m_SessionInterface->JoinSession(*localPlayer->GetPreferredUniqueNetId(), NAME_GameSession, sessionSearchResult))
            {
                m_SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(m_JoinSessionCompleteDeleegateHandle);
                m_MPOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
            }
        }
    }
}

void UMPGameSessionSubsystem::DestroySession()
{
    if (!m_SessionInterface.IsValid())
    {
        return;
    }

    m_DestroySessionCompleteDelegateHandle = m_SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(m_DestroySessionCompleteDelegate);

    if (!m_SessionInterface->DestroySession(NAME_GameSession))
    {
        m_SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(m_DestroySessionCompleteDelegateHandle);
        m_MPOnDestroySessionComplete.Broadcast(false);
    }
}

void UMPGameSessionSubsystem::StartSession()
{
}

void UMPGameSessionSubsystem::OnCreateSessionComplete(FName sessionName, bool wasSuccessful)
{
    if (m_SessionInterface.IsValid())
    {
        m_SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(m_CreateSessionCompleteDelegateHandle);
    }
    m_MPOnCreateSessionComplete.Broadcast(wasSuccessful);
}

void UMPGameSessionSubsystem::OnStartSessionComplete(FName sessionName, bool wasSuccessful)
{
}

void UMPGameSessionSubsystem::OnDestroySessionComplete(FName sessionName, bool wasSuccessful)
{
    if (m_SessionInterface.IsValid())
    {
        m_SessionInterface->ClearOnDestroySessionRequestedDelegate_Handle(m_DestroySessionCompleteDelegateHandle);
    }

    if (wasSuccessful && m_CreateSessionOnDestroy)
    {
        m_CreateSessionOnDestroy = false;
        CreateSession(m_LastNumConnections, m_LastMatchType);
    }
    m_MPOnDestroySessionComplete.Broadcast(wasSuccessful);
}

void UMPGameSessionSubsystem::OnFindSessionComplete(bool wasSuccessful)
{
    if (!m_LastSessionSearch.IsValid())
    {
        return;
    }

    if (m_SessionInterface.IsValid())
    {
        m_SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(m_FindSessionCompleteDelegateHandle);
    }

    CorrectSessionSettingsForSearchResult();
    m_MPOnFindSessionComplete.Broadcast(m_LastSessionSearch->SearchResults, wasSuccessful);
}

void UMPGameSessionSubsystem::OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type result)
{
    if (m_SessionInterface.IsValid())
    {
        m_SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(m_JoinSessionCompleteDeleegateHandle);
    }

    m_MPOnJoinSessionComplete.Broadcast(result);
}

void UMPGameSessionSubsystem::CorrectSessionSettingsForSearchResult()
{
    for (auto& result : m_LastSessionSearch->SearchResults)
    {
        result.Session.SessionSettings.bUseLobbiesIfAvailable = true;
    }
}
