// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "AP_DamageExecuter.generated.h"

/**
 * 
 */
UCLASS()
class LINHIBLADE_API UAP_DamageExecuter : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
public:
	UAP_DamageExecuter();
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
