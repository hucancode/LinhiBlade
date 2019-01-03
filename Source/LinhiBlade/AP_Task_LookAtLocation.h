// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AP_Task_LookAtLocation.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRotationReachedDelegate);

/**
 * Look at a location, target is a AI controlled character
 * The task compeleted when target is reached or character is not idle
 */
UCLASS()
class LINHIBLADE_API UAP_Task_LookAtLocation : public UAbilityTask
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
		static UAP_Task_LookAtLocation* LookAtLocation(UGameplayAbility* OwningAbility, FName TaskInstanceName, FVector Location, float TurnRate);
public:
	UPROPERTY(BlueprintAssignable)
		FRotationReachedDelegate OnTargetRotationReached;
	UPROPERTY(BlueprintAssignable)
		FRotationReachedDelegate OnRotationCancelled;
public:
	UAP_Task_LookAtLocation();
	~UAP_Task_LookAtLocation();
	virtual void Activate() override;
	virtual void OnDestroy(bool AbilityIsEnding) override;
	virtual void TickTask(float DeltaTime) override;
protected:
	UPROPERTY()
		FVector TargetLocation;
	UPROPERTY()
		float TurnRate;
private:
	FRotator Destination;
	bool Reached;
};
