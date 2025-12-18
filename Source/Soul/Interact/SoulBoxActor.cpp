#include "SoulBoxActor.h"
#include "../Character/SoulCharacter.h"

#include "Components/BoxComponent.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

ASoulBoxActor::ASoulBoxActor()
{
	PrimaryActorTick.bCanEverTick = false;

	ChestMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChestMesh"));
	SetRootComponent(ChestMesh);

	InteractBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractBox"));
	InteractBox->SetupAttachment(RootComponent);

	InteractBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractBox->SetCollisionObjectType(ECC_WorldDynamic);
	InteractBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	InteractBox->SetBoxExtent(FVector(600, 600, 200));
}

void ASoulBoxActor::BeginPlay()
{
	Super::BeginPlay();
	
	InteractBox->OnComponentBeginOverlap.AddDynamic(this, &ASoulBoxActor::OnInteractBoxBeginOverlap);
	InteractBox->OnComponentEndOverlap.AddDynamic(this, &ASoulBoxActor::OnInteractBoxEndOverlap);
}

void ASoulBoxActor::Interact_Implementation(ASoulCharacter* Interactor)
{
	if (bOpened)
	{
		return;
	}

	if (Interactor)
	{
		Interactor->FaceToActor(this);
	}

	bOpened = true;

	InteractBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (OpenParticle)
	{
		const FVector SpawnLoc = ChestMesh->GetComponentLocation();
		const FRotator SpawnRot = ChestMesh->GetComponentRotation();

		SpawnedParticle = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), OpenParticle, SpawnLoc, SpawnRot, true);
	}

	ChestMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetWorldTimerManager().SetTimer(DisappearTimerHandle, this, &ASoulBoxActor::FinishDisappear, DisappearDelay, false);
}

bool ASoulBoxActor::CanInteract_Implementation(ASoulCharacter* Interactor) const
{
	return !bOpened;
}

void ASoulBoxActor::FinishOpen()
{
	ChestMesh->SetVisibility(false, true);
	ChestMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SetLifeSpan(5.f);
}

void ASoulBoxActor::OnInteractBoxBeginOverlap(UPrimitiveComponent*, AActor* OtherActor,
	UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (bOpened) return;

	ASoulCharacter* Player = Cast<ASoulCharacter>(OtherActor);
	if (!Player) return;

	Player->SetInteractTarget(this);
}

void ASoulBoxActor::OnInteractBoxEndOverlap(UPrimitiveComponent*, AActor* OtherActor,
	UPrimitiveComponent*, int32)
{
	ASoulCharacter* Player = Cast<ASoulCharacter>(OtherActor);
	if (!Player) return;

	Player->ClearInteractTarget(this);
}

void ASoulBoxActor::FinishDisappear()
{
	ChestMesh->SetVisibility(false, true);

	if (SpawnedParticle)
	{
		SpawnedParticle->DeactivateSystem();
		SpawnedParticle = nullptr;
	}

	Destroy();
}