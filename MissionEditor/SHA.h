#pragma once

class SHAEngine
{
public:
	explicit SHAEngine() noexcept;

	SHAEngine(const SHAEngine&) = default;
	SHAEngine& operator=(const SHAEngine&) = default;
	SHAEngine(SHAEngine&&) = default;
	SHAEngine& operator=(SHAEngine&&) = default;

	~SHAEngine() = default;

	void Init();
	int Result(void* result) const;
	void Hash(const void* data, int length);

	static int Digest_Size() { return(sizeof(SHADigest)); }

private:
	union SHADigest
	{
		unsigned int Int[5];
		unsigned char Char[20];
	};

	int Get_Constant(int index) const
	{
		if (index < 20) return K1;
		if (index < 40) return K2;
		if (index < 60) return K3;
		return K4;
	};

	int Function1(int X, int Y, int Z) const
	{
		return Z ^ (X & (Y ^ Z));
	};

	int Function2(int X, int Y, int Z) const
	{
		return X ^ Y ^ Z;
	};

	int Function3(int X, int Y, int Z) const
	{
		return (X & Y) | (Z & (X | Y));
	};

	int Function4(int X, int Y, int Z) const
	{
		return X ^ Y ^ Z;
	};

	int Do_Function(int index, int X, int Y, int Z) const
	{
		if (index < 20) return Function1(X, Y, Z);
		if (index < 40) return Function2(X, Y, Z);
		if (index < 60) return Function3(X, Y, Z);
		return Function4(X, Y, Z);
	};

	void Process_Block(const void* source, SHADigest& acc) const;
	void Process_Partial(const void*& data, int& length);

private:
	enum
	{
		SA = 0x67452301L,
		SB = 0xefcdab89L,
		SC = 0x98badcfeL,
		SD = 0x10325476L,
		SE = 0xc3d2e1f0L,

		K1 = 0x5a827999L,
		K2 = 0x6ed9eba1L,
		K3 = 0x8f1bbcdcL,
		K4 = 0xca62c1d6L,

		SRC_BLOCK_SIZE = 16 * sizeof(int),
		PROC_BLOCK_SIZE = 80 * sizeof(int)
	};

	bool IsCached;
	SHADigest FinalResult;
	SHADigest Acc;
	int Length;
	int PartialCount;
	char Partial[SRC_BLOCK_SIZE];
};

#define	SHA_SOURCE1		"abc"
#define	SHA_DIGEST1a	"\xA9\x99\x3E\x36\x47\x06\x81\x6A\xBA\x3E\x25\x71\x78\x50\xC2\x6C\x9C\xD0\xD8\x9D"
#define	SHA_DIGEST1b	"\x01\x64\xB8\xA9\x14\xCD\x2A\x5E\x74\xC4\xF7\xFF\x08\x2C\x4D\x97\xF1\xED\xF8\x80"

#define	SHA_SOURCE2		"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
#define	SHA_DIGEST2a	"\x84\x98\x3E\x44\x1C\x3B\xD2\x6E\xBA\xAE\x4A\xA1\xF9\x51\x29\xE5\xE5\x46\x70\xF1"
#define	SHA_DIGEST2b	"\xD2\x51\x6E\xE1\xAC\xFA\x5B\xAF\x33\xDF\xC1\xC4\x71\xE4\x38\x44\x9E\xF1\x34\xC8"

#define	SHA_SOURCE3		"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
#define	SHA_DIGEST3a	"\x34\xAA\x97\x3C\xD4\xC4\xDA\xA4\xF6\x1E\xEB\x2B\xDB\xAD\x27\x31\x65\x34\x01\x6F"
#define	SHA_DIGEST3b	"\x32\x32\xAF\xFA\x48\x62\x8A\x26\x65\x3B\x5A\xAA\x44\x54\x1F\xD9\x0D\x69\x06\x03"