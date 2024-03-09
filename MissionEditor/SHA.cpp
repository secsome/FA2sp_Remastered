#include "StdAfx.h"

#include "SHA.h"

#include <cstring>

#define	Reverse_INT(a)	((a>>24)&0x000000FFL) | ((a>>8)&0x0000FF00L) | ((a<<8)&0x00FF0000L) | ((a<<24)&0xFF000000L)

SHAEngine::SHAEngine() noexcept
{
	Init();
}

void SHAEngine::Init()
{
	IsCached = false;
	Length = 0;
	PartialCount = 0;
	Acc.Int[0] = SA;
	Acc.Int[1] = SB;
	Acc.Int[2] = SC;
	Acc.Int[3] = SD;
	Acc.Int[4] = SE;
}

int SHAEngine::Result(void* result) const
{
	if (IsCached)
		memcpy(result, &FinalResult, sizeof(FinalResult));

	int length = Length + PartialCount;
	int partialcount = PartialCount;
	char partial[SRC_BLOCK_SIZE];
	memcpy(partial, Partial, sizeof(Partial));

	partial[partialcount] = (char)0x80;

	SHADigest acc = Acc;
	if ((SRC_BLOCK_SIZE - partialcount) < 9)
	{
		if (partialcount + 1 < SRC_BLOCK_SIZE)
			memset(&partial[partialcount + 1], 0, SRC_BLOCK_SIZE - (partialcount + 1));

		Process_Block(&partial[0], acc);
		partialcount = 0;
	}
	else
		++partialcount;

	memset(&partial[partialcount], 0, SRC_BLOCK_SIZE - partialcount);
	*(int*)(&partial[SRC_BLOCK_SIZE - 4]) = Reverse_INT((length * 8));
	Process_Block(&partial[0], acc);

	memcpy((char*)&FinalResult, &acc, sizeof(acc));
	for (int index = 0; index < sizeof(FinalResult) / sizeof(int); ++index)
		(int&)FinalResult.Int[index] = Reverse_INT(FinalResult.Int[index]);

	(bool&)IsCached = true;
	memcpy(result, &FinalResult, sizeof(FinalResult));
	return sizeof(FinalResult);
}

void SHAEngine::Hash(const void* data, int length)
{
	IsCached = false;

	Process_Partial(data, length);

	if (length == 0)
		return;

	int blocks = (length / SRC_BLOCK_SIZE);
	int const* source = (const int*)data;
	for (int bcount = 0; bcount < blocks; ++bcount)
	{
		Process_Block(source, Acc);
		Length += SRC_BLOCK_SIZE;
		source += SRC_BLOCK_SIZE / sizeof(int);
		length -= SRC_BLOCK_SIZE;
	}

	data = source;
	Process_Partial(data, length);
}

void SHAEngine::Process_Block(const void* source, SHADigest& acc) const
{
	int block[PROC_BLOCK_SIZE / sizeof(int)];

	const int* data = (const int*)source;
	int index;
	for (index = 0; index < SRC_BLOCK_SIZE / sizeof(int); ++index)
		block[index] = Reverse_INT(data[index]);

	for (index = SRC_BLOCK_SIZE / sizeof(int); index < PROC_BLOCK_SIZE / sizeof(int); ++index)
		block[index] = _rotl(block[index - 3] ^ block[index - 8] ^ block[index - 14] ^ block[index - 16], 1);

	SHADigest alt = acc;
	for (index = 0; index < PROC_BLOCK_SIZE / sizeof(int); ++index)
	{
		int temp = _rotl(alt.Int[0], 5) + Do_Function(index, alt.Int[1], alt.Int[2], alt.Int[3]) + alt.Int[4] + block[index] + Get_Constant(index);
		alt.Int[4] = alt.Int[3];
		alt.Int[3] = alt.Int[2];
		alt.Int[2] = _rotl(alt.Int[1], 30);
		alt.Int[1] = alt.Int[0];
		alt.Int[0] = temp;
	}
	acc.Int[0] += alt.Int[0];
	acc.Int[1] += alt.Int[1];
	acc.Int[2] += alt.Int[2];
	acc.Int[3] += alt.Int[3];
	acc.Int[4] += alt.Int[4];
}

void SHAEngine::Process_Partial(const void*& data, int& length)
{
	if (length == 0 || data == nullptr)
		return;

	if (PartialCount == 0 && length >= SRC_BLOCK_SIZE)
		return;

	int add_count = std::min(length, SRC_BLOCK_SIZE - PartialCount);
	memcpy(&Partial[PartialCount], data, add_count);
	data = ((const char*&)data) + add_count;
	PartialCount += add_count;
	length -= add_count;

	if (PartialCount == SRC_BLOCK_SIZE)
	{
		Process_Block(&Partial[0], Acc);
		Length += SRC_BLOCK_SIZE;
		PartialCount = 0;
	}
}
