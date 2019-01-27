// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "AP_Hero.h"
#include "AP_TargetActor_Hero.generated.h"

/**
 * point targeting, return hero on the ground that mouse is targeting to, null if no hero
 */
UCLASS()
class LINHIBLADE_API AAP_TargetActor_Hero : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()
public:
	/** Initialize and begin targeting logic  */
	virtual void BeginPlay() override;
	virtual void StartTargeting(UGameplayAbility* Ability) override;
	/** Outside code is saying 'stop and just give me what you have.' Returns true if the ability accepts this and can be forgotten. */
	virtual void ConfirmTargetingAndContinue() override;
	virtual void CancelTargeting() override;
	virtual void Tick(float DeltaTime) override;
	void TurnOffTargetHighlight();
protected:
	FHitResult PerformTrace();
private:
	AAP_Hero* LastHeroHighlighted;
};
