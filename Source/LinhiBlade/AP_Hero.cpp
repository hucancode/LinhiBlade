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
	UE_LOG(LogTemp, Warning, TEXT("about to activate ability %d, valid=%d"), SpellSlot, SpellAbilityHandles.IsValidIndex(SpellSlot));
	if (SpellAbilityHandles.IsValidIndex(SpellSlot) && AbilitySystem)
	{
		// If bAllowRemoteActivation is true, it will remotely activate local / server abilities, if false it will only try to locally activate the ability
		bool ret = AbilitySystem->TryActivateAbility(SpellAbilityHandles[SpellSlot], true);
		if (ret)
		{
			SpellCastDelegate.Broadcast(SpellSlot);
		}
		UE_LOG(LogTemp, Warning, TEXT("activate ability, ret = %d"), ret);
	}
}

float AAP_Hero::GetSpellCooldown(int SpellSlot)
{
	UE_LOG(LogTemp, Warning, TEXT("about to check ability cooldown %d, valid=%d"), SpellSlot, SpellAbilityHandles.IsValidIndex(SpellSlot));
	float ret = 0.0f;
	if (SpellAbilityHandles.IsValidIndex(SpellSlot) && AbilitySystem)
	{
		UGameplayAbility* ability = AbilitySystem->GetActivatableAbilities()[SpellSlot].Ability;
		// ret = ability->GetCooldownTimeRemaining();// this won't work
		const FGameplayTagContainer* CooldownTags = ability->GetCooldownTags();
		if (CooldownTags && CooldownTags->Num() > 0)
		{
			FGameplayEffectQuery const Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(*CooldownTags);
			TArray< float > Durations = AbilitySystem->GetActiveEffectsTimeRemaining(Query);
			if (Durations.Num() > 0)
			{
				Durations.Sort();
				ret = Durations[Durations.Num() - 1];
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("return %f"), ret);
	return ret;
}

// Called every frame
void AAP_Hero::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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