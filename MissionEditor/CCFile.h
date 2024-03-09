#pragma once

#include <Windows.h>

#include "Buffer.h"

#define HANDLE_TYPE HANDLE
#define NULL_HANDLE INVALID_HANDLE_VALUE

#define YEAR(dt)	(((dt & 0xFE000000) >> (9 + 16)) + 1980)
#define MONTH(dt)	 ((dt & 0x01E00000) >> (5 + 16))
#define DAY(dt)	 ((dt & 0x001F0000) >> (0 + 16))
#define HOUR(dt)	 ((dt & 0x0000F800) >> 11)
#define MINUTE(dt) ((dt & 0x000007E0) >> 5)
#define SECOND(dt) ((dt & 0x0000001F) << 1)

enum ENUM_FILE_ACCESS
{
	FILE_ACCESS_NONE = 0,
	FILE_ACCESS_READ = 1,
	FILE_ACCESS_WRITE = 2,
	FILE_ACCESS_READWRITE = FILE_ACCESS_READ | FILE_ACCESS_WRITE
};

class FileClass
{
public:
	virtual ~FileClass() = default;
	virtual const char* File_Name() const = 0;
	virtual const char* Set_Name(const char* filename) = 0;
	virtual bool Create() = 0;
	virtual bool Delete() = 0;
	virtual bool Is_Available(bool forced = false) = 0;
	virtual bool Is_Open() const = 0;
	virtual bool Open(const char* filename, ENUM_FILE_ACCESS rights = FILE_ACCESS_READ) = 0;
	virtual bool Open(ENUM_FILE_ACCESS rights = FILE_ACCESS_READ) = 0;
	virtual int Read(void* buffer, int size) = 0;
	virtual int Seek(int pos, int dir = SEEK_CUR) = 0;
	virtual int Size() = 0;
	virtual int Write(void* const buffer, int size) = 0;
	virtual void Close() = 0;
	virtual unsigned int Get_Date_Time() { return 0; }
	virtual bool Set_Date_Time(unsigned int dt) { return false; }
	virtual void Error(int error, bool canretry = false, const char* filename = nullptr) = 0;

	operator const char* () { return File_Name(); }
};

class RawFileClass : public FileClass
{
public:
	explicit RawFileClass(const char* filename) noexcept;
	explicit RawFileClass() noexcept;

	RawFileClass(const RawFileClass&) = delete;
	RawFileClass& operator=(const RawFileClass&) = delete;
	RawFileClass(RawFileClass&&) = default;
	RawFileClass& operator=(RawFileClass&&) = default;

	virtual ~RawFileClass() override;

	virtual const char* File_Name() const override;
	virtual const char* Set_Name(const char* filename) override;
	virtual bool Create() override;
	virtual bool Delete() override;
	virtual bool Is_Available(bool forced = false) override;
	virtual bool Is_Open() const override;
	virtual bool Open(const char* filename, ENUM_FILE_ACCESS rights = FILE_ACCESS_READ) override;
	virtual bool Open(ENUM_FILE_ACCESS rights = FILE_ACCESS_READ) override;
	virtual int Read(void* buffer, int size) override;
	virtual int Seek(int pos, int dir = SEEK_CUR) override;
	virtual int Size() override;
	virtual int Write(void* const buffer, int size) override;
	virtual void Close() override;
	virtual unsigned int Get_Date_Time() override;
	virtual bool Set_Date_Time(unsigned int dt) override;
	virtual void Error(int error, bool canretry = false, const char* filename = nullptr) override;

	void Bias(int start, int length = -1);

	HANDLE_TYPE Get_File_Handle() { return Handle; }

protected:
	int Transfer_Block_Size() { return static_cast<int>(UINT_MAX - 16L); }

	int Raw_Seek(int pos, int dir = SEEK_CUR);

private:

public:
	ENUM_FILE_ACCESS Rights;
	int BiasStart;
	int BiasLength;

protected:

private:
	HANDLE_TYPE Handle;

	const char* Filename;

	struct
	{
		unsigned short Day : 5;		// [0, 31]
		unsigned short Month : 4;	// [1, 12]
		unsigned short Year : 7;	// [0, 119], representing [1980, 2099]
	} Date;
	struct
	{
		unsigned short Second : 5;	// [0, 29], second / 2
		unsigned short Minutes : 6;	// [0, 59]
		unsigned short Hours : 5;	// [0, 23]
	} Time;

	/*
	**	Filenames that were assigned as part of the construction process
	**	are not allocated. It is assumed that the filename string is a
	**	constant in that case and thus making duplication unnecessary.
	**	This value will be non-zero if the filename has be allocated
	**	(using strdup()).
	*/
	bool Allocated;
};

class BufferIOFileClass : public RawFileClass
{
public:
	explicit BufferIOFileClass(const char* filename) noexcept;
	explicit BufferIOFileClass() noexcept;

	BufferIOFileClass(const BufferIOFileClass&) = delete;
	BufferIOFileClass& operator=(const BufferIOFileClass&) = delete;
	BufferIOFileClass(BufferIOFileClass&&) = default;
	BufferIOFileClass& operator=(BufferIOFileClass&&) = default;

	virtual ~BufferIOFileClass() override;

	virtual const char* Set_Name(const char* filename) override;
	virtual bool Is_Available(bool forced = false) override;
	virtual bool Is_Open() const override;
	virtual bool Open(const char* filename, ENUM_FILE_ACCESS rights = FILE_ACCESS_READ) override;
	virtual bool Open(ENUM_FILE_ACCESS rights = FILE_ACCESS_READ) override;
	virtual int Read(void* buffer, int size) override;
	virtual int Seek(int pos, int dir = SEEK_CUR) override;
	virtual int Size() override;
	virtual int Write(void* const buffer, int size) override;
	virtual void Close() override;

	bool Cache(int size = 0, void* ptr = nullptr);
	void Free();
	bool Commit();

	static constexpr int MINIMUM_BUFFER_SIZE = 1024;

private:
	bool IsAllocated;
	bool IsOpen;
	bool IsDiskOpen;
	bool IsCached;
	bool IsChanged;
	bool UseBuffer;
	ENUM_FILE_ACCESS BufferRights;
	void* Buffer;
	int BufferSize;
	int BufferPos;
	int BufferFilePos;
	int BufferChangeBegin;
	int BufferChangeEnd;
	int FileSize;
	int FilePos;
	int TrueFileStart;
};

// We don't need to search for CDs, so CDFileClass is abandoned.
// Therefore, CCFileClass just derived from BufferIOFileClass
class CCFileClass : public BufferIOFileClass
{
public:
	explicit CCFileClass(char const* filename) noexcept;
	explicit CCFileClass() noexcept;

	CCFileClass(const CCFileClass&) = delete;
	CCFileClass& operator=(const CCFileClass&) = delete;
	CCFileClass(CCFileClass&&) = default;
	CCFileClass& operator=(CCFileClass&&) = default;

	virtual ~CCFileClass() override { Position = 0; };

	virtual bool Is_Available(bool forced = false) override;
	virtual bool Is_Open() const override;
	virtual bool Open(char const* filename, ENUM_FILE_ACCESS rights = FILE_ACCESS_READ) override;
	virtual bool Open(ENUM_FILE_ACCESS rights = FILE_ACCESS_READ) override;
	virtual int Read(void* buffer, int size) override;
	virtual int Seek(int pos, int dir = SEEK_CUR) override;
	virtual int Size() override;
	virtual int Write(void* const buffer, int size) override;
	virtual void Close() override;
	virtual unsigned int Get_Date_Time() override;
	virtual bool Set_Date_Time(unsigned int datetime) override;
	virtual void Error(int error, bool canretry = false, char const* filename = nullptr) override;

	bool Is_Resident() const { return Data.Get_Buffer() != nullptr; }

	template <typename T>
	int Read(T& item)
	{
		return Read(&item, sizeof(T));
	}

private:
	::Buffer Data;
	int Position;
};

class RAMFileClass : public FileClass
{
public:
	explicit RAMFileClass(void* buffer, int len) noexcept;

	RAMFileClass(const RAMFileClass&) = delete;
	RAMFileClass& operator=(const RAMFileClass&) = delete;
	RAMFileClass(RAMFileClass&&) = default;
	RAMFileClass& operator=(RAMFileClass&&) = default;

	virtual ~RAMFileClass() override;

	virtual const char* File_Name() const override { return "UNKNOWN"; }
	virtual const char* Set_Name(const char*) override { return File_Name(); }
	virtual bool Create() override;
	virtual bool Delete() override;
	virtual bool Is_Available(bool forced = false) override;
	virtual bool Is_Open() const override;
	virtual bool Open(const char* filename, ENUM_FILE_ACCESS access = FILE_ACCESS_READ) override;
	virtual bool Open(ENUM_FILE_ACCESS access = FILE_ACCESS_READ) override;
	virtual int Read(void* buffer, int size) override;
	virtual int Seek(int pos, int dir = SEEK_CUR) override;
	virtual int Size() override;
	virtual int Write(void* const buffer, int size) override;
	virtual void Close() override;
	virtual unsigned int Get_Date_Time() override { return 0; }
	virtual bool Set_Date_Time(unsigned int) override { return true; }
	virtual void Error(int, bool = false, const char* = nullptr) override {}

	operator const char* () { return File_Name(); }

private:
	char* Buffer;
	int MaxLength;
	int Length;
	int Offset;
	int Access;
	bool IsOpen;
	bool IsAllocated;
};