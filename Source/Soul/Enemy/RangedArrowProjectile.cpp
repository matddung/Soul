#include "RangedArrowProjectile.h"
#include "../Character/SoulCharacter.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ARangedArrowProjectile::ARangedArrowProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

    Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    Collision->InitSphereRadius(5.f);
    Collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    Collision->SetCollisionObjectType(ECC_WorldDynamic);
    Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
    Collision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    Collision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
    Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

    Collision->SetNotifyRigidBodyCollision(true);
    Collision->OnComponentHit.AddDynamic(this, &ARangedArrowProjectile::OnHit);
    RootComponent = Collision;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(RootComponent);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->SetUpdatedComponent(Collision);
    ProjectileMovement->InitialSpeed = 3000.f;
    ProjectileMovement->MaxSpeed = 3000.f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;
    ProjectileMovement->bInitialVelocityInLocalSpace = false;

    InitialLifeSpan = 10.f;
}

void ARangedArrowProjectile::BeginPlay()
{
    Super::BeginPlay();

    if (AActor* OwnerActor = GetOwner())
    {
        Collision->IgnoreActorWhenMoving(OwnerActor, true);
    }

    if (APawn* Inst = GetInstigator())
    {
        Collision->IgnoreActorWhenMoving(Inst, true);
    }
}

void ARangedArrowProjectile::InitProjectile(AController* InInstigatorController, AActor* InDamageCauser, float InDamage)
{
    InstigatorController = InInstigatorController;
    Damage = InDamage;

    SetOwner(InDamageCauser);
}

void ARangedArrowProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!IsValid(OtherActor) || OtherActor == GetOwner())
    {
        return;
    }

    if (!Cast<ASoulCharacter>(OtherActor))
    {
        return;
    }

    UGameplayStatics::ApplyPointDamage(OtherActor, Damage, GetActorForwardVector(), Hit, InstigatorController, this, nullptr);

    Destroy();
}