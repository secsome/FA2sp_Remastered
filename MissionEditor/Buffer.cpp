#include "StdAfx.h"

#include "Buffer.h"

Buffer::Buffer(char* ptr, int size) noexcept
	: BufferPtr{ ptr }
	, Size{ size }
	, IsAllocated{ false }
{
}

Buffer::Buffer(void* ptr, int size) noexcept
	: BufferPtr{ ptr }
	, Size{ size }
	, IsAllocated{ false }
{
}

Buffer::Buffer(const void* ptr, int size) noexcept
	: BufferPtr{ const_cast<void*>(ptr) }
	, Size{ size }
	, IsAllocated{ false }
{
}

Buffer::Buffer(int size) noexcept
	: BufferPtr{ nullptr }
	, Size{ size }
	, IsAllocated{ false }
{
	if (size > 0)
	{
		BufferPtr = new char[size];
		IsAllocated = true;
	}
}

Buffer::Buffer(Buffer const& another)
	: IsAllocated{ false }
{
	BufferPtr = another.BufferPtr;
	Size = another.Size;
}

Buffer& Buffer::operator=(Buffer const& another)
{
	if (&another != this)
	{
		if (IsAllocated)
			delete[] BufferPtr;

		IsAllocated = false;
		BufferPtr = another.BufferPtr;
		Size = another.Size;
	}

	return *this;
}

Buffer::~Buffer()
{
	Reset();
}

void Buffer::Reset(void* ptr, int size)
{
	if (IsAllocated)
		delete[] BufferPtr;

	BufferPtr = ptr;
	Size = size;
	IsAllocated = false;
}
