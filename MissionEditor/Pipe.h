#pragma once

#include "Buffer.h"
#include "Random.h"
#include "Blowfish.h"
#include "SHA.h"

#include <vector>

class Pipe
{
public:
	explicit Pipe() noexcept;

	Pipe(const Pipe&) = delete;
	Pipe& operator=(const Pipe&) = delete;
	Pipe(Pipe&&) = default;
	Pipe& operator=(Pipe&&) = default;

	virtual ~Pipe();

	virtual int Flush();
	virtual int End();
	virtual void Put_To(Pipe* pipe);
	virtual int Put(const void* source, int slen);

	void Put_To(Pipe& pipe) { Put_To(&pipe); }

	Pipe* ChainTo;
	Pipe* ChainFrom;
};

class FileClass;
class FilePipe : public Pipe
{
public:
	explicit FilePipe(FileClass* file) noexcept;
	explicit FilePipe(FileClass& file) noexcept;

	FilePipe(const FilePipe&) = delete;
	FilePipe& operator=(const FilePipe&) = delete;
	FilePipe(FilePipe&&) = default;
	FilePipe& operator=(FilePipe&&) = default;

	virtual ~FilePipe() override;

	virtual int Put(const void* source, int slen) override;
	virtual int End() override;

private:
	bool Valid_File() { return File != nullptr; }

private:
	FileClass* File;
	bool HasOpened;
};

class BufferPipe : public Pipe
{
public:
	explicit BufferPipe(Buffer const& buffer) noexcept;
	explicit BufferPipe(void* buffer, int length) noexcept;

	BufferPipe(const BufferPipe&) = delete;
	BufferPipe& operator=(const BufferPipe&) = delete;
	BufferPipe(BufferPipe&&) = default;
	BufferPipe& operator=(BufferPipe&&) = default;

	virtual ~BufferPipe() override;

	virtual int Put(const void* source, int slen) override;

private:
	bool Is_Valid() { return BufferPtr.Is_Valid(); }

private:
	Buffer BufferPtr;
	int Index;
};

class BlowfishEngine;
class BlowPipe : public Pipe
{
public:
	enum CryptControl
	{
		ENCRYPT,
		DECRYPT
	};

	explicit BlowPipe(CryptControl control) noexcept;

	BlowPipe(const BlowPipe&) = delete;
	BlowPipe& operator=(const BlowPipe&) = delete;
	BlowPipe(BlowPipe&&) = default;
	BlowPipe& operator=(BlowPipe&&) = default;

	virtual ~BlowPipe() override;

	virtual int Flush() override;
	virtual int Put(const void* source, int slen) override;

	void Key(const void* key, int length);

private:
	BlowfishEngine* BF;
	char Buffer[8];
	int Counter;
	CryptControl Control;
};

class LZOPipe : public Pipe
{
public:
	enum CompControl
	{
		COMPRESS,
		DECOMPRESS
	};

	LZOPipe(CompControl, int blocksize = 1024 * 8);

	LZOPipe(const LZOPipe&) = delete;
	LZOPipe& operator=(const LZOPipe&) = delete;
	LZOPipe(LZOPipe&&) = default;
	LZOPipe& operator=(LZOPipe&&) = default;

	virtual ~LZOPipe(void);

	virtual int Flush(void);
	virtual int Put(void const* source, int slen);

private:
	CompControl Control;
	int Counter;
	char* Buffer;
	char* Buffer2;
	int BlockSize;
	int SafetyMargin;
	struct
	{
		unsigned short CompCount;
		unsigned short UncompCount;
	} BlockHeader;
};

class RandomStraw;
class PKey;
class PKPipe : public Pipe
{
public:
	enum CryptControl
	{
		ENCRYPT,
		DECRYPT
	};

	explicit PKPipe(CryptControl control, RandomStraw& rnd) noexcept;

	PKPipe(const PKPipe&) = delete;
	PKPipe& operator=(const PKPipe&) = delete;
	PKPipe(PKPipe&&) = default;
	PKPipe& operator=(PKPipe&&) = default;

	virtual ~PKPipe() override;

	virtual void Put_To(Pipe* pipe) override;
	virtual int Put(const void* source, int slen) override;

	void Key(const PKey* key);

private:
	int Encrypted_Key_Length() const;
	int Plain_Key_Length() const;

private:
	enum
	{
		BLOWFISH_KEY_SIZE = BlowfishEngine::MAX_KEY_LENGTH,
		MAX_KEY_BLOCK_SIZE = 256
	};

	bool IsGettingKey;

	RandomStraw& Rand;
	BlowPipe BF;
	CryptControl Control;
	PKey const* CipherKey;
	char Buffer[MAX_KEY_BLOCK_SIZE];
	int Counter;
	int BytesLeft;
};

class SHAPipe : public Pipe
{
public:
	explicit SHAPipe() noexcept;

	SHAPipe(const SHAPipe&) = delete;
	SHAPipe& operator=(const SHAPipe&) = delete;
	SHAPipe(SHAPipe&&) = default;
	SHAPipe& operator=(SHAPipe&&) = default;

	virtual ~SHAPipe() override;

	virtual int Put(const void* source, int slen) override;

	int Result(void* result) const;

protected:
	SHAEngine SHA;
};

class LCWPipe : public Pipe
{
public:
	enum CompControl
	{
		COMPRESS,
		DECOMPRESS
	};

	LCWPipe(CompControl, int blocksize = 1024 * 8);

	LCWPipe(const LCWPipe&) = delete;
	LCWPipe& operator=(const LCWPipe&) = delete;
	LCWPipe(LCWPipe&&) = default;
	LCWPipe& operator=(LCWPipe&&) = default;

	virtual ~LCWPipe(void);

	virtual int Flush(void);
	virtual int Put(void const* source, int slen);

private:
	CompControl Control;
	int Counter;
	char* Buffer;
	char* Buffer2;
	int BlockSize;
	int SafetyMargin;

	struct
	{
		unsigned short CompCount;
		unsigned short UncompCount;
	} BlockHeader;
};

// A buffer pipe that can grow dynamically
class DynamicBufferPipe : public Pipe
{
public:
	explicit DynamicBufferPipe(int length = 0x1000) noexcept;

	DynamicBufferPipe(const DynamicBufferPipe&) = delete;
	DynamicBufferPipe& operator=(const DynamicBufferPipe&) = delete;
	DynamicBufferPipe(DynamicBufferPipe&&) = default;
	DynamicBufferPipe& operator=(DynamicBufferPipe&&) = default;

	virtual ~DynamicBufferPipe() override;

	virtual int Put(const void* source, int slen) override;

	const void* GetBuffer() const { return Buffer.data(); }
	size_t GetLength() const { return Buffer.size(); }

private:
	std::vector<unsigned char> Buffer;
};