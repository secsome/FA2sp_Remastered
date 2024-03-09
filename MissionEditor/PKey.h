#pragma once

#include "Int.h"

class PKey
{
public:
	static PKey FastKey;

	explicit PKey() noexcept;
	explicit PKey(const void* exponent, const void* modulus) noexcept;

	PKey(const PKey&) = delete;
	PKey& operator=(const PKey&) = delete;
	PKey(PKey&&) = default;
	PKey& operator=(PKey&&) = default;

	~PKey() = default;

	int Encrypt(void const* source, int slen, void* dest) const;
	int Decrypt(void const* source, int slen, void* dest) const;
	int Plain_Block_Size(void) const { return((BitPrecision - 1) / 8); }
	int Crypt_Block_Size(void) const { return(Plain_Block_Size() + 1); }
	int Block_Count(int plaintext_length) const { return((((plaintext_length - 1) / Plain_Block_Size()) + 1)); }
	int Encode_Modulus(void* buffer) const;
	int Encode_Exponent(void* buffer) const;
	void Decode_Modulus(void* buffer);
	void Decode_Exponent(void* buffer);

	static void Generate(Straw& random, int bits, PKey& fastkey, PKey& slowkey);

	enum { FAST_EXPONENT = 65537L };

private:
	BigInt Modulus;
	BigInt Exponent;
	int BitPrecision;
};