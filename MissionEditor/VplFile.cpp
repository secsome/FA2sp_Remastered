#include "StdAfx.h"

#include "VplFile.h"

#include "CCFile.h"

VplFile::VplFile(const char* filename)
{
    CCFileClass file{ filename };
    Data.reset(reinterpret_cast<VplStruct*>(new char[file.Size()]));
    if (file.Read(Data.get(), file.Size()) != file.Size())
        throw std::runtime_error("Failed to read VPL file");
    file.Close();
}

VplFile::VplFile(const void* buffer, size_t size, bool copy)
{
    if (copy)
    {
        Data.reset(reinterpret_cast<VplStruct*>(new char[size]));
        std::memcpy(Data.get(), buffer, size);
    }
    else
        Data.reset(reinterpret_cast<VplStruct*>(const_cast<void*>(buffer)));
}
