// Fill out your copyright notice in the Description page of Project Settings.

#include "AP_Hero.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTargetActor.h"

// Sets default values
AAP_Hero::AAP_Hero()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	SelectionRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SelectionRing"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Plane'"));
	SelectionRing->SetStaticMesh(MeshAsset.Object);
	SelectionRing->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SelectionRing->SetCollisionProfileName(TEXT("NoCollision"));
	SelectionRing->SetVisibility(false);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Our ability system component.
	AbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
	AbilitySystem->OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &AAP_Hero::OnGameplayEffectAppliedToSelf);
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	AttributeSet = CreateDefaultSubobject<UAP_AttributeSet>(TEXT("AttributeSet"));
	
}

// Called when the game starts or when spawned
void AAP_Hero::BeginPlay()
{
	Super::BeginPlay();
	AddStartupGameplayAbilities();
	
}

void AAP_Hero::AddStartupGameplayAbilities()
{
	if (Role == ROLE_Authority && !bAbilitiesInitialized)
	{
		int32 Level = 1;
		SpellAbilityHandles.Empty();
		// Grant abilities, but only on the server	
		for (TSubclassOf<UGameplayAbility>& StartupAbility : SpellAbilities)
		{
			if (StartupAbility)
			{
				FGameplayAbilitySpecHandle handle = AbilitySystem->GiveAbility(FGameplayAbilitySpec(StartupAbility, Level, INDEX_NONE, this));
				SpellAbilityHandles.Add(handle);
				SpellStates.AddDefaulted();
			}
		}
		if (WeaponAbility)
		{
			WeaponAbilityHandle = AbilitySystem->GiveAbility(FGameplayAbilitySpec(WeaponAbility, Level, INDEX_NONE, this));
		}
		
		bAbilitiesInitialized = true;
	}
}
void AAP_Hero::WeaponAttack()
{
	UE_LOG(LogTemp, Warning, TEXT("about to activate weapon attack"));
	if (WeaponAbilityHandle.IsValid() && AbilitySystem)
	{
		// If bAllowRemoteActivation is true, it will remotely activate local / server abilities, if false it will only try to locally activate the ability
		bool ret = AbilitySystem->TryActivateAbility(WeaponAbilityHandle, true);
		UE_LOG(LogTemp, Warning, TEXT("activate ability, ret = %d"), ret);
	}
}
void AAP_Hero::SpellAttack(int SpellSlot)
{
	bool valid = SpellAbilityHandles.IsValidIndex(SpellSlot);
	UE_LOG(LogTemp, Warning, TEXT("about to activate ability %d, valid=%d"), SpellSlot, valid);
	if (valid && AbilitySystem)
	{
		// If bAllowRemoteActivation is true, it will remotely activate local / server abilities, if false it will only try to locally activate the ability
		bool ret = AbilitySystem->TryActivateAbility(SpellAbilityHandles[SpellSlot], true);
		if (ret)
		{
			SpellStates[SpellSlot] = ESpellState::Casting;
			SpellCastDelegate.Broadcast(SpellSlot);
		}
		UE_LOG(LogTemp, Warning, TEXT("activate ability, ret = %d"), ret);
	}
}

float AAP_Hero::GetSpellCooldown(int SpellSlot)
{
	bool valid = SpellAbilityHandles.IsValidIndex(SpellSlot);
	UE_LOG(LogTemp, Warning, TEXT("about to check ability cooldown %d, valid=%d"), SpellSlot, valid);
	float ret = 0.0f;
	if (valid && AbilitySystem)
	{
		if (!AbilitySystem->GetActivatableAbilities().IsValidIndex(SpellSlot))
		{
			return ret;
		}
		if (!AbilitySystem->GetActivatableAbilities()[SpellSlot].GetAbilityInstances().Num())
		{
			return ret;
		}
		UGameplayAbility* ability = AbilitySystem->GetActivatableAbilities()[SpellSlot].GetAbilityInstances().Last();
		ret = ability->GetCooldownTimeRemaining();
	}
	UE_LOG(LogTemp, Warning, TEXT("return %f"), ret);
	return ret;
}

float AAP_Hero::GetSpellCooldownPercent(int SpellSlot)
{
	bool valid = SpellAbilityHandles.IsValidIndex(SpellSlot);
	UE_LOG(LogTemp, Warning, TEXT("about to check ability cooldown percent %d, valid=%d"), SpellSlot, valid);
	float ret = 0.0f;
	if (valid && AbilitySystem)
	{
		if (!AbilitySystem->GetActivatableAbilities().IsValidIndex(SpellSlot))
		{
			return ret;
		}
		if (!AbilitySystem->GetActivatableAbilities()[SpellSlot].GetAbilityInstances().Num())
		{
			return ret;
		}
		UGameplayAbility* ability = AbilitySystem->GetActivatableAbilities()[SpellSlot].GetAbilityInstances().Last();
		FGameplayAbilitySpecHandle handle = SpellAbilityHandles[SpellSlot];
		float remaining;
		float duration;
		ability->GetCooldownTimeRemainingAndDuration(handle, ability->GetCurrentActorInfo(), remaining, duration);
		if (duration == 0.0f)
		{
			return ret;
		}
		ret = ret = remaining / duration;
	}
	UE_LOG(LogTemp, Warning, TEXT("return %f"), ret);
	return ret;
}

ESpellState AAP_Hero::GetSpellState(int SpellSlot)
{
	if (!SpellStates.IsValidIndex(SpellSlot))
	{
		return ESpellState::Disabled;
	}
	return SpellStates[SpellSlot];
}

// Called every frame
void AAP_Hero::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	for (size_t i = 0; i < SpellStates.Num(); i++)
	{
		if (!AbilitySystem->GetActivatableAbilities().IsValidIndex(i))
		{
			continue;
		}
		if (!AbilitySystem->GetActivatableAbilities()[i].GetAbilityInstances().Num())
		{
			continue;
		}
		UGameplayAbility* ability = AbilitySystem->GetActivatableAbilities()[i].GetAbilityInstances().Last();
		FGameplayAbilitySpecHandle handle = SpellAbilityHandles[i];
		
		if (SpellStates[i] == ESpellState::Casting)
		{
			if (!ability->CheckCooldown(handle, ability->GetCurrentActorInfo()))
			{
				SpellStates[i] = ESpellState::OnCooldown;
				SpellGoneCooldown.Broadcast(i);
			}
		}
		else if (SpellStates[i] == ESpellState::OnCooldown)
		{
			if (ability->CheckCooldown(handle, ability->GetCurrentActorInfo()))
			{
				SpellStates[i] = ESpellState::Ready;
				SpellOffCooldown.Broadcast(i);
			}
		}
	}

}

// Called to bind functionality to input
void AAP_Hero::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AAP_Hero::PossessedBy(AController * NewController)
{
	Super::PossessedBy(NewController);

	AbilitySystem->RefreshAbilityActorInfo();
}

void AAP_Hero::OnRep_Controller()
{
	Super::OnRep_Controller();

	// Our controller changed, must update ActorInfo on AbilitySystemComponent
	if (AbilitySystem)
	{
		AbilitySystem->RefreshAbilityActorInfo();
	}
}

void AAP_Hero::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(AAP_Hero, CharacterLevel);
}

UAbilitySystemComponent * AAP_Hero::GetAbilitySystemComponent() const
{
	return AbilitySystem;
}

void AAP_Hero::OnGameplayEffectAppliedToSelf(UAbilitySystemComponent * Source, const FGameplayEffectSpec & Spec, FActiveGameplayEffectHandle Handle)
{
	// add visual display for gameplay effect
	// if a invisible effect is applied, make character transparent
	// if a stun effect is applied, make character play stun animation
	// if an ice effect is applied, make character blue
	// ...
	GameplayEffectAppliedToSelf.Broadcast(Source, Spec, Handle);
}

void AAP_Hero::HandleDamage(float DamageAmount, const FHitResult& HitInfo, const struct FGameplayTagContainer& DamageTags, AAP_Hero* InstigatorPawn, AActor* DamageCauser)
{
	OnDamaged(DamageAmount, HitInfo, DamageTags, InstigatorPawn, DamageCauser);
}

void AAP_Hero::HandleHealthChanged(float DeltaValue, const struct FGameplayTagContainer& EventTags)
{
	// We only call the BP callback if this is not the initial ability setup
	if (bAbilitiesInitialized)
	{
		OnHealthChanged(DeltaValue, EventTags);
	}
}

void AAP_Hero::HandleManaChanged(float DeltaValue, const struct FGameplayTagContainer& EventTags)
{
	if (bAbilitiesInitialized)
	{
		OnManaChanged(DeltaValue, EventTags);
	}
}

void AAP_Hero::HandleMoveSpeedChanged(float DeltaValue, const struct FGameplayTagContainer& EventTags)
{
	// Update the character movement's walk speed
	GetCharacterMovement()->MaxWalkSpeed = GetMoveSpeed();

	if (bAbilitiesInitialized)
	{
		OnMoveSpeedChanged(DeltaValue, EventTags);
	}
}

void AAP_Hero::SelectHero(bool selected)
{
	SelectionRing->SetVisibility(selected);
}

float AAP_Hero::GetHealth() const
{
	return AttributeSet->GetHealth();
}

float AAP_Hero::GetMaxHealth() const
{
	return AttributeSet->GetMaxHealth();
}

float AAP_Hero::GetMana() const
{
	return AttributeSet->GetMana();
}

float AAP_Hero::GetMaxMana() const
{
	return AttributeSet->GetMaxMana();
}

float AAP_Hero::GetMoveSpeed() const
{
	return AttributeSet->GetMoveSpeed();
}

int32 AAP_Hero::GetCharacterLevel() const
{
	return CharacterLevel;
}

int32 AAP_Hero::GetSpellCount() const
{
	return SpellAbilityHandles.Num();
}

bool AAP_Hero::IsTargeting() const
{
	UE_LOG(LogTemp, Warning, TEXT("querying if hero is targeting or not"));
	for (size_t i = 0; i < AbilitySystem->SpawnedTargetActors.Num(); i++)
	{
		if (!AbilitySystem->SpawnedTargetActors[i]->IsActorBeingDestroyed())
		{
			UE_LOG(LogTemp, Warning, TEXT("return true at %d"), i);
			return true;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("return false"));
	return false;
}

bool AAP_Hero::SetCharacterLevel(int32 NewLevel)
{
	if (CharacterLevel != NewLevel && NewLevel > 0)
	{
		CharacterLevel = NewLevel;
		return true;
	}
	return false;
}