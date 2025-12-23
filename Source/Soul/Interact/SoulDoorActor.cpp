#include "SoulDoorActor.h"
#include "../Character/SoulCharacter.h"

#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

ASoulDoorActor::ASoulDoorActor()
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(false);

	FrameMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrameMesh"));
	SetRootComponent(FrameMesh);

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(RootComponent);

	InteractBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractBox"));
	InteractBox->SetupAttachment(RootComponent);
	InteractBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractBox->SetBoxExtent(FVector(60, 20, 120));

	PortalTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("PortalTrigger"));
	PortalTrigger->SetupAttachment(RootComponent);
	PortalTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PortalTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	PortalTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PortalTrigger->SetBoxExtent(FVector(60, 20, 120));
}

void ASoulDoorActor::BeginPlay()
{
	Super::BeginPlay();

	DoorStartRot = DoorMesh->GetRelativeRotation();
	DoorTargetRot = DoorStartRot + FRotator(0, OpenYawDelta, 0);

	InteractBox->OnComponentBeginOverlap.AddDynamic(this, &ASoulDoorActor::OnInteractBoxBeginOverlap);
	InteractBox->OnComponentEndOverlap.AddDynamic(this, &ASoulDoorActor::OnInteractBoxEndOverlap);

	PortalTrigger->OnComponentBeginOverlap.AddDynamic(this, &ASoulDoorActor::OnPortalBeginOverlap);
}

void ASoulDoorActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bOpening)
	{
		return;
	}

	OpenElapsed += DeltaSeconds;
	const float Alpha = FMath::Clamp(OpenElapsed / OpenDuration, 0, 1);

	const FRotator NewRot = FMath::Lerp(DoorStartRot, DoorTargetRot, Alpha);
	DoorMesh->SetRelativeRotation(NewRot);

	if (Alpha >= 1)
	{
		bOpening = false;
		bOpened = true;

		PortalTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		SetActorTickEnabled(false);
	}
}

void ASoulDoorActor::Interact_Implementation(ASoulCharacter* Interactor)
{
	if (!Interactor)
	{
		return;
	}

	if (bOpened || bOpening)
	{
		return;
	}

	CachedInteractor = Interactor;

	Interactor->SetWeaponType(EWeaponType::Empty);

	Interactor->FaceToActor(this);

	Interactor->OnAutoFaceEnd.RemoveAll(this);
	Interactor->OnAutoFaceEnd.AddUObject(this, &ASoulDoorActor::OnInteractorAutoFaceEnd);

	InteractBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

bool ASoulDoorActor::CanInteract_Implementation(ASoulCharacter* Interactor) const
{
	return !bOpened && !bOpening;
}

FText ASoulDoorActor::GetInteractText_Implementation() const
{
	return FText::FromString(TEXT("F: Open Door"));
}

void ASoulDoorActor::StartOpen()
{
	bOpening = true;
	OpenElapsed = 0;

	DoorStartRot = DoorMesh->GetRelativeRotation();
	DoorTargetRot = DoorStartRot + FRotator(0, OpenYawDelta, 0);

	SetActorTickEnabled(true);
}

void ASoulDoorActor::OnInteractorAutoFaceEnd()
{
	if (bOpened || bOpening)
	{
		return;
	}

	if (!CachedInteractor.IsValid())
	{
		return;
	}

	CachedInteractor->PlayOpenDoorAnim();
	StartOpen();
}

void ASoulDoorActor::OnInteractBoxBeginOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (bOpened || bOpening)
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

void ASoulDoorActor::OnInteractBoxEndOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32)
{
	ASoulCharacter* Player = Cast<ASoulCharacter>(OtherActor);

	if (!Player)
	{
		return;
	}

	Player->ClearInteractTarget(this);
}

void ASoulDoorActor::OnPortalBeginOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	ASoulCharacter* Player = Cast<ASoulCharacter>(OtherActor);

	if (!Player)
	{
		return;
	}

	if (TargetLevelName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("Door portal has no TargetLevelName set."));
		return;
	}

	UGameplayStatics::OpenLevel(GetWorld(), TargetLevelName);
}