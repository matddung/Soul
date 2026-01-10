#include "BTService_UpdateTargetAggroRange.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

UBTService_UpdateTargetAggroRange::UBTService_UpdateTargetAggroRange()
{
	NodeName = TEXT("Update Target In Aggro Range");
	Interval = 0.2f;
	RandomDeviation = 0.05f;
}

void UBTService_UpdateTargetAggroRange::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		return;
	}

	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
	{
		return;
	}

	APawn* SelfPawn = AICon->GetPawn();
	if (!SelfPawn)
	{
		return;
	}

	AActor* Player = UGameplayStatics::GetPlayerPawn(SelfPawn, 0);
	if (!IsValid(Player))
	{
		BB->ClearValue(TargetActorKey.SelectedKeyName);
		return;
	}

	const float AggroRange = BB->GetValueAsFloat(AggroRangeKey.SelectedKeyName);
	if (AggroRange <= 0.0f)
	{
		BB->ClearValue(TargetActorKey.SelectedKeyName);
		return;
	}

	const float DistSq = FVector::DistSquared(SelfPawn->GetActorLocation(), Player->GetActorLocation());
	const float AggroRangeSq = FMath::Square(AggroRange);
	if (DistSq <= AggroRangeSq)
	{
		BB->SetValueAsObject(TargetActorKey.SelectedKeyName, Player);
		BB->SetValueAsVector(TargetLocationKey.SelectedKeyName, Player->GetActorLocation());
	}
	else
	{
		BB->ClearValue(TargetActorKey.SelectedKeyName);
	}
}