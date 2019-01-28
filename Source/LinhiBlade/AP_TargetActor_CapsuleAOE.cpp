// Fill out your copyright notice in the Description page of Project Settings.

#include "AP_TargetActor_CapsuleAOE.h"
#include "DrawDebugHelpers.h"

void AAP_TargetActor_CapsuleAOE::BeginPlay()
{
	Super::BeginPlay();
}
void AAP_TargetActor_CapsuleAOE::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);
	UE_LOG(LogTemp, Warning, TEXT("AAP_TargetActor_BoxAOE::StartTargeting()"));
	TArray <TWeakObjectPtr<AActor>> HitActors = PerformTrace();
	FGameplayAbilityTargetDataHandle Handle = StartLocation.MakeTargetDataHandleFromActors(HitActors);
	TargetDataReadyDelegate.Broadcast(Handle);
	Destroy();
}

bool AAP_TargetActor_CapsuleAOE::ShouldProduceTargetData() const
{
	bool LocallyOwned = true;// TODO: make this true only for local player
	return LocallyOwned || ShouldProduceTargetDataOnServer;
}
TArray<TWeakObjectPtr<AActor>> AAP_TargetActor_CapsuleAOE::PerformTrace()
{
	FCollisionObjectQueryParams ObjectToScan = FCollisionObjectQueryParams(ECC_GameTraceChannel1);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(BoxTargetingOverlap), false);
	Params.bReturnPhysicalMaterial = false;
	Params.bTraceAsyncScene = false;
	TArray<FOverlapResult> Overlaps;
	TArray<TWeakObjectPtr<AActor>>	HitActors;
	const float HalfHeight = 300.0f;
	FCollisionShape Shape = FCollisionShape::MakeCapsule(Radius, HalfHeight);
	GetWorld()->OverlapMultiByObjectType(Overlaps, Location, FQuat::Identity, ObjectToScan, Shape, Params);
	DrawDebugCapsule(GetWorld(), Location, HalfHeight, Radius, FQuat::Identity, FColor::Green, false, 1.5f);
	for (int32 i = 0; i < Overlaps.Num(); ++i)
	{
		//Should this check to see if these pawns are in the AimTarget list?
		APawn* PawnActor = Cast<APawn>(Overlaps[i].GetActor());
		if (PawnActor && !HitActors.Contains(PawnActor) && Filter.FilterPassesForActor(PawnActor))
		{
			HitActors.Add(PawnActor);
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("AAP_TargetActor_BoxAOE::PerformTrace() trace finished, hit %d targets"), HitActors.Num());
	return HitActors;
}
