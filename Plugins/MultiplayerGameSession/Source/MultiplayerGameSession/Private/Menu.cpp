// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include<Components/Button.h>
#include<MPGameSessionSubsystem.h>
#include <OnlineSessionSettings.h>


void UMenu::MenuSetup(int32 numPublicConnections, FString matchType, FString pathToLobby)
{
    //Blueprint Callable

    m_NumPublicConnections = numPublicConnections;
    m_MatchType = matchType;
    m_PathToLobby = pathToLobby + "?listen";

    AddToViewport();
    SetVisibility(ESlateVisibility::Visible);
    bIsFocusable = true;

    if (const UWorld* world = GetWorld())
    {
        if (APlayerController* playerController = world->GetFirstPlayerController())
        {
            FInputModeUIOnly inputModeData;
            inputModeData.SetWidgetToFocus(TakeWidget());
            inputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            
            playerController->SetInputMode(inputModeData);
            playerController->SetShowMouseCursor(true);
        }
    }

    if (const UGameInstance* gameInstance = GetGameInstance())
    {
        m_MPGameSessionSubSystem = gameInstance->GetSubsystem<UMPGameSessionSubsystem>();
        if(m_MPGameSessionSubSystem != nullptr)
        {
            m_MPGameSessionSubSystem->m_MPOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
            m_MPGameSessionSubSystem->m_MPOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
            m_MPGameSessionSubSystem->m_MPOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
            m_MPGameSessionSubSystem->m_MPOnFindSessionComplete.AddUObject(this, &ThisClass::OnFindSessions);
            m_MPGameSessionSubSystem->m_MPOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);

        }
    }
}

void UMenu::MenuTearDown()
{
    RemoveFromParent();

    if (const UWorld* world = GetWorld())
    {
        if (APlayerController* playerController = world->GetFirstPlayerController())
        {
            FInputModeGameOnly inputModeData;

            playerController->SetInputMode(inputModeData);
            playerController->SetShowMouseCursor(false);
        }
    }
}

bool UMenu::Initialize()
{
    if (!Super::Initialize())
    {
        return false;
    }

    if (m_HostButton != nullptr)
    {
        m_HostButton->OnClicked.AddDynamic(this, &ThisClass::OnHostButtonClicked);
    }

    if (m_JoinButton != nullptr)
    {
        m_JoinButton->OnClicked.AddDynamic(this, &ThisClass::OnJoinButtonClicked);
    }

    return true;
}

void UMenu::NativeDestruct()
{
    MenuTearDown();

    Super::NativeDestruct();
}

void UMenu::OnCreateSession(const bool wasSuccessful)
{
    if (wasSuccessful)
    {
        UE_LOG(LogTemp, Log, TEXT("Session Created from Menu!!!"));

        if (UWorld* world = GetWorld())
        {
            world->ServerTravel(m_PathToLobby);
        }
    }
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& sessionResults, const bool wasSuccessful)
{
    if (m_MPGameSessionSubSystem == nullptr)
    {
        return;
    }

    for (auto& result : sessionResults)
    {
        FString resultMatchType;
        result.Session.SessionSettings.Get(FName("MatchType"), resultMatchType);

        if (resultMatchType == m_MatchType)
        {
            m_MPGameSessionSubSystem->JoinSession(result);
            return;
        }
    }
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type result)
{
    if (IOnlineSubsystem* subSystem = IOnlineSubsystem::Get())
    {
        IOnlineSessionPtr sessionInterface = subSystem->GetSessionInterface();
        if (sessionInterface.IsValid())
        {
            FString address;
            sessionInterface->GetResolvedConnectString(NAME_GameSession, address);

            if (APlayerController* playerController = GetGameInstance()->GetFirstLocalPlayerController())
            {
                playerController->ClientTravel(*address, ETravelType::TRAVEL_Absolute);
            }
        }
    }
}

void UMenu::OnDestroySession(const bool wasSuccessful)
{
}

void UMenu::OnStartSession(const bool wasSuccessful)
{
}

void UMenu::OnHostButtonClicked()
{
    UE_LOG(LogTemp, Log, TEXT("Host Button Clicked!"));

    if (m_MPGameSessionSubSystem != nullptr)
    {
        m_MPGameSessionSubSystem->CreateSession(m_NumPublicConnections, m_MatchType);
    }
}

void UMenu::OnJoinButtonClicked()
{
    UE_LOG(LogTemp, Log, TEXT("Join Button Clicked!"));

    if (m_MPGameSessionSubSystem != nullptr)
    {
        m_MPGameSessionSubSystem->FindSessions(10000);
    }
}
