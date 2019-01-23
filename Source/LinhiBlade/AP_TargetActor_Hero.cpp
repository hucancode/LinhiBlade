// Fill out your copyright notice in the Description page of Project Settings.

#include "AP_TargetActor_Hero.h"
#include "AP_Hero.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Abilities/GameplayAbility.h"
#include "DrawDebugHelpers.h"

void AAP_TargetActor_Hero::StartTargeting(UGameplayAbility* Ability)
{
	UE_LOG(LogTemp, Warning, TEXT("AAP_TargetActor_Hero::StartTargeting()"));
	Super::StartTargeting(Ability);
	GetWorld()->GetFirstPlayerController()->CurrentMouseCursor = EMouseCursor::Crosshairs;
	// change cursor, init targetting effect
}

void AAP_TargetActor_Hero::ConfirmTargetingAndContinue()
{
	// check if this is a local controller or this is server. currently crash so disabled
	//check(ShouldProduceTargetData());
	UE_LOG(LogTemp, Warning, TEXT("AAP_TargetActor_Hero::ConfirmTargetingAndContinue()"));
	if (IsConfirmTargetingAllowed())
	{	
		TArray <TWeakObjectPtr<AActor>> HitActors;
		FGameplayAbilityTargetDataHandle Handle;
		FHitResult HitResult = PerformTrace();
		TWeakObjectPtr<AActor> HitActor = HitResult.Actor;

		AAP_Hero* Hero = Cast<AAP_Hero>(HitActor);
		if (Hero)
		{
			UE_LOG(LogTemp, Warning, TEXT("ConfirmTargetingAndContinue(), Nice, hit a hero"));
			HitActors.Add(Hero);
			Handle = StartLocation.MakeTargetDataHandleFromActors(HitActors);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ConfirmTargetingAndContinue(), Hit nothing"));
			Handle = StartLocation.MakeTargetDataHandleFromHitResult(OwningAbility, HitResult);
		}

		TargetDataReadyDelegate.Broadcast(Handle);
		GetWorld()->GetFirstPlayerController()->CurrentMouseCursor = GetWorld()->GetFirstPlayerController()->DefaultMouseCursor;
	}

}
void AAP_TargetActor_Hero::CancelTargeting()
{
	Super::CancelTargeting();
	GetWorld()->GetFirstPlayerController()->CurrentMouseCursor = GetWorld()->GetFirstPlayerController()->DefaultMouseCursor;
}
FHitResult AAP_TargetActor_Hero::PerformTrace()
{
	UE_LOG(LogTemp, Warning, TEXT("AAP_TargetActor_Hero::PerformTrace()"));
	// make hit result from cursor
	FHitResult HitResult;
	FVector MouseLoc;
	FVector MouseRot;
	UE_LOG(LogTemp, Warning, TEXT("AAP_TargetActor_Hero::PerformTrace() GetFirstPlayerController"));
	GetWorld()->GetFirstPlayerController()->DeprojectMousePositionToWorld(MouseLoc, MouseRot);;
	const FVector StartTrace = MouseLoc;
	const FVector ShootDir = MouseRot;
	const FVector EndTrace = StartTrace + ShootDir * 2000.0f;
	UE_LOG(LogTemp, Warning, TEXT("AAP_TargetActor_Hero::PerformTrace() LineTraceSingleByChannel"));
	GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECollisionChannel::ECC_GameTraceChannel1);
	return HitResult;
}
void AAP_TargetActor_Hero::Tick(float DeltaSeconds)
{
	UE_LOG(LogTemp, Warning, TEXT("AAP_TargetActor_Hero::Tick()"));
	// very temp - do a mostly hardcoded trace from the source actor
	if (SourceActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("AAP_TargetActor_Hero::Tick() if true"));
		FHitResult HitResult = PerformTrace();
		FVector EndPoint = HitResult.Component.IsValid() ? HitResult.ImpactPoint : HitResult.TraceEnd;

#if ENABLE_DRAW_DEBUG
		if (bDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("AAP_TargetActor_Hero::Tick() DrawDebugSphere"));
			DrawDebugSphere(GetWorld(), EndPoint, 16, 10, FColor::Green, false);
		}
#endif // ENABLE_DRAW_DEBUG
		// update targeting effect here, draw a circle indicating the target location?
	}
}