// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Algo/Reverse.h"


DEFINE_LOG_CATEGORY(LogLagCompensationComponent);

// Sets default values for this component's properties
ULagCompensationComponent::ULagCompensationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	bEnable = true;
	bDebug = false;

	LocationRecordRate = 0.1f;
	LocationRecordPeriod = 1.0f;
	AllowableError = 10.0f;
	DebugDrawTime = 0.1f;
	Thickness = 1.0f;
	ServerPositionColor = FLinearColor::Blue;
	ClientPositionColor = FLinearColor::Yellow;

	bShowAllCompensationTrack = false;
	bEnableServerDebugPosition = true;
	bEnableClientDebugPosition = true;
	bHitBoxDebug = true;
}

//Replicator
void ULagCompensationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(ULagCompensationComponent, OwnerConditionArray);
}

// Called when the game starts
void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());

	if (OwnerCharacter == nullptr)
	{
		UE_LOG(LogLagCompensationComponent, Warning, TEXT("WARNING: Owner is not character. LogLagCompensationComponent supports only character class. Logic Stoped"));
		return;
	}

	if (GetOwnerRole() == ROLE_Authority)
	{
		EnableLagCompensation(bEnable);		
	}	
}

void ULagCompensationComponent::EnableLagCompensation_Implementation(bool bEnableCompensation)
{
	if (bEnableCompensation == true)
	{
		if (GetOwnerRole() == ROLE_Authority)
		{
			GetWorld()->GetTimerManager().SetTimer(Timer_RecordPosition, this, &ULagCompensationComponent::RecordLocation, LocationRecordRate, true);
		}
	}
	else
	{
		StopTimersAndClearValues();
	}	
}

// Called every frame
void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bEnable == true)
	{
		//RecordTime
		Time += DeltaTime;

		//If the value is very large
		if (Time >= 5000)
		{
			StopTimersAndClearValues();
			EnableLagCompensation(true);
		}
	}	
}

void ULagCompensationComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	//Prevent crit error
	StopTimersAndClearValues();

	Super::EndPlay(EndPlayReason);
}

void ULagCompensationComponent::StopTimersAndClearValues()
{
	GetWorld()->GetTimerManager().ClearTimer(Timer_RecordPosition);
	OwnerConditionArray.Empty();
}

void ULagCompensationComponent::RecordLocation()
{
	FConditionInfo CurrentCondition;
	InitCurrentCondition(CurrentCondition);
	int32 IndexToRemove = -1;

	if (OwnerConditionArray.Num() > 0)
	{
		for (int32 i = 0; i < OwnerConditionArray.Num(); ++i)
		{
			float DeltaTime = Time - OwnerConditionArray[i].TimeStamp;

			if ((DeltaTime) > LocationRecordPeriod)
			{
				IndexToRemove = i;
				break;
			}
		}

		if (IndexToRemove != -1)
		{
			OwnerConditionArray.RemoveAt(IndexToRemove);
			OwnerConditionArray.Emplace(CurrentCondition);
			//Call OnRep_ at server side
			OnRep_OwnerConditionArray();
		}
		else
		{
			OwnerConditionArray.Emplace(CurrentCondition);
		}
	}
	else
	{
		OwnerConditionArray.Emplace(CurrentCondition);
	}
}

void ULagCompensationComponent::OnRep_OwnerConditionArray()
{
	if (GetNetMode() != NM_DedicatedServer)
	{
		if (bEnableServerDebugPosition == true && bDebug == true && bShowAllCompensationTrack == false)
		{
			DrawPositionAtClients(OwnerConditionArray.Last(), ServerPositionColor, Thickness);
		}
		else if(bEnableServerDebugPosition == true && bDebug == true && bShowAllCompensationTrack == true)
		{
			DrawAllCompensationPositionsAtClients(FColor::Purple);
		}
		
		if (bEnableClientDebugPosition == true && bDebug == true)
		{
			//GetLocaly CurrentCondition
			FConditionInfo CurrentCondition;
			InitCurrentCondition(CurrentCondition);
			
			DrawPositionAtClients(CurrentCondition, ClientPositionColor, Thickness);
		}
	}
}

void ULagCompensationComponent::DrawPositionAtClients(const FConditionInfo Condition, FLinearColor ColorToDraw, float LineThickness)
{
	if (OwnerCharacter != nullptr)
	{
		const float ScaledCrouchCapsuleHalfHeight = OwnerCharacter->GetCharacterMovement()->CrouchedHalfHeight * OwnerCharacter->GetCapsuleComponent()->GetComponentScale().Z;
		const float HalfHeight = (Condition.bIsCrouch == true) ? ScaledCrouchCapsuleHalfHeight : OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		const float Radius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
		const FRotator Rotation = OwnerCharacter->GetCapsuleComponent()->GetComponentRotation();

		UKismetSystemLibrary::DrawDebugCapsule(this, Condition.OwnerPosition, HalfHeight, Radius, Rotation, ColorToDraw, DebugDrawTime, LineThickness);
	}
}

void ULagCompensationComponent::DrawAllCompensationPositionsAtClients(FLinearColor Color)
{
	if (OwnerConditionArray.Num() == 0)
	{
		return;
	}

	const TArray<FConditionInfo> CurruntConditionsArray = OwnerConditionArray;	
	
	const float NewDebugLineThickness = Thickness * 0.5f;
	bool FirstColor = false;

	for (int32 i = CurruntConditionsArray.Num() - 1; i != -1; --i)
	{
		if (FirstColor == false)
		{
			FirstColor = true;
			DrawPositionAtClients(CurruntConditionsArray[i], ServerPositionColor, Thickness);
		}
		
		DrawPositionAtClients(CurruntConditionsArray[i], Color, NewDebugLineThickness);
	}
}

bool ULagCompensationComponent::CheckPreviousPosition(UPARAM(ref)const FVector& LocationToCheck)
{
	if (OwnerCharacter == nullptr)
	{
		return false;
	}

	TArray<FConditionInfo> LocalConditionArray = OwnerConditionArray;	

	FConditionInfo CurrentCondition;
	InitCurrentCondition(CurrentCondition);

	LocalConditionArray.Emplace(CurrentCondition);
	
	for (int32 i = LocalConditionArray.Num() - 1; i != -1; --i)
	{
		// Get the component bounding box
		const FBox HitBox = OwnerCharacter->GetCapsuleComponent()->Bounds.GetBox();

		// calculate the box extent
		FVector BoxExtent = 0.5 * (HitBox.Max - HitBox.Min);

		//Set Hit box min 40 unit
		BoxExtent.X = FMath::Max(40.0f, BoxExtent.X) + AllowableError;
		BoxExtent.Y = FMath::Max(40.0f, BoxExtent.Y) + AllowableError;
		BoxExtent.Z = FMath::Max(40.0f, BoxExtent.Z) + AllowableError;

		// Box center
		const FVector BoxCenter = LocalConditionArray[i].OwnerPosition;

		const float CalculatedX = FMath::Abs(LocationToCheck.X - BoxCenter.X);
		const float CalculatedY = FMath::Abs(LocationToCheck.Y - BoxCenter.Y);
		const float CalculatedZ = FMath::Abs(LocationToCheck.Z - BoxCenter.Z);

		UKismetSystemLibrary::PrintString(this,
			"CalculatedX: " + FString::SanitizeFloat(CalculatedX) + 
			"CalculatedY: " + FString::SanitizeFloat(CalculatedY) +
			"CalculatedZ: " + FString::SanitizeFloat(CalculatedZ)
			, true, true, FLinearColor::Red, 5);

		// if we are within client tolerance
		if (CalculatedX <= BoxExtent.X &&
			CalculatedY <= BoxExtent.Y &&
			CalculatedZ <= BoxExtent.Z)
		{
			//Editor debug
			if (bHitBoxDebug == true)
			{
				UKismetSystemLibrary::DrawDebugBox(this, BoxCenter, BoxExtent, FColor::Red, FRotator::ZeroRotator, 5.0f, 1);
				UKismetSystemLibrary::DrawDebugSphere(this, LocationToCheck, 10.0f, 10, FColor::Green, 5.0f, 0.5f);
			}			

			UKismetSystemLibrary::PrintString(this,
				"CalculatedX: " + FString::SanitizeFloat(CalculatedX) +
				"CalculatedY: " + FString::SanitizeFloat(CalculatedY) +
				"CalculatedZ: " + FString::SanitizeFloat(CalculatedZ)
				, true, true, FLinearColor::Green, 5);

			return true;
		}
	}

	return false;
}

bool ULagCompensationComponent::InitCurrentCondition(FConditionInfo &CurrentCondition)
{
	if (OwnerCharacter == nullptr)
	{
		return false;
	}

	CurrentCondition.OwnerPosition = GetOwner()->GetActorLocation();
	CurrentCondition.bIsCrouch = OwnerCharacter->bIsCrouched;
	CurrentCondition.TimeStamp = Time;

	return true;
}