#include "EnemyAIController.h"
#include "../EnemyBase.h"

#include "BehaviorTree/BlackboardComponent.h"

AEnemyAIController::AEnemyAIController()
{
    PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
    SetPerceptionComponent(*PerceptionComp);

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

    PerceptionComp->ConfigureSense(*SightConfig);
    PerceptionComp->SetDominantSense(UAISense_Sight::StaticClass());

    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (BlackboardAsset)
    {
        UBlackboardComponent* BBComp = nullptr;
        UseBlackboard(BlackboardAsset, BBComp);
    }

    if (BehaviorTreeAsset)
    {
        RunBehaviorTree(BehaviorTreeAsset);
    }

    AEnemyBase* Enemy = Cast<AEnemyBase>(InPawn);
    if (!Enemy)
    {
        return;
    }

    const UEnemyAIConfig* Config = Enemy->AIConfig;
    if (!Config)
    {
        return;
    }

    ApplyAIConfig(Config);

    PerceptionComp->RequestStimuliListenerUpdate();
}

void AEnemyAIController::ApplyAIConfig(const UEnemyAIConfig* Config)
{
    EnemyType = Config->EnemyType;
    PatrolMode = Config->PatrolMode;

    AggroRange = Config->AggroRange;
    AttackRange = Config->AttackRange;
    PatrolRadius = Config->PatrolRadius;

    if (UBlackboardComponent* BB = GetBlackboardComponent())
    {
        BB->SetValueAsEnum(TEXT("EnemyType"),  (uint8)EnemyType);
        BB->SetValueAsEnum(TEXT("PatrolMode"), (uint8)PatrolMode);
        BB->SetValueAsFloat(TEXT("AggroRange"), AggroRange);
        BB->SetValueAsFloat(TEXT("AttackRange"), AttackRange);
        BB->SetValueAsFloat(TEXT("PatrolRadius"), PatrolRadius);
        BB->SetValueAsFloat(TEXT("ChaseStopRange"), FMath::Max(0.f, AttackRange - 50.f));
    }

    if (SightConfig)
    {
        SightConfig->SightRadius = Config->SightRange;
        SightConfig->LoseSightRadius = Config->LoseSightRadius;
        SightConfig->PeripheralVisionAngleDegrees = Config->SightFOV;

        if (PerceptionComp)
        {
            PerceptionComp->ConfigureSense(*SightConfig);
            PerceptionComp->SetDominantSense(UAISense_Sight::StaticClass());
            PerceptionComp->RequestStimuliListenerUpdate();
        }
    }
}