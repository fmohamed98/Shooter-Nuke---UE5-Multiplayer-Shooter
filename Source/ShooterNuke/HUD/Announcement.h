// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Announcement.generated.h"

class UTextBlock;

UCLASS()
class SHOOTERNUKE_API UAnnouncement : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta=(BindWidget))
	UTextBlock* m_WarmupTime;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* m_AnnouncementText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* m_InfoText;
};
