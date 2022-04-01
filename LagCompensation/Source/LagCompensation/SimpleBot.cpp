// Fill out your copyright notice in the Description page of Project Settings.


#include "SimpleBot.h"
#include "Components/LagCompensationComponent.h"

DEFINE_LOG_CATEGORY(LogSimpleBot);

// Sets default values
ASimpleBot::ASimpleBot()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	LagCompensationComponent = CreateDefaultSubobject<ULagCompensationComponent>("LagCompensationComponent");
}

// Called when the game starts or when spawned
void ASimpleBot::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASimpleBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASimpleBot::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

float ASimpleBot::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	ApplyDamage(DamageAmount);

	return DamageAmount;
}
