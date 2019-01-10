// Fill out your copyright notice in the Description page of Project Settings.

#include "AP_Task_LookAtLocation.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"


UAP_Task_LookAtLocation* UAP_Task_LookAtLocation::LookAtLocation(UGameplayAbility* OwningAbility, FName TaskInstanceName, FVector Location, float TurnRate)
{
	UAP_Task_LookAtLocation* MyObj = NewAbilityTask<UAP_Task_LookAtLocation>(OwningAbility, TaskInstanceName);
	MyObj->TargetLocation = Location;
	MyObj->TurnRate = TurnRate;
	UE_LOG(LogTemp, Warning, TEXT("UAP_Task_MoveNearLocation::LookAtLocation() %s"), *Location.ToCompactString());
	return MyObj;
}
UAP_Task_LookAtLocation::UAP_Task_LookAtLocation()
{
	Reached = false;
	bTickingTask = true;
}

UAP_Task_LookAtLocation::~UAP_Task_LookAtLocation()
{
}

void UAP_Task_LookAtLocation::Activate()
{
	Destination = UKismetMathLibrary::FindLookAtRotation(GetAvatarActor()->GetActorLocation(), TargetLocation);
	Destination.Pitch = 0.0f;
	Destination.Roll = 0.0f;
}

void UAP_Task_LookAtLocation::OnDestroy(bool AbilityIsEnding)
{
}

void UAP_Task_LookAtLocation::TickTask(float DeltaTime)
{
	if (!Reached)
	{
		ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
		//check(Character);
		if (!Character) return;
		AAIController* Controller = Character->GetController<AAIController>();
		//check(Controller);
		if (!Controller) return;
		EPathFollowingStatus::Type MoveStatus = Controller->GetMoveStatus();
		if (MoveStatus != EPathFollowingStatus::Idle)
		{
			UE_LOG(LogTemp, Warning, TEXT("UAP_Task_MoveNearLocation::TickTask() rotation cancelled, GetMoveStatus = %d"), MoveStatus);
			Reached = true;
			OnRotationCancelled.Broadcast();
			return;
		}
		FRotator Source = GetAvatarActor()->GetActorRotation();
		FRotator Delta = Destination - Source;
		if (UKismetMathLibrary::Abs(Delta.Yaw) > 180.0f)
		{
			Delta.Yaw = 180.0f - Delta.Yaw;
		}
		float MicroTurn = TurnRate * DeltaTime;
		if (UKismetMathLibrary::Abs(Delta.Yaw) < MicroTurn)
		{
			UE_LOG(LogTemp, Warning, TEXT("UAP_Task_MoveNearLocation::TickTask() rotation finished"));
			GetAvatarActor()->SetActorRotation(Destination);
			Reached = true;
			OnTargetRotationReached.Broadcast();
			return;
		}
		Source.Yaw += MicroTurn * UKismetMathLibrary::SignOfFloat(Delta.Yaw);
		GetAvatarActor()->SetActorRotation(Source);
	}
}
