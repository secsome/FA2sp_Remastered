#include "StdAfx.h"

#include "Straw.h"
#include "CCFile.h"
#include "Blowfish.h"
#include "LZO1X.h"
#include "LCW.h"
#include "PKey.h"
#include "SHA.h"

#include <algorithm>

Straw::Straw() noexcept
	: ChainTo{ nullptr }
	, ChainFrom{ nullptr }
{
}

Straw::~Straw()
{
}

void Straw::Get_From(Straw* straw)
{
	if (ChainTo != straw)
	{
		if (straw != nullptr && straw->ChainFrom != nullptr)
		{
			straw->ChainFrom->Get_From(nullptr);
			straw->ChainFrom = nullptr;
		}

		if (ChainTo != nullptr)
			ChainTo->ChainFrom = nullptr;

		ChainTo = straw;
		if (ChainTo != nullptr)
			ChainTo->ChainFrom = this;
	}
}

int Straw::Get(void* buffer, int length)
{
	return ChainTo != nullptr ? ChainTo->Get(buffer, length) : 0;
}

FileStraw::FileStraw(FileClass* file) noexcept
	: Straw{}
	, File{ file }
	, HasOpened{ false }
{
}

FileStraw::FileStraw(FileClass& file) noexcept
	: Straw{}
	, File{ &file }
	, HasOpened{ false }
{
}

FileStraw::~FileStraw()
{
	if (Valid_File() && HasOpened)
	{
		File->Close();
		HasOpened = false;
		File = nullptr;
	}
}

int FileStraw::Get(void* buffer, int length)
{
	if (Valid_File() && buffer != nullptr && length > 0)
	{
		if (!File->Is_Open())
		{
			HasOpened = true;
			if (!File->Is_Available())
				return 0;

			if (!File->Open(FILE_ACCESS_READ))
				return 0;
		}

		return File->Read(buffer, length);
	}
	return 0;
}

BufferStraw::BufferStraw(const Buffer& buffer) noexcept
	: Straw{}
	, BufferPtr{ buffer }
	, Index{ 0 }
{
}

BufferStraw::BufferStraw(const void* buffer, int length) noexcept
	: Straw{}
	, BufferPtr{ (void*)buffer, length }
	, Index{ 0 }
{
}

BufferStraw::~BufferStraw()
{
}

int BufferStraw::Get(void* buffer, int length)
{
	int total = 0;

	if (Is_Valid() && buffer != nullptr && length > 0)
	{
		int len = length;
		if (BufferPtr.Get_Size() != 0)
			len = std::min(length, BufferPtr.Get_Size() - Index);

		if (len > 0)
			memmove(buffer, ((char*)BufferPtr.Get_Buffer()) + Index, len);

		Index += len;
		total += len;
	}
	return total;
}

BlowStraw::BlowStraw(CryptControl control) noexcept
	: Straw{}
	, BF{ nullptr }
	, Counter{ 0 }
	, Control{ control }
	, Buffer{ 0 }
{
}

BlowStraw::~BlowStraw()
{
	delete BF;
	BF = nullptr;
}

int BlowStraw::Get(void* buffer, int length)
{
	if (buffer == nullptr || length <= 0)
		return 0;

	if (BF == nullptr)
		return Straw::Get(buffer, length);

	int total = 0;

	while (length > 0)
	{
		if (Counter > 0)
		{
			int sublen = std::min(length, Counter);
			memmove(buffer, &Buffer[sizeof(Buffer) - Counter], sublen);
			Counter -= sublen;
			buffer = ((char*)buffer) + sublen;
			length -= sublen;
			total += sublen;
		}
		if (length == 0)
			break;

		int incount = Straw::Get(Buffer, sizeof(Buffer));
		if (incount == 0)
			break;

		if (incount == sizeof(Buffer))
		{
			if (Control == DECRYPT)
				BF->Decrypt(Buffer, incount, Buffer);
			else
				BF->Encrypt(Buffer, incount, Buffer);
		}
		else
			memmove(&Buffer[sizeof(Buffer) - incount], Buffer, incount);

		Counter = incount;
	}

	return total;
}

void BlowStraw::Key(void const* key, int length)
{
	if (BF == nullptr)
		BF = new BlowfishEngine;

	if (BF != nullptr)
		BF->Submit_Key(key, length);
}

CacheStraw::CacheStraw(Buffer const& buffer)
	: Straw{}
	, BufferPtr{ buffer }
	, Index{ 0 }
	, Length{ 0 }
{
}

CacheStraw::CacheStraw(int length)
	: Straw{}
	, BufferPtr{ length }
	, Index{ 0 }
	, Length{ 0 }
{
}

CacheStraw::~CacheStraw()
{
}

int CacheStraw::Get(void* buffer, int length)
{
	int total = 0;

	if (Is_Valid() && buffer != nullptr && length > 0)
	{
		while (length > 0)
		{
			if (Length > 0)
			{
				int tocopy = std::min(Length, length);
				memmove(buffer, ((char*)BufferPtr.Get_Buffer()) + Index, tocopy);
				length -= tocopy;
				Index += tocopy;
				total += tocopy;
				Length -= tocopy;
				buffer = (char*)buffer + tocopy;
			}
			if (length == 0)
				break;

			Length = Straw::Get(BufferPtr, BufferPtr.Get_Size());
			Index = 0;
			if (Length == 0)
				break;
		}
	}
	return total;
}

LZOStraw::LZOStraw(CompControl control, int blocksize) noexcept
	: Control{ control }
	, Counter{ 0 }
	, Buffer{ nullptr }
	, Buffer2{ nullptr }
	, BlockSize{ blocksize }
	, BlockHeader{ 0,0 }
{
	SafetyMargin = BlockSize;
	Buffer = new char[BlockSize + SafetyMargin];
	if (control == COMPRESS)
		Buffer2 = new char[BlockSize + SafetyMargin];
}

LZOStraw::~LZOStraw()
{
	delete[] Buffer;
	Buffer = nullptr;

	if (Buffer2)
	{
		delete[] Buffer2;
		Buffer2 = nullptr;
	}
}

int LZOStraw::Get(void* buffer, int length)
{
	if (buffer == nullptr || length < 1)
		return 0;

	int total = 0;
	while (length > 0)
	{
		if (Counter)
		{
			int len = std::min(length, Counter);
			if (Control == DECOMPRESS)
				memmove(buffer, &Buffer[BlockHeader.UncompCount - Counter], len);
			else
				memmove(buffer, &Buffer2[(BlockHeader.CompCount + sizeof(BlockHeader)) - Counter], len);

			buffer = ((char*)buffer) + len;
			length -= len;
			Counter -= len;
			total += len;
		}
		if (length == 0)
			break;

		if (Control == DECOMPRESS)
		{
			int incount = Straw::Get(&BlockHeader, sizeof(BlockHeader));
			if (incount != sizeof(BlockHeader))
				break;

			char* staging_buffer = new char[BlockHeader.CompCount];
			incount = Straw::Get(staging_buffer, BlockHeader.CompCount);
			if (incount != BlockHeader.CompCount)
				break;
			unsigned int length = sizeof(Buffer);
			lzo1x_decompress((unsigned char*)staging_buffer, BlockHeader.CompCount, (unsigned char*)Buffer, &length, NULL);
			delete[] staging_buffer;
			Counter = BlockHeader.UncompCount;
		}
		else
		{
			BlockHeader.UncompCount = (unsigned short)Straw::Get(Buffer, BlockSize);
			if (BlockHeader.UncompCount == 0)
				break;
			char* dictionary = new char[64 * 1024];
			unsigned int length = sizeof(Buffer2) - sizeof(BlockHeader);
			lzo1x_1_compress((unsigned char*)Buffer, BlockHeader.UncompCount, (unsigned char*)(&Buffer2[sizeof(BlockHeader)]), &length, dictionary);
			BlockHeader.CompCount = (unsigned short)length;
			delete[] dictionary;
			memmove(Buffer2, &BlockHeader, sizeof(BlockHeader));
			Counter = BlockHeader.CompCount + sizeof(BlockHeader);
		}
	}

	return total;
}

RandomStraw RandomStraw::CryptRandom;

RandomStraw::RandomStraw() noexcept
	: Straw{}
	, SeedBits{ 0 }
	, Current{ 0 }
{
	Reset();
}

RandomStraw::~RandomStraw()
{
	Reset();
}

int RandomStraw::Get(void* buffer, int length)
{
	if (buffer == nullptr || length < 1)
		return Straw::Get(buffer, length);

	int total = 0;
	while (length > 0)
	{
		*(char*)buffer = (char)Random[Current++];
		Current = Current % (sizeof(Random) / sizeof(Random[0]));
		buffer = (char*)buffer + sizeof(char);
		--length;
		++total;
	}
	return total;
}

void RandomStraw::Reset()
{
	SeedBits = 0;
	Current = 0;
	memset(Random, 0, sizeof(Random));
}

void RandomStraw::Seed_Bit(int seed)
{
	char* ptr = ((char*)&Random[0]) + ((SeedBits / CHAR_BIT) % sizeof(Random));
	char frac = (char)(1 << (SeedBits & (CHAR_BIT - 1)));

	if (seed & 0x01)
		*ptr ^= frac;

	++SeedBits;

	if (SeedBits == (sizeof(Random) * CHAR_BIT))
		Scramble_Seed();
}

void RandomStraw::Seed_Byte(char seed)
{
	for (int index = 0; index < CHAR_BIT; ++index)
	{
		Seed_Bit(seed);
		seed >>= 1;
	}
}

void RandomStraw::Seed_Short(short seed)
{
	for (int index = 0; index < (sizeof(seed) * CHAR_BIT); ++index)
	{
		Seed_Bit(seed);
		seed >>= 1;
	}
}

void RandomStraw::Seed_Int(int seed)
{
	for (int index = 0; index < (sizeof(seed) * CHAR_BIT); ++index)
	{
		Seed_Bit(seed);
		seed >>= 1;
	}
}

int RandomStraw::Seed_Bits_Needed(void) const
{
	const int total = sizeof(Random) * CHAR_BIT;

	return SeedBits < total ? total - SeedBits : 0;
}

void RandomStraw::Scramble_Seed()
{
	SHAEngine sha;

	for (int index = 0; index < sizeof(Random); ++index)
	{
		char digest[20];

		sha.Hash(&Random[0], sizeof(Random));
		sha.Result(digest);

		int tocopy = sizeof(digest) < (sizeof(Random) - index) ? sizeof(digest) : (sizeof(Random) - index);
		memmove(((char*)&Random[0]) + index, digest, tocopy);
	}
}

PKStraw::PKStraw(CryptControl control, RandomStraw& rnd) noexcept
	: Straw{}
	, IsGettingKey{ true }
	, Rand{ rnd }
	, BF{ (control == ENCRYPT) ? BlowStraw::ENCRYPT : BlowStraw::DECRYPT }
	, Control{ control }
	, CipherKey{ nullptr }
	, Counter{ 0 }
	, BytesLeft{ 0 }
	, Buffer{ 0 }
{
	Straw::Get_From(BF);
}

PKStraw::~PKStraw()
{
}

void PKStraw::Get_From(Straw* straw)
{
	if (BF.ChainTo != straw)
	{
		if (straw != nullptr && straw->ChainFrom != nullptr)
		{
			straw->ChainFrom->Get_From(nullptr);
			straw->ChainFrom = nullptr;
		}

		if (BF.ChainTo != nullptr)
			BF.ChainTo->ChainFrom = nullptr;

		BF.ChainTo = straw;
		BF.ChainFrom = this;
		ChainTo = &BF;
		if (BF.ChainTo != nullptr)
			BF.ChainTo->ChainFrom = this;
	}
}

int PKStraw::Get(void* buffer, int length)
{
	if (buffer == nullptr || length < 1 || CipherKey == nullptr)
		return Straw::Get(buffer, length);

	int total = 0;

	if (IsGettingKey)
	{
		if (Control == DECRYPT)
		{
			char cbuffer[MAX_KEY_BLOCK_SIZE];
			int got = Straw::Get(cbuffer, Encrypted_Key_Length());

			if (got != Encrypted_Key_Length())
				return 0;

			CipherKey->Decrypt(cbuffer, got, Buffer);
			BF.Key(Buffer, BLOWFISH_KEY_SIZE);
		}
		else
		{
			char buffer[MAX_KEY_BLOCK_SIZE];
			memset(buffer, 0, sizeof(buffer));
			Rand.Get(buffer, BLOWFISH_KEY_SIZE);

			Counter = BytesLeft = CipherKey->Encrypt(buffer, Plain_Key_Length(), Buffer);
			BF.Key(buffer, BLOWFISH_KEY_SIZE);
		}

		IsGettingKey = false;
	}

	if (BytesLeft > 0) {
		int tocopy = (length < BytesLeft) ? length : BytesLeft;
		memmove(buffer, &Buffer[Counter - BytesLeft], tocopy);
		buffer = (char*)buffer + tocopy;
		BytesLeft -= tocopy;
		length -= tocopy;
		total += tocopy;
	}

	total += Straw::Get(buffer, length);

	return total;
}

void PKStraw::Key(const PKey* key)
{
	CipherKey = key;
	if (key != nullptr)
		IsGettingKey = true;

	Counter = 0;
	BytesLeft = 0;
}

int PKStraw::Encrypted_Key_Length() const
{
	return CipherKey == nullptr ? 0 : CipherKey->Block_Count(BLOWFISH_KEY_SIZE) * CipherKey->Crypt_Block_Size();
}

int PKStraw::Plain_Key_Length() const
{
	return CipherKey == nullptr ? 0 : CipherKey->Block_Count(BLOWFISH_KEY_SIZE) * CipherKey->Plain_Block_Size();
}

SHAStraw::~SHAStraw()
{
}

int SHAStraw::Get(void* buffer, int length)
{
	if (buffer == nullptr || length < 1)
		return 0;

	int counter = Straw::Get(buffer, length);
	SHA.Hash(buffer, counter);
	return counter;
}

int SHAStraw::Result(void* result) const
{
	return SHA.Result(result);
}

LCWStraw::LCWStraw(CompControl control, int blocksize) noexcept
	: Control{ control }
	, Counter{ 0 }
	, Buffer{ nullptr }
	, Buffer2{ nullptr }
	, BlockSize{ blocksize }
	, BlockHeader{ 0,0 }
{
	SafetyMargin = BlockSize / 128 + 1;
	Buffer = new char[BlockSize + SafetyMargin];
	if (control == COMPRESS)
		Buffer2 = new char[BlockSize + SafetyMargin];
}

LCWStraw::~LCWStraw()
{
	delete[] Buffer;
	Buffer = nullptr;

	if (Buffer2)
	{
		delete[] Buffer2;
		Buffer2 = nullptr;
	}
}

int LCWStraw::Get(void* buffer, int length)
{
	if (buffer == nullptr || length < 1)
		return 0;

	int total = 0;
	while (length > 0)
	{
		if (Counter)
		{
			int len = std::min(length, Counter);
			if (Control == DECOMPRESS)
				memmove(buffer, &Buffer[BlockHeader.UncompCount - Counter], len);
			else
				memmove(buffer, &Buffer2[(BlockHeader.CompCount + sizeof(BlockHeader)) - Counter], len);

			buffer = ((char*)buffer) + len;
			length -= len;
			Counter -= len;
			total += len;
		}
		if (length == 0)
			break;

		if (Control == DECOMPRESS)
		{
			int incount = Straw::Get(&BlockHeader, sizeof(BlockHeader));
			if (incount != sizeof(BlockHeader))
				break;

			void* ptr = &Buffer[(BlockSize + SafetyMargin) - BlockHeader.CompCount];
			incount = Straw::Get(ptr, BlockHeader.CompCount);
			if (incount != BlockHeader.CompCount)
				break;
			LCW_Uncompress(ptr, Buffer);
			Counter = BlockHeader.UncompCount;
		}
		else
		{
			BlockHeader.UncompCount = (unsigned short)Straw::Get(Buffer, BlockSize);
			if (BlockHeader.UncompCount == 0)
				break;
			BlockHeader.CompCount = (unsigned short)LCW_Compress(Buffer, &Buffer2[sizeof(BlockHeader)], BlockHeader.UncompCount);
			memmove(Buffer2, &BlockHeader, sizeof(BlockHeader));
			Counter = BlockHeader.CompCount + sizeof(BlockHeader);
		}
	}

	return total;
}