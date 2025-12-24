#include "SoulBoxActor.h"
#include "../Character/SoulCharacter.h"

#include "Components/BoxComponent.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

ASoulBoxActor::ASoulBoxActor()
{
	PrimaryActorTick.bCanEverTick = false;

	BoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxMesh"));
	SetRootComponent(BoxMesh);

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
		Interactor->SetWeaponType(EWeaponType::Empty);
		Interactor->FaceToActor(this);
		Interactor->PlayOpenBoxAnim();
		Interactor->GiveGunFromBox();
	}

	bOpened = true;

	InteractBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (OpenParticle)
	{
		const FVector SpawnLoc = BoxMesh->GetComponentLocation();
		const FRotator SpawnRot = BoxMesh->GetComponentRotation();

		SpawnedParticle = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), OpenParticle, SpawnLoc, SpawnRot, true);
	}

	BoxMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetWorldTimerManager().SetTimer(DisappearTimerHandle, this, &ASoulBoxActor::FinishDisappear, DisappearDelay, false);
}

bool ASoulBoxActor::CanInteract_Implementation(ASoulCharacter* Interactor) const
{
	return !bOpened;
}

FText ASoulBoxActor::GetInteractText_Implementation() const
{
	return FText::FromString(TEXT("F: Open Box"));
}

void ASoulBoxActor::OnInteractBoxBeginOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (bOpened)
	{
		return;
	}

	ASoulCharacter* Player = Cast<ASoulCharacter>(OtherActor);

	if (!Player)
	{
		return;
	}

	Player->SetInteractTarget(this);
}

void ASoulBoxActor::OnInteractBoxEndOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32)
{
	ASoulCharacter* Player = Cast<ASoulCharacter>(OtherActor);

	if (!Player)
	{
		return;
	}

	Player->ClearInteractTarget(this);
}

void ASoulBoxActor::FinishDisappear()
{
	BoxMesh->SetVisibility(false, true);

	if (SpawnedParticle)
	{
		SpawnedParticle->DeactivateSystem();
		SpawnedParticle = nullptr;
	}

	Destroy();
}