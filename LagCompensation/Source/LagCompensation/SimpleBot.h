// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SimpleBot.generated.h"

class ULagCompensationComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogSimpleBot, Log, All);

UCLASS()
class LAGCOMPENSATION_API ASimpleBot : public ACharacter
{
	GENERATED_BODY()

protected:

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "SimpleBot|Components")
	ULagCompensationComponent* LagCompensationComponent;

public:

	// Sets default values for this character's properties
	ASimpleBot();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	//Debug
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintAuthorityOnly, Category = "SimpleBot|Damage")
	void ApplyDamage(float Damage);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
