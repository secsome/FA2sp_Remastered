/*
	FinalSun/FinalAlert 2 Mission Editor

	Copyright (C) 1999-2024 Electronic Arts, Inc.
	Authored by Matthias Wagner

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

// IniFile.h: Schnittstelle für die Klasse CIniFile.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INIFILE_H__96455620_6528_11D3_99E0_DB2A1EF71411__INCLUDED_)
#define AFX_INIFILE_H__96455620_6528_11D3_99E0_DB2A1EF71411__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <map>
#include <CString>
#include <fstream>
#include <ios>


using namespace std;

class SortDummy
{
public:
	bool operator() (const CString&, const CString&) const;
};



class CIniFileSection
{
public:
	CIniFileSection();
	virtual ~CIniFileSection();

	CString GetString(const CString& name, const CString& defaultValue = CString()) const;
	const CString* TryGetString(const CString& name) const;
	CString& AccessValueByName(const CString& name);

	auto begin() noexcept
	{
		return values.begin();
	}

	auto begin() const noexcept
	{
		return values.begin();
	}

	auto end() noexcept
	{
		return values.end();
	}

	auto end() const noexcept
	{
		return values.end();
	}

	auto& operator[](const CString& name) noexcept
	{
        return values[name];
    }

	[[deprecated("instead use iterators or for_each")]]
	int GetValueOrigPos(int index) const noexcept;

	[[deprecated("instead use iterators or for_each")]]
	int FindName(CString sval) const noexcept;

	[[deprecated("instead use iterators or for_each")]]
	int FindValue(CString sval) const noexcept;

	[[deprecated("instead use iterators or for_each")]]
	const CString* GetValueName(std::size_t index) const noexcept;

	[[deprecated("instead use iterators or for_each")]]
	const CString* GetValue(std::size_t index) const noexcept;

	[[deprecated("instead use iterators or for_each")]] 
	CString* GetValue(std::size_t index) noexcept;

	const auto& GetOriginalPosition() const
	{
		return value_orig_pos;
	}

public:
	map<CString, CString, SortDummy> values;
	map<CString, int, SortDummy> value_orig_pos;
};

class CIniFile
{
public:
	void DeleteEndingSpaces(BOOL bValueNames, BOOL bValues);
	void DeleteLeadingSpaces(BOOL bValueNames, BOOL bValues);
	
	const CString* GetSectionName(std::size_t Index) const noexcept;
	const CIniFileSection* GetSection(std::size_t index) const;
	CIniFileSection* GetSection(std::size_t index);
	const CIniFileSection* GetSection(const CString& section) const;
	CIniFileSection* GetSection(const CString& section);
	size_t GetKeyCount(const CString& section) const;
	size_t GetSectionCount() const;

	bool DeleteSection(const CString& sectionName);
	bool DeleteKey(const CString& section, const CString& keyName);

	CString GetString(const CString& sectionName, const CString& valueName, const CString& defaultValue = "") const;
	const CString* TryGetString(const CString& sectionName, const CString& valueName) const;
	int GetInteger(const CString& sectionName, const CString& valueName, int defaultValue = 0) const;
	float GetSingle(const CString& sectionName, const CString& valueName, float defaultValue = 0.0f) const;
	double GetDouble(const CString& sectionName, const CString& valueName, double defaultValue = 0.0) const;
	bool GetBoolean(const CString& sectionName, const CString& valueName, bool defaultValue = false) const;

	// Write encoded UUBlock
	std::string ReadBase64String(const CString& sectionName) const;
	size_t WriteBase64String(const CString& sectionName, const void* data, size_t length);

	void Clear();
	WORD InsertFile(const CString& filename, const char* Section, BOOL bNoSpaces = FALSE);
	WORD InsertFile(const std::string& filename, const char* Section, BOOL bNoSpaces = FALSE);
	WORD InsertFile(std::istream& file, const char* Section, BOOL bNoSpaces = FALSE);
	BOOL SaveFile(const CString& Filename) const;
	BOOL SaveFile(const std::string& Filename) const;
	WORD LoadFile(const CString& filename, BOOL bNoSpaces = FALSE);
	WORD LoadFile(const std::string& filename, BOOL bNoSpaces = FALSE);
	WORD LoadFile(std::istream& file, BOOL bNoSpaces = FALSE);

	std::map<unsigned int, CString> ParseIndiciesData(CString pSection);

	static CIniFile Rules;
	static CIniFile Art;
	static CIniFile Sound;
	static CIniFile Eva;
	static CIniFile Theme;
	static CIniFile Ai;
	static CIniFile Temperate;
	static CIniFile Snow;
	static CIniFile Urban;
	static CIniFile NewUrban;
	static CIniFile Lunar;
	static CIniFile Desert;
	static CIniFile FAData;
	static CIniFile FALanguage;
	static CIniFile* CurrentTheater;

	auto begin() noexcept
	{
		return sections.begin();
	}

	auto begin() const noexcept
	{
		return sections.begin();
	}

	auto end() noexcept
	{
		return sections.end();
	}

	auto end() const noexcept
	{
		return sections.end();
	}

	auto& operator[](const CString& section) noexcept
	{
        return sections[section];
    }

	map<CString, CIniFileSection> sections;
	CIniFile();
	virtual ~CIniFile();

private:
	std::string m_filename;
};

#endif // !defined(AFX_INIFILE_H__96455620_6528_11D3_99E0_DB2A1EF71411__INCLUDED_)
