// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/NetSerialization.h"
#include "LagCompensationComponent.generated.h"

/*
This  component supports the character owner class, but can be rewritten to support the Actor class.
*/

class ACharacter;
class UCapsuleComponent;
class UKismetSystemLibrary;
class UCharacterMovementComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogLagCompensationComponent, Log, All);

USTRUCT(BlueprintType)
struct FConditionInfo
{
	GENERATED_BODY()

public:

	UPROPERTY()
	FVector_NetQuantize OwnerPosition;

	UPROPERTY()
	uint8 bIsCrouch : 1;

	UPROPERTY()
	float TimeStamp;
	
	//Constructor
	FConditionInfo()
	{
		OwnerPosition = FVector(0.0f, 0.0f, 0.0f);
		bIsCrouch = false;
		TimeStamp = 0;
	}

	//Constructor
	FConditionInfo(FVector_NetQuantize InOwnerPosition, bool InbIsCrouch, float InTimeStamp)
	{
		OwnerPosition = InOwnerPosition;
		bIsCrouch = InbIsCrouch;
		TimeStamp = InTimeStamp;
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LAGCOMPENSATION_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULagCompensationComponent();

	/*Enable component logic */
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "LagCompensationComponen|Init")
	bool bEnable;

	/*Enable Debug */
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "LagCompensationComponen|Init")
	bool bDebug;

	/*Time period in seconds */
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "LagCompensationComponen|Init")
	float LocationRecordPeriod;

	/*Record rate in seconds */
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "LagCompensationComponen|Init")
	float LocationRecordRate;

	/*Allowable error between server and client*/
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "LagCompensationComponen|Init")
	float AllowableError;

	/*Show debug hit box */
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "LagCompensationComponen|Debug", meta = (EditCondition = "bDebug == true"))
	bool bHitBoxDebug;

	/*Debug draw time*/
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "LagCompensationComponen|Debug", meta = (EditCondition = "bDebug == true"))
	float DebugDrawTime;

	/*Debug wireframe thickness */
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "LagCompensationComponen|Debug", meta = (EditCondition = "bDebug == true"))
	float Thickness;

	/*Show all server positions, works in build  */
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "LagCompensationComponen|Debug", meta = (EditCondition = "bDebug == true"))
	bool bShowAllCompensationTrack;

	/*Show current server positions, works in build too*/
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "LagCompensationComponen|Debug", meta = (EditCondition = "bDebug == true"))
	bool bEnableServerDebugPosition;

	/*Show current client positions, works in build too*/
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "LagCompensationComponen|Debug", meta = (EditCondition = "bDebug == true"))
	bool bEnableClientDebugPosition;

	/*Server position wireframe color */
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "LagCompensationComponen|Debug", meta = (EditCondition = "bDebug == true"))
	FLinearColor ServerPositionColor;

	/*Client position wireframe color */
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "LagCompensationComponen|Debug", meta = (EditCondition = "bDebug == true"))
	FLinearColor ClientPositionColor;	

private:

	/*Only for debuging, works in build too */
	UPROPERTY(ReplicatedUsing = "OnRep_OwnerConditionArray", BlueprintReadOnly, Category = "LagCompensationComponen|Private", meta = (AllowPrivateAccess = "true"))
	TArray<FConditionInfo> OwnerConditionArray;

	/*Time for timestamps */
	float Time;

	/*Character owner */
	UPROPERTY()
	ACharacter const* OwnerCharacter;

public:

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "LagCompensationComponen|Control")
	void EnableLagCompensation(bool bEnableCompensation = false);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly,  Category = "LagCompensationComponen|Control")
	bool CheckPreviousPosition(UPARAM(ref)const FVector &LocationToCheck);
		
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION()
	void RecordLocation();

	void StopTimersAndClearValues();

	UFUNCTION(BlueprintCallable, Category = "LagCompensationComponen|Control")
	void DrawPositionAtClients(const FConditionInfo Condition, FLinearColor ColorToDraw, float LineThickness);

	UFUNCTION(BlueprintCallable, Category = "LagCompensationComponen|Control")
	void DrawAllCompensationPositionsAtClients(FLinearColor Color);

	UFUNCTION(BlueprintCallable, Category = "LagCompensationComponen|Control")
	bool InitCurrentCondition(FConditionInfo &CurrentCondition);

	UFUNCTION()
	void OnRep_OwnerConditionArray();

	FTimerHandle Timer_RecordPosition;
	FTimerHandle Timer_ResetWritePosition;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//Replicator
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//End play
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);
};
