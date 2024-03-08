#pragma once

#include "StdAfx.h"

#include <vector>
#include <map>

#include "IniFile.h"

class MultimapHelper
{
public:
    MultimapHelper() = default;
    MultimapHelper(std::initializer_list<CIniFile*> list);

    void AddINI(CIniFile* pINI);

    CIniFile* GetINIAt(int idx);

    const CString* TryGetString(CString pSection, CString pKey);
    int GetInteger(CString pSection, CString pKey, int nDefault = 0);
    float GetSingle(CString pSection, CString pKey, float nDefault = 0.0f);
    CString GetString(CString pSection, CString pKey, CString pDefault = "");
    bool GetBool(CString pSection, CString pKey, bool nDefault = false);

    std::vector<CString> ParseIndicies(CString pSection, bool bParseIntoValue = false);
    std::map<CString, CString, SortDummy> GetSection(CString pSection);

private:
    std::vector<CIniFile*> data;
};