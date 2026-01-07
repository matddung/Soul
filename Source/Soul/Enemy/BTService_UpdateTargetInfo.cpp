#include "BTService_UpdateTargetInfo.h"
#include "EnemyAIController.h"

UBTService_UpdateTargetInfo::UBTService_UpdateTargetInfo()
{
    NodeName = TEXT("Update Target Info");
    Interval = 0.2f;
    RandomDeviation = 0.0f;
}

void UBTService_UpdateTargetInfo::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AEnemyAIController* EnemyController = Cast<AEnemyAIController>(OwnerComp.GetAIOwner());
    if (!EnemyController)
    {
        return;
    }

    EnemyController->RefreshTargetInfo();
}