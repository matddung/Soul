#include "SoulCharacterStatComponent.h"

USoulCharacterStatComponent::USoulCharacterStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void USoulCharacterStatComponent::BeginPlay()
{
	Super::BeginPlay();

	RecalculateDerivedStats(true);
}

int32 USoulCharacterStatComponent::GetLevel() const
{
	return STR + DEX + VIT + END;
}

int32 USoulCharacterStatComponent::GetCurrentInvestCost() const
{
	const float CostFloat = (float)BaseInvestCost * FMath::Pow(CostMultiplier, (float)InvestCount);
	return FMath::CeilToInt(CostFloat);
}

void USoulCharacterStatComponent::RecalculateDerivedStats(bool bKeepCurrentRatio)
{
	const float OldMaxHP = MaxHP;
	const float OldMaxStamina = MaxStamina;

	MaxHP = HP_Base + (float)(FMath::Max(1, VIT) - 1) * HP_PerVIT;
	MaxStamina = Stamina_Base + (float)(FMath::Max(1, END) - 1) * Stamina_PerEND;

	if (bKeepCurrentRatio)
	{
		const float HPRatio = (OldMaxHP > 0.f) ? (HP / OldMaxHP) : 1.f;
		const float StRatio = (OldMaxStamina > 0.f) ? (Stamina / OldMaxStamina) : 1.f;

		HP = FMath::Clamp(MaxHP * HPRatio, 0.f, MaxHP);
		Stamina = FMath::Clamp(MaxStamina * StRatio, 0.f, MaxStamina);
	}
	else
	{
		HP = FMath::Clamp(HP, 0.f, MaxHP);
		Stamina = FMath::Clamp(Stamina, 0.f, MaxStamina);
	}
}

bool USoulCharacterStatComponent::TryInvestStat(ECharacterStatType StatToIncrease)
{
	const int32 Cost = GetCurrentInvestCost();
	if (Souls < Cost)
	{
		return false;
	}

	Souls -= Cost;

	AddToStat(StatToIncrease, 1);

	InvestCount += 1;

	RecalculateDerivedStats(true);

	return true;
}

void USoulCharacterStatComponent::AddSouls(int32 Amount)
{
	Souls += FMath::Max(0, Amount);
}

bool USoulCharacterStatComponent::ApplyDamage(float DamageAmount)
{
	if (DamageAmount <= 0.f)
	{
		return false;
	}

	if (IsDead())
	{
		return false;
	}

	const float OldHP = HP;

	HP = FMath::Clamp(HP - DamageAmount, 0.f, MaxHP);

	if (HP <= 0.f && OldHP > 0.f)
	{
		OnDead.Broadcast();
	}

	return !FMath::IsNearlyEqual(OldHP, HP);
}

bool USoulCharacterStatComponent::IsDead() const
{
	return HP <= 0.f;
}

void USoulCharacterStatComponent::ResetCurrentToMax()
{
	RecalculateDerivedStats(false);
	HP = MaxHP;
	Stamina = MaxStamina;
}

int32 USoulCharacterStatComponent::GetStatRef(ECharacterStatType StatType) const
{
	switch (StatType)
	{
	case ECharacterStatType::STR: return STR;
	case ECharacterStatType::DEX: return DEX;
	case ECharacterStatType::VIT: return VIT;
	case ECharacterStatType::END: return END;
	default: return 0;
	}
}

void USoulCharacterStatComponent::AddToStat(ECharacterStatType StatType, int32 Delta)
{
	switch (StatType)
	{
	case ECharacterStatType::STR: STR = FMath::Max(1, STR + Delta); break;
	case ECharacterStatType::DEX: DEX = FMath::Max(1, DEX + Delta); break;
	case ECharacterStatType::VIT: VIT = FMath::Max(1, VIT + Delta); break;
	case ECharacterStatType::END: END = FMath::Max(1, END + Delta); break;
	default: break;
	}
}