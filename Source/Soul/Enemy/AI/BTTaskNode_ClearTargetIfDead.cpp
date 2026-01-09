#include "BTTaskNode_ClearTargetIfDead.h"
#include "../../Character/SoulCharacter.h"
#include "../../Character/SoulCharacterStatComponent.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTTaskNode_ClearTargetIfDead::UBTTaskNode_ClearTargetIfDead()
{
    NodeName = TEXT("Clear Target If Dead");
}

EBTNodeResult::Type UBTTaskNode_ClearTargetIfDead::ExecuteTask(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory)
{
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB)
    {
        return EBTNodeResult::Succeeded;
    }

    AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));

    if (!IsValid(TargetActor))
    {
        BB->ClearValue(TargetActorKey.SelectedKeyName);
        return EBTNodeResult::Succeeded;
    }

    if (const ASoulCharacter* PlayerCharacter = Cast<ASoulCharacter>(TargetActor))
    {
        if (PlayerCharacter->GetIsDead())
        {
            BB->ClearValue(TargetActorKey.SelectedKeyName);
        }
        return EBTNodeResult::Succeeded;
    }

    if (const USoulCharacterStatComponent* StatComp =
        TargetActor->FindComponentByClass<USoulCharacterStatComponent>())
    {
        if (StatComp->IsDead())
        {
            BB->ClearValue(TargetActorKey.SelectedKeyName);
        }
    }

    return EBTNodeResult::Succeeded;
}