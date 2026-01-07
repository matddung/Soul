#include "EnemyAIController.h"
#include "../Character/SoulCharacter.h"
#include "../Character/SoulCharacterStatComponent.h"
#include "EnemyAIConfig.h"
#include "EnemyBase.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"

const FName AEnemyAIController::Key_TargetActor(TEXT("TargetActor"));
const FName AEnemyAIController::Key_LastKnownTargetLocation(TEXT("LastKnownTargetLocation"));
const FName AEnemyAIController::Key_HasLineOfSight(TEXT("HasLineOfSight"));
const FName AEnemyAIController::Key_DistanceToTarget(TEXT("DistanceToTarget"));
const FName AEnemyAIController::Key_HomeLocation(TEXT("HomeLocation"));
const FName AEnemyAIController::Key_EnemyType(TEXT("EnemyType"));
const FName AEnemyAIController::Key_PatrolMode(TEXT("PatrolMode"));
const FName AEnemyAIController::Key_PatrolRadius(TEXT("PatrolRadius"));
const FName AEnemyAIController::Key_AggroRange(TEXT("AggroRange"));
const FName AEnemyAIController::Key_AttackRange(TEXT("AttackRange"));

AEnemyAIController::AEnemyAIController()
{
    PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
    SetPerceptionComponent(*PerceptionComp);

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

    PerceptionComp->ConfigureSense(*SightConfig);
    PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());

    BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
}

void AEnemyAIController::BeginPlay()
{
    Super::BeginPlay();

    if (PerceptionComp)
    {
        PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIController::HandleTargetPerceptionUpdated);
    }

    CachedPlayerPawn = GetWorld() ? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr;
    BindToPlayerDeath();

    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(PerceptionUpdateTimer, this, &AEnemyAIController::UpdateTargeting, 0.2f, true);
    }
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (BehaviorTreeAsset)
    {
        RunBehaviorTree(BehaviorTreeAsset);
        BlackboardComp = GetBlackboardComponent();
    }

    if (BlackboardComp && InPawn)
    {
        BlackboardComp->SetValueAsVector(Key_HomeLocation, InPawn->GetActorLocation());
    }

    ApplyConfigToPerception();
    ApplyConfigToBlackboard();
}

void AEnemyAIController::BindToPlayerDeath()
{
    if (!CachedPlayerPawn)
    {
        return;
    }

    const ASoulCharacter* PlayerCharacter = Cast<ASoulCharacter>(CachedPlayerPawn);
    if (!PlayerCharacter)
    {
        return;
    }

    if (USoulCharacterStatComponent* StatComp = PlayerCharacter->FindComponentByClass<USoulCharacterStatComponent>())
    {
        StatComp->OnDead.AddDynamic(this, &AEnemyAIController::HandlePlayerDead);
    }
}

void AEnemyAIController::RefreshTargetInfo()
{
    UpdateTargeting();
}

AActor* AEnemyAIController::GetNextPatrolPoint()
{
    APawn* MyPawn = GetPawn();
    if (!MyPawn)
    {
        return nullptr;
    }

    AEnemyBase* Enemy = Cast<AEnemyBase>(MyPawn);
    if (!Enemy)
    {
        return nullptr;
    }

    if (Enemy->PatrolRoutePoints.IsEmpty())
    {
        return nullptr;
    }

    if (!Enemy->PatrolRoutePoints.IsValidIndex(Enemy->PatrolRouteIndex))
    {
        Enemy->PatrolRouteIndex = 0;
    }

    AActor* PatrolPoint = Enemy->PatrolRoutePoints[Enemy->PatrolRouteIndex].Get();
    Enemy->PatrolRouteIndex = (Enemy->PatrolRouteIndex + 1) % Enemy->PatrolRoutePoints.Num();

    return PatrolPoint;
}

bool AEnemyAIController::TryClearTargetIfDead()
{
    if (!BlackboardComp)
    {
        return false;
    }

    AActor* CurrentTarget = Cast<AActor>(BlackboardComp->GetValueAsObject(Key_TargetActor));
    if (CurrentTarget && IsTargetDead(CurrentTarget))
    {
        ClearTarget();
        return true;
    }

    return false;
}

void AEnemyAIController::HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!AIConfig || !Actor || Actor != CachedPlayerPawn)
    {
        return;
    }

    if (Stimulus.WasSuccessfullySensed())
    {
        if (AIConfig->EnemyType != EEnemyType::Boss)
        {
            SetTarget(Actor);
        }
    }
    else
    {
        if (BlackboardComp)
        {
            BlackboardComp->SetValueAsBool(Key_HasLineOfSight, false);
        }
    }
}

void AEnemyAIController::HandlePlayerDead()
{
    ClearTarget();
}

void AEnemyAIController::ApplyConfigToPerception()
{
    if (!AIConfig || !PerceptionComp || !SightConfig)
    {
        return;
    }

    SightConfig->SightRadius = AIConfig->SightRange;
    SightConfig->LoseSightRadius = AIConfig->LoseSightRadius;
    SightConfig->PeripheralVisionAngleDegrees = AIConfig->SightFOV;

    PerceptionComp->ConfigureSense(*SightConfig);
    PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
    PerceptionComp->RequestStimuliListenerUpdate();
}

void AEnemyAIController::ApplyConfigToBlackboard()
{
    if (!AIConfig || !BlackboardComp)
    {
        return;
    }

    BlackboardComp->SetValueAsEnum(Key_EnemyType, static_cast<uint8>(AIConfig->EnemyType));
    BlackboardComp->SetValueAsEnum(Key_PatrolMode, static_cast<uint8>(AIConfig->PatrolMode));
    BlackboardComp->SetValueAsFloat(Key_PatrolRadius, AIConfig->PatrolRadius);
    BlackboardComp->SetValueAsFloat(Key_AggroRange, AIConfig->AggroRange);
    BlackboardComp->SetValueAsFloat(Key_AttackRange, AIConfig->AttackRange);
}

void AEnemyAIController::UpdateTargeting()
{
    if (!BlackboardComp)
    {
        return;
    }

    AActor* CurrentTarget = Cast<AActor>(BlackboardComp->GetValueAsObject(Key_TargetActor));
    if (CurrentTarget && IsTargetDead(CurrentTarget))
    {
        ClearTarget();
        return;
    }

    if (!CurrentTarget)
    {
        if (!CachedPlayerPawn || !AIConfig)
        {
            return;
        }

        const FVector PawnLocation = GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
        const float DistanceToPlayer = FVector::Dist(PawnLocation, CachedPlayerPawn->GetActorLocation());

        if (AIConfig->EnemyType == EEnemyType::Boss)
        {
            if (DistanceToPlayer <= AIConfig->AggroRange)
            {
                SetTarget(CachedPlayerPawn);
            }
        }
        else if (CanSeePlayer())
        {
            SetTarget(CachedPlayerPawn);
        }

        CurrentTarget = Cast<AActor>(BlackboardComp->GetValueAsObject(Key_TargetActor));
    }

    if (CurrentTarget)
    {
        const FVector PawnLocation = GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
        const float DistanceToTarget = FVector::Dist(PawnLocation, CurrentTarget->GetActorLocation());
        BlackboardComp->SetValueAsFloat(Key_DistanceToTarget, DistanceToTarget);

        const bool bHasLineOfSight = CanSeePlayer();
        BlackboardComp->SetValueAsBool(Key_HasLineOfSight, bHasLineOfSight);
        if (bHasLineOfSight)
        {
            BlackboardComp->SetValueAsVector(Key_LastKnownTargetLocation, CurrentTarget->GetActorLocation());
        }
    }
}

void AEnemyAIController::SetTarget(AActor* NewTarget)
{
    if (!BlackboardComp || !NewTarget)
    {
        return;
    }

    BlackboardComp->SetValueAsObject(Key_TargetActor, NewTarget);
    BlackboardComp->SetValueAsBool(Key_HasLineOfSight, true);
    BlackboardComp->SetValueAsVector(Key_LastKnownTargetLocation, NewTarget->GetActorLocation());
    SetFocus(NewTarget);
}

void AEnemyAIController::ClearTarget()
{
    if (!BlackboardComp)
    {
        return;
    }

    BlackboardComp->ClearValue(Key_TargetActor);
    BlackboardComp->SetValueAsBool(Key_HasLineOfSight, false);
    ClearFocus(EAIFocusPriority::Gameplay);
}

bool AEnemyAIController::IsTargetDead(const AActor* TargetActor) const
{
    const ASoulCharacter* PlayerCharacter = Cast<ASoulCharacter>(TargetActor);
    if (PlayerCharacter)
    {
        return PlayerCharacter->GetIsDead();
    }

    if (const USoulCharacterStatComponent* StatComp = TargetActor ? TargetActor->FindComponentByClass<USoulCharacterStatComponent>() : nullptr)
    {
        return StatComp->IsDead();
    }

    return false;
}

bool AEnemyAIController::CanSeePlayer() const
{
    if (!PerceptionComp || !CachedPlayerPawn)
    {
        return false;
    }

    FActorPerceptionBlueprintInfo PerceptionInfo;
    PerceptionComp->GetActorsPerception(CachedPlayerPawn, PerceptionInfo);

    for (const FAIStimulus& Stimulus : PerceptionInfo.LastSensedStimuli)
    {
        if (Stimulus.Type == UAISense::GetSenseID(UAISense_Sight::StaticClass()))
        {
            return Stimulus.WasSuccessfullySensed();
        }
    }

    return false;
}