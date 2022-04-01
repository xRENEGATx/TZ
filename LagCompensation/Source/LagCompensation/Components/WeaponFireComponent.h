// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Engine/NetSerialization.h"
#include "WeaponFireComponent.generated.h"

class UKismetSystemLibrary;
class ULagCompensationComponent;
class UGameplayStatics;

DECLARE_LOG_CATEGORY_EXTERN(LogWeaponFireComponent, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamage, AActor*, Target);

/*
* This component must be in weapon class, so there may be slight inaccuracies in the logic,
* since for this project the weapon logic implemented in the character class.
*/

USTRUCT(BlueprintType)
struct FFireInfo
{
	GENERATED_BODY()

public:
	
	UPROPERTY()
	FVector_NetQuantize StartVector;

	UPROPERTY()
	FVector_NetQuantize ImpactPoint;

	UPROPERTY()
	AActor* Target;

	//Constructor
	FFireInfo()
	{
		//Empty
	}	

	//Constructor
	FFireInfo(FVector_NetQuantize A, FVector_NetQuantize B, AActor* C)
	{
		StartVector = A;
		ImpactPoint = B;
		Target = C;
	}
};

USTRUCT(BlueprintType)
struct FConfirmedHitInfo
{
	GENERATED_BODY()

public:

	//Maybe add something else, for example spread

	UPROPERTY()
	FVector_NetQuantize StartLocation;	

	//Constructor
	FConfirmedHitInfo()
	{
		//Empty
	}

	//Constructor
	FConfirmedHitInfo(FVector_NetQuantize A)
	{
		StartLocation = A;
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LAGCOMPENSATION_API UWeaponFireComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponFireComponent();

	/** Shot offset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponFireComponent|Shot")
	float ShotOffset;

	/** Shot offset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponFireComponent|Shot")
	float ShotDistance;

	/** Trace channel*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponFireComponent|Shot")
	TEnumAsByte<enum ETraceTypeQuery> TraceType;

	/** Shot debug */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponFireComponent|Debug")
	bool bDebugTrace;

	//Delegate
	UPROPERTY(BlueprintAssignable)
	FOnDamage OnMakeDamage;

protected:
	/** Rep var for VFX simulation */
	UPROPERTY(ReplicatedUsing = "OnRep_ConfirmedHit")
	FConfirmedHitInfo ConfirmedHit;

public:

	//Client side fire
	UFUNCTION(BlueprintCallable, Category = "LagCompensationCharacter|Fire")
	void Fire();

	//Server side fire
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "LagCompensationCharacter|Fire")
	void SERVER_Fire(FFireInfo FireInfo);

	//Server side fire confirm. Used for VFX and ather effects
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "LagCompensationCharacter|Fire")
	void SERVER_Confirm(FVector_NetQuantize StartLocation);

	UFUNCTION()
	void OnRep_ConfirmedHit();

protected:

	//Weapon trace
	FHitResult WeaponTrace(const FVector& StartLocation, const FVector& ForwardVector);

	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//Replicator
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;		
};
