// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletShell.generated.h"

class USoundCue;

UCLASS()
class SHOOTERNUKE_API ABulletShell : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABulletShell();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* hitComp, AActor* otherActor, UPrimitiveComponent* otherComp, FVector NormalImpulse, const FHitResult& hitResult);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere, meta = (DisplayName = "Bullet Shell Mesh"))
	UStaticMeshComponent* m_BulletShellMesh;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Shell Sound"))
	USoundCue* m_ShellSound;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Shell Ejection Impulse"))
	float m_ShellEjectionImpulse;
};
