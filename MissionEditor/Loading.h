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

#if !defined(AFX_LOADING_H__5D5C3284_8962_11D3_B63B_AAA51FD322E3__INCLUDED_)
#define AFX_LOADING_H__5D5C3284_8962_11D3_B63B_AAA51FD322E3__INCLUDED_

#include "FinalSunDlg.h"	
#include <memory>
#include <optional>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Loading.h : header file
//

class VoxelNormalTables;

class CFinalSunDlg;


/////////////////////////////////////////////////////////////////////////////
// dialog field CLoading 


class CLoading : public CDialog
{
// Construction
public:
	void LoadStrings();
	void FreeAll();
	void FreeTileSet();
	void InitTMPs();
	
	~CLoading();
	void Unload();
	bool InitMixFiles();
	void LoadTSIni(LPCTSTR lpFilename, CIniFile& ini);
	void CreateINI();
	CLoading(CWnd* pParent = NULL);   // Standardconstructor
	void Load();
	char current_theater;
	

// Dialog data
	//{{AFX_DATA(CLoading)
	enum { IDD = IDD_LOADING };
	CStatic	m_Version;
	CStatic	m_BuiltBy;
	CStatic	m_cap;
	CProgressCtrl	m_progress;
	//}}AFX_DATA


// Overwriteables
	// class wizard generated overwriteables
	//{{AFX_VIRTUAL(CLoading)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:
	void PrepareBuildingTheaters();

	// generated message handlers
	//{{AFX_MSG(CLoading)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void HackRules();
	void PrepareHouses(void);
	int m_pic_count;
	int m_bmp_count;
	bool loaded;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_LOADING_H__5D5C3284_8962_11D3_B63B_AAA51FD322E3__INCLUDED_
