#include "BTService_UpdateTarget.h"

#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_UpdateTarget::UBTService_UpdateTarget()
{
    NodeName = TEXT("Update Target From Perception");
    Interval = 0.2f;
    RandomDeviation = 0.05f;
}

void UBTService_UpdateTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
    
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon)
    {
        return;
    }

    UAIPerceptionComponent* Perc = AICon->FindComponentByClass<UAIPerceptionComponent>();
    if (!Perc)
    {
        return;
    }

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB)
    {
        return;
    }

    TArray<AActor*> SeenActors;
    Perc->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), SeenActors);
    
    APawn* SelfPawn = AICon->GetPawn();
    if (!SelfPawn)
    {
        return;
    }

    AActor* Best = nullptr;
    float BestDistSq = TNumericLimits<float>::Max();

    for (AActor* A : SeenActors)
    {
        if (!IsValid(A) || A == SelfPawn)
        {
            continue;
        }

        APawn* SeenPawn = Cast<APawn>(A);
        if (!SeenPawn || !SeenPawn->IsPlayerControlled())
        {
            continue;
        }
        
        const float DistSq = FVector::DistSquared(SelfPawn->GetActorLocation(), A->GetActorLocation());
        if (DistSq < BestDistSq)
        {
            BestDistSq = DistSq;
            Best = A;
        }
    }

    if (Best)
    {
        BB->SetValueAsObject(TargetActorKey.SelectedKeyName, Best);
        BB->SetValueAsVector(TargetLocationKey.SelectedKeyName, Best->GetActorLocation());
    }
    else
    {
        BB->ClearValue(TargetActorKey.SelectedKeyName);
    }
}