// Fill out your copyright notice in the Description page of Project Settings.

#include "AP_Task_MoveNearLocation.h"
#include "GameFramework/Character.h"

UAP_Task_MoveNearLocation * UAP_Task_MoveNearLocation::MoveNearLocation(UGameplayAbility * OwningAbility, FName TaskInstanceName, FVector Location, float Range)
{
	UE_LOG(LogTemp, Warning, TEXT("UAP_Task_MoveNearLocation::MoveNearLocation creating task"));
	// receive call from blueprint
	UAP_Task_MoveNearLocation* MyObj = NewAbilityTask<UAP_Task_MoveNearLocation>(OwningAbility, TaskInstanceName);
	MyObj->TargetLocation = Location;
	MyObj->AcceptanceRadius = Range;
	UE_LOG(LogTemp, Warning, TEXT("UAP_Task_MoveNearLocation::MoveNearLocation task created"));
	return MyObj;
}

void UAP_Task_MoveNearLocation::Activate()
{
	UE_LOG(LogTemp, Warning, TEXT("UAP_Task_MoveNearLocation::Activate()"));
	
	ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
	if (!Character)
	{
		OnTargetLocationReached.Broadcast();
		return;
	}
	AAIController* Controller = Character->GetController<AAIController>();
	if (!Controller)
	{
		OnTargetLocationReached.Broadcast();
		return;
	}
	Controller->MoveToLocation(TargetLocation, AcceptanceRadius);
	OnMovementCompletedDelegate.BindUFunction(this, "OnMovementCompleted");
	Controller->ReceiveMoveCompleted.Add(OnMovementCompletedDelegate);
}

void UAP_Task_MoveNearLocation::OnDestroy(bool AbilityIsEnding)
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
	if (!Character)
	{
		OnTargetLocationReached.Broadcast();
		return;
	}
	AAIController* Controller = Character->GetController<AAIController>();
	if (!Controller)
	{
		OnTargetLocationReached.Broadcast();
		return;
	}
	Controller->ReceiveMoveCompleted.Remove(OnMovementCompletedDelegate);
}

void UAP_Task_MoveNearLocation::OnMovementCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
	if (!Character)
	{
		OnTargetLocationReached.Broadcast();
		return;
	}
	AAIController* Controller = Character->GetController<AAIController>();
	if (!Controller)
	{
		OnTargetLocationReached.Broadcast();
		return;
	}
	Controller->ReceiveMoveCompleted.Remove(OnMovementCompletedDelegate);
	// broadcast result back to blueprint
	OnTargetLocationReached.Broadcast();
}
