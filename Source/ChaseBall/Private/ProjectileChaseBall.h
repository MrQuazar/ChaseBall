#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileChaseBall.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class CHASEBALL_API AProjectileChaseBall : public AActor
{
	GENERATED_BODY()

public:
	AProjectileChaseBall();

	virtual void Tick(float DeltaTime) override;

	void SetTarget(AActor* NewTarget);

	float GetAimAssistRadius() const;

	/** Called by ShooterComponent after spawning */
	void CreateChargeShot(float ChargeAlpha);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chase")
	float MoveSpeed = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chase")
	float BaseSpeed = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chase")
	float MinSpeed = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chase")
	float BaseRadius = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chase")
	float MinRadius = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chase")
	float MaxRadius = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Lifetime")
	float LifeTime = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Chase")
	bool CanMove = false;

	FVector InitialDirection;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	USphereComponent* Collision;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* DetectionSphere;

	UPROPERTY()
	AActor* TargetActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Visual")
	UMaterialInterface* NormalMat;

	UPROPERTY(EditAnywhere, Category = "Visual")
	UMaterialInterface* TargetMat;


	void MoveTowardsTarget(float DeltaTime);

	UFUNCTION()
	void OnHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);

	UFUNCTION()
	void OnDetectEnemy(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 BodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};