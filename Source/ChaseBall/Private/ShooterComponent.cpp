#include "ShooterComponent.h"
#include "ProjectileChaseBall.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UShooterComponent::UShooterComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetComponentTickEnabled(false);
}

void UShooterComponent::BeginPlay()
{
	Super::BeginPlay();

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
		return;

	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC)
		return;

	// Add input mapping context
	if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
		}
	}

	// Bind input
	if (UEnhancedInputComponent* EIC =
		Cast<UEnhancedInputComponent>(PC->InputComponent))
	{
		EIC->BindAction(FireAction, ETriggerEvent::Started,
			this, &UShooterComponent::StartCharge);

		EIC->BindAction(FireAction, ETriggerEvent::Completed,
			this, &UShooterComponent::ReleaseShot);
	}
}

void UShooterComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsCharging)
		return;

	ChargeTime += DeltaTime;

	if (!Projectile)
		return;

	const float ChargeAlpha = FMath::Clamp(ChargeTime / MaxChargeTime, 0.f, 1.f);
	Projectile->CreateChargeShot(ChargeAlpha);

	if (ChargeTime >= MaxChargeTime)
	{
		ChargeTime = MaxChargeTime;
		bIsCharging = false;
		SetComponentTickEnabled(false);
		Shoot();
	}
}

void UShooterComponent::StartCharge()
{
	bIsCharging = true;
	ChargeTime = 0.f;
	SpawnBall();
	SetComponentTickEnabled(true);
}

void UShooterComponent::ReleaseShot()
{
	if (!bIsCharging)
		return;

	bIsCharging = false;
	SetComponentTickEnabled(false);

	Shoot();
}

void UShooterComponent::Shoot()
{
	if (!ProjectileClass)
		return;

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
		return;

	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC)
		return;

	// Get camera data (center of screen)
	FVector CameraLocation;
	FRotator CameraRotation;
	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	const FVector TraceStart = CameraLocation;
	const FVector TraceEnd =
		TraceStart + (CameraRotation.Vector() * AimAssistDistance);
	AimAssistRadius = Projectile->GetAimAssistRadius();
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerPawn);

	AActor* TargetActor = nullptr;

	const bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeSphere(AimAssistRadius),
		Params
	);

	if (bHit && HitResult.GetActor())
	{
		if (HitResult.GetActor()->ActorHasTag(TEXT("Enemy")))
		{
			TargetActor = HitResult.GetActor();
		}
	}

	//FColor LineColor = TargetActor ? FColor::Red : FColor::Green;
	//DrawDebugCapsule(
	//	GetWorld(),
	//	(TraceStart + TraceEnd) * 0.5f,
	//	(TraceEnd - TraceStart).Size() * 0.5f,
	//	AimAssistRadius,
	//	FRotationMatrix::MakeFromZ(TraceEnd - TraceStart).ToQuat(),
	//	LineColor,
	//	false,
	//	2.0f
	//);


	// Assign target if valid enemy was hit
	Projectile->SetTarget(TargetActor);

}

void UShooterComponent::SpawnBall()
{
	if (!ProjectileClass)
		return;

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
		return;

	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC)
		return;

	// Get camera data (center of screen)
	FVector CameraLocation;
	FRotator CameraRotation;
	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	// Spawn projectile at player
	const FVector SpawnLocation =
		OwnerPawn->GetActorLocation() +
		CameraRotation.RotateVector(MuzzleOffset);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerPawn;
	SpawnParams.Instigator = OwnerPawn;

	Projectile =
		GetWorld()->SpawnActor<AProjectileChaseBall>(
			ProjectileClass,
			SpawnLocation,
			CameraRotation,
			SpawnParams
		);
}