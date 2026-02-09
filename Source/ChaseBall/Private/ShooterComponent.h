#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShooterComponent.generated.h"

class AProjectileChaseBall;
class UInputAction;
class UInputMappingContext;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CHASEBALL_API UShooterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UShooterComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;
	AProjectileChaseBall* Projectile;

private:
	/** Charging system */
	void StartCharge();
	void ReleaseShot();
	void SpawnBall();
	void Shoot();

	float ChargeTime = 0.f;
	bool bIsCharging = false;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	float MaxChargeTime = 5.f; // seconds

protected:
	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = "Shooting")
	TSubclassOf<AProjectileChaseBall> ProjectileClass;

	/** Enhanced Input */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* InputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* FireAction;

	/** Spawn offset relative to owner */
	UPROPERTY(EditDefaultsOnly, Category = "Shooting")
	FVector MuzzleOffset = FVector(100.f, 0.f, 50.f);

	UPROPERTY(EditDefaultsOnly, Category = "Aim Assist")
	float AimAssistRadius = 75.f;

	UPROPERTY(EditDefaultsOnly, Category = "Aim Assist")
	float AimAssistDistance = 10000.f;
};