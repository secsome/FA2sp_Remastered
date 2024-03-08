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

// TSOptions.cpp: implementation
//

#include "stdafx.h"
#include "FinalSun.h"
#include "TSOptions.h"
#include "resource.h"
#include "mapdata.h"
#include "variables.h"
#include "functions.h"

extern CFinalSunApp theApp;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CTSOptions 


CTSOptions::CTSOptions(CWnd* pParent /*=NULL*/)
	: CDialog(CTSOptions::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTSOptions)
	//}}AFX_DATA_INIT
}


void CTSOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTSOptions)
	DDX_Control(pDX, IDC_LANGUAGE, m_Language);
	DDX_Control(pDX, IDC_EDIT1, m_TSExe);
	//}}AFX_DATA_MAP	
}


BEGIN_MESSAGE_MAP(CTSOptions, CDialog)
	//{{AFX_MSG_MAP(CTSOptions)
	ON_BN_CLICKED(IDC_CHOOSE, OnChoose)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CTSOptions 

void CTSOptions::OnChoose() 
{
	CFolderPickerDialog fd{ nullptr, OFN_READONLY | OFN_EXPLORER };

	fd.DoModal();

	this->GetDlgItem(IDC_EDIT1)->SetWindowText((LPCTSTR)fd.GetFolderPath());

	delete fd;
}

void CTSOptions::OnOK() 
{
	this->GetDlgItem(IDC_EDIT1)->GetWindowText(m_TSEXE);
	int n=m_Language.GetItemData(m_Language.GetCurSel()); 
	
	m_LanguageName=*language.sections["Languages"].GetValue(n);

	CDialog::OnOK();
}

BOOL CTSOptions::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_TSExe.SetWindowText((LPCTSTR)theApp.m_Options.TSExe);
	
	UpdateData(FALSE);

	int i;
	for(i=0;i<language.sections["Languages"].values.size();i++)
	{
		CString lang=*language.sections["Languages"].GetValue(i);
		lang=language.sections[lang+"Header"].values["Name"];

		

		m_Language.SetItemData(m_Language.AddString(lang),i);
		if (lang=="English")
			m_Language.SetCurSel(i);
	}

	
	return TRUE;  
}
