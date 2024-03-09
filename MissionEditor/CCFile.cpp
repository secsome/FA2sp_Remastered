#include "StdAfx.h"

#include "CCFile.h"
#include "MixFile.h"

#include <algorithm>

#include <cstring>

RawFileClass::RawFileClass(const char* filename) noexcept
	: Rights{ FILE_ACCESS_READ }
	, BiasStart{ 0 }
	, BiasLength{ -1 }
	, Handle{ NULL_HANDLE }
	, Filename{ filename }
	, Date{ 0 }
	, Time{ 0 }
	, Allocated{ false }
{
}

RawFileClass::RawFileClass() noexcept
	: Rights{ FILE_ACCESS_READ }
	, BiasStart{ 0 }
	, BiasLength{ -1 }
	, Handle{ NULL_HANDLE }
	, Filename{ nullptr }
	, Date{ 0 }
	, Time{ 0 }
	, Allocated{ false }
{
}

RawFileClass::~RawFileClass()
{
	Close();
	if (Allocated && Filename)
	{
		free((char*)Filename);
		Filename = nullptr;
		Allocated = false;
	}
}

const char* RawFileClass::File_Name() const
{
	return Filename;
}

const char* RawFileClass::Set_Name(const char* filename)
{
	if (Filename != nullptr && Allocated)
	{
		free((char*)Filename);
		Filename = nullptr;
		Allocated = false;
	}

	if (filename == nullptr)
		return nullptr;

	Bias(0);

	Filename = _strdup(filename);

	if (!Filename)
		Error(ENOMEM, false, filename);

	Allocated = true;

	return Filename;
}

bool RawFileClass::Create()
{
	Close();

	if (Open(FILE_ACCESS_WRITE))
	{
		if (BiasLength != -1)
			Seek(0, SEEK_CUR);

		Close();
		return true;
	}

	return false;
}

bool RawFileClass::Delete()
{
	Close();

	if (Filename == nullptr)
	{
		Error(ENOENT, false);
		return false;
	}

	for (;;)
	{
		if (!Is_Available())
			return false;

		if (!DeleteFile(Filename))
		{
			Error(GetLastError(), false, Filename);
			return false;
		}
	}

	return true;
}

bool RawFileClass::Is_Available(bool forced)
{
	if (Filename == nullptr)
		return false;

	if (Is_Open())
		return true;

	if (forced)
	{
		RawFileClass::Open(FILE_ACCESS_READ);
		RawFileClass::Close();
		return true;
	}

	for (;;)
	{
		Handle = CreateFile(Filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (Handle == NULL_HANDLE)
			return false;

		break;
	}

	if (!CloseHandle(Handle))
		Error(GetLastError(), false, Filename);

	Handle = NULL_HANDLE;

	return true;
}

bool RawFileClass::Is_Open() const
{
	return Handle != NULL_HANDLE;
}

bool RawFileClass::Open(const char* filename, ENUM_FILE_ACCESS rights)
{
	Set_Name(filename);
	return Open(rights);
}

bool RawFileClass::Open(ENUM_FILE_ACCESS rights)
{
	Close();

	if (Filename == nullptr)
	{
		Error(ENOENT, false);
		return false;
	}

	Rights = rights;

	for (;;)
	{
		switch (rights)
		{
		case FILE_ACCESS_READ:
			Handle = CreateFile(Filename, GENERIC_READ, FILE_SHARE_READ,
				nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			break;

		case FILE_ACCESS_WRITE:
			Handle = CreateFile(Filename, GENERIC_WRITE, NULL,
				nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			break;

		case FILE_ACCESS_READWRITE:
			Handle = CreateFile(Filename, GENERIC_READ | GENERIC_WRITE, NULL,
				nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			break;

		default:
			errno = EINVAL;
			break;
		}

		if (BiasStart != 0 || BiasLength != -1)
			Seek(0, SEEK_SET);

		if (Handle == NULL_HANDLE)
		{
			Error(GetLastError(), false, Filename);
			return false;
		}

		break;
	}

	return true;
}

int RawFileClass::Read(void* buffer, int size)
{
	int bytesread = 0;
	bool opened = false;

	if (!Is_Open())
	{
		if (!Open(FILE_ACCESS_READ))
			return 0;

		opened = true;
	}

	if (BiasLength != -1)
	{
		int remainder = BiasLength - Seek(0);
		if (size > remainder)
			size = remainder;
	}

	int total = 0;
	while (size > 0)
	{
		bytesread = 0;
		SetErrorMode(SEM_FAILCRITICALERRORS);
		if (!ReadFile(Handle, buffer, size, reinterpret_cast<LPDWORD>(&bytesread), nullptr))
		{
			size -= bytesread;
			total += bytesread;
			Error(GetLastError(), true, Filename);
			SetErrorMode(NULL);
			continue;
		}
		SetErrorMode(NULL);
		size -= bytesread;
		total += bytesread;
		if (bytesread == 0)
			break;
	}
	bytesread = total;

	if (opened)
		Close();

	return bytesread;
}

int RawFileClass::Seek(int pos, int dir)
{
	if (BiasLength != -1)
	{
		switch (dir)
		{
		case SEEK_SET:
			if (pos > BiasLength)
				pos = BiasLength;

			pos += BiasStart;
			break;

		case SEEK_CUR:
			break;

		case SEEK_END:
			dir = SEEK_SET;
			pos += BiasStart + BiasLength;
			break;
		}

		int newpos = Raw_Seek(pos, dir) - BiasStart;
		if (newpos < 0)
			newpos = Raw_Seek(BiasStart, SEEK_SET) - BiasStart;
		if (newpos > BiasLength)
			newpos = Raw_Seek(BiasStart + BiasLength, SEEK_SET) - BiasStart;

		return newpos;
	}

	return Raw_Seek(pos, dir);
}

int RawFileClass::Size()
{
	int size = 0;

	if (BiasLength != -1)
		return BiasLength;

	if (Is_Open())
	{
		size = GetFileSize(Handle, nullptr);

		if (size == 0xFFFFFFFF)
			Error(GetLastError(), false, Filename);
	}
	else
	{
		if (Open())
		{
			size = Size();
			Close();
		}
	}

	BiasLength = size - BiasLength;
	return BiasLength;
}

int RawFileClass::Write(void* const buffer, int size)
{
	int bytesread = 0;
	bool opened = false;

	if (!Is_Open())
	{
		if (!Open(FILE_ACCESS_WRITE))
			return 0;

		opened = true;
	}

	if (!WriteFile(Handle, buffer, size, reinterpret_cast<LPDWORD>(&bytesread), nullptr))
		Error(GetLastError(), false, Filename);

	if (BiasLength != -1)
	{
		if (Raw_Seek(0) > BiasStart + BiasLength)
			BiasLength = Raw_Seek(0) - BiasStart;
	}

	if (opened)
		Close();

	return bytesread;
}

void RawFileClass::Close()
{
	if (Is_Open())
	{
		if (!CloseHandle(Handle))
			Error(GetLastError(), false, Filename);

		Handle = NULL_HANDLE;
	}
}

unsigned int RawFileClass::Get_Date_Time()
{
	BY_HANDLE_FILE_INFORMATION info;

	if (GetFileInformationByHandle(Handle, &info))
	{
		WORD dosdate;
		WORD dostime;
		FileTimeToDosDateTime(&info.ftLastWriteTime, &dosdate, &dostime);
		return (dosdate << 16) | dostime;
	}

	return 0;
}

bool RawFileClass::Set_Date_Time(unsigned int dt)
{
	if (RawFileClass::Is_Open())
	{
		BY_HANDLE_FILE_INFORMATION info;

		if (GetFileInformationByHandle(Handle, &info))
		{
			FILETIME filetime;
			if (DosDateTimeToFileTime(HIWORD(dt), LOWORD(dt), &filetime))
				return SetFileTime(Handle, &info.ftCreationTime, &filetime, &filetime);
		}
	}

	return false;
}

void RawFileClass::Error(int error, bool canretry, const char* filename)
{
}

void RawFileClass::Bias(int start, int length)
{
	if (start == 0)
	{
		BiasStart = 0;
		BiasLength = -1;
		return;
	}

	BiasLength = RawFileClass::Size();
	BiasStart += start;
	if (length != -1 && length < BiasLength)
		BiasLength = length;

	if (BiasLength < 0)
		BiasLength = 0;

	if (Is_Open())
		RawFileClass::Seek(0, SEEK_SET);
}

int RawFileClass::Raw_Seek(int pos, int dir)
{
	if (!Is_Open())
		Error(EBADF, false, Filename);

	switch (dir)
	{
	case SEEK_SET:
		dir = FILE_BEGIN;
		break;

	case SEEK_CUR:
		dir = FILE_CURRENT;
		break;

	case SEEK_END:
		dir = FILE_END;
		break;
	}

	pos = SetFilePointer(Handle, pos, nullptr, dir);

	if (pos == 0xFFFFFFFF)
		Error(GetLastError(), false, Filename);

	return pos;
}

BufferIOFileClass::BufferIOFileClass(const char* filename) noexcept
	: RawFileClass{ filename }
	, IsAllocated{ false }
	, IsOpen{ false }
	, IsDiskOpen{ false }
	, IsCached{ false }
	, IsChanged{ false }
	, UseBuffer{ false }
	, BufferRights{ FILE_ACCESS_NONE }
	, Buffer{ 0 }
	, BufferSize{ 0 }
	, BufferPos{ 0 }
	, BufferFilePos{ 0 }
	, BufferChangeBegin{ -1 }
	, BufferChangeEnd{ -1 }
	, FileSize{ 0 }
	, FilePos{ 0 }
	, TrueFileStart{ 0 }
{
	BufferIOFileClass::Set_Name(filename);
}

BufferIOFileClass::BufferIOFileClass() noexcept
	: RawFileClass{}
	, IsAllocated{ false }
	, IsOpen{ false }
	, IsDiskOpen{ false }
	, IsCached{ false }
	, IsChanged{ false }
	, UseBuffer{ false }
	, BufferRights{ FILE_ACCESS_NONE }
	, Buffer{ 0 }
	, BufferSize{ 0 }
	, BufferPos{ 0 }
	, BufferFilePos{ 0 }
	, BufferChangeBegin{ -1 }
	, BufferChangeEnd{ -1 }
	, FileSize{ 0 }
	, FilePos{ 0 }
	, TrueFileStart{ 0 }
{
}

BufferIOFileClass::~BufferIOFileClass()
{
	Free();
}

const char* BufferIOFileClass::Set_Name(const char* filename)
{
	if (File_Name() && UseBuffer)
	{
		if (strcmp(filename, File_Name()) == 0)
			return File_Name();
		else
		{
			Commit();
			IsCached = false;
		}
	}

	RawFileClass::Set_Name(filename);
	return File_Name();
}

bool BufferIOFileClass::Is_Available(bool forced)
{
	return UseBuffer ? true : RawFileClass::Is_Available();
}

bool BufferIOFileClass::Is_Open() const
{
	return IsOpen && UseBuffer ? true : RawFileClass::Is_Open();
}

bool BufferIOFileClass::Open(const char* filename, ENUM_FILE_ACCESS rights)
{
	Set_Name(filename);
	return BufferIOFileClass::Open(rights);
}

bool BufferIOFileClass::Open(ENUM_FILE_ACCESS rights)
{
	BufferIOFileClass::Close();

	if (UseBuffer)
	{
		BufferRights = rights;
		if (rights != FILE_ACCESS_READ || (rights == FILE_ACCESS_READ && FileSize > BufferSize))
		{
			if (rights == FILE_ACCESS_WRITE)
			{
				RawFileClass::Open(rights);
				RawFileClass::Close();
				rights = FILE_ACCESS_READWRITE;
				TrueFileStart = 0;
			}

			if (TrueFileStart)
			{
				UseBuffer = false;
				Open(rights);
				UseBuffer = true;
			}
			else
				return RawFileClass::Open(rights);

			IsDiskOpen = true;

			if (BufferRights == FILE_ACCESS_WRITE)
				FileSize = 0;
		}
		else
			IsDiskOpen = false;

		BufferPos = 0;
		BufferFilePos = 0;
		BufferChangeBegin = -1;
		BufferChangeEnd = -1;
		FilePos = 0;
		IsOpen = true;
	}
	else
		return RawFileClass::Open(rights);

	return true;
}

int BufferIOFileClass::Read(void* buffer, int size)
{
	bool opened = false;

	if (!Is_Open())
	{
		if (Open())
		{
			TrueFileStart = RawFileClass::Seek(0);
			opened = true;
		}
	}

	if (UseBuffer)
	{
		int sizeread = 0;

		if (BufferRights != FILE_ACCESS_WRITE)
		{
			while (size)
			{
				int sizetoread = std::min(size, BufferSize - BufferPos);

				if (!IsCached)
				{
					int readsize;

					if (FileSize < BufferSize)
					{
						readsize = FileSize;
						BufferFilePos = 0;
					}
					else
					{
						readsize = BufferSize;
						BufferFilePos = FilePos;
					}

					if (TrueFileStart)
					{
						UseBuffer = false;
						Seek(FilePos, SEEK_SET);
						Read(Buffer, BufferSize);
						Seek(FilePos, SEEK_SET);
						UseBuffer = true;
					}
					else
					{
						RawFileClass::Seek(BufferFilePos, SEEK_SET);
						RawFileClass::Read(Buffer, readsize);
					}

					BufferPos = 0;
					BufferChangeBegin = -1;
					BufferChangeEnd = -1;

					IsCached = true;
				}

				memmove((char*)buffer + sizeread, (char*)Buffer + BufferPos, sizetoread);

				sizeread += sizetoread;
				size -= sizetoread;
				BufferPos += sizetoread;
				FilePos = BufferFilePos + BufferPos;

				if (BufferPos == BufferSize)
				{
					Commit();

					BufferPos = 0;
					BufferFilePos = FilePos;
					BufferChangeBegin = -1;
					BufferChangeEnd = -1;

					if (size && FileSize > FilePos)
					{
						if (TrueFileStart)
						{
							UseBuffer = false;
							Seek(FilePos, SEEK_SET);
							Read(Buffer, BufferSize);
							Seek(FilePos, SEEK_SET);
							UseBuffer = true;
						}
						else
						{
							RawFileClass::Seek(FilePos, SEEK_SET);
							RawFileClass::Read(Buffer, BufferSize);
						}
					}
					else
						IsCached = false;
				}
			}
		}
		else
			Error(EACCES);

		size = sizeread;
	}
	else
		size = RawFileClass::Read(buffer, size);

	if (opened)
		Close();

	return size;
}

int BufferIOFileClass::Seek(int pos, int dir)
{
	if (UseBuffer)
	{
		bool adjusted = false;

		switch (dir)
		{
		case SEEK_END:
			FilePos = FileSize;
			break;

		case SEEK_SET:
			FilePos = 0;
			break;

		case SEEK_CUR:
		default:
			break;
		}

		if (TrueFileStart)
		{
			if (pos >= TrueFileStart)
			{
				pos -= TrueFileStart;
				adjusted = true;
			}
		}

		FilePos += pos;

		FilePos = std::clamp(FilePos, 0, FileSize);

		if (FileSize <= BufferSize)
			BufferPos = FilePos;
		else
		{
			if (FilePos >= BufferFilePos && FilePos < BufferFilePos + BufferSize)
				BufferPos = FilePos - BufferFilePos;
			else
			{
				Commit();

				if (TrueFileStart)
				{
					UseBuffer = false;
					Seek(FilePos, SEEK_SET);
					UseBuffer = true;
				}
				else
					RawFileClass::Seek(FilePos, SEEK_SET);

				IsCached = false;
			}
		}

		if (TrueFileStart && adjusted)
			return FilePos + TrueFileStart;

		return FilePos;
	}

	return RawFileClass::Seek(pos, dir);
}

int BufferIOFileClass::Size()
{
	return IsOpen && UseBuffer ? FileSize : RawFileClass::Size();
}

int BufferIOFileClass::Write(void* const buffer, int size)
{
	bool opened = false;

	if (!Is_Open())
	{
		if (!Open(FILE_ACCESS_WRITE))
			return 0;

		TrueFileStart = RawFileClass::Seek(0);
		opened = true;
	}

	if (UseBuffer)
	{
		int sizewritten = 0;

		if (BufferRights != FILE_ACCESS_READ)
		{
			while (size)
			{
				int sizetowrite = std::min(BufferSize - BufferPos, size);

				if (sizetowrite != BufferSize)
				{
					if (!IsCached)
					{
						int readsize;

						if (FileSize < BufferSize)
						{
							readsize = FileSize;
							BufferFilePos = 0;
						}
						else
						{
							readsize = BufferSize;
							BufferFilePos = FilePos;
						}

						if (TrueFileStart)
						{
							UseBuffer = false;
							Seek(FilePos, SEEK_SET);
							Read(Buffer, BufferSize);
							Seek(FilePos, SEEK_SET);
							UseBuffer = true;
						}
						else
						{
							RawFileClass::Seek(BufferFilePos, SEEK_SET);
							RawFileClass::Read(Buffer, readsize);
						}

						BufferPos = 0;
						BufferChangeBegin = -1;
						BufferChangeEnd = -1;

						IsCached = true;
					}
				}

				memmove((char*)Buffer + BufferPos, (char*)buffer + sizewritten, sizetowrite);

				IsChanged = true;
				sizewritten += sizetowrite;
				size -= sizetowrite;

				if (BufferChangeBegin == -1)
				{
					BufferChangeBegin = BufferPos;
					BufferChangeEnd = BufferPos;
				}
				else
				{
					if (BufferChangeBegin > BufferPos)
						BufferChangeBegin = BufferPos;
				}

				BufferPos += sizetowrite;

				if (BufferChangeEnd < BufferPos)
					BufferChangeEnd = BufferPos;

				FilePos = BufferFilePos + BufferPos;

				if (FileSize < FilePos)
					FileSize = FilePos;

				if (BufferPos == BufferSize)
				{
					Commit();

					BufferPos = 0;
					BufferFilePos = FilePos;
					BufferChangeBegin = -1;
					BufferChangeEnd = -1;

					if (size && FileSize > FilePos)
					{
						if (TrueFileStart)
						{
							UseBuffer = false;
							Seek(FilePos, SEEK_SET);
							Read(Buffer, BufferSize);
							Seek(FilePos, SEEK_SET);
							UseBuffer = true;
						}
						else
						{
							RawFileClass::Seek(FilePos, SEEK_SET);
							RawFileClass::Read(Buffer, BufferSize);
						}
					}
					else
						IsCached = false;
				}
			}
		}
		else
			Error(EACCES);

		size = sizewritten;
	}
	else
		size = RawFileClass::Write(buffer, size);

	if (opened)
		Close();

	return size;
}

void BufferIOFileClass::Close()
{
	if (UseBuffer)
	{
		Commit();

		if (IsDiskOpen)
		{
			if (TrueFileStart)
			{
				UseBuffer = false;
				Close();
				UseBuffer = true;
			}
			else
				RawFileClass::Close();

			IsDiskOpen = false;
		}

		IsOpen = false;
	}
	else
		RawFileClass::Close();
}

bool BufferIOFileClass::Cache(int size, void* ptr)
{
	if (Buffer)
	{
		if (size || ptr)
			return false;
		else
			return true;
	}

	FileSize = Is_Available() ? Size() : 0;

	if (size)
	{
		if (size < MINIMUM_BUFFER_SIZE)
		{
			size = MINIMUM_BUFFER_SIZE;

			if (ptr)
				Error(EINVAL);
		}

		BufferSize = size;
	}
	else
		BufferSize = FileSize;

	Buffer = ptr ? ptr : new char[BufferSize];

	if (Buffer)
	{
		IsAllocated = true;
		IsDiskOpen = false;
		BufferPos = 0;
		BufferFilePos = 0;
		BufferChangeBegin = -1;
		BufferChangeEnd = -1;
		FilePos = 0;
		TrueFileStart = 0;

		if (FileSize)
		{
			int readsize;
			bool opened = false;
			int prevpos = 0;

			readsize = std::min(FileSize, BufferSize);

			if (Is_Open())
			{
				prevpos = Seek(0);

				TrueFileStart = RawFileClass::Is_Open() ? RawFileClass::Seek(0) : prevpos;

				if (FileSize <= BufferSize)
				{
					if (prevpos)
						Seek(0, SEEK_SET);

					BufferPos = prevpos;
				}
				else
					BufferFilePos = prevpos;

				FilePos = prevpos;
			}
			else
			{
				if (Open())
				{
					TrueFileStart = RawFileClass::Seek(0);
					opened = true;
				}
			}

			int actual = Read(Buffer, readsize);

			if (actual != readsize)
				Error(EIO);

			if (opened)
				Close();
			else
				Seek(prevpos, SEEK_SET);

			IsCached = true;
		}

		UseBuffer = true;
		return true;
	}

	Error(ENOMEM);
	return false;
}

void BufferIOFileClass::Free()
{
	if (Buffer)
	{
		if (IsAllocated)
		{
			delete[] Buffer;
			IsAllocated = false;
		}

		Buffer = nullptr;
	}

	BufferSize = 0;
	IsOpen = false;
	IsCached = false;
	IsChanged = false;
	UseBuffer = false;
}

bool BufferIOFileClass::Commit()
{
	if (UseBuffer)
	{
		if (IsChanged)
		{
			if (IsDiskOpen)
			{
				RawFileClass::Seek(TrueFileStart + BufferFilePos + BufferChangeBegin, SEEK_SET);
				RawFileClass::Write(Buffer, BufferChangeEnd - BufferChangeBegin);
				RawFileClass::Seek(TrueFileStart + FilePos, SEEK_SET);
			}
			else
			{
				RawFileClass::Open();
				RawFileClass::Seek(TrueFileStart + BufferFilePos + BufferChangeBegin, SEEK_SET);
				RawFileClass::Write(Buffer, BufferChangeEnd - BufferChangeBegin);
				RawFileClass::Close();
			}

			IsChanged = false;
			return true;
		}
	}

	return false;
}

CCFileClass::CCFileClass(char const* filename) noexcept
	: BufferIOFileClass{ filename }
	, Position{ 0 }
{
	CCFileClass::Set_Name(filename);
}

CCFileClass::CCFileClass() noexcept
	: BufferIOFileClass{}
	, Position{ 0 }
{
}

bool CCFileClass::Is_Available(bool forced)
{
	return Is_Open() || MFCD::Offset(File_Name()) ? true : BufferIOFileClass::Is_Available();
}

bool CCFileClass::Is_Open() const
{
	return Is_Resident() ? true : BufferIOFileClass::Is_Open();
}

bool CCFileClass::Open(char const* filename, ENUM_FILE_ACCESS rights)
{
	Set_Name(filename);
	return Open(rights);
}

bool CCFileClass::Open(ENUM_FILE_ACCESS rights)
{
	Close();

	if ((rights & FILE_ACCESS_WRITE) || BufferIOFileClass::Is_Available())
		return BufferIOFileClass::Open(rights);

	MFCD* mixfile = NULL;
	void* pointer = NULL;
	int length = 0;
	int start = 0;
	if (MFCD::Offset(File_Name(), &pointer, &mixfile, &start, &length))
	{
		if (pointer == nullptr && mixfile != nullptr)
		{
			char* dupfile = _strdup(File_Name());
			Open(mixfile->Filename, FILE_ACCESS_READ);
			Set_Name(dupfile);
			free(dupfile);
			Bias(0);
			Bias(start, length);
			Seek(0, SEEK_SET);
		}
		else
		{
			Data.Reset(pointer, length);
			Position = 0;
		}

	}
	else
		return BufferIOFileClass::Open(rights);

	return true;
}

int CCFileClass::Read(void* buffer, int size)
{
	bool opened = false;

	if (!Is_Open())
	{
		if (Open())
			opened = true;
	}

	if (Is_Resident())
	{
		int maximum = Data.Get_Size() - Position;
		size = std::min(maximum, size);

		if (size)
		{
			memmove(buffer, (char*)Data + Position, size);
			Position += size;
		}

		if (opened)
			Close();

		return size;
	}

	int s = BufferIOFileClass::Read(buffer, size);

	if (opened)
		Close();

	return s;
}

int CCFileClass::Seek(int pos, int dir)
{
	if (Is_Resident())
	{
		switch (dir)
		{
		case SEEK_END:
			Position = Data.Get_Size();
			break;

		case SEEK_SET:
			Position = 0;
			break;

		case SEEK_CUR:
		default:
			break;
		}

		return std::clamp(Position + pos, 0, Data.Get_Size());
	}
	return BufferIOFileClass::Seek(pos, dir);
}

int CCFileClass::Size()
{
	if (Is_Resident())
		return(Data.Get_Size());

	if (!BufferIOFileClass::Is_Available())
	{
		int length = 0;
		MFCD::Offset(File_Name(), nullptr, nullptr, nullptr, &length);
		return length;
	}

	return BufferIOFileClass::Size();
}

int CCFileClass::Write(void* const buffer, int size)
{
	if (Is_Resident())
		Error(EACCES, false, File_Name());

	return BufferIOFileClass::Write(buffer, size);
}

void CCFileClass::Close()
{
	Data.Reset();
	Position = 0;
	BufferIOFileClass::Close();
}

unsigned int CCFileClass::Get_Date_Time()
{
	unsigned int datetime;
	MFCD* mixfile;

	datetime = BufferIOFileClass::Get_Date_Time();

	if (!datetime)
	{
		if (MFCD::Offset(File_Name(), nullptr, &mixfile, nullptr, nullptr))
			return CCFileClass(mixfile->Filename).Get_Date_Time();
	}

	return datetime;
}

bool CCFileClass::Set_Date_Time(unsigned int datetime)
{
	bool status;
	MFCD* mixfile;

	status = BufferIOFileClass::Set_Date_Time(datetime);

	if (!status)
	{
		if (MFCD::Offset(File_Name(), nullptr, &mixfile, nullptr, nullptr))
			return CCFileClass(mixfile->Filename).Set_Date_Time(datetime);
	}

	return(status);
}

void CCFileClass::Error(int error, bool canretry, char const* filename)
{
}

RAMFileClass::RAMFileClass(void* buffer, int len) noexcept
	: Buffer{ (char*)buffer }
	, MaxLength{ len }
	, Length{ len }
	, Offset{ 0 }
	, Access{ FILE_ACCESS_READ }
	, IsOpen{ false }
	, IsAllocated{ false }
{
	if (buffer == nullptr && len > 0)
	{
		Buffer = new char[len];
		IsAllocated = true;
	}
}

RAMFileClass::~RAMFileClass()
{
	Close();
	if (IsAllocated)
	{
		delete[] Buffer;
		Buffer = NULL;
		IsAllocated = false;
	}
}

bool RAMFileClass::Create()
{
	if (!Is_Open())
	{
		Length = 0;
		return true;
	}
	return false;
}

bool RAMFileClass::Delete()
{
	if (!Is_Open())
	{
		Length = 0;
		return true;
	}
	return false;
}

bool RAMFileClass::Is_Available(bool forced)
{
	return true;
}

bool RAMFileClass::Is_Open() const
{
	return IsOpen;
}

bool RAMFileClass::Open(const char* filename, ENUM_FILE_ACCESS access)
{
	return Open(access);
}

bool RAMFileClass::Open(ENUM_FILE_ACCESS access)
{
	if (Buffer == nullptr || Is_Open())
		return false;

	Offset = 0;
	Access = access;
	IsOpen = true;

	switch (access)
	{
	default:
	case FILE_ACCESS_READ:
		break;

	case FILE_ACCESS_WRITE:
		Length = 0;
		break;

	case FILE_ACCESS_READWRITE:
		break;
	}

	return Is_Open();
}

int RAMFileClass::Read(void* buffer, int size)
{
	if (Buffer == nullptr || buffer == nullptr || size == 0)
		return 0;

	bool hasopened = false;
	if (!Is_Open())
	{
		Open(FILE_ACCESS_READ);
		hasopened = true;
	}
	else
	{
		if ((Access & FILE_ACCESS_READ) == 0)
			return 0;
	}

	int tocopy = std::min(size, Length - Offset);
	memmove(buffer, &Buffer[Offset], tocopy);
	Offset += tocopy;

	if (hasopened)
		Close();

	return tocopy;
}

int RAMFileClass::Seek(int pos, int dir)
{
	if (Buffer == nullptr || !Is_Open())
		return Offset;

	int maxoffset = Length;
	if ((Access & FILE_ACCESS_WRITE) != 0)
		maxoffset = MaxLength;

	switch (dir)
	{
	case SEEK_CUR:
		Offset += pos;
		break;

	case SEEK_SET:
		Offset = 0 + pos;
		break;

	case SEEK_END:
		Offset = maxoffset + pos;
		break;
	}

	if (Offset < 0)
		Offset = 0;
	if (Offset > maxoffset)
		Offset = maxoffset;

	Offset = std::clamp(Offset, 0, maxoffset);

	if (Offset > Length)
		Length = Offset;

	return Offset;
}

int RAMFileClass::Size()
{
	return Length;
}

int RAMFileClass::Write(void* const buffer, int size)
{
	if (Buffer == nullptr || buffer == nullptr || size == 0)
		return 0;

	bool hasopened = false;
	if (!Is_Open())
	{
		Open(FILE_ACCESS_WRITE);
		hasopened = true;
	}
	else
	{
		if ((Access & FILE_ACCESS_WRITE) == 0)
			return 0;
	}

	int towrite = std::min(size, MaxLength - Offset);
	memmove(&Buffer[Offset], buffer, towrite);
	Offset += towrite;

	if (Offset > Length)
		Length = Offset;

	if (hasopened)
		Close();

	return towrite;
}

void RAMFileClass::Close()
{
	IsOpen = false;
}