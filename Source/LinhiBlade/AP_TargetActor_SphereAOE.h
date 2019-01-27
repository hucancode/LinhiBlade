// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "AP_TargetActor_SphereAOE.generated.h"

/**
 * given a location and a sphere, query all target that hit the sphere
 */
UCLASS()
class LINHIBLADE_API AAP_TargetActor_SphereAOE : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void StartTargeting(UGameplayAbility* Ability) override;
	virtual void ConfirmTargetingAndContinue() override;
	virtual void CancelTargeting() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Radius of target acquisition around the ability's start location. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = AOE)
		FVector Location;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = AOE)
		float Radius;
protected:
	TArray<TWeakObjectPtr<AActor>> PerformTrace();
};
