#include "StdAfx.h"

/*
* NOTICE
* Implement of this class could be really composite and
* hard to understand. Not only to other users, but for me
* as well, so why not just use it as it's easy to call
* rather than understand how it works, isn't it?
*
* UPDATE ON 3/31/2021
* "I cannot understand this one either now." - secsome
*/

#include "MultimapHelper.h"

#include <set>

#include "functions.h"

MultimapHelper::MultimapHelper(std::initializer_list<CIniFile*> list)
{
    for (auto pINI : list)
        data.push_back(pINI);
}

void MultimapHelper::AddINI(CIniFile* pINI)
{
    data.push_back(pINI);
}

CIniFile* MultimapHelper::GetINIAt(int idx)
{
    return data.at(idx);
}

const CString* MultimapHelper::TryGetString(CString pSection, CString pKey)
{
    for (auto ritr = data.rbegin(); ritr != data.rend(); ++ritr)
    {
        if (auto pRet = (*ritr)->TryGetString(pSection, pKey))
            if (!pRet->IsEmpty())
                return pRet;
    }
    return nullptr;
}

CString MultimapHelper::GetString(CString pSection, CString pKey, CString pDefault)
{
    auto const pResult = TryGetString(pSection, pKey);
    return pResult ? *pResult : pDefault;
}

int MultimapHelper::GetInteger(CString pSection, CString pKey, int nDefault) {
    CString&& pStr = this->GetString(pSection, pKey, "");
    int ret = 0;
    if (sscanf_s(pStr, "%d", &ret) == 1)
        return ret;
    return nDefault;
}

float MultimapHelper::GetSingle(CString pSection, CString pKey, float nDefault)
{
    CString&& pStr = this->GetString(pSection, pKey, "");
    float ret = 0.0f;
    if (sscanf_s(pStr, "%f", &ret) == 1)
    {
        if (strchr(pStr, '%%'))
            ret *= 0.01f;
        return ret;
    }
    return nDefault;
}

bool MultimapHelper::GetBool(CString pSection, CString pKey, bool nDefault) {
    CString&& pStr = this->GetString(pSection, pKey, "");
    switch (toupper(static_cast<unsigned char>(*pStr)))
    {
    case '1':
    case 'T':
    case 'Y':
        return true;
    case '0':
    case 'F':
    case 'N':
        return false;
    default:
        return nDefault;
    }
}

std::vector<CString> MultimapHelper::ParseIndicies(CString pSection, bool bParseIntoValue)
{
    std::vector<CString> ret;
    std::map<unsigned int, CString> tmp;
    std::map<CString, unsigned int> tmp2;
    std::map<CString, CString> tmp3; // Value - Key
    std::map<unsigned int, CIniFile*> belonging;

    for (auto& pINI : data)
    {
        if (pINI)
        {
            auto&& cur = pINI->ParseIndiciesData(pSection);
            for (auto& pair : cur)
            {
                CString value = pINI->GetString(pSection, pair.second, pair.second);
                auto&& unitr = tmp2.find(value);
                if (unitr == tmp2.end())
                {
                    belonging[tmp2.size()] = pINI;
                    tmp2[value] = tmp2.size();
                }
                else
                {
                    belonging[unitr->second] = pINI;
                }
                tmp3[value] = pair.second;
            }
        }
    }

    for (auto& pair : tmp2)
        tmp[pair.second] = pair.first;

    ret.resize(tmp.size());
    size_t idx = 0;
    for (auto& x : tmp)
        ret[idx++] = x.second;

    if (!bParseIntoValue)
        for (size_t i = 0, sz = ret.size(); i < sz; ++i)
            ret[i] = tmp3[ret[i]];

    return ret;
}

std::map<CString, CString, SortDummy> MultimapHelper::GetSection(CString pSection)
{
    std::map<CString, CString, SortDummy> ret;
    int index = 0;
    CString tmp;
    for (auto& pINI : data)
    {
        if (pINI)
            if (auto section = pINI->GetSection(pSection))
            {
                for (const auto& [key, value] : *section)
                {
                    if (IsNoneOrEmpty(key) ||
                        IsNoneOrEmpty(value) ||
                        key == "Name")
                    {
                        ++index;
                        continue;
                    }
                    tmp.Format("%d", index++);
                    ret[tmp] = value;
                }
            }
    }
    return ret;
}