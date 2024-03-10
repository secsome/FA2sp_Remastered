#include "StdAfx.h"

#include "Pipe.h"
#include "CCFile.h"
#include "LZO1X.h"
#include "LCW.h"
#include "Straw.h"
#include "PKey.h"

#include <ranges>

Pipe::Pipe() noexcept
	: ChainTo{ nullptr }
	, ChainFrom{ nullptr }
{
}

Pipe::~Pipe()
{
}

int Pipe::Flush()
{
	return ChainTo != nullptr ? ChainTo->Flush() : 0;
}

int Pipe::End()
{
	return Flush();
}

void Pipe::Put_To(Pipe* pipe)
{
	if (ChainTo != pipe)
	{
		if (pipe != nullptr && pipe->ChainFrom != nullptr)
		{
			pipe->ChainFrom->Put_To(nullptr);
			pipe->ChainFrom = nullptr;
		}

		if (ChainTo != nullptr)
		{
			ChainTo->ChainFrom = nullptr;
			ChainTo->Flush();
		}

		ChainTo = pipe;
		if (ChainTo != nullptr)
			ChainTo->ChainFrom = this;
	}
}

int Pipe::Put(const void* source, int slen)
{
	return ChainTo != nullptr ? ChainTo->Put(source, slen) : slen;
}

FilePipe::FilePipe(FileClass* file) noexcept
	: Pipe{}
	, File{ file }
	, HasOpened{ false }
{
}

FilePipe::FilePipe(FileClass& file) noexcept
	: Pipe{}
	, File{ &file }
	, HasOpened{ false }
{
}

FilePipe::~FilePipe()
{
	if (Valid_File() && HasOpened)
	{
		HasOpened = false;
		File->Close();
		File = nullptr;
	}
}

int FilePipe::Put(const void* source, int slen)
{
	if (Valid_File() && source != nullptr && slen > 0)
	{
		if (!File->Is_Open())
		{
			HasOpened = true;
			File->Open(FILE_ACCESS_WRITE);
		}

		return File->Write((void*)source, slen);
	}
	return 0;
}

int FilePipe::End()
{
	int total = Pipe::End();
	if (Valid_File() && HasOpened)
	{
		HasOpened = false;
		File->Close();
	}
	return total;
}

BufferPipe::BufferPipe(Buffer const& buffer) noexcept
	: Pipe{}
	, BufferPtr{ buffer }
	, Index{ 0 }
{
}

BufferPipe::BufferPipe(void* buffer, int length) noexcept
	: Pipe{}
	, BufferPtr{ buffer, length }
	, Index{ 0 }
{
}

BufferPipe::~BufferPipe()
{
}

int BufferPipe::Put(const void* source, int slen)
{
	int total = 0;

	if (Is_Valid() && source != nullptr && slen > 0)
	{
		int len = slen;
		if (BufferPtr.Get_Size() != 0)
			len = std::min(slen, BufferPtr.Get_Size() - Index);

		if (len > 0)
			memmove(((char*)BufferPtr.Get_Buffer()) + Index, source, len);

		Index += len;
		total += len;
	}

	return total;
}

BlowPipe::BlowPipe(CryptControl control) noexcept
	: BF{ nullptr }
	, Counter{ 0 }
	, Control{ control }
	, Buffer{ 0 }
{
}

BlowPipe::~BlowPipe()
{
	delete BF;
	BF = nullptr;
}

int BlowPipe::Flush()
{
	int total = 0;
	if (Counter > 0 && BF != nullptr)
		total += Pipe::Put(Buffer, Counter);
	Counter = 0;
	total += Pipe::Flush();
	return total;
}

int BlowPipe::Put(const void* source, int slen)
{
	if (source == nullptr || slen < 1)
		return Pipe::Put(source, slen);

	if (BF == nullptr)
		return Pipe::Put(source, slen);

	int total = 0;
	if (Counter)
	{
		int sublen = std::min((int)(sizeof(Buffer) - Counter), slen);
		memmove(&Buffer[Counter], source, sublen);
		Counter += sublen;
		source = ((char*)source) + sublen;
		slen -= sublen;

		if (Counter == sizeof(Buffer))
		{
			if (Control == DECRYPT)
				BF->Decrypt(Buffer, sizeof(Buffer), Buffer);
			else
				BF->Encrypt(Buffer, sizeof(Buffer), Buffer);
			total += Pipe::Put(Buffer, sizeof(Buffer));
			Counter = 0;
		}
	}

	while (slen >= sizeof(Buffer))
	{
		if (Control == DECRYPT)
			BF->Decrypt(source, sizeof(Buffer), Buffer);
		else
			BF->Encrypt(source, sizeof(Buffer), Buffer);
		total += Pipe::Put(Buffer, sizeof(Buffer));
		source = ((char*)source) + sizeof(Buffer);
		slen -= sizeof(Buffer);
	}

	if (slen > 0)
	{
		memmove(Buffer, source, slen);
		Counter = slen;
	}

	return total;
}

void BlowPipe::Key(void const* key, int length)
{
	if (BF == nullptr)
		BF = new BlowfishEngine;

	if (BF != nullptr)
		BF->Submit_Key(key, length);
}

LZOPipe::LZOPipe(CompControl control, int blocksize)
	: Control{ control }
	, Counter{ 0 }
	, Buffer{ nullptr }
	, Buffer2{ nullptr }
	, BlockSize{ blocksize }
{
	SafetyMargin = BlockSize;
	Buffer = new char[BlockSize + SafetyMargin];
	Buffer2 = new char[BlockSize + SafetyMargin];
	BlockHeader.CompCount = 0xFFFF;
}

LZOPipe::~LZOPipe(void)
{
	delete[] Buffer;
	Buffer = nullptr;

	delete[] Buffer2;
	Buffer2 = nullptr;
}

int LZOPipe::Put(void const* source, int slen)
{
	if (source == nullptr || slen < 1)
		return Pipe::Put(source, slen);

	int total = 0;
	if (Control == DECOMPRESS)
	{
		while (slen > 0)
		{
			if (BlockHeader.CompCount == 0xFFFF)
			{
				int len = std::min(slen, 4 - Counter);
				memmove(&Buffer[Counter], source, len);
				source = ((char*)source) + len;
				slen -= len;
				Counter += len;

				if (Counter == sizeof(BlockHeader))
				{
					memmove(&BlockHeader, Buffer, sizeof(BlockHeader));
					Counter = 0;
				}
			}
			if (slen > 0)
			{
				int len = std::min(slen, BlockHeader.CompCount - Counter);
				memmove(&Buffer[Counter], source, len);
				slen -= len;
				source = ((char*)source) + len;
				Counter += len;
				if (Counter == BlockHeader.CompCount)
				{
					unsigned int length = sizeof(Buffer2);
					lzo1x_decompress((unsigned char*)Buffer, BlockHeader.CompCount, (unsigned char*)Buffer2, &length, nullptr);
					total += Pipe::Put(Buffer2, BlockHeader.UncompCount);
					Counter = 0;
					BlockHeader.CompCount = 0xFFFF;
				}
			}
		}
	}
	else
	{
		if (Counter > 0)
		{
			int tocopy = std::min(slen, BlockSize - Counter);
			memmove(&Buffer[Counter], source, tocopy);
			source = ((char*)source) + tocopy;
			slen -= tocopy;
			Counter += tocopy;
			if (Counter == BlockSize)
			{
				unsigned int len = sizeof(Buffer2);
				char* dictionary = new char[64 * 1024];
				lzo1x_1_compress((unsigned char*)Buffer, BlockSize, (unsigned char*)Buffer2, &len, dictionary);
				delete[] dictionary;
				BlockHeader.CompCount = (unsigned short)len;
				BlockHeader.UncompCount = (unsigned short)BlockSize;
				total += Pipe::Put(&BlockHeader, sizeof(BlockHeader));
				total += Pipe::Put(Buffer2, len);
				Counter = 0;
			}
		}
		while (slen >= BlockSize)
		{
			unsigned int len = sizeof(Buffer2);
			char* dictionary = new char[64 * 1024];
			lzo1x_1_compress((unsigned char*)source, BlockSize, (unsigned char*)Buffer2, &len, dictionary);
			delete[] dictionary;
			source = ((char*)source) + BlockSize;
			slen -= BlockSize;
			BlockHeader.CompCount = (unsigned short)len;
			BlockHeader.UncompCount = (unsigned short)BlockSize;
			total += Pipe::Put(&BlockHeader, sizeof(BlockHeader));
			total += Pipe::Put(Buffer2, len);
		}
		if (slen > 0)
		{
			memmove(Buffer, source, slen);
			Counter = slen;
		}
	}

	return total;
}

int LZOPipe::Flush()
{
	int total = 0;

	if (Counter > 0)
	{
		if (Control == DECOMPRESS)
		{
			if (BlockHeader.CompCount == 0xFFFF)
			{
				total += Pipe::Put(Buffer, Counter);
				Counter = 0;
			}
			if (Counter > 0)
			{
				total += Pipe::Put(&BlockHeader, sizeof(BlockHeader));
				total += Pipe::Put(Buffer, Counter);
				Counter = 0;
				BlockHeader.CompCount = 0xFFFF;
			}
		}
		else
		{
			unsigned int len = sizeof(Buffer2);
			char* dictionary = new char[64 * 1024];
			lzo1x_1_compress((unsigned char*)Buffer, Counter, (unsigned char*)Buffer2, &len, dictionary);
			delete[] dictionary;
			BlockHeader.CompCount = (unsigned short)len;
			BlockHeader.UncompCount = (unsigned short)Counter;
			total += Pipe::Put(&BlockHeader, sizeof(BlockHeader));
			total += Pipe::Put(Buffer2, len);
			Counter = 0;
		}
	}

	total += Pipe::Flush();
	return total;
}

PKPipe::PKPipe(CryptControl control, RandomStraw& rnd) noexcept
	: IsGettingKey{ true }
	, Rand{ rnd }
	, BF{ (control == ENCRYPT) ? BlowPipe::ENCRYPT : BlowPipe::DECRYPT }
	, Control{ control }
	, CipherKey{ nullptr }
	, Counter{ 0 }
	, BytesLeft{ 0 }
	, Buffer{ 0 }
{
}

PKPipe::~PKPipe()
{
}

void PKPipe::Put_To(Pipe* pipe)
{
	if (BF.ChainTo != pipe)
	{
		if (pipe != nullptr && pipe->ChainFrom != nullptr)
		{
			pipe->ChainFrom->Put_To(nullptr);
			pipe->ChainFrom = nullptr;
		}
		if (BF.ChainTo != nullptr)
			BF.ChainTo->ChainFrom = nullptr;
		BF.ChainTo = pipe;
		if (pipe != nullptr)
			pipe->ChainFrom = &BF;
		BF.ChainFrom = this;
		ChainTo = &BF;
	}
}

void PKPipe::Key(PKey const* key)
{
	if (key == nullptr)
	{
		Flush();
		IsGettingKey = false;
	}
	CipherKey = key;

	if (CipherKey != nullptr)
	{
		IsGettingKey = true;
		if (Control == DECRYPT)
			Counter = BytesLeft = Encrypted_Key_Length();
	}
}

int PKPipe::Put(const void* source, int length)
{
	if (source == nullptr || length < 1 || CipherKey == nullptr)
		return Pipe::Put(source, length);

	int total = 0;
	if (IsGettingKey)
	{
		if (Control == ENCRYPT)
		{
			char buffer[MAX_KEY_BLOCK_SIZE];
			memset(buffer, '\0', sizeof(buffer));
			Rand.Get(buffer, BLOWFISH_KEY_SIZE);
			int didput = CipherKey->Encrypt(buffer, Plain_Key_Length(), Buffer);
			total += Pipe::Put(Buffer, didput);
			BF.Key(buffer, BLOWFISH_KEY_SIZE);
			IsGettingKey = false;
		}
		else
		{
			int toget = (BytesLeft < length) ? BytesLeft : length;
			memmove(&Buffer[Counter - BytesLeft], source, toget);
			length -= toget;
			BytesLeft -= toget;
			source = (char*)source + toget;

			if (BytesLeft == 0)
			{
				char buffer[MAX_KEY_BLOCK_SIZE];
				CipherKey->Decrypt(Buffer, Counter, buffer);
				BF.Key(buffer, BLOWFISH_KEY_SIZE);
				IsGettingKey = false;
			}
		}
	}

	total += Pipe::Put(source, length);
	return total;
}

int PKPipe::Encrypted_Key_Length() const
{
	if (CipherKey == nullptr)
		return 0;
	return CipherKey->Block_Count(BLOWFISH_KEY_SIZE) * CipherKey->Crypt_Block_Size();
}

int PKPipe::Plain_Key_Length() const
{
	if (CipherKey == nullptr)
		return 0;
	return CipherKey->Block_Count(BLOWFISH_KEY_SIZE) * CipherKey->Plain_Block_Size();
}

SHAPipe::SHAPipe() noexcept
	: Pipe{}
{
}

SHAPipe::~SHAPipe()
{
}

int SHAPipe::Put(const void* source, int slen)
{
	SHA.Hash(source, slen);
	return Pipe::Put(source, slen);
}

int SHAPipe::Result(void* result) const
{
	return SHA.Result(result);
}

LCWPipe::LCWPipe(CompControl control, int blocksize)
	: Control{ control }
	, Counter{ 0 }
	, Buffer{ nullptr }
	, Buffer2{ nullptr }
	, BlockSize{ blocksize }
{
	SafetyMargin = BlockSize / 128 + 1;
	Buffer = new char[BlockSize + SafetyMargin];
	Buffer2 = new char[BlockSize + SafetyMargin];
	BlockHeader.CompCount = 0xFFFF;
}

LCWPipe::~LCWPipe()
{
	delete[] Buffer;
	Buffer = nullptr;

	delete[] Buffer2;
	Buffer2 = nullptr;
}

int LCWPipe::Put(void const* source, int slen)
{
	if (source == nullptr || slen < 1)
		return Pipe::Put(source, slen);

	int total = 0;
	if (Control == DECOMPRESS)
	{
		while (slen > 0)
		{
			if (BlockHeader.CompCount == 0xFFFF)
			{
				int len = (slen < ((int)sizeof(BlockHeader) - Counter)) ? slen : ((int)sizeof(BlockHeader) - Counter);
				memmove(&Buffer[Counter], source, len);
				source = ((char*)source) + len;
				slen -= len;
				Counter += len;
				if (Counter == sizeof(BlockHeader))
				{
					memmove(&BlockHeader, Buffer, sizeof(BlockHeader));
					Counter = 0;
				}
			}
			if (slen > 0)
			{
				int len = std::min(slen, BlockHeader.CompCount - Counter);
				memmove(&Buffer[Counter], source, len);
				slen -= len;
				source = ((char*)source) + len;
				Counter += len;
				if (Counter == BlockHeader.CompCount)
				{
					LCW_Uncompress(Buffer, Buffer2);
					total += Pipe::Put(Buffer2, BlockHeader.UncompCount);
					Counter = 0;
					BlockHeader.CompCount = 0xFFFF;
				}
			}
		}
	}
	else
	{
		if (Counter > 0)
		{
			int tocopy = (slen < (BlockSize - Counter)) ? slen : (BlockSize - Counter);
			memmove(&Buffer[Counter], source, tocopy);
			source = ((char*)source) + tocopy;
			slen -= tocopy;
			Counter += tocopy;

			if (Counter == BlockSize)
			{
				int len = LCW_Compress(Buffer, Buffer2, BlockSize);
				BlockHeader.CompCount = (unsigned short)len;
				BlockHeader.UncompCount = (unsigned short)BlockSize;
				total += Pipe::Put(&BlockHeader, sizeof(BlockHeader));
				total += Pipe::Put(Buffer2, len);
				Counter = 0;
			}
		}

		while (slen >= BlockSize)
		{
			int len = LCW_Compress(source, Buffer2, BlockSize);
			source = ((char*)source) + BlockSize;
			slen -= BlockSize;
			BlockHeader.CompCount = (unsigned short)len;
			BlockHeader.UncompCount = (unsigned short)BlockSize;
			total += Pipe::Put(&BlockHeader, sizeof(BlockHeader));
			total += Pipe::Put(Buffer2, len);
		}

		if (slen > 0)
		{
			memmove(Buffer, source, slen);
			Counter = slen;
		}
	}

	return total;
}

int LCWPipe::Flush(void)
{
	int total = 0;
	if (Counter > 0)
	{
		if (Control == DECOMPRESS)
		{
			if (BlockHeader.CompCount == 0xFFFF)
			{
				total += Pipe::Put(Buffer, Counter);
				Counter = 0;
			}
			if (Counter > 0)
			{
				total += Pipe::Put(&BlockHeader, sizeof(BlockHeader));
				total += Pipe::Put(Buffer, Counter);
				Counter = 0;
				BlockHeader.CompCount = 0xFFFF;
			}
		}
		else
		{
			int len = LCW_Compress(Buffer, Buffer2, Counter);
			BlockHeader.CompCount = (unsigned short)len;
			BlockHeader.UncompCount = (unsigned short)Counter;
			total += Pipe::Put(&BlockHeader, sizeof(BlockHeader));
			total += Pipe::Put(Buffer2, len);
			Counter = 0;
		}
	}

	total += Pipe::Flush();
	return total;
}

DynamicBufferPipe::DynamicBufferPipe(int length) noexcept
	: Pipe{}
{
	Buffer.reserve(length);
}

DynamicBufferPipe::~DynamicBufferPipe()
{
}

int DynamicBufferPipe::Put(const void* source, int slen)
{
	auto ptr = (const unsigned char*)source;
	Buffer.append_range(std::ranges::subrange(ptr, ptr + slen));
	return slen;
}
