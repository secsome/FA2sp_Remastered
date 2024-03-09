#pragma once

#include "CCFile.h"
#include "ListNode.h"
#include "Straw.h"
#include "CRC.h"

class PKey;
class CCFileClass;

template<class T>
class MixFileClass : public Node<MixFileClass<T>>
{
public:
	static List<MixFileClass> List;

	explicit MixFileClass(char const* filename, PKey const* key) noexcept
		: IsDigest{ false }
		, IsEncrypted{ false }
		, IsAllocated{ false }
		, Filename{ nullptr }
		, Count{ 0 }
		, DataSize{ 0 }
		, DataStart{ 0 }
		, HeaderBuffer{ nullptr }
		, Data{ nullptr }
	{
		T file(filename);
		Filename = _strdup(file.File_Name());
		FileStraw fstraw(file);
		PKStraw pstraw(PKStraw::DECRYPT, RandomStraw::CryptRandom);
		Straw* straw = &fstraw;

		if (!file.Is_Available())
			return;

#pragma pack(push, 1)
		struct
		{
			short count;
			int	size;
		} fileheader;
#pragma pack(pop)
		struct
		{
			short First;
			short Second;
		} alternate;

		int got = straw->Get(&alternate, sizeof(alternate));

		if (alternate.First == 0)
		{
			IsDigest = ((alternate.Second & 0x01) != 0);
			IsEncrypted = ((alternate.Second & 0x02) != 0);

			if (IsEncrypted)
			{
				pstraw.Key(key);
				pstraw.Get_From(&fstraw);
				straw = &pstraw;
			}
			straw->Get(&fileheader, sizeof(fileheader));
		}
		else
		{
			memmove(&fileheader, &alternate, sizeof(alternate));
			straw->Get(((char*)&fileheader) + sizeof(alternate), sizeof(fileheader) - sizeof(alternate));
		}

		Count = fileheader.count;
		DataSize = fileheader.size;

		HeaderBuffer = new SubBlock[Count];
		if (HeaderBuffer == nullptr)
			return;

		straw->Get(HeaderBuffer, Count * sizeof(SubBlock));

		qsort(HeaderBuffer, Count, sizeof(SubBlock), compfunc);

		DataStart = file.Seek(0, SEEK_CUR) + file.BiasStart;

		List.Add_Tail(this);
	}

	MixFileClass(const MixFileClass&) = delete;
	MixFileClass& operator=(const MixFileClass&) = delete;
	MixFileClass(MixFileClass&&) = default;
	MixFileClass& operator=(MixFileClass&&) = default;

	virtual ~MixFileClass() override
	{
		if (Filename)
			free((char*)Filename);

		if (Data != nullptr && IsAllocated)
		{
			delete[] Data;
			IsAllocated = false;
		}
		Data = nullptr;

		if (HeaderBuffer != nullptr)
		{
			delete[] HeaderBuffer;
			HeaderBuffer = nullptr;
		}

		GenericNode::Unlink();
	}

	void Free()
	{
		if (Data != nullptr && IsAllocated)
			delete[] Data;

		Data = nullptr;
		IsAllocated = false;
	}
	bool Cache(const Buffer* buffer = nullptr)
	{
		if (Data != nullptr)
			return true;

		if (buffer != nullptr)
		{
			if (buffer->Get_Size() == 0 || buffer->Get_Size() >= DataSize)
				Data = buffer->Get_Buffer();
		}
		else
		{
			Data = new char[DataSize];
			IsAllocated = true;
		}

		if (Data != nullptr)
		{
			T file(Filename);

			FileStraw fstraw(file);
			Straw* straw = &fstraw;

			SHAStraw sha;
			if (IsDigest)
			{
				sha.Get_From(fstraw);
				straw = &sha;
			}

			file.Open(FILE_ACCESS_READ);
			file.Bias(0);
			file.Bias(DataStart);

			int actual = straw->Get(Data, DataSize);
			if (actual != DataSize)
			{
				delete[] Data;
				Data = nullptr;
				file.Error(EIO);
				return false;
			}

			if (IsDigest)
			{
				char digest1[20];
				char digest2[20];
				sha.Result(digest2);
				fstraw.Get(digest1, sizeof(digest1));
				if (memcmp(digest1, digest2, sizeof(digest1)) != 0)
				{
					delete[] Data;
					Data = nullptr;
					return false;
				}
			}

			return true;
		}
		IsAllocated = false;
		return false;
	}
	static void Free_All()
	{
		MixFileClass<T>* ptr = List.First();
		while (ptr->Is_Valid())
		{
			delete ptr;
			ptr = List.First();
		}
	}
	static bool Free(const char* filename)
	{
		MixFileClass<T>* mixer = Finder(filename);

		if (mixer != nullptr)
		{
			mixer->Free();
			return true;
		}

		return false;
	}
	static bool Cache(const char* filename, const Buffer* buffer = nullptr)
	{
		MixFileClass<T>* mixer = Finder(filename);

		return mixer != nullptr ? mixer->Cache(buffer) : false;
	}
	static bool Offset(const char* filename, void** realptr = nullptr,
		MixFileClass<T>** mixfile = nullptr, int* offset = nullptr, int* size = nullptr)
	{
		MixFileClass<T>* ptr;

		if (filename == nullptr)
			return false;

		char filename_upper[_MAX_PATH];
		strcpy(filename_upper, filename);
		_strupr(filename_upper);
		int crc = Calculate_CRC(filename_upper, strlen(filename_upper));
		SubBlock key;
		key.CRC = crc;

		ptr = List.First();
		while (ptr->Is_Valid())
		{
			SubBlock* block;

			block = (SubBlock*)bsearch(&key, ptr->HeaderBuffer, ptr->Count, sizeof(SubBlock), compfunc);
			if (block != nullptr)
			{
				if (mixfile != nullptr) *mixfile = ptr;
				if (size != nullptr) *size = block->Size;
				if (realptr != nullptr) *realptr = nullptr;
				if (offset != nullptr) *offset = block->Offset;
				if (realptr != nullptr && ptr->Data != nullptr)
					*realptr = (char*)ptr->Data + block->Offset;

				if (ptr->Data == nullptr && offset != nullptr)
					*offset += ptr->DataStart;

				return true;
			}

			ptr = ptr->Next();
		}

		return false;
	}
	static void const* Retrieve(const char* filename)
	{
		void* ptr = 0;
		Offset(filename, &ptr);
		return ptr;
	}

	struct SubBlock
	{
		int CRC;
		int Offset;
		int Size;

		std::strong_ordering operator<=>(const SubBlock& another)
		{
			if (CRC < another.CRC)
				return std::strong_ordering::less;
			else if (CRC > another.CRC)
				return std::strong_ordering::greater;
			else
				return std::strong_ordering::equal;
		}
	};

private:
	static MixFileClass* Finder(char const* filename)
	{
		MixFileClass<T>* ptr = List.First();
		while (ptr->Is_Valid())
		{
			char path[_MAX_PATH];
			char name[_MAX_FNAME];
			char ext[_MAX_EXT];

			_splitpath(ptr->Filename, NULL, NULL, name, ext);
			_makepath(path, NULL, NULL, name, ext);

			if (_stricmp(path, filename) == 0)
				return ptr;

			ptr = ptr->Next();
		}
		return nullptr;
	}
	static int __cdecl compfunc(void const* ptr1, void const* ptr2)
	{
		if (*(const int*)ptr1 < *(int const*)ptr2) return -1;
		if (*(const int*)ptr1 > *(int const*)ptr2) return 1;
		return 0;
	}

public:
	const char* Filename;

private:
	bool IsDigest;
	bool IsEncrypted;
	bool IsAllocated;

	int Count;
	int DataSize;
	int DataStart;
	SubBlock* HeaderBuffer;
	void* Data;
};

using MFCD = MixFileClass<CCFileClass>;