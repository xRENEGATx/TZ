// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponFireComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetSystemLibrary.h"
#include "LagCompensationComponent.h"

DEFINE_LOG_CATEGORY(LogWeaponFireComponent);

// Sets default values for this component's properties
UWeaponFireComponent::UWeaponFireComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	//ShotOffset 
	ShotOffset = 30.0f;

	//Trace debug
	bDebugTrace = true;

	//TraceType
	TraceType = ETraceTypeQuery::TraceTypeQuery1;

	//Distance
	ShotDistance = 10000.0f;
}

void UWeaponFireComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	DOREPLIFETIME_CONDITION(UWeaponFireComponent, ConfirmedHit, COND_SkipOwner);
}

// Called when the game starts
void UWeaponFireComponent::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void UWeaponFireComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UWeaponFireComponent::Fire()
{
	const FVector ForwardVector = GetComponentRotation().Quaternion().GetForwardVector();
	const FVector StartLocation = GetComponentLocation() + (ForwardVector * ShotOffset);

	FHitResult HitResult = WeaponTrace(StartLocation, ForwardVector);

	if (HitResult.bBlockingHit == true)
	{
		//Spawn trail VFX and Impact VFX
		StartLocation;
		HitResult.ImpactPoint;
	}
	else
	{
		//Spawn trail VFX
		StartLocation;
		HitResult.TraceEnd;
	}

	if (HitResult.bBlockingHit == true && HitResult.GetActor() &&
		(HitResult.GetActor()->GetRemoteRole() == ROLE_Authority || HitResult.GetActor()->GetLocalRole() == ROLE_Authority))
	{
		FFireInfo FireInfo{StartLocation, HitResult.ImpactPoint, HitResult.GetActor()};

		SERVER_Fire(FireInfo);
	}
	else
	{
		SERVER_Confirm(StartLocation);
	}	
}

void UWeaponFireComponent::SERVER_Fire_Implementation(FFireInfo FireInfo)
{
	if (FireInfo.Target != nullptr)
	{
		//Try find LagCompensationComponent in target
		UActorComponent* FoundComponent = FireInfo.Target->FindComponentByClass(ULagCompensationComponent::StaticClass());
		ULagCompensationComponent* LagCompensationComponent = Cast<ULagCompensationComponent>(FoundComponent);

		if (LagCompensationComponent != nullptr)
		{
			if (LagCompensationComponent->CheckPreviousPosition(FireInfo.ImpactPoint) == true)
			{
				//Call delegate
				if (OnMakeDamage.IsBound() == true)
				{
					OnMakeDamage.Broadcast(FireInfo.Target);
				}

				FDamageEvent DamageEvent;
				
				//Make damage
				FireInfo.Target->TakeDamage(1.0f, DamageEvent, GetOwner()->GetInstigatorController(), GetOwner());
			}
		}
	}
	
	//Play VFX if not dedicated server
	if (GetNetMode() != NM_DedicatedServer)
	{
		//Spawn trail VFX and Impact VFX (FireInfo.ImpactPoint anyway)
	}

	SERVER_Confirm(FireInfo.StartVector);
}

FHitResult UWeaponFireComponent::WeaponTrace(const FVector &StartLocation, const FVector& ForwardVector)
{
	const FVector EndLocation = (ForwardVector * ShotDistance) + StartLocation;
	const TArray<AActor*> ActorToIgnore;
	FHitResult HitResult;
	EDrawDebugTrace::Type DebugTrace = (bDebugTrace == true) ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

	UKismetSystemLibrary::LineTraceSingle(this, StartLocation, EndLocation, TraceType, false, ActorToIgnore, DebugTrace, HitResult, true);

	return HitResult;
}

void UWeaponFireComponent::SERVER_Confirm_Implementation(FVector_NetQuantize StartLocation)
{
	//VFX for remote clients
	ConfirmedHit.StartLocation = StartLocation;
}

void UWeaponFireComponent::OnRep_ConfirmedHit()
{
	const FVector ForwardVector = GetComponentRotation().Quaternion().GetForwardVector();
	const FVector StartLocation = ConfirmedHit.StartLocation;

	FHitResult HitResult = WeaponTrace(StartLocation, ForwardVector);

	if (HitResult.bBlockingHit == true)
	{
		//Spawn trail VFX and Impact VFX
		StartLocation;
		HitResult.ImpactPoint;
	}
	else
	{
		//Spawn trail VFX
		StartLocation;
		HitResult.TraceEnd;
	}
}