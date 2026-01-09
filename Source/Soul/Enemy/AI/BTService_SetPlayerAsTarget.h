#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_SetPlayerAsTarget.generated.h"

UCLASS()
class SOUL_API UBTService_SetPlayerAsTarget : public UBTService_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTService_SetPlayerAsTarget();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};