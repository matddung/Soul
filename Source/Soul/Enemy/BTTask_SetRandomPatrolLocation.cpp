#include "BTTask_SetRandomPatrolLocation.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h" 

UBTTask_SetRandomPatrolLocation::UBTTask_SetRandomPatrolLocation()
{
	NodeName = TEXT("Set Random Patrol Location");
}

void UBTTask_SetRandomPatrolLocation::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	BlackboardKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_SetRandomPatrolLocation, BlackboardKey));
}

EBTNodeResult::Type UBTTask_SetRandomPatrolLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!BlackboardComp || !AIController)
	{
		return EBTNodeResult::Failed;
	}

	const FVector HomeLocation = BlackboardComp->GetValueAsVector(HomeLocationKeyName);
	const float PatrolRadius = BlackboardComp->GetValueAsFloat(PatrolRadiusKeyName);

	if (PatrolRadius <= 0.0f)
	{
		return EBTNodeResult::Failed;
	}

	UWorld* World = AIController->GetWorld();
	if (!World)
	{
		return EBTNodeResult::Failed;
	}

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	if (!NavSys)
	{
		return EBTNodeResult::Failed;
	} 
	
	FNavLocation RandomLocation;
	if (!NavSys->GetRandomPointInNavigableRadius(HomeLocation, PatrolRadius, RandomLocation))
	{
		return EBTNodeResult::Failed;
	}

	BlackboardComp->SetValueAsVector(GetSelectedBlackboardKey(), RandomLocation.Location);

	return EBTNodeResult::Succeeded;
}