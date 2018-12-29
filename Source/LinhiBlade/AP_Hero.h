// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbility.h"
#include "AP_Hero.generated.h"

UCLASS()
class LINHIBLADE_API AAP_Hero : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAP_Hero();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController * NewController);
	virtual void OnRep_Controller() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UAbilitySystemComponent* GetAbilitySystemComponent() const override
	{
		return AbilitySystem;
	} //We add this function, overriding it from IAbilitySystemInterface.
	/**
	 * Attempts to activate any ability in the specified item slot. Will return false if no activatable ability found or activation fails
	 * Returns true if it thinks it activated, but it may return false positives due to failure later in activation.
	 * If bAllowRemoteActivation is true, it will remotely activate local/server abilities, if false it will only try to locally activate the ability
	 */
	UFUNCTION(BlueprintCallable, Category = "Abilities")
		bool ActivateAbilitySlot(int32 AbilitySlot, bool bAllowRemoteActivation = true);

	/** Apply the startup gameplay abilities and effects */
	void AddStartupGameplayAbilities();
protected:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;

	/** Our ability system */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Abilities, meta = (AllowPrivateAccess = "true"))
		class UAbilitySystemComponent* AbilitySystem;

	/** The level of this character, should not be modified directly once it has already spawned */
	UPROPERTY(EditAnywhere, Replicated, Category = Abilities)
		int32 CharacterLevel;

	/** Abilities to grant to this character on creation. These will be activated by tag or event and are not bound to specific inputs */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Abilities)
		TArray<TSubclassOf<class UGameplayAbility>> GameplayAbilities;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Abilities)
		TArray<FGameplayAbilitySpecHandle> GameplayAbilityHandles;

	/** If true we have initialized our abilities */
	UPROPERTY()
		int32 bAbilitiesInitialized;

};

//Example for an enum the FGameplayAbiliyInputBinds may use to map input to ability slots.

//It's very important that this enum is UENUM, because the code will look for UENUM by the given name and crash if the UENUM can't be found. BlueprintType is there so we can use these in blueprints, too. Just in case. Can be neat to define ability packages.
UENUM(BlueprintType)
enum class AbilityInput : uint8
{
	UseAbility1 UMETA(DisplayName = "Use Spell 1"), //This maps the first ability(input ID should be 0 in int) to the action mapping(which you define in the project settings) by the name of "UseAbility1". "Use Spell 1" is the blueprint name of the element.
	UseAbility2 UMETA(DisplayName = "Use Spell 2"), //Maps ability 2(input ID 1) to action mapping UseAbility2. "Use Spell 2" is mostly used for when the enum is a blueprint variable.
	UseAbility3 UMETA(DisplayName = "Use Spell 3"),
	UseAbility4 UMETA(DisplayName = "Use Spell 4"),
	WeaponAbility UMETA(DisplayName = "Use Weapon"), //This finally maps the fifth ability(here designated to be your weaponability, or auto-attack, or whatever) to action mapping "WeaponAbility".

		//You may also do something like define an enum element name that is not actually mapped to an input, for example if you have a passive ability that isn't supposed to have an input. This isn't usually necessary though as you usually grant abilities via input ID,
		//which can be negative while enums cannot. In fact, a constant called "INDEX_NONE" exists for the exact purpose of rendering an input as unavailable, and it's simply defined as -1.
		//Because abilities are granted by input ID, which is an int, you may use enum elements to describe the ID anyway however, because enums are fancily dressed up ints.
};