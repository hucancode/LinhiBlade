// Fill out your copyright notice in the Description page of Project Settings.

#include "AP_Task_MoveNearLocation.h"
#include "GameFramework/Character.h"

UAP_Task_MoveNearLocation * UAP_Task_MoveNearLocation::MoveNearLocation(UGameplayAbility * OwningAbility, FName TaskInstanceName, FVector Location, float Range)
{
	// receive call from blueprint
	UAP_Task_MoveNearLocation* MyObj = NewAbilityTask<UAP_Task_MoveNearLocation>(OwningAbility, TaskInstanceName);
	MyObj->TargetLocation = Location;
	MyObj->AcceptanceRadius = Range;
	UE_LOG(LogTemp, Warning, TEXT("UAP_Task_MoveNearLocation::MoveNearLocation() %s"), *Location.ToCompactString());
	return MyObj;
}

UAP_Task_MoveNearLocation::UAP_Task_MoveNearLocation()
{
	OnMovementCompletedDelegate.BindUFunction(this, "OnMovementCompleted");
	AcceptanceRadius = -1.0f;
}

UAP_Task_MoveNearLocation::~UAP_Task_MoveNearLocation()
{
	OnMovementCompletedDelegate.Unbind();
}

void UAP_Task_MoveNearLocation::Activate()
{
	UE_LOG(LogTemp, Warning, TEXT("UAP_Task_MoveNearLocation::Activate()"));
	
	ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
	//check(Character);
	if (!Character) return;
	AAIController* Controller = Character->GetController<AAIController>();
	//check(Controller);
	if (!Controller) return;
	EPathFollowingRequestResult::Type RequestResult = Controller->MoveToLocation(TargetLocation, AcceptanceRadius);
	switch (RequestResult)
	{
	case EPathFollowingRequestResult::Failed:
		UE_LOG(LogTemp, Warning, TEXT("UAP_Task_MoveNearLocation::Activate() request = Failed"));
		OnMovementCancelled.Broadcast();
		break;
	case EPathFollowingRequestResult::AlreadyAtGoal:
		UE_LOG(LogTemp, Warning, TEXT("UAP_Task_MoveNearLocation::Activate() request = AlreadyAtGoal"));
		OnTargetLocationReached.Broadcast();
		break;
	case EPathFollowingRequestResult::RequestSuccessful:
		UE_LOG(LogTemp, Warning, TEXT("UAP_Task_MoveNearLocation::Activate() request = RequestSuccessful"));
		Controller->ReceiveMoveCompleted.Add(OnMovementCompletedDelegate);
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("UAP_Task_MoveNearLocation::Activate() request = unknown"));
		break;
	}
}

void UAP_Task_MoveNearLocation::OnDestroy(bool AbilityIsEnding)
{
	UE_LOG(LogTemp, Warning, TEXT("UAP_Task_MoveNearLocation::OnDestroy() AbilityIsEnding = %d"), AbilityIsEnding);
	ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
	//check(Character);
	if (!Character) return;
	AAIController* Controller = Character->GetController<AAIController>();
	//check(Controller);
	if (!Controller) return;
	Controller->ReceiveMoveCompleted.Remove(OnMovementCompletedDelegate);
}

void UAP_Task_MoveNearLocation::OnMovementCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	UE_LOG(LogTemp, Warning, TEXT("UAP_Task_MoveNearLocation::OnMovementCompleted()"));
	ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
	//check(Character);
	if (!Character) return;
	AAIController* Controller = Character->GetController<AAIController>();
	//check(Controller);
	if (!Controller) return;
	// broadcast result back to blueprint
	switch (Result)
	{
	case EPathFollowingResult::Type::Success:
		// success
		OnTargetLocationReached.Broadcast();
		Controller->ReceiveMoveCompleted.Remove(OnMovementCompletedDelegate);
		UE_LOG(LogTemp, Warning, TEXT("UAP_Task_MoveNearLocation::OnMovementCompleted() Success"));
		break;
	case EPathFollowingResult::Type::Blocked:
	case EPathFollowingResult::Type::OffPath:
		// do it again
		UE_LOG(LogTemp, Warning, TEXT("UAP_Task_MoveNearLocation::OnMovementCompleted() Blocked/OffPath"));
		Controller->MoveToLocation(TargetLocation, AcceptanceRadius);
		break;
	case EPathFollowingResult::Type::Aborted:
	case EPathFollowingResult::Type::Invalid:
		// cancelled or something?
		UE_LOG(LogTemp, Warning, TEXT("UAP_Task_MoveNearLocation::OnMovementCompleted() Aborted/Invalid"));
		OnMovementCancelled.Broadcast();
		Controller->ReceiveMoveCompleted.Remove(OnMovementCompletedDelegate);
		break;
	}
}
