#include "SoulLadderActor.h"
#include "../Character/SoulCharacter.h"

#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"

ASoulLadderActor::ASoulLadderActor()
{
	PrimaryActorTick.bCanEverTick = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    LadderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LadderMesh"));
    LadderMesh->SetupAttachment(Root);

    LadderForward = CreateDefaultSubobject<UArrowComponent>(TEXT("LadderForward"));
    LadderForward->SetupAttachment(Root);

    BottomInteractBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BottomInteractBox"));
    BottomInteractBox->SetupAttachment(LadderMesh);
    BottomInteractBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    BottomInteractBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    BottomInteractBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    BottomInteractBox->SetBoxExtent(FVector(30, 30, 50));
    BottomInteractBox->SetRelativeLocation(FVector(0, 0, 40));
    BottomInteractBox->SetGenerateOverlapEvents(true);

    TopInteractBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TopInteractBox"));
    TopInteractBox->SetupAttachment(LadderMesh);
    TopInteractBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TopInteractBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    TopInteractBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    TopInteractBox->SetBoxExtent(FVector(30, 30, 50));
    TopInteractBox->SetRelativeLocation(FVector(0, 0, 220));
    TopInteractBox->SetGenerateOverlapEvents(true);

    BottomPoint = CreateDefaultSubobject<USceneComponent>(TEXT("BottomPoint"));
    BottomPoint->SetupAttachment(LadderMesh);

    TopPoint = CreateDefaultSubobject<USceneComponent>(TEXT("TopPoint"));
    TopPoint->SetupAttachment(LadderMesh);

    TopMountStartPoint = CreateDefaultSubobject<USceneComponent>(TEXT("TopMountStartPoint"));
    TopMountStartPoint->SetupAttachment(LadderMesh);
    TopMountStartPoint->SetRelativeLocation(FVector(0, 0, 240));
}

void ASoulLadderActor::BeginPlay()
{
	Super::BeginPlay();
	
    BottomInteractBox->OnComponentBeginOverlap.AddDynamic(this, &ASoulLadderActor::OnBottomBeginOverlap);
    BottomInteractBox->OnComponentEndOverlap.AddDynamic(this, &ASoulLadderActor::OnBottomEndOverlap);

    TopInteractBox->OnComponentBeginOverlap.AddDynamic(this, &ASoulLadderActor::OnTopBeginOverlap);
    TopInteractBox->OnComponentEndOverlap.AddDynamic(this, &ASoulLadderActor::OnTopEndOverlap);
}

void ASoulLadderActor::Interact_Implementation(ASoulCharacter* Interactor)
{
    if (!Interactor)
    {
        return;
    }

    if (Interactor->IsOnLadder())
    {
        Interactor->EndLadder();
        return;
    }

    Interactor->SetWeaponType(EWeaponType::Empty);
    Interactor->BeginLadder(this);
}

bool ASoulLadderActor::CanInteract_Implementation(ASoulCharacter* Interactor) const
{
    return Interactor != nullptr;
}

FText ASoulLadderActor::GetInteractText_Implementation() const
{
    return FText::FromString(TEXT("F: Use Ladder"));
}

void ASoulLadderActor::OnBottomBeginOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    if (ASoulCharacter* Player = Cast<ASoulCharacter>(OtherActor))
    {
        if (Player->IsOnLadder())
        {
            return;
        }

        LastUseSide = ELadderUseSide::Bottom;
        Player->SetInteractTarget(this);
    }
}

void ASoulLadderActor::OnTopBeginOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    if (ASoulCharacter* Player = Cast<ASoulCharacter>(OtherActor))
    {
        if (Player->IsOnLadder())
        {
            return;
        }

        LastUseSide = ELadderUseSide::Top;
        Player->SetInteractTarget(this);
    }
}

void ASoulLadderActor::OnBottomEndOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32)
{
    if (ASoulCharacter* Player = Cast<ASoulCharacter>(OtherActor))
    {
        Player->ClearInteractTarget(this);
        LastUseSide = ELadderUseSide::None;
    }
}

void ASoulLadderActor::OnTopEndOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32)
{
    if (ASoulCharacter* Player = Cast<ASoulCharacter>(OtherActor))
    {
        Player->ClearInteractTarget(this);
        LastUseSide = ELadderUseSide::None;
    }
}

void ASoulLadderActor::GetClimbZRange(float& OutMinZ, float& OutMaxZ) const
{
    const float BottomZ = BottomPoint ? BottomPoint->GetComponentLocation().Z : GetActorLocation().Z;
    const float TopZ = TopPoint ? TopPoint->GetComponentLocation().Z : GetActorLocation().Z + 300;

    OutMinZ = FMath::Min(BottomZ, TopZ);
    OutMaxZ = FMath::Max(BottomZ, TopZ);
}

void ASoulLadderActor::GetSnapTransform(const ASoulCharacter* Character, FVector& OutLoc, FRotator& OutRot) const
{
    const FVector Forward = LadderForward ? LadderForward->GetForwardVector() : GetActorForwardVector();

    const FVector FaceDir = -Forward;
    OutRot = FaceDir.Rotation();

    FVector Desired = GetActorLocation() + FaceDir * SnapDistanceFromLadder;

    float MinZ, MaxZ;
    GetClimbZRange(MinZ, MaxZ);

    float Z = Character ? Character->GetActorLocation().Z : Desired.Z;
    Z = FMath::Clamp(Z, MinZ, MaxZ);
    Desired.Z = Z;

    OutLoc = Desired;
}

FVector ASoulLadderActor::GetForward() const
{
    return LadderForward ? LadderForward->GetForwardVector() : GetActorForwardVector();
}

FVector ASoulLadderActor::GetTopExitLocation() const
{
    const FVector Forward = GetForward();
    return TopPoint->GetComponentLocation() + Forward * ExitForwardDistance;
}

FVector ASoulLadderActor::GetBottomExitLocation() const
{
    const FVector Forward = GetForward();
    return BottomPoint->GetComponentLocation() + Forward * BottomExitForwardDistance;
}

FVector ASoulLadderActor::GetTopMountStartLocation() const
{
    return TopMountStartPoint ? TopMountStartPoint->GetComponentLocation() : GetActorLocation();
}

FRotator ASoulLadderActor::GetTopMountStartRotation() const
{
    const FVector Forward = -GetForward();
    return (-Forward).Rotation();
}