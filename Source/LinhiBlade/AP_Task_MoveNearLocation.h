// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AP_Task_MoveNearLocation.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTargetReachedDelegate);

/**
 * 
 */
UCLASS()
class LINHIBLADE_API UAP_Task_MoveNearLocation : public UAbilityTask
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FTargetReachedDelegate OnTargetLocationReached;
	FTargetReachedDelegate OnMovementCancelled;
	TScriptDelegate<FWeakObjectPtr> OnMovementCompletedDelegate;
	TScriptDelegate<FWeakObjectPtr> OnMovementCancelledDelegate;
public:
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
		static UAP_Task_MoveNearLocation* MoveNearLocation(UGameplayAbility* OwningAbility, FName TaskInstanceName, FVector Location, float Range);
public:
	virtual void Activate() override;
	virtual void OnDestroy(bool AbilityIsEnding) override;
public:
	//UFUNCTION(BlueprintInternalUseOnly, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	void OnMovementCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);
protected:
	UPROPERTY()
		FVector TargetLocation;

	UPROPERTY()
		float AcceptanceRadius;

};
