// Fill out your copyright notice in the Description page of Project Settings.

#include "AP_TargetActor_GroundPoint.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Abilities/GameplayAbility.h"
#include "DrawDebugHelpers.h"

void AAP_TargetActor_GroundPoint::StartTargeting(UGameplayAbility* Ability)
{
	UE_LOG(LogTemp, Warning, TEXT("AAP_TargetActor_GroundPoint::StartTargeting()"));
	Super::StartTargeting(Ability);
	GetWorld()->GetFirstPlayerController()->CurrentMouseCursor = EMouseCursor::Crosshairs;
	// change cursor, init targetting effect
}

void AAP_TargetActor_GroundPoint::ConfirmTargetingAndContinue()
{
	// check if this is a local controller or this is server. currently crash so disabled
	//check(ShouldProduceTargetData());
	UE_LOG(LogTemp, Warning, TEXT("AAP_TargetActor_GroundPoint::ConfirmTargetingAndContinue()"));
	if (IsConfirmTargetingAllowed())
	{
		FGameplayAbilityTargetDataHandle Handle = StartLocation.MakeTargetDataHandleFromHitResult(OwningAbility, PerformTrace());
		TargetDataReadyDelegate.Broadcast(Handle);
		GetWorld()->GetFirstPlayerController()->CurrentMouseCursor = GetWorld()->GetFirstPlayerController()->DefaultMouseCursor;
	}

}
FHitResult AAP_TargetActor_GroundPoint::PerformTrace()
{
	UE_LOG(LogTemp, Warning, TEXT("AAP_TargetActor_GroundPoint::PerformTrace()"));
	// make hit result from cursor
	FHitResult HitResult;
	FVector MouseLoc;
	FVector MouseRot;
	UE_LOG(LogTemp, Warning, TEXT("AAP_TargetActor_GroundPoint::PerformTrace() GetFirstPlayerController"));
	GetWorld()->GetFirstPlayerController()->DeprojectMousePositionToWorld(MouseLoc, MouseRot);;
	const FVector StartTrace = MouseLoc;
	const FVector ShootDir = MouseRot;
	const FVector EndTrace = StartTrace + ShootDir * 2000.0f;
	UE_LOG(LogTemp, Warning, TEXT("AAP_TargetActor_GroundPoint::PerformTrace() LineTraceSingleByChannel"));
	GetWorld()->LineTraceSingleByChannel(HitResult,
		StartTrace, EndTrace, ECollisionChannel::ECC_Visibility);// not sure if ECC_Visibility or ECC_WorldStatic
	return HitResult;
}
void AAP_TargetActor_GroundPoint::Tick(float DeltaSeconds)
{
	UE_LOG(LogTemp, Warning, TEXT("AAP_TargetActor_GroundPoint::Tick()"));
	// very temp - do a mostly hardcoded trace from the source actor
	if (SourceActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("AAP_TargetActor_GroundPoint::Tick() if true"));
		FHitResult HitResult = PerformTrace();
		FVector EndPoint = HitResult.Component.IsValid() ? HitResult.ImpactPoint : HitResult.TraceEnd;

#if ENABLE_DRAW_DEBUG
		if (bDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("AAP_TargetActor_GroundPoint::Tick() DrawDebugSphere"));
			DrawDebugSphere(GetWorld(), EndPoint, 16, 10, FColor::Green, false);
		}
#endif // ENABLE_DRAW_DEBUG
		// update targeting effect here, draw a circle indicating the target location?
	}
}