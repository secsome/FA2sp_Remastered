#include "StdAfx.h"

#include "PKey.h"

PKey PKey::FastKey;

PKey::PKey() noexcept
	: Modulus{ 0 }
	, Exponent{ 0 }
	, BitPrecision{ 0 }
{
}

PKey::PKey(const void* exponent, const void* modulus) noexcept
{
	Modulus.DERDecode((unsigned char*)modulus);
	Exponent.DERDecode((unsigned char*)exponent);
	BitPrecision = Modulus.BitCount() - 1;
}

int PKey::Encrypt(void const* source, int slen, void* dest) const
{
	int total = 0;

	while (slen >= Plain_Block_Size())
	{
		BigInt temp(0);
		memmove(&temp, source, Plain_Block_Size());
		temp = temp.exp_b_mod_c(Exponent, Modulus);

		memmove(dest, &temp, Crypt_Block_Size());
		slen -= Plain_Block_Size();
		source = (char*)source + Plain_Block_Size();
		dest = (char*)dest + Crypt_Block_Size();
		total += Crypt_Block_Size();
	}

	return total;
}

int PKey::Decrypt(void const* source, int slen, void* dest) const
{
	int total = 0;

	while (slen >= Crypt_Block_Size())
	{
		BigInt temp(0);
		memmove(&temp, source, Crypt_Block_Size());
		temp = temp.exp_b_mod_c(Exponent, Modulus);

		memmove(dest, &temp, Plain_Block_Size());
		slen -= Crypt_Block_Size();
		source = (char*)source + Crypt_Block_Size();
		dest = (char*)dest + Plain_Block_Size();
		total += Plain_Block_Size();
	}

	return total;
}

int PKey::Encode_Modulus(void* buffer) const
{
	return buffer == nullptr ? 0 : Modulus.DEREncode((unsigned char*)buffer);
}

int PKey::Encode_Exponent(void* buffer) const
{
	return buffer == nullptr ? 0 : Exponent.DEREncode((unsigned char*)buffer);
}

void PKey::Decode_Modulus(void* buffer)
{
	Modulus.DERDecode((unsigned char*)buffer);
	BitPrecision = Modulus.BitCount() - 1;
}

void PKey::Decode_Exponent(void* buffer)
{
	Exponent.DERDecode((unsigned char*)buffer);
}

void PKey::Generate(Straw& random, int bits, PKey& fastkey, PKey& slowkey)
{
#if (0)
	/*
	**	Key generation consists of create a key pair and then testing the key
	**	pair. If the test fails, then repeat the process. The test and repeat
	**	method is required since the prime number generating process can't
	**	guarantee the generation of a prime number -- it can only generate a
	**	highly likely prime number.
	*/
	for (;;) {
		/*
		**	Generate the two random prime numbers. This is the longest
		**	step.
		*/
		BigInt p = Generate_Prime(random, bits, &p);
		BigInt q = Generate_Prime(random, bits, &q);

		/*
		**	The exponent factors are easy to calculate from the prime numbers.
		*/
		BigInt e = FAST_EXPONENT;
		BigInt n = p * q;
		BigInt pqmin = (p - (unsigned short)1) * (q - (unsigned short)1);
		BigInt d = e.Inverse(pqmin);

		/*
		**	Store the data into the key objects. Notice that the modulus is the
		**	same for both the fast and slow keys. Also notice that the exponent for
		**	the fast key is ALWAYS 65537. Given this, it is possible to economize the
		**	fast key into being just the modulus and the slow key to being just the
		**	exponent (presuming the slow key also has access to the fast key so that
		**	it can get the modulus).
		*/
		fastkey.Exponent = e;
		fastkey.Modulus = n;
		fastkey.BitPrecision = n.BitCount() - 1;

		slowkey.Exponent = d;
		slowkey.Modulus = n;
		slowkey.BitPrecision = fastkey.BitPrecision;

		/*
		**	Test the keys by encrypting a block of random bytes. If it decrypts
		**	correctly, then a valid key pair has been generated -- bail.
		*/
		char before[256];
		char after[256];

		for (int index = 0; index < fastkey.Plain_Block_Size(); index++) {
			before[index] = (char)rand();
		}
		fastkey.Encrypt(before, fastkey.Plain_Block_Size(), after);
		slowkey.Decrypt(after, slowkey.Crypt_Block_Size(), after);

		/*
		**	Compare the pre and post processing buffer. A match indicates
		**	a valid key pair.
		*/
		if (memcmp(before, after, fastkey.Plain_Block_Size()) == 0) break;
	}
#else
	UNREFERENCED_PARAMETER(random);
	UNREFERENCED_PARAMETER(bits);
	UNREFERENCED_PARAMETER(fastkey);
	UNREFERENCED_PARAMETER(slowkey);
#endif
}