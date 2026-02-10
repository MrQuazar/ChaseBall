#include "ProjectileChaseBall.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

AProjectileChaseBall::AProjectileChaseBall()
{
	PrimaryActorTick.bCanEverTick = true;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->SetSphereRadius(BaseRadius);
	Collision->SetCollisionProfileName(TEXT("NoCollision"));
	Collision->SetNotifyRigidBodyCollision(true);
	Collision->OnComponentHit.AddDynamic(this, &AProjectileChaseBall::OnHit);

	DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
	DetectionSphere->SetupAttachment(RootComponent);
	DetectionSphere->SetSphereRadius(BaseRadius);
	DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DetectionSphere->SetCollisionResponseToAllChannels(ECR_Overlap);
	DetectionSphere->SetGenerateOverlapEvents(true);
	DetectionSphere->OnComponentBeginOverlap.AddDynamic(
		this, &AProjectileChaseBall::OnDetectEnemy);

	NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
	NiagaraComp->SetupAttachment(Collision);
	NiagaraComp->SetAutoActivate(true);


}

void AProjectileChaseBall::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(LifeTime);
	if (ProjectileVFX)
	{
		NiagaraComp->SetAsset(ProjectileVFX);	
		NiagaraComp->Activate();
	}
}

void AProjectileChaseBall::CreateChargeShot(float ChargeAlpha)
{
	ChargeAlpha = FMath::Clamp(ChargeAlpha, 0.f, 1.f);

	// Scale radius
	const float NewRadius = FMath::Lerp(MinRadius, MaxRadius, ChargeAlpha);
	Collision->SetSphereRadius(NewRadius);
	DetectionSphere->SetSphereRadius(NewRadius*1.5f);
	const float ScaleFactor = NewRadius / BaseRadius;

	// Scale speed (inverse)
	MoveSpeed = FMath::Lerp(BaseSpeed, MinSpeed, ChargeAlpha);

	// Scale Niagara effect
	NiagaraComp->SetWorldScale3D(FVector(ScaleFactor));
	NiagaraComp->SetVariableFloat(TEXT("User.GlobalScale"), ScaleFactor* NiagaraScaleConst);
}

float AProjectileChaseBall::GetAimAssistRadius() const
{
	return DetectionSphere->GetScaledSphereRadius();
}

void AProjectileChaseBall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (TargetActor)
	{
		MoveTowardsTarget(DeltaTime);
	}
	else if(CanMove)
	{
		const FVector NewLocation =
			GetActorLocation() + InitialDirection * MoveSpeed * DeltaTime;

		SetActorLocation(NewLocation, true);
	}
}

void AProjectileChaseBall::SetTarget(AActor* NewTarget)
{
	if (!NewTarget)
	{
		APawn* OwnerPawn = Cast<APawn>(GetOwner());
		if (!OwnerPawn)
			return;
		APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());

		if (PC)
		{
			FVector CamLocation;
			FRotator CamRotation;
			PC->GetPlayerViewPoint(CamLocation, CamRotation);

			InitialDirection = CamRotation.Vector();
			CanMove = true;
		}
	}
	else {
		TargetActor = NewTarget;
		NiagaraComp->SetVariableLinearColor(TEXT("User.OriginalColor"), FLinearColor(0,1,1,1));
		NiagaraComp->SetVariableLinearColor(TEXT("User.OriginalSmokeColor"), FLinearColor::Black);
		NiagaraComp->SetVariableLinearColor(TEXT("User.OriginalEmberColorMin"), FLinearColor::Blue);
		NiagaraComp->SetVariableLinearColor(TEXT("User.OriginalEmberColorMax"), FLinearColor::White);
	}
	Collision->SetCollisionProfileName(TEXT("BlockAllDynamic"));
}

void AProjectileChaseBall::MoveTowardsTarget(float DeltaTime)
{
	if (!TargetActor)
		return;

	const FVector Direction =
		(TargetActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();

	const FVector NewLocation =
		GetActorLocation() + Direction * MoveSpeed * DeltaTime;

	SetActorLocation(NewLocation, true);
}

void AProjectileChaseBall::OnDetectEnemy(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 BodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!TargetActor && OtherActor && OtherActor->ActorHasTag(TEXT("Enemy")))
	{
		TargetActor = OtherActor;
	}
}

void AProjectileChaseBall::OnHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (OtherActor && OtherActor != this)
	{
		// Damage / effects here if you want
		Destroy();
	}
}