#include "BTTask_SetHomeLocation.h"
#include "EnemyAIController.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTTask_SetHomeLocation::UBTTask_SetHomeLocation()
{
    NodeName = TEXT("Set Home Location");
}

EBTNodeResult::Type UBTTask_SetHomeLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::ExecuteTask(OwnerComp, NodeMemory);

    AEnemyAIController* EnemyController = Cast<AEnemyAIController>(OwnerComp.GetAIOwner());
    if (!EnemyController)
    {
        return EBTNodeResult::Failed;
    }

    UBlackboardComponent* BlackboardComp = EnemyController->GetBlackboardComponent();
    APawn* ControlledPawn = EnemyController->GetPawn();
    if (!BlackboardComp || !ControlledPawn)
    {
        return EBTNodeResult::Failed;
    }

    static const FName Key_HomeLocation(TEXT("HomeLocation"));
    BlackboardComp->SetValueAsVector(Key_HomeLocation, ControlledPawn->GetActorLocation());
    return EBTNodeResult::Succeeded;
}