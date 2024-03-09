#pragma once

class Buffer
{
public:
	explicit Buffer(char* ptr, int size = 0) noexcept;
	explicit Buffer(void* ptr = nullptr, int size = 0) noexcept;
	explicit Buffer(const void* ptr, int size = 0) noexcept;
	explicit Buffer(int size) noexcept;

	Buffer(const Buffer&);
	Buffer& operator=(const Buffer&);
	Buffer(Buffer&&) = default;
	Buffer& operator=(Buffer&&) = default;

	~Buffer();
	operator void* () const { return BufferPtr; }
	operator char* () const { return reinterpret_cast<char*>(BufferPtr); }

	void Reset(void* ptr = nullptr, int size = 0);
	void* Get_Buffer() const { return BufferPtr; }
	int Get_Size() const { return Size; }
	bool Is_Valid() const { return BufferPtr != nullptr; }

protected:
	void* BufferPtr;
	int Size;
	bool IsAllocated;
};