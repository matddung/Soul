#include "BTService_SetPlayerAsTarget.h"

#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_SetPlayerAsTarget::UBTService_SetPlayerAsTarget()
{
    NodeName = "Set Player As Target";
}

void UBTService_SetPlayerAsTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AActor* Player = UGameplayStatics::GetPlayerPawn(OwnerComp.GetWorld(), 0);
    if (!Player)
    {
        return;
    }

    OwnerComp.GetBlackboardComponent()->SetValueAsObject(GetSelectedBlackboardKey(), Player);
}