#pragma once

#include "Buffer.h"
#include "Random.h"
#include "Blowfish.h"
#include "SHA.h"

class Straw
{
public:
	explicit Straw() noexcept;

	Straw(const Straw&) = delete;
	Straw& operator=(const Straw&) = delete;
	Straw(Straw&&) = default;
	Straw& operator=(Straw&&) = default;

	virtual ~Straw();

	virtual void Get_From(Straw* straw);
	virtual int Get(void* buffer, int length);

	void Get_From(Straw& straw) { Get_From(&straw); }

	Straw* ChainTo;
	Straw* ChainFrom;
};

class FileClass;
class FileStraw : public Straw
{
public:
	explicit FileStraw(FileClass* file) noexcept;
	explicit FileStraw(FileClass& file) noexcept;

	FileStraw(const FileStraw&) = delete;
	FileStraw& operator=(const FileStraw&) = delete;
	FileStraw(FileStraw&&) = default;
	FileStraw& operator=(FileStraw&&) = default;

	virtual ~FileStraw() override;

	virtual int Get(void* buffer, int length) override;

private:
	bool Valid_File() { return File != nullptr; }

private:
	FileClass* File;
	bool HasOpened;
};

class BufferStraw : public Straw
{
public:
	explicit BufferStraw(const Buffer& buffer) noexcept;
	explicit BufferStraw(const void* buffer, int length) noexcept;

	BufferStraw(const BufferStraw&) = delete;
	BufferStraw& operator=(const BufferStraw&) = delete;
	BufferStraw(BufferStraw&&) = default;
	BufferStraw& operator=(BufferStraw&&) = default;

	virtual ~BufferStraw() override;

	virtual int Get(void* buffer, int length) override;

private:
	bool Is_Valid() { return(BufferPtr.Is_Valid()); }

private:
	Buffer BufferPtr;
	int Index;
};

class BlowfishEngine;
class BlowStraw : public Straw
{
public:
	enum CryptControl
	{
		ENCRYPT,
		DECRYPT
	};

	explicit BlowStraw(CryptControl control) noexcept;

	BlowStraw(const BlowStraw&) = delete;
	BlowStraw& operator=(const BlowStraw&) = delete;
	BlowStraw(BlowStraw&&) = default;
	BlowStraw& operator=(BlowStraw&&) = default;

	virtual ~BlowStraw() override;

	virtual int Get(void* buffer, int length) override;

	void Key(void const* key, int length);

protected:
	BlowfishEngine* BF;

private:
	char Buffer[8];
	int Counter;
	CryptControl Control;
};

class CacheStraw : public Straw
{
public:
	CacheStraw(Buffer const& buffer);
	CacheStraw(int length = 4096);

	CacheStraw(const CacheStraw&) = delete;
	CacheStraw& operator=(const CacheStraw&) = delete;
	CacheStraw(CacheStraw&&) = default;
	CacheStraw& operator=(CacheStraw&&) = default;

	virtual ~CacheStraw() override;

	virtual int Get(void* buffer, int length) override;

private:
	bool Is_Valid() { return BufferPtr.Is_Valid(); }

private:
	Buffer BufferPtr;
	int Index;
	int Length;
};

class LZOStraw : public Straw
{
public:
	enum CompControl
	{
		COMPRESS,
		DECOMPRESS
	};

	explicit LZOStraw(CompControl control, int blocksize = 1024 * 8) noexcept;

	LZOStraw(const LZOStraw&) = delete;
	LZOStraw& operator=(const LZOStraw&) = delete;
	LZOStraw(LZOStraw&&) = default;
	LZOStraw& operator=(LZOStraw&&) = default;

	virtual ~LZOStraw() override;

	virtual int Get(void* buffer, int length) override;

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

class RandomStraw : public Straw
{
public:
	static RandomStraw CryptRandom;

	explicit RandomStraw() noexcept;

	RandomStraw(const RandomStraw&) = delete;
	RandomStraw& operator=(const RandomStraw&) = delete;
	RandomStraw(RandomStraw&&) = default;
	RandomStraw& operator=(RandomStraw&&) = default;

	virtual ~RandomStraw() override;

	virtual int Get(void* buffer, int length) override;

	void Reset();
	void Seed_Bit(int seed);
	void Seed_Byte(char seed);
	void Seed_Short(short seed);
	void Seed_Int(int seed);
	int Seed_Bits_Needed() const;

private:
	void Scramble_Seed();

private:
	int SeedBits;
	int Current;
	RandomClass Random[32];
};

class PKey;
class PKStraw : public Straw
{
public:
	enum CryptControl
	{
		ENCRYPT,
		DECRYPT
	};

	explicit PKStraw(CryptControl control, RandomStraw& rnd) noexcept;

	PKStraw(const PKStraw&) = delete;
	PKStraw& operator=(const PKStraw&) = delete;
	PKStraw(PKStraw&&) = default;
	PKStraw& operator=(PKStraw&&) = default;

	virtual ~PKStraw() override;

	virtual void Get_From(Straw* straw) override;
	virtual int Get(void* buffer, int length) override;
	virtual void Get_From(Straw& straw) { Get_From(&straw); }

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
	BlowStraw BF;
	CryptControl Control;
	const PKey* CipherKey;
	char Buffer[256];
	int Counter;
	int BytesLeft;
};

class SHAStraw : public Straw
{
public:
	explicit SHAStraw() noexcept = default;

	SHAStraw(const SHAStraw&) = delete;
	SHAStraw& operator=(const SHAStraw&) = delete;
	SHAStraw(SHAStraw&&) = default;
	SHAStraw& operator=(SHAStraw&&) = default;

	virtual ~SHAStraw() override;

	virtual int Get(void* buffer, int length) override;

	int Result(void* result) const;

protected:
	SHAEngine SHA;
};