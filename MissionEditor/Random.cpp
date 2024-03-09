#include "StdAfx.h"

#include "Random.h"

RandomClass::RandomClass(unsigned seed) noexcept
	: Seed{ seed }
{
}

int RandomClass::operator()()
{
	Seed = (Seed * MULT_CONSTANT) + ADD_CONSTANT;
	return (Seed >> THROW_AWAY_BITS) & (~((~0) << SIGNIFICANT_BITS));
}

int RandomClass::operator()(int minval, int maxval)
{
	if (minval == maxval)
		return minval;

	if (minval > maxval)
	{
		int temp = minval;
		minval = maxval;
		maxval = temp;
	}

	int magnitude = maxval - minval;
	int highbit = SIGNIFICANT_BITS - 1;
	while ((magnitude & (1 << highbit)) == 0 && highbit > 0)
		--highbit;

	int mask = ~((~0L) << (highbit + 1));

	int pick = magnitude + 1;
	while (pick > magnitude)
		pick = operator()() & mask;

	return pick + minval;
}