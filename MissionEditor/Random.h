#pragma once

class RandomClass
{
public:
	explicit RandomClass(unsigned seed = 0) noexcept;

	RandomClass(const RandomClass&) = delete;
	RandomClass& operator=(const RandomClass&) = delete;
	RandomClass(RandomClass&&) = default;
	RandomClass& operator=(RandomClass&&) = default;

	~RandomClass() = default;

	operator int() { return(operator()()); };
	int operator()();
	int operator()(int minval, int maxval);

	enum { SIGNIFICANT_BITS = 15 };

	unsigned int Seed;

protected:
	enum
	{
		MULT_CONSTANT = 0x41C64E6D,
		ADD_CONSTANT = 0x00003039,
		THROW_AWAY_BITS = 10
	};
};