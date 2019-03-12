// Fill out your copyright notice in the Description page of Project Settings.

#include "AP_TargetActor_Hero.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Abilities/GameplayAbility.h"
#include "DrawDebugHelpers.h"

void AAP_TargetActor_Hero::BeginPlay()
{
	Super::BeginPlay();
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.SetTickFunctionEnable(true);
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
	// workaround for an engine bug, turn the tick function off and on again
	RegisterAllActorTickFunctions(false, false);
	RegisterAllActorTickFunctions(true, false);
	bDestroyOnConfirmation = true;
}

void AAP_TargetActor_Hero::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);
	UE_LOG(LogTemp, Warning, TEXT("AAP_TargetActor_Hero::StartTargeting() %d"));
	GetWorld()->GetFirstPlayerController()->CurrentMouseCursor = EMouseCursor::Crosshairs;
	LastHeroHighlighted = nullptr;
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
			HitResult.Actor = NULL;
			Handle = StartLocation.MakeTargetDataHandleFromHitResult(OwningAbility, HitResult);
		}
		TargetDataReadyDelegate.Broadcast(Handle);
		GetWorld()->GetFirstPlayerController()->CurrentMouseCursor = GetWorld()->GetFirstPlayerController()->DefaultMouseCursor;
		TurnOffTargetHighlight();
	}
}
void AAP_TargetActor_Hero::CancelTargeting()
{
	Super::CancelTargeting();
	GetWorld()->GetFirstPlayerController()->CurrentMouseCursor = GetWorld()->GetFirstPlayerController()->DefaultMouseCursor;
	TurnOffTargetHighlight();
}
FHitResult AAP_TargetActor_Hero::PerformTrace()
{
	//UE_LOG(LogTemp, Warning, TEXT("AAP_TargetActor_Hero::PerformTrace()"));
	// make hit result from cursor
	FHitResult HitResult;
	FVector MouseLoc;
	FVector MouseRot;
	GetWorld()->GetFirstPlayerController()->DeprojectMousePositionToWorld(MouseLoc, MouseRot);;
	const FVector StartTrace = MouseLoc;
	const FVector ShootDir = MouseRot;
	const FVector EndTrace = StartTrace + ShootDir * 2000.0f;
	
	ECollisionChannel Channel(ECollisionChannel::ECC_GameTraceChannel2);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(HeroTargetingOverlap), false);
	//GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, Channel);
	GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, Channel, Params);
	return HitResult;
}
void AAP_TargetActor_Hero::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//UE_LOG(LogTemp, Warning, TEXT("AAP_TargetActor_Hero::Tick()"));
	if (IsConfirmTargetingAllowed())
	{
		FHitResult HitResult = PerformTrace();
		FVector EndPoint = HitResult.Component.IsValid() ? HitResult.ImpactPoint : HitResult.TraceEnd;

#if ENABLE_DRAW_DEBUG
		if (bDebug)
		{
			//UE_LOG(LogTemp, Warning, TEXT("AAP_TargetActor_Hero::Tick() DrawDebugSphere"));
			if (HitResult.bBlockingHit)
			{
				DrawDebugSphere(GetWorld(), EndPoint, 16, 10, FColor::Green, false);
			}
			else
			{
				DrawDebugSphere(GetWorld(), EndPoint, 16, 10, FColor::Red, false);
			}
		}
#endif // ENABLE_DRAW_DEBUG
		TWeakObjectPtr<AActor> HitActor = HitResult.Actor;

		AAP_Hero* Hero = Cast<AAP_Hero>(HitActor);
		if (Hero)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Tick(), Nice, hit a hero"));
			if (LastHeroHighlighted && LastHeroHighlighted != Hero)
			{
				//UE_LOG(LogTemp, Warning, TEXT("Tick(), turn off highlight for last hero"));
				LastHeroHighlighted->GetMesh()->SetRenderCustomDepth(false);
				LastHeroHighlighted = Hero;
				//UE_LOG(LogTemp, Warning, TEXT("Tick(), turn on highlight for current hero"));
				Hero->GetMesh()->SetRenderCustomDepth(true);
			}
			else
			{
				//UE_LOG(LogTemp, Warning, TEXT("Tick(), turn on highlight for newly found hero"));
				Hero->GetMesh()->SetRenderCustomDepth(true);
				LastHeroHighlighted = Hero;
			}
		}
		else
		{
			//UE_LOG(LogTemp, Warning, TEXT("Tick(), Hit nothing"));
			TurnOffTargetHighlight();
		}
		// update targeting effect here, draw a circle indicating the target location?
	}
}

bool AAP_TargetActor_Hero::ShouldProduceTargetData() const
{
	bool LocallyOwned = true;// TODO: make this true only for local player
	return LocallyOwned || ShouldProduceTargetDataOnServer;
}

void AAP_TargetActor_Hero::TurnOffTargetHighlight()
{
	if (LastHeroHighlighted)
	{
		UE_LOG(LogTemp, Warning, TEXT("TurnOffTargetHighlight(), turn off highlight for last hero"));
		LastHeroHighlighted->GetMesh()->SetRenderCustomDepth(false);
		LastHeroHighlighted = nullptr;
	}
}
