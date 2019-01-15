// Fill out your copyright notice in the Description page of Project Settings.

#include "AP_Hero.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "AbilitySystemComponent.h"

// Sets default values
AAP_Hero::AAP_Hero()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

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
bool AAP_Hero::ActivateAbilitySlot(int32 AbilitySlot, bool bAllowRemoteActivation)
{
	UE_LOG(LogTemp, Warning, TEXT("about to activate ability %d, valid=%d"), AbilitySlot, GameplayAbilityHandles.IsValidIndex(AbilitySlot));
	if (GameplayAbilityHandles.IsValidIndex(AbilitySlot) && AbilitySystem)
	{
		bool ret = AbilitySystem->TryActivateAbility(GameplayAbilityHandles[AbilitySlot], bAllowRemoteActivation);
		UE_LOG(LogTemp, Warning, TEXT("activate ability, ret = %d"), ret);
		return ret;
	}

	return false;
}
void AAP_Hero::AddStartupGameplayAbilities()
{
	if (Role == ROLE_Authority && !bAbilitiesInitialized)
	{
		GameplayAbilityHandles.Empty();
		// Grant abilities, but only on the server	
		for (TSubclassOf<UGameplayAbility>& StartupAbility : GameplayAbilities)
		{
			int32 level = 1;
			FGameplayAbilitySpecHandle handle = AbilitySystem->GiveAbility(FGameplayAbilitySpec(StartupAbility, level, INDEX_NONE, this));
			GameplayAbilityHandles.Add(handle);
		}
		bAbilitiesInitialized = true;
	}
}
;
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

bool AAP_Hero::SetCharacterLevel(int32 NewLevel)
{
	if (CharacterLevel != NewLevel && NewLevel > 0)
	{
		CharacterLevel = NewLevel;
		return true;
	}
	return false;
}