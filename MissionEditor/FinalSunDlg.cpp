﻿/*
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

//
// Main dialog implementation
//

#include "stdafx.h"
#include "structs.h"
#include "FinalSun.h"
#include "FinalSunDlg.h"
#include "tsoptions.h"
#include "info.h"
#include "loading.h"
#include "MapOpenDialog.h"
//#include "NewMap->h"
#include "newmapcreatedlg.h"
#include "newmapcreatenewdlg.h"
#include "newmapimportdlg.h"
#include "newmapspdlg.h"
#include "newmaptypedlg.h"
#include "MapValidator.h"
#include "ShutDownDlg.h"
#include "SavingDlg.h"
#include "functions.h"
#include "mapdata.h"
#include "variables.h"
#include "GlobalsDlg.h"
#include "savemapoptionsdlg.h"
#include "bitmap2mapconverter.h"
#include "multisaveoptionsdlg.h"
#include <afxinet.h>
#include "inlines.h"
#include "MapCode.h"
#include "SearchWaypointDlg.h"
#include "userscriptsdlg.h"

#include <sstream>
#include "SHA.h"
#include "Base64.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern ACTIONDATA AD;
extern BOOL bNoDraw;

extern char ExePath[MAX_PATH + 1];
extern const std::string u8ExePath;
extern BOOL bMiniMapClosedByUser;

HCURSOR m_hArrowCursor = NULL;

static UINT indicators[] =
{
	ID_SEPARATOR,           // Sstatus bar 
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};



#define ID_STATBAR 26111

/////////////////////////////////////////////////////////////////////////////
// CFinalSunDlg Dialogfeld

CFinalSunDlg::CFinalSunDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFinalSunDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFinalSunDlg)
		// HINWEIS: Der Klassenassistent fügt hier Member-Initialisierung ein
	//}}AFX_DATA_INIT
	// Beachten Sie, dass LoadIcon unter Win32 keinen nachfolgenden DestroyIcon-Aufruf benötigt
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hGameCursor = AfxGetApp()->LoadCursor(MAKEINTRESOURCE(IDC_EDITOR_ARROW));
	m_hArrowCursor = m_hGameCursor;

}

void CFinalSunDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CFinalSunDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_FILE_QUIT, OnFileQuit)
	ON_WM_SYSCOMMAND()
	ON_COMMAND(ID_OPTIONS_TIBERIANSUNOPTIONS, OnOptionsTiberiansunoptions)
	ON_COMMAND(ID_FILE_OPENMAP, OnFileOpenmap)
	ON_COMMAND(ID_FILE_SAVEAS, OnFileSaveas)
	ON_COMMAND(ID_OPTIONS_EXPORTRULESINI, OnOptionsExportrulesini)
	ON_COMMAND(ID_HELP_INFO, OnHelpInfo)
	ON_WM_SHOWWINDOW()
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_WM_MENUSELECT()
	ON_COMMAND(ID_FILE_RUNTIBERIANSUN, OnFileRuntiberiansun)
	ON_COMMAND(ID_FILE_IMPORTMOD, OnFileImportmod)
	ON_COMMAND(ID_DEBUG_EXPORTMAPPACKNOSECTIONS, OnDebugExportmappacknosections)
	ON_COMMAND(ID_DEBUG_EXPORTMAPPACK, OnDebugExportmappack)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_HELP_TIPOFTHEDAY, OnHelpTipoftheday)
	ON_COMMAND(ID_OPTIONS_SIMPLEVIEW, OnOptionsSimpleview)
	ON_COMMAND(ID_OPTIONS_SHOWMINIMAP, OnOptionsShowminimap)
	ON_COMMAND(ID_FILE_VALIDATEMAP, OnFileValidatemap)
	ON_WM_ENTERIDLE()
	ON_COMMAND(ID_EDIT_BASICSETTINGS, OnEditBasicsettings)
	ON_COMMAND(ID_EDIT_SINGLEPLAYERSETTINGS, OnEditSingleplayersettings)
	ON_WM_SIZING()
	ON_WM_SIZE()
	ON_COMMAND(ID_EDIT_HOUSES, OnEditHouses)
	ON_COMMAND(ID_EDIT_AITRIGGERS, OnEditAitriggers)
	ON_COMMAND(ID_EDIT_AITRIGGERENABLING, OnEditAitriggerenabling)
	ON_COMMAND(ID_EDIT_INIEDITING, OnEditIniediting)
	ON_COMMAND(ID_EDIT_LIGHTING, OnEditLighting)
	ON_COMMAND(ID_EDIT_MAP, OnEditMap)
	ON_COMMAND(ID_EDIT_SCRIPTS, OnEditScripts)
	ON_COMMAND(ID_EDIT_SPECIALFLAGS, OnEditSpecialflags)
	ON_COMMAND(ID_EDIT_TAGS, OnEditTags)
	ON_COMMAND(ID_EDIT_TASKFORCES, OnEditTaskforces)
	ON_COMMAND(ID_EDIT_TEAMS, OnEditTeams)
	ON_COMMAND(ID_EDIT_TRIGGERS, OnEditTriggers)
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_TERRAIN_HEIGHTENGROUND, OnTerrainHeightenground)
	ON_COMMAND(ID_TERRAIN_LOWERGROUND, OnTerrainLowerground)
	ON_COMMAND(ID_TERRAIN_LOWERTILE, OnTerrainLowertile)
	ON_COMMAND(ID_TERRAIN_RAISETILE, OnTerrainRaisetile)
	ON_COMMAND(ID_TERRAIN_FLATTEN, OnTerrainFlatten)
	ON_COMMAND(ID_TERRAIN_CLOAK, OnTerrainCloak)
	ON_COMMAND(ID_TERRAIN_SHOWEVERYTILE, OnTerrainShoweverytile)
	ON_COMMAND(ID_TERRAIN_SHOWALLFIELDS, OnTerrainShowallfields)
	ON_COMMAND(ID_TERRAIN_HIDEFIELD, OnTerrainHidefield)
	ON_COMMAND(ID_EDIT_TRIGGEREDITOR, OnEditTriggereditor)
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_MAPTOOLS_CHANGEMAPHEIGHT, OnMaptoolsChangemapheight)
	ON_COMMAND(ID_EDIT_GLOBALVARIABLES, OnEditGlobalvariables)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_COMMAND(ID_HELP, OnHelpManual)
	ON_COMMAND(ID_HELP_SHOWLOGS, &CFinalSunDlg::OnHelpShowlogs)
	ON_COMMAND(ID_MAPTOOLS_AUTOLEVEL, OnMaptoolsAutolevel)
	ON_UPDATE_COMMAND_UI(ID_EDIT_AITRIGGERENABLING, OnUpdateEditAitriggerenabling)
	ON_UPDATE_COMMAND_UI(ID_EDIT_AITRIGGERS, OnUpdateEditAitriggers)
	ON_UPDATE_COMMAND_UI(ID_EDIT_BASICSETTINGS, OnUpdateEditBasicsettings)
	ON_UPDATE_COMMAND_UI(ID_EDIT_GLOBALVARIABLES, OnUpdateEditGlobalvariables)
	ON_UPDATE_COMMAND_UI(ID_EDIT_HOUSES, OnUpdateEditHouses)
	ON_UPDATE_COMMAND_UI(ID_EDIT_INIEDITING, OnUpdateEditIniediting)
	ON_UPDATE_COMMAND_UI(ID_EDIT_LIGHTING, OnUpdateEditLighting)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MAP, OnUpdateEditMap)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SCRIPTS, OnUpdateEditScripts)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SINGLEPLAYERSETTINGS, OnUpdateEditSingleplayersettings)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SPECIALFLAGS, OnUpdateEditSpecialflags)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TAGS, OnUpdateEditTags)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TASKFORCES, OnUpdateEditTaskforces)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TEAMS, OnUpdateEditTeams)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TRIGGEREDITOR, OnUpdateEditTriggereditor)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TRIGGERS, OnUpdateEditTriggers)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVEAS, OnUpdateFileSaveas)
	ON_UPDATE_COMMAND_UI(ID_FILE_VALIDATEMAP, OnUpdateFileValidatemap)
	ON_UPDATE_COMMAND_UI(ID_MAPTOOLS_AUTOLEVEL, OnUpdateMaptoolsAutolevel)
	ON_UPDATE_COMMAND_UI(ID_MAPTOOLS_CHANGEMAPHEIGHT, OnUpdateMaptoolsChangemapheight)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_SHOWMINIMAP, OnUpdateOptionsShowminimap)
	ON_UPDATE_COMMAND_UI(ID_TERRAIN_CLOAK, OnUpdateTerrainCloak)
	ON_UPDATE_COMMAND_UI(ID_TERRAIN_FLATTEN, OnUpdateTerrainFlatten)
	ON_UPDATE_COMMAND_UI(ID_TERRAIN_HEIGHTENGROUND, OnUpdateTerrainHeightenground)
	ON_UPDATE_COMMAND_UI(ID_TERRAIN_HIDEFIELD, OnUpdateTerrainHidefield)
	ON_UPDATE_COMMAND_UI(ID_TERRAIN_LOWERGROUND, OnUpdateTerrainLowerground)
	ON_UPDATE_COMMAND_UI(ID_TERRAIN_LOWERTILE, OnUpdateTerrainLowertile)
	ON_UPDATE_COMMAND_UI(ID_TERRAIN_RAISETILE, OnUpdateTerrainRaisetile)
	ON_UPDATE_COMMAND_UI(ID_TERRAIN_SHOWALLFIELDS, OnUpdateTerrainShowallfields)
	ON_UPDATE_COMMAND_UI(ID_TERRAIN_SHOWEVERYTILE, OnUpdateTerrainShoweverytile)
	ON_COMMAND(ID_MAPTOOLS_FRONTCLIFF, OnMaptoolsFrontcliff)
	ON_COMMAND(ID_MAPTOOLS_BACKCLIFF, OnMaptoolsBackcliff)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_COMMAND(ID_MAPTOOLS_AUTOCREATESHORES, OnMaptoolsAutocreateshores)
	ON_COMMAND(ID_OPTIONS_DISABLEAUTOSHORE, OnOptionsDisableautoshore)
	ON_COMMAND(ID_OPTIONS_DISABLEAUTOLAT, OnOptionsDisableautolat)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_COPYWHOLEMAP, OnEditCopywholemap)
	ON_COMMAND(ID_EDIT_PASTEWHOLEMAP, OnEditPastewholemap)
	ON_COMMAND(ID_MARBLEMADNESS, OnMarblemadness)
	ON_COMMAND(ID_OPTIONS_SOUNDS, OnOptionsSounds)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_SOUNDS, OnUpdateOptionsSounds)
	ON_COMMAND(ID_OPTIONS_DISABLESLOPECORRECTION, OnOptionsDisableslopecorrection)
	ON_COMMAND(ID_OPTIONS_SHOWBUILDINGOUTLINE, OnOptionsShowbuildingoutline)
	ON_COMMAND(ID_FILE_FILE1, OnFileFile1)
	ON_COMMAND(ID_FILE_FILE2, OnFileFile2)
	ON_COMMAND(ID_FILE_FILE3, OnFileFile3)
	ON_COMMAND(ID_FILE_FILE4, OnFileFile4)
	ON_COMMAND(ID_MAPTOOLS_SEARCHWAYPOINT, OnMaptoolsSearchwaypoint)
	ON_COMMAND(ID_MAPTOOLS_TOOLSCRIPTS, OnMaptoolsToolscripts)
	ON_COMMAND(ID_OPTIONS_SMOOTHZOOM, &OnOptionsSmoothzoom)
	ON_WM_SETCURSOR()
	ON_COMMAND(ID_OPTIONS_USEDEFAULTMOUSECURSOR, &OnOptionsUsedefaultmousecursor)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFinalSunDlg message handler

BOOL CFinalSunDlg::OnInitDialog()
{

	CDialog::OnInitDialog();

	m_hArrowCursor = theApp.m_Options.useDefaultMouseCursor ? LoadCursor(NULL, IDC_ARROW) : m_hGameCursor;

	errstream << "CFinalSunDlg::OnInitDialog() called" << endl;
	errstream.flush();
	
	ShowWindow(SW_HIDE);
	SetWindowPos(NULL, 0, 0, GetSystemMetrics(SM_CXFULLSCREEN), GetSystemMetrics(SM_CYFULLSCREEN), 0);

	SetIcon(m_hIcon, TRUE);			// use big symbol
	SetIcon(m_hIcon, FALSE);		// use small symbol
	
	CString cap;
	cap=GetLanguageStringACP("MainDialogCaptionRA2");

	cap+=" (";
	cap+=GetLanguageStringACP("NoMapLoaded");
	cap+=")";
	SetWindowText(cap);


	// Matze:
	// July 9th:
	// Added toolbars
	
	BOOL success=true;
	RECT r;
	SIZE size_mainbar;
	SIZE size_terrainbar;
	GetClientRect(&r);
	r.top+=32;
	success = success && m_view.Create(NULL, GetLanguageStringACP("IsoCaption"), WS_CHILD, r, this);

	r.top=0;
	r.bottom=32;
	success = success && m_bar.Create(RBS_AUTOSIZE, r, this, 5000);
	success = success && m_maintoolbar.Create(this);
	success = success && m_maintoolbar.LoadToolBar(IDR_MAINFRAME);
	m_maintoolbar.GetToolBarCtrl().SetStyle(m_maintoolbar.GetToolBarCtrl().GetStyle() | TBSTYLE_FLAT | TBSTYLE_TRANSPARENT | TBSTYLE_TOOLTIPS);
	success = success && m_maintoolbar.GetToolBarCtrl().GetMaxSize(&size_mainbar);
	success = success && m_terraintoolbar.Create(this);
	success = success && m_terraintoolbar.LoadToolBar(IDR_TERRAINBAR);
	m_terraintoolbar.GetToolBarCtrl().SetStyle(m_terraintoolbar.GetToolBarCtrl().GetStyle() | TBSTYLE_FLAT | TBSTYLE_TRANSPARENT | TBSTYLE_TOOLTIPS );
	success = success && m_terraintoolbar.GetToolBarCtrl().GetMaxSize(&size_terrainbar);
	m_terraintoolbar.SetBarStyle(m_terraintoolbar.GetBarStyle());
	success = success && m_clifftoolbar.Create(this);
	success = success && m_clifftoolbar.LoadToolBar(IDR_CLIFFBAR);
	m_clifftoolbar.GetToolBarCtrl().SetStyle(m_clifftoolbar.GetToolBarCtrl().GetStyle() | TBSTYLE_FLAT | TBSTYLE_TRANSPARENT | TBSTYLE_TOOLTIPS );
	m_clifftoolbar.SetBarStyle(m_terraintoolbar.GetBarStyle());

	success = success && m_settingsbar.Create(this, IDD_TOOLSETTINGS, CBRS_TOP, 6000);
	
	REBARBANDINFO rbi = { 0 };
	rbi.cbSize= sizeof(REBARBANDINFO);
	rbi.fMask= RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_STYLE | RBBIM_TEXT /*| RBBIM_SIZE*/ | RBBIM_IDEALSIZE;
	rbi.fStyle= RBBS_GRIPPERALWAYS;
	rbi.cxMinChild= size_mainbar.cx+30;
	rbi.cyMinChild= 30;
	rbi.cch= 0;
	rbi.cx= size_mainbar.cx+30; 
	rbi.cxIdeal=size_mainbar.cx+30;
	rbi.hbmBack= NULL;
	rbi.hwndChild= (HWND)m_maintoolbar;
	success = success && m_bar.InsertBand(0, &rbi);
	rbi.hwndChild= (HWND)m_terraintoolbar;
	rbi.cx=size_terrainbar.cx+30;
	rbi.cxIdeal=size_terrainbar.cx+30;
	rbi.cxMinChild= size_terrainbar.cx+30;
	success = success && m_bar.InsertBand(1, &rbi);
	rbi.hwndChild = (HWND)m_clifftoolbar;
	rbi.cx = 560;
	rbi.cxMinChild = 170;
	rbi.cxIdeal = 560;
	success = success && m_bar.InsertBand(2, &rbi);
	rbi.hwndChild= (HWND)m_settingsbar;
	rbi.cxMinChild= 170;
	rbi.cx=170;
	rbi.cxIdeal=170;
	success = success && m_bar.InsertBand(3, &rbi);
	

	m_bar.ShowWindow(SW_SHOW);
	m_maintoolbar.ShowWindow(SW_SHOW);
	m_terraintoolbar.ShowWindow(SW_SHOW);
	m_settingsbar.ShowWindow(SW_SHOW);

	m_bar.MinimizeBand(0);
	m_bar.MaximizeBand(3);



	if(!success) 
	{
		// sometimes, insufficient resources are available.
		// this is a very rare case - but it happened on Win9x
		// especially when running the developer studio and IE5, this happens often
		// 04/02/2000: Should not anymore occur.
		MessageBox(GetLanguageStringACP("Err_InsufficientResources"));
		exit(-5);
	}

	
	errstream << "Updating menu" << endl;
	errstream.flush();

	UpdateStrings();




	// Matze:
	// June 21, CLoading now non-modal.
	// CLoading will  free itself
	// September 6th, CLoading will not free itself as it is a member of CFinalSunApp
	if(theApp.m_loading->m_hWnd ==NULL)
	{
		MessageBox("Loading dialog could not be created. Please close any programs and start again.");
		exit(-99);
	}
	theApp.m_loading->ShowWindow(SW_SHOW);
	theApp.m_loading->UpdateWindow();
	theApp.m_loading->Load();

	if(!theApp.m_Options.bSupportMarbleMadness)
	{
		TBBUTTON b;
		m_terraintoolbar.GetToolBarCtrl().GetButton(9, &b);
		
		m_terraintoolbar.GetToolBarCtrl().HideButton(b.idCommand);
		m_terraintoolbar.GetToolBarCtrl().HideButton(ID_MARBLEMADNESS);
	}
	
	ShowWindow(SW_SHOWMAXIMIZED);
	CDialog::BringWindowToTop();
	
	if(strlen(MapPath)==0) // no map file specified
	{
		// ok, let the user choose a map!
		// hmm... no, don´t let him. we already have our tips dialog.
		// OnFileOpenmap();

		theApp.ShowTipAtStartup();
	}
	else // yah, map file specified
	{
		CString str = GetLanguageStringACP("MainDialogCaption");
 		str+=" (";
		str+=MapPath;
 		str+=")";
 
		this->SetWindowText(str);
		SetCursor(LoadCursor(NULL, IDC_WAIT));
 
		Map->LoadMap(MapPath);
 
		SetCursor(m_hArrowCursor);

	}

	UpdateDialogs();

	return TRUE;  
}

void CFinalSunDlg::OnPaint() 
{
	
	if (IsIconic())
	{
		CPaintDC dc(this); // DC for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// center symbol
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// draw symbol
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}

	
}

HCURSOR CFinalSunDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CFinalSunDlg::OnFileQuit() 
{
	UnloadAll();
}


void CFinalSunDlg::OnSysCommand(UINT nID, LPARAM lParam) 
{	
	if(nID==SC_CLOSE)
	{
		UnloadAll();
		return;
	}
	CDialog::OnSysCommand(nID, lParam);
}



void CFinalSunDlg::OnOK() 
{
	//just do nothing...
	// this is a stub
}

void CFinalSunDlg::OnCancel() 
{
	//do nothing...
	// stub
}

void CFinalSunDlg::OnOptionsTiberiansunoptions() 
{
	
	ShowOptionsDialog();
	
}

void CFinalSunDlg::OnFileOpenmap() 
{
	
	//CMapOpenDialog dlg(TRUE, NULL, NULL,  OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST, "TS maps|*.mpr;*.map|TS multi maps|*.mpr|TS single maps|*.map|");
	CString r=GetLanguageStringACP("SAVEDLG_FILETYPES_YR");
    r=TranslateStringVariables(8, r, ";");

	CFileDialog dlg(TRUE, NULL, NULL,  OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST, r);

	char cuPath[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, cuPath);
	dlg.m_ofn.lpstrInitialDir=cuPath;

	if(dlg.DoModal()==IDCANCEL) return;	

	m_PKTHeader.Clear();

	CString fileToOpen=dlg.GetPathName();
	fileToOpen.MakeLower();	
	CString ext=dlg.GetFileExt();
	ext.MakeLower();
	
	CIniFile f;
	f.InsertFile(fileToOpen, "Map");
	if((f.sections["Map"].values["Theater"]==THEATER0 && theApp.m_Options.bDoNotLoadTemperateGraphics) || (f.sections["Map"].values["Theater"]==THEATER1 && theApp.m_Options.bDoNotLoadSnowGraphics))
	{
		MessageBox("You have selected to don´t show temperate or snow theater, but this map uses this theater. You cannot load it without restarting FinalSun/FinalAlert 2 with this theater enabled.", "Error");
		return;
	}

	bNoDraw=TRUE;

	CString str;
	str = GetLanguageStringACP("MainDialogCaption");
	str+=" (";
	str+=(char*)(LPCTSTR)dlg.GetPathName();
	str+=")";

	// MW 07/20/01: Update prev. files
	InsertPrevFile(dlg.GetPathName());

	this->SetWindowText(str);

	SetCursor(LoadCursor(NULL, IDC_WAIT));

	errstream << "Map->LoadMap() will be called" << endl;
	errstream.flush();

	

	Map->LoadMap((char*)(LPCTSTR)fileToOpen);

	
	BOOL bNoMapFile=FALSE;
	if(!Map->CheckMapPackData())
	{
		int res=MessageBox("This map seems to be corrupt. Do you want to try repairing it? If you click cancel, a empty map will be created, if you click no, it will load the map as it is","Corrupt", MB_YESNOCANCEL);
		if(res==IDCANCEL)
		{
			Map->CreateMap(32,32,THEATER0,0);
			bNoMapFile=TRUE;
		}
		else
		{
			if(res==IDYES) // try repair
			{
				int fielddata_size=Map->GetIsoSize()*Map->GetIsoSize();

				int i;
				for(i=0;i<fielddata_size;i++)
				{
					int gr=Map->GetFielddataAt(i)->wGround;
					if(gr==0xFFFF) gr=0;
					
					if(gr>=(*tiledata_count))
					{
						Map->SetTileAt(i, 0, 0);					
					}
					else
					{				
						if((*tiledata)[gr].wTileCount<=Map->GetFielddataAt(i)->bSubTile)
						{
							Map->SetTileAt(i, 0, 0);
						}
					}
				}

			}
		}
	}
	
	if(!bNoMapFile)
		strcpy(MapPath, fileToOpen);

	Sleep(200);

	SetCursor(m_hArrowCursor);


	bNoDraw=FALSE;

	m_view.m_isoview->UpdateDialog(TRUE);
	UpdateDialogs();
}

void CFinalSunDlg::UpdateDialogs(BOOL bOnlyMissionControl, BOOL bNoRepos)
{
	RepairRulesHouses();

	if(m_basic.m_hWnd) m_basic.UpdateDialog();
	if(m_all.m_hWnd) m_all.UpdateDialog();
	if(m_map.m_hWnd) m_map.UpdateDialog();
	if(m_lighting.m_hWnd) m_lighting.UpdateDialog();
	if(m_specialflags.m_hWnd) m_specialflags.UpdateDialog();
	if(m_teamtypes.m_hWnd) m_teamtypes.UpdateDialog();
	if(m_houses.m_hWnd) m_houses.UpdateDialog();
	if(m_taskforces.m_hWnd) m_taskforces.UpdateDialog();
	if(m_Scripttypes.m_hWnd) m_Scripttypes.UpdateDialog();
	if(m_triggers.m_hWnd) m_triggers.UpdateDialog();
	if(m_triggereditor.m_hWnd) m_triggereditor.UpdateDialog();
	if(m_tags.m_hWnd) m_tags.UpdateDialog();
	if(m_aitriggertypesenable.m_hWnd) m_aitriggertypesenable.UpdateDialog();
	if(m_aitriggertypes.m_hWnd) m_aitriggertypes.UpdateDialog();
	if(m_singleplayersettings.m_hWnd) m_singleplayersettings.UpdateDialog();

	CIniFile& ini=Map->UpdateAndGetIniFile();
	if(ini.sections.find(MAPHOUSES)!=ini.sections.end() && ini.sections[MAPHOUSES].values.size()>0)
	{
		if(ini.sections[MAPHOUSES].FindValue("Neutral")>=0)
			currentOwner="Neutral";
		else
			currentOwner=*ini.sections[MAPHOUSES].GetValue(0);
	}
	else
		currentOwner="Neutral";//*CIniFile::Rules.sections[HOUSES].GetValue(0);

	if(!bOnlyMissionControl)
	{
		UpdateWindow();
		UpdateStrings();



		m_view.m_isoview->UpdateDialog(!bNoRepos);
		m_view.m_objectview->UpdateDialog();
		m_view.m_minimap.UpdateView();

		if(CIniFile::CurrentTheater!=NULL && tiledata!=NULL && tiledata_count!=NULL)
		m_view.m_browser->m_bar.Update();
	}

	AD.reset();
}

void CFinalSunDlg::OnFileSaveas() 
{
	if(Map->GetIsoSize()==0)
	{
		Sound(SOUND_NEGATIVE);
		return;
	}

	CMapValidator validator;
	int iCancel=validator.DoModal();
	if(iCancel==IDCANCEL) return;
	CString r=GetLanguageStringACP("SAVEDLG_FILETYPES_YR");
    r=TranslateStringVariables(8, r, ";");

	CString ext, fname;
	if(Map->IsMultiplayer())
	{
		ext=".yrm";
		fname="noname.yrm";
	}
	else{
		ext=".map";
		fname="noname.map";
	}

	CFileDialog dlg(FALSE, ext, /*fname*/ NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, r);
	char cuPath[MAX_PATH];
	
	GetCurrentDirectory(MAX_PATH, cuPath);
	dlg.m_ofn.lpstrInitialDir=cuPath;

	if(dlg.DoModal()!=IDCANCEL)
	{
		strcpy(MapPath, dlg.GetPathName());

		CString str=GetLanguageStringACP("MainDialogCaption");
		str+=" (";
		str+=(char*)(LPCTSTR)dlg.GetPathName();
		str+=")";

		this->SetWindowText(str);
		
		SaveMap(dlg.GetPathName());
	}

	

	SetCursor(m_hArrowCursor);

}

void CFinalSunDlg::OnOptionsExportrulesini() 
{
	MessageBox("Not implemented");
}

void CFinalSunDlg::OnHelpInfo() 
{
	// handle the help dialog
	CInfo info;
	info.DoModal();
}


void CFinalSunDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);

	m_view.ShowWindow(SW_SHOW);

	if(!bShow) return;
	
	
	
}




void CFinalSunDlg::OnFileSave() 
{
	if(Map->GetIsoSize()==0)
	{
		Sound(SOUND_NEGATIVE);
		return;
	}

	if(strlen(MapPath)==0) { OnFileSaveas(); return; }
	
	CMapValidator validator;
	int iCancel=validator.DoModal();
	if(iCancel==IDCANCEL) return;

	SaveMap(MapPath);		
}




void CFinalSunDlg::SaveMap(CString FileName_)
{
	SetCursor(LoadCursor(NULL, IDC_WAIT));

	errstream << "SaveMap() called" << endl;
	errstream.flush();
	BOOL hidePreview=FALSE;
	BOOL previewPrinted=FALSE;
	FileName_.MakeLower();
	FileName_=(LPCSTR)FileName_; // GetLength() needs to return proper size


	CString CoreName=FileName_;
	CString Description;
	int minplayers;
	int maxplayers;
	BOOL standard;
	BOOL meatgrind;
	BOOL navalwar;
	BOOL nukewar;
	BOOL airwar;
	BOOL megawealth;
	BOOL duel;
	BOOL cooperative;
	// MW Apr 17th, 2002: Added Teamgame!
	BOOL teamgame;

	if(CoreName.ReverseFind('\\')>=0) CoreName=CoreName.Right(CoreName.GetLength()-CoreName.ReverseFind('\\')-1);
	if(CoreName.Find(".")>=0) CoreName=CoreName.Left(CoreName.Find("."));

	if(Map->IsMultiplayer())
	{
		if(FileName_.Find(".map")>=0) FileName_.Replace(".map",".mpr");
		
		// MW 07/27/01: Check for YRM
		if(FileName_.Find(".mpr")>=0 && Map->IsYRMap()) FileName_.Replace(".mpr",".yrm");

		
		// MW 07/28/01: Create [Header]
		int i;
		int wp_count=0;
		int xw[8]={0,0,0,0,0,0,0,0};
		int yw[8]={0,0,0,0,0,0,0,0};
		for(i=0;i<Map->GetWaypointCount();i++)
		{
			CString id;
			DWORD pos;
			Map->GetWaypointData(i, &id, &pos);
			int idi;
			idi=atoi(id);
			if(idi!=i) break;
			if(idi>=0 && idi<8) 
			{
				int x,y;
				x=pos/Map->GetIsoSize();
				y=pos%Map->GetIsoSize();
				//PosToXY(pos, x,y);
				xw[wp_count]=calcXPos(x,y);
				yw[wp_count]=calcYPos(x,y);
				wp_count++;	
			}
			
		}
		char c[50];
		CIniFile& ini=Map->UpdateAndGetIniFile();
		CIniFileSection& sec=ini.sections["Header"];
		itoa(wp_count, c, 10);
		sec.values["NumberStartingPoints"]=c;
		for(i=0;i<8;i++)
		{
			CString s="Waypoint";
			itoa(i+1, c, 10);
			s+=c;
			itoa(xw[i], c, 10);
			CString val=c;
			val+=",";
			itoa(yw[i], c, 10);
			val+=c;
			sec.values[s]=val;
		}

		int startx, starty, width, height;
		MC_GetHeaderRect(startx, starty, width, height);

		itoa(height, c, 10);
		sec.values["Height"]=c;
		itoa(width, c, 10); 
		sec.values["Width"]=c;

		//CIniFile& ini=Map->UpdateAndGetIniFile();

		CString left=GetParam(ini.sections["Map"].values["LocalSize"], 0);
		CString top=GetParam(ini.sections["Map"].values["LocalSize"], 1);
		
		//startx=1;//Map->GetHeight()/2;//atoi(left);//Map->GetIsoSize()/2-Map->GetWidth()/2;//198/2-50;//Map->GetIsoSize()/2-Map->GetHeight()/2;//Map->GetWidth()/2-50;
		//starty=Map->GetWidth();//Map->GetIsoSize()/2-Map->GetWidth()/2;//198/2-50;//Map->GetIsoSize()/2-Map->GetWidth()/2;//Map->GetHeight()/2-50;
		itoa(startx, c, 10);
		sec.values["StartX"]=c;
		itoa(starty, c, 10);
		sec.values["StartY"]=c;
	}

	CString MAPFileName=CoreName;
	MAPFileName+=".map";

	DWORD dwFlags=MAPDATA_UPDATE_TO_INI_ALL;

	CSaveMapOptionsDlg opt;


	CString gm = Map->UpdateAndGetIniFile().sections["Basic"].values["GameMode"];
	gm.MakeLower();
	if (gm.GetLength())
	{
		opt.m_Standard = gm.Find("standard") >= 0;
		opt.m_AirWar = gm.Find("airwar") >= 0;
		opt.m_Cooperative = gm.Find("cooperative") >= 0;
		opt.m_Duel = gm.Find("duel") >= 0;
		opt.m_Navalwar = gm.Find("navalwar") >= 0;
		opt.m_Nukewar = gm.Find("nukewar") >= 0;
		opt.m_Meatgrind = gm.Find("meatgrind") >= 0;
		opt.m_Megawealth = gm.Find("megawealth") >= 0;
		opt.m_TeamGame = gm.Find("teamgame") >= 0;
	}
	else
		opt.m_Standard = TRUE;


	if (opt.DoModal() == IDCANCEL) return;

	gm = "";
	if (opt.m_Standard) gm += "standard, ";
	if (opt.m_Meatgrind) gm += "meatgrind, ";
	if (opt.m_Navalwar) gm += "navalwar, ";
	if (opt.m_Nukewar) gm += "nukewar, ";
	if (opt.m_AirWar) gm += "airwar, ";
	if (opt.m_Megawealth) gm += "megawealth, ";
	if (opt.m_Duel) gm += "duel, ";
	if (opt.m_Cooperative) gm += "cooperative, ";
	if (opt.m_TeamGame) gm += "teamgame, ";

	if (gm.ReverseFind(',') >= 0) gm = gm.Left(gm.ReverseFind(','));

	if (gm.GetLength() == 0) gm = "standard";


	Map->UpdateAndGetIniFile().sections["Basic"].values["Name"] = opt.m_MapName;
	Map->UpdateAndGetIniFile().sections["Basic"].values["GameMode"] = gm;

	int i;
	int count = 0;
	for (i = 0; i < Map->GetWaypointCount(); i++)
	{
		CString id;
		DWORD pos;
		Map->GetWaypointData(i, &id, &pos);
		int idi;
		idi = atoi(id);
		if (idi != i) break;
		if (idi >= 0 && idi < 8) count++;
	}

	if (count < 2) count = 2;

	Map->UpdateAndGetIniFile().sections["Basic"].values["MinPlayer"] = "2";
	char c[50];
	itoa(count, c, 10);
	Map->UpdateAndGetIniFile().sections["Basic"].values["MaxPlayer"] = c;

	if (opt.m_Compress == 0) dwFlags |= MAPDATA_UPDATE_TO_INI_ALL_COMPRESSED;
	if (opt.m_PreviewMode == 0) dwFlags |= MAPDATA_UPDATE_TO_INI_ALL_PREVIEW;
	if (opt.m_PreviewMode == 2) hidePreview = TRUE;
	
	SetText("Packing data...");
	UpdateWindow();	
	errstream << "Calling UpdateIniFile()"<<endl;
	
	CSavingDlg dlg;
	dlg.ShowWindow(SW_SHOW);
	dlg.BringWindowToTop();
	dlg.UpdateWindow();
	Map->UpdateIniFile(dwFlags);

	SetText("Saving...");
	UpdateWindow();

	// Save .map only
	{
		auto idx = FileName_.ReverseFind('.');
		if (idx == -1)
			FileName_ += ".map";
		else
			FileName_ = FileName_.Mid(0, idx) + ".map";
	}
	CIniFile& ini = Map->UpdateAndGetIniFile();

	const auto u16FileName = utf8ToUtf16(FileName_.GetString());
	const auto u8Filename = utf16ToUtf8(u16FileName);

	// Remove empty sections and keys
	{
		std::vector<CString> sections_to_remove;
		for (auto& [section_name, section] : ini.sections)
		{
			CString name = section_name;
			name.Trim();
			if (name.GetLength() == 0 || section.values.empty())
			{
				sections_to_remove.push_back(section_name);
				continue;
			}

			std::vector<CString> keys_to_remove;
			for (const auto& [key_name, value] : section.values)
            {
                CString key = key_name;
				key.Trim();
                if (key.GetLength() == 0)
                    keys_to_remove.push_back(key_name);
            }

			for (const auto& key : keys_to_remove)
				section.DeleteKey(key);

			if (section.values.empty())
				sections_to_remove.push_back(section_name);
		}
		for (const auto& section : sections_to_remove)
            ini.sections.erase(section);
	}

	std::ofstream fout;
	fout.open(u8Filename, fout.out | fout.trunc);
	if (!fout.is_open()) [[unlikely]]
		MessageBox("Failed to create file!", "Error", MB_OK | MB_ICONERROR);
	else
	{
		ini.DeleteSection("Digest");

		std::ostringstream oss;
		
		oss <<
			"; Map created with FinalAlert 2(tm) Mission Editor\n"
			"; Get it at http://www.ea.com\n"
			"; note that all comments were truncated\n"
			"\n"
			"; This FA2 modified version for YR is created by secsome\n\n";

		// YR requires Preview and PreviewPack before Map section
		if (const auto section = ini.GetSection("Preview"))
		{
			oss << "[Preview]\n";
			for (const auto& [key, value] : section->values)
                oss << key << "=" << value << "\n";
			oss << "\n";
		}
		if (const auto section = ini.GetSection("PreviewPack"))
		{
            oss << "[PreviewPack]\n";
            for (const auto& [key, value] : section->values)
                oss << key << "=" << value << "\n";
            oss << "\n";
        }

		for (const auto& [section_name, section] : ini)
		{
			if (!strcmp(section_name, "Preview") || !strcmp(section_name, "PreviewPack"))
                continue;

			oss << "[" << section_name << "]\n";
			for (const auto& [key, value] : section.values)
                oss << key << "=" << value << "\n";
			oss << "\n";
		}

		// Generate digest
		SHAEngine sha;
		sha.Hash(oss.str().c_str(), oss.str().size());
		unsigned char digest[20];
		sha.Result(digest);

		// As sha1 hash length is only 20, the length of base64 result won't
		// go over the limitation of uublock's 70 per line. So only one row!
		oss << "[Digest]\n1=" << base64::encode(digest, sizeof(digest)) << "\n";

		// Now just write to the file
		fout << oss.str();
		fout.flush();
		fout.close();
	}

	SetText(TranslateStringVariables(1,GetLanguageStringACP("FileSaved"), u8Filename.c_str()));
	InsertPrevFile(u8Filename.c_str());
	
	SetCursor(m_hArrowCursor);
}

void CFinalSunDlg::SetReady()
{
	m_view.m_statbar.GetStatusBarCtrl().SetText(TranslateStringACP("Ready"), 0,0);
}

void CFinalSunDlg::SetText(const char *text)
{
	m_view.m_statbar.GetStatusBarCtrl().SetText(text,0,0);
}

void CFinalSunDlg::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu) 
{
	

	CDialog::OnMenuSelect(nItemID, nFlags, hSysMenu);
	
	switch(nItemID)
	{
	case ID_FILE_OPENMAP:
		SetText(GetLanguageStringACP("FileOpenHelp"));
		break;
	case ID_FILE_SAVE:
		SetText(GetLanguageStringACP("FileSaveHelp"));
		break;
	case ID_FILE_SAVEAS:
		SetText(GetLanguageStringACP("FileSaveAsHelp"));
		break;
	case ID_FILE_IMPORTMOD:
		SetText(GetLanguageStringACP("FileImportModHelp"));
		break;
	case ID_FILE_QUIT:
		SetText(GetLanguageStringACP("FileQuitHelp"));
		break;
	case ID_FILE_VALIDATEMAP:
		SetText(GetLanguageStringACP("FileCheckMapHelp"));
		break;
	case ID_FILE_RUNTIBERIANSUN:
		SetText(GetLanguageStringACP("FileRunTiberianSunHelp"));
		break;
	case ID_FILE_NEW:
		SetText(GetLanguageStringACP("FileNewHelp"));
		break;
	case ID_MAPVIEW:
		SetText(GetLanguageStringACP("OptionsShowMapViewHelp"));
		break;
	case ID_HELP_INFO:
		SetText(GetLanguageStringACP("HelpInfoHelp"));
		break;
	case ID_HELP_TIPOFTHEDAY:
		SetText(GetLanguageStringACP("HelpTipOfTheDayHelp"));
		break;
	case ID_OPTIONS_EXPORTRULESINI:
		SetText("Export the file CIniFile::Rules.ini");
		break;
	default:
		SetReady();
	}
}


void se_translator(unsigned int e, _EXCEPTION_POINTERS* p)
{
	throw(unsigned int)(e);
}




void CFinalSunDlg::OnFileRuntiberiansun()
{
	std::println(errstream, __FUNCTION__" NOT SUPPORTED YET");
	return;
}




void CFinalSunDlg::OnFileImportmod() 
{
	/*CImportModDlg dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST, "TS maps|*.mpr;*.map|TS multi maps|*.mpr|TS single maps|*.map|");



	char cuPath[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, cuPath);
	dlg.m_ofn.lpstrInitialDir=cuPath;

	//if(theApp.m_Options.TSExe.GetLength()) dlg.m_ofn.lpstrInitialDir=(char*)(LPCTSTR)theApp.m_Options.TSExe;

	if(dlg.DoModal()==IDCANCEL) return;	*/

	
}

void CFinalSunDlg::OnDebugExportmappacknosections() 
{
	CIniFile& ini=Map->UpdateAndGetIniFile();

	CString ovrl;
	int i;


	ovrl="";

	for(i=0;i<ini.sections["IsoMapPack5"].values.size();i++)
	{
		ovrl+=*ini.sections["IsoMapPack5"].GetValue(i);
	}


	BYTE hex[400000];
	BYTE values[400000];

	//int hexlen=ConvertToHex((BYTE*)(LPCTSTR)ovrl, hex);

	
	

	//ExtractIsoMapPack5(hex,hexlen, values);

	HANDLE hFile=CreateFile("c:\\MAPPACKNOSECTIONS.BIN", GENERIC_WRITE, 0, NULL, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		CloseHandle(hFile);
		hFile=CreateFile("c:\\MAPPACKNOSECTIONS.BIN", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	DWORD dw;
	WriteFile(hFile, values, 400000, &dw, NULL);

	CloseHandle(hFile);
	
}

void CFinalSunDlg::OnDebugExportmappack() 
{
	
}

void DeleteTypeList(CString SectionType)
{
	CIniFile& ini=Map->UpdateAndGetIniFile();
	
	int i;
	for(i=0;i<ini.sections[SectionType].values.size();i++)
	{
		ini.sections.erase(*ini.sections[SectionType].GetValue(i));
	}
	ini.sections.erase(SectionType);
}

void CFinalSunDlg::OnFileNew() 
{
	
	CNewMapTypeDlg typedlg;
	if(typedlg.DoModal()==IDCANCEL) return;

	BOOL bSingleplayer=!typedlg.m_Singleplayer;

	CNewMapCreateDlg createdlg;
	if(createdlg.DoModal()==IDCANCEL) return;

	BOOL bCreateNew=!createdlg.m_CreateType;
	
	int width, height, theater, stdheight;
	CString importmap;
	BOOL bImportTrees, bImportUnits, bImportOverlay;
	if(bCreateNew)
	{
		CNewMapCreateNewDlg createnewdlg;
		if(createnewdlg.DoModal()==IDCANCEL) return;
		width=createnewdlg.m_Width;
		height=createnewdlg.m_Height;
		stdheight=createnewdlg.m_StartingHeight;
		theater=createnewdlg.m_Theater;
	}	
	else
	{
		while(!DoesFileExist(importmap))
		{
			CNewMapImportDlg impdlg;
			if(impdlg.DoModal()==IDCANCEL) return;
			importmap=impdlg.m_ImportFile;
			bImportTrees=impdlg.m_ImportTrees;
			bImportUnits=impdlg.m_ImportUnits;
			bImportOverlay=impdlg.m_ImportOverlay;
		}
	}

	CString plhouse;
	BOOL bPrepareHouses=FALSE;
	BOOL bAutoProd=FALSE;
	if(bSingleplayer)
	{
		CNewMapSpDlg spdlg;
		if(spdlg.DoModal()==IDCANCEL) return;
		bPrepareHouses=spdlg.m_PrepareHouses;
		bAutoProd=spdlg.m_AutoProd;
		plhouse=*CIniFile::Rules.sections[HOUSES].GetValue(spdlg.m_House);
	}

	bNoDraw=TRUE;

	m_PKTHeader.Clear();
	
	// first hide the terrain browser window
	m_TerrainDlg.DestroyWindow();

	// set MapPath to nothing and update window caption
	strcpy(MapPath,"");
	CString cap;
	cap=GetLanguageStringACP("MainDialogCaption");
	cap+=" (";
	cap+=GetLanguageStringACP("NewMap");
	cap+=")";
	SetWindowText(cap);

	// set cursor to wait
	SetCursor(LoadCursor(NULL,IDC_WAIT));

	if(!bCreateNew)
	{
		CString file=importmap;
		file.MakeLower();
		if(file.Find(".bmp")>=0)
		{
			CBitmap2MapConverter conv;
			HBITMAP hBitmap=(HBITMAP)LoadImageW(NULL, utf8ToUtf16(file.GetString()).c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
			
			conv.Convert(hBitmap, *Map);
			
			DeleteObject(hBitmap);
		}
		else
		{
			Map->LoadMap(file.GetString());
			
			if(!bImportOverlay) {
				Map->ClearOverlay();
				Map->ClearOverlayData();
			}
			
			CIniFile& ini=Map->UpdateAndGetIniFile();

			int i;
			int count=Map->GetTerrainCount();
			if(!bImportTrees)
			{
				for(i=0;i<count;i++)
						Map->DeleteTerrain(0);
			}

			if(!bImportUnits)
			{
				count=Map->GetInfantryCount();
				for(i=0;i<count;i++)
					Map->DeleteInfantry(0);
				count=Map->GetUnitCount();
				for(i=0;i<count;i++)
					Map->DeleteUnit(0);
				count=Map->GetStructureCount();
				for(i=0;i<count;i++)
					Map->DeleteStructure(0);
				count=Map->GetAircraftCount();
				for(i=0;i<count;i++)
					Map->DeleteAircraft(0);
			}

			ini.sections["Basic"].values["Name"]="Noname";

			for(i=0;i<ini.sections[MAPHOUSES].values.size();i++)
			{
				ini.sections.erase(*ini.sections[MAPHOUSES].GetValue(i));
			}
			ini.sections.erase(MAPHOUSES);

			for(i=0;i<ini.sections[HOUSES].values.size();i++)
			{
				ini.sections.erase(*ini.sections[HOUSES].GetValue(i));
			}
			ini.sections.erase(HOUSES);

			if(TRUE)
			{
				ini.sections.erase("Tags");
				ini.sections.erase("Triggers");
				ini.sections.erase("Actions");
				ini.sections.erase("Events");
				DeleteTypeList("TeamTypes");
				DeleteTypeList("TaskForces");
				DeleteTypeList("ScriptTypes");
				ini.sections.erase("CellTags");

				// ini.sections.erase("AITriggerTypesEnable");
				// ini.sections.erase("AITriggerTypes");
			}
		}
	
	}
	else
	{
		// ok, create a new map
		CString theater_s;
		if(theater==0) theater_s=THEATER0; else if(theater==1) theater_s=THEATER1;
		else if(theater==2) theater_s=THEATER2;  else if(theater==3) theater_s=THEATER3;
		else if(theater==4) theater_s=THEATER4; else if(theater==5) theater_s=THEATER5;
	
		Map->CreateMap(width, height, theater_s, stdheight);	
		
		if(!bSingleplayer)
		{
			last_succeeded_operation=11003;

			// create map function was created for SP. Fix it here;
			Map->DeleteWaypoint(0);
			Map->DeleteWaypoint(0);
			int midx=Map->GetIsoSize()/2;
			int midy=Map->GetIsoSize()/2;
			Map->AddWaypoint("0", midx+midy*Map->GetIsoSize());
			Map->AddWaypoint("1", midx+1+midy*Map->GetIsoSize());
			Map->AddWaypoint("2", midx+2+midy*Map->GetIsoSize());
			Map->AddWaypoint("3", midx+3+midy*Map->GetIsoSize());
			Map->AddWaypoint("4", midx+(midy+1)*Map->GetIsoSize());
			Map->AddWaypoint("5", midx+1+(midy+1)*Map->GetIsoSize());
			Map->AddWaypoint("6", midx+2+(midy+1)*Map->GetIsoSize());
			Map->AddWaypoint("7", midx+3+(midy+1)*Map->GetIsoSize());
		
		}

	}

	
	last_succeeded_operation=11004;


	CIniFile& ini=Map->UpdateAndGetIniFile();

	if(!bSingleplayer) ini.sections["Basic"].values["MultiplayerOnly"]="1";

	if(bSingleplayer)
	{
		ini.sections["Basic"].values["MultiplayerOnly"]="0";

		ini.sections.erase("Preview");
		ini.sections.erase("PreviewPack");
		

		
		
		if(bPrepareHouses)
		{
			//CString plhouse;
			//plhouse=GetHouseSectionName(dlg.m_House);
			plhouse+=" House";
			ini.sections["Basic"].values["Player"]=plhouse;
			
			int i;
			for (i=0;i<CIniFile::Rules.sections[HOUSES].values.size();i++)
			{
				CString j=*CIniFile::Rules.sections[HOUSES].GetValue(i);
				j.MakeLower();
				if(j=="nod" || j=="gdi") continue;

				char c[50];
				int k=i;
				itoa(k,c,10);
				CString country=*CIniFile::Rules.sections[HOUSES].GetValue(i);
				CString house=GetHouseSectionName(country);
				ini.sections[MAPHOUSES].values[c]=house;
				ini.sections[HOUSES].values[c]=country;

				// is it a player house or a ai house?
				if(house!=(LPCTSTR)plhouse)
				{
					ini.sections[house].values["IQ"]="5";
					
					ini.sections[house].values["PlayerControl"]="no";

					// now, if the user wants to, check if this house is a passive or active house
					if(stricmp(CIniFile::Rules.sections[house].values["MultiplayPassive"],"true")!=NULL && bAutoProd)
					{
						CString id=GetFreeID();

						char c[50];
						k=i;
						itoa(i,c,10);
						//k=i+CIniFile::Rules.sections[HOUSES].values.size();
						//itoa(k,c,10);
						
						ini.sections["Triggers"].values[id]=country;
						ini.sections["Triggers"].values[id]+=",<none>,AI Auto Production ";
						ini.sections["Triggers"].values[id]+=TranslateHouse(country, TRUE);
						ini.sections["Triggers"].values[id]+=",0,1,1,1,0";

						ini.sections["Events"].values[id]="1,13,0,10"; // after 10 secs, start prod.

						ini.sections["Actions"].values[id]="3,3,0,";
						ini.sections["Actions"].values[id]+=c;
						ini.sections["Actions"].values[id]+=",0,0,0,0,A,13,0,";
						ini.sections["Actions"].values[id]+=c;
						ini.sections["Actions"].values[id]+=",0,0,0,0,A,74,0,";
						ini.sections["Actions"].values[id]+=c;
						ini.sections["Actions"].values[id]+=",0,0,0,0,A";



						CString ID_TAG=GetFreeID();
						ini.sections["Tags"].values[ID_TAG]="0,AI Auto Production ";
						ini.sections["Tags"].values[ID_TAG]+=TranslateHouse(house, TRUE);
						ini.sections["Tags"].values[ID_TAG]+=",";
						ini.sections["Tags"].values[ID_TAG]+=id;
					}
						
				}
				else
				{
					ini.sections[house].values["IQ"]="0";

					ini.sections[house].values["PlayerControl"]="yes";
				}

				ini.sections[house].values["Country"]=*CIniFile::Rules.sections[HOUSES].GetValue(i);
				ini.sections[house].values["Edge"]="North";
				
				ini.sections[house].values["Color"]=CIniFile::Rules.sections[*CIniFile::Rules.sections[HOUSES].GetValue(i)].values["Color"];
				ini.sections[house].values["Allies"]=house;
				ini.sections[house].values["Credits"]="0";
				ini.sections[house].values["NodeCount"]="0";
				ini.sections[house].values["TechLevel"]="10";
				ini.sections[house].values["PercentBuilt"]="100";

				ini.sections[country].values["ParentCountry"]=country;
				ini.sections[country].values["Name"]=country;
				ini.sections[country].values["Suffix"]=CIniFile::Rules.sections[country].values["Suffix"];
				ini.sections[country].values["Prefix"]=CIniFile::Rules.sections[country].values["Prefix"];
				ini.sections[country].values["Color"]=CIniFile::Rules.sections[country].values["Color"];
				ini.sections[country].values["Side"]=CIniFile::Rules.sections[country].values["Side"];
				ini.sections[country].values["SmartAI"]=CIniFile::Rules.sections[country].values["SmartAI"];
				ini.sections[country].values["CostUnitsMult"]="1";

			}
		}

	}
	else
	{
		// for RA2, we create standard houses
		int i;
		for (i=0;i<CIniFile::Rules.sections[HOUSES].values.size();i++)
		{
			char c[50];
			int k=i;
			itoa(k,c,10);
			CString country=*CIniFile::Rules.sections[HOUSES].GetValue(i);

			if(country.GetLength()==0) continue; // make sure we don´t have an empty entry

			// we now create a HOUSE with the same name as the current rules house
			ini.sections[MAPHOUSES].values[c]=country;
			
			ini.sections[country].values["IQ"]="0";
			ini.sections[country].values["Edge"]="North";
			ini.sections[country].values["Color"]=CIniFile::Rules.sections[country].values["Color"];
			ini.sections[country].values["Allies"]=country;
			ini.sections[country].values["Country"]=country;
			ini.sections[country].values["Credits"]="0";
			ini.sections[country].values["NodeCount"]="0";
			ini.sections[country].values["TechLevel"]="1";
			ini.sections[country].values["PercentBuilt"]="0";
			ini.sections[country].values["PlayerControl"]="no";

		}

	}

	// MW: added March 31st
	// now allow automatic ai trigger enabling
	if(createdlg.m_AITriggers)
	{
		int i;
		for(i=0;i<CIniFile::Ai.sections["AITriggerTypes"].values.size();i++)
		{
			ini.sections["AITriggerTypesEnable"].values[*CIniFile::Ai.sections["AITriggerTypes"].GetValueName(i)]="yes";		
		}
	}

	last_succeeded_operation=11000;

	Map->UpdateIniFile(MAPDATA_UPDATE_FROM_INI);	

	last_succeeded_operation=11001;
	
	Sleep(200);
	
	SetCursor(m_hArrowCursor);

	bNoDraw=FALSE;

	//if(RepairRulesHouses())
	//	MessageBox("Repaired houses");

	m_view.m_isoview->UpdateDialog(TRUE);
	m_view.m_minimap.RedrawWindow();
	UpdateDialogs();

	last_succeeded_operation=11002;
}


/*
	UpdateStrings();
	Updates all dialog CStrings, and the CStrings from the isoview object list and the child dialogs
*/
void CFinalSunDlg::UpdateStrings()
{
	last_succeeded_operation=20;

	CString str;
	
	// update the menu
	CMenu* menu=GetMenu();

	
	// destroy the old menu
	if(menu) menu->DestroyMenu();
	// if(menu) delete menu;

	CMenu* my_menu=new(CMenu);
	// first, we load the original menu (we can´t translate from for example german!)
	my_menu->LoadMenu(IDR_MAIN);

	int i,e;
	for(i=0;i<my_menu->GetMenuItemCount();i++)
	{
		MENUITEMINFO mii;
		ZeroMemory(&mii, sizeof(MENUITEMINFO));
		mii.cbSize=sizeof(MENUITEMINFO);
		mii.fMask=MIIM_ID | MIIM_STATE | MIIM_TYPE;

		my_menu->GetMenuItemInfo(i, &mii, TRUE);
		my_menu->GetMenuString(i, str, MF_BYPOSITION);
		my_menu->ModifyMenu(i,mii.fState | mii.fType | MF_BYPOSITION | MF_STRING, mii.wID, TranslateStringACP((LPCTSTR)str));
		for(e=0;e<my_menu->GetSubMenu(i)->GetMenuItemCount();e++)
		{
			int id=my_menu->GetSubMenu(i)->GetMenuItemID(e);
			
			ZeroMemory(&mii, sizeof(MENUITEMINFO));
			mii.cbSize=sizeof(MENUITEMINFO);
			mii.fMask=MIIM_ID | MIIM_STATE | MIIM_TYPE;
			my_menu->GetSubMenu(i)->GetMenuItemInfo(e, &mii, TRUE);
			my_menu->GetSubMenu(i)->GetMenuString(e, str, MF_BYPOSITION);
			my_menu->GetSubMenu(i)->ModifyMenu(e,mii.fState | mii.fType | MF_BYPOSITION | MF_STRING, mii.wID, TranslateStringACP((LPCTSTR)str));
			
		}
	}

	
	if(theApp.m_Options.bDisableAutoShore) my_menu->CheckMenuItem(ID_OPTIONS_DISABLEAUTOSHORE, MF_BYCOMMAND | MF_CHECKED);
	if(theApp.m_Options.bDisableAutoLat) my_menu->CheckMenuItem(ID_OPTIONS_DISABLEAUTOLAT, MF_BYCOMMAND | MF_CHECKED);
	if(theApp.m_Options.bDisableSlopeCorrection) my_menu->CheckMenuItem(ID_OPTIONS_DISABLESLOPECORRECTION, MF_BYCOMMAND | MF_CHECKED);
	if(theApp.m_Options.bShowCells) my_menu->CheckMenuItem(ID_OPTIONS_SHOWBUILDINGOUTLINE, MF_BYCOMMAND | MF_CHECKED);
	if(theApp.m_Options.useDefaultMouseCursor) my_menu->CheckMenuItem(ID_OPTIONS_USEDEFAULTMOUSECURSOR, MF_BYCOMMAND | MF_CHECKED);
	if(!theApp.m_Options.viewScaleUseSteps) my_menu->CheckMenuItem(ID_OPTIONS_SMOOTHZOOM, MF_BYCOMMAND | MF_CHECKED);

	
	// MW 07/20/01: Show prev. opened files
	int prev_maps_count=0;
	for(i=0;i<4;i++)
	{
		if(theApp.m_Options.prev_maps[i].GetLength()>0)
		{
			prev_maps_count++;

			int id=0;
			CString str="bla";
			str=theApp.m_Options.prev_maps[i];
		
			if(i==0) id=ID_FILE_FILE1;
			else if(i==1) id=ID_FILE_FILE2;
			else if(i==2) id=ID_FILE_FILE3;
			else if(i==3) id=ID_FILE_FILE4;
				
			my_menu->GetSubMenu(0)->InsertMenu(10+prev_maps_count, MF_BYPOSITION | MF_STRING, id, str);
		}
	}


	// my_menu->DeleteMenu(4, MF_BYPOSITION);

#ifndef SCRIPT_SUPPORT
	my_menu->GetSubMenu(3)->DeleteMenu(ID_MAPTOOLS_TOOLSCRIPTS, MF_BYCOMMAND);
#endif

	// now attach this my_menu to the window
	SetMenu(my_menu);

	
	// update the tabs
	/*TCITEM tci;
	memset(&tci, 0, sizeof(TCITEM));
	tci.mask=TCIF_TEXT;
	tci.pszText=(char*)(LPCTSTR)GetLanguageStringACP("TabBasic");
	m_TabCtrl.SetItem(TAB_BASIC, &tci);
	tci.pszText=(char*)(LPCTSTR)GetLanguageStringACP("TabSingleplayerSettings");
	m_TabCtrl.SetItem(TAB_SINGLEPLAYERSETTINGS, &tci);
	tci.pszText=(char*)(LPCTSTR)GetLanguageStringACP("TabMap");
	m_TabCtrl.SetItem(TAB_MAP, &tci);
	tci.pszText=(char*)(LPCTSTR)GetLanguageStringACP("TabLighting");
	m_TabCtrl.SetItem(TAB_LIGHTNING, &tci);
	tci.pszText=(char*)(LPCTSTR)GetLanguageStringACP("TabSpecial");
	m_TabCtrl.SetItem(TAB_SPECIALFLAGS, &tci);
	tci.pszText=(char*)(LPCTSTR)GetLanguageStringACP("TabHouses");
	m_TabCtrl.SetItem(TAB_HOUSES, &tci);
	tci.pszText=(char*)(LPCTSTR)GetLanguageStringACP("TabTaskForces");
	m_TabCtrl.SetItem(TAB_TASKFORCES, &tci);
	tci.pszText=(char*)(LPCTSTR)GetLanguageStringACP("TabScriptTypes");
	m_TabCtrl.SetItem(TAB_SCRIPTTYPES, &tci);
	tci.pszText=(char*)(LPCTSTR)GetLanguageStringACP("TabTeamTypes");
	m_TabCtrl.SetItem(TAB_TEAMTYPES, &tci);
	tci.pszText=(char*)(LPCTSTR)GetLanguageStringACP("TabTriggers");
	m_TabCtrl.SetItem(TAB_TRIGGERS, &tci);
	tci.pszText=(char*)(LPCTSTR)GetLanguageStringACP("TabTags");
	m_TabCtrl.SetItem(TAB_TAGS, &tci);
	tci.pszText=(char*)(LPCTSTR)GetLanguageStringACP("TabAITriggers");
	m_TabCtrl.SetItem(TAB_AITRIGGERTYPES, &tci);
	tci.pszText=(char*)(LPCTSTR)GetLanguageStringACP("TabAITriggerEnable");
	m_TabCtrl.SetItem(TAB_AITRIGGERTYPESENABLE, &tci);
	tci.pszText=(char*)(LPCTSTR)GetLanguageStringACP("TabOther");
	m_TabCtrl.SetItem(TAB_OTHER, &tci);
	*/

	// now the child windows and the iso view
	m_view.m_objectview->UpdateDialog();
	m_view.m_browser->m_bar.Update();
	if(m_basic.m_hWnd) m_basic.UpdateStrings();
	if(m_houses.m_hWnd) m_houses.UpdateStrings();
	if(m_map.m_hWnd) m_map.UpdateStrings();
	if(m_singleplayersettings.m_hWnd) m_singleplayersettings.UpdateStrings();

	// we need to check SimpleView if using easy view
	if(!theApp.m_Options.bNoSounds) GetMenu()->CheckMenuItem(ID_OPTIONS_SOUNDS, MF_BYCOMMAND | MF_CHECKED);

	
	
	RedrawWindow(NULL,NULL,RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
}

void CFinalSunDlg::OnHelpTipoftheday() 
{
	CTipDlg tip;
	tip.DoModal();
	
}

void CFinalSunDlg::UnloadAll()
{
	int iQuit=MessageBox(GetLanguageStringACP("MainDialogExitQuestion"), GetLanguageStringACP("MainDialogExitQuestionCap"), MB_ICONQUESTION | MB_YESNO);
	if(iQuit==IDNO) return;
	else{
		try
		{
			
		CShutDownDlg dlg(this);
		dlg.ShowWindow(SW_SHOW);
		dlg.UpdateWindow();

		tiledata=0;

		theApp.m_loading->FreeAll();
		
		CIniFile::Rules.Clear();
		CIniFile::Ai.Clear();
		CIniFile::Art.Clear();
		CIniFile::Temperate.Clear();
		CIniFile::Snow.Clear();
		CIniFile::Urban.Clear();
		CIniFile::NewUrban.Clear();
		CIniFile::Lunar.Clear();
		CIniFile::Desert.Clear();
		Map->UpdateAndGetIniFile().Clear();
		CIniFile::Sound.Clear();
		CIniFile::FAData.Clear();
		CIniFile::FALanguage.Clear();
	
		DestroyWindow();
		}
		catch(...)
		{
			DestroyWindow();
		}
	}

	
}

void CFinalSunDlg::OnOptionsSimpleview() 
{
}


void CFinalSunDlg::OnOptionsShowminimap() 
{
	bMiniMapClosedByUser=FALSE;
	this->m_view.m_minimap.UpdateView();
}

void CFinalSunDlg::HideAllDialogs()
{
	if (m_aitriggertypes)
		m_aitriggertypes.ShowWindow(SW_HIDE);
	if (m_aitriggertypesenable)
		m_aitriggertypesenable.ShowWindow(SW_HIDE);
	if (m_all)
		m_all.ShowWindow(SW_HIDE);
	if (m_basic)
		m_basic.ShowWindow(SW_HIDE);
	if (m_houses)
		m_houses.ShowWindow(SW_HIDE);
	if (m_lighting)
		m_lighting.ShowWindow(SW_HIDE);
	if (m_map)
		m_map.ShowWindow(SW_HIDE);
	if (m_Scripttypes)
		m_Scripttypes.ShowWindow(SW_HIDE);
	if (m_singleplayersettings)
		m_singleplayersettings.ShowWindow(SW_HIDE);
	if (m_specialflags)
		m_specialflags.ShowWindow(SW_HIDE);
	if (m_tags)
		m_tags.ShowWindow(SW_HIDE);
	if (m_taskforces)
		m_taskforces.ShowWindow(SW_HIDE);
	if (m_teamtypes)
		m_teamtypes.ShowWindow(SW_HIDE);
	if (m_triggers)
		m_triggers.ShowWindow(SW_HIDE);
}

void CFinalSunDlg::OnFileValidatemap() 
{
	CMapValidator validator;
	int iCancel=validator.DoModal();
	if(iCancel==IDCANCEL) return;	
}




void CFinalSunDlg::OnEnterIdle(UINT nWhy, CWnd* pWho) 
{
	CDialog::OnEnterIdle(nWhy, pWho);
	
	
}



void CFinalSunDlg::OnEditBasicsettings() 
{
	if(m_basic.m_hWnd==NULL)
	{		
		if(!m_basic.Create(CBasic::IDD, NULL))
		{
			MessageBox(GetLanguageStringACP("Err_CreateErr"), "Error");
		}
		
	}

	if(m_basic.m_hWnd!=NULL)
	{
		m_basic.UpdateStrings();
		m_basic.UpdateDialog();
		m_basic.ShowWindow(SW_SHOW);
		Sound(SOUND_POSITIVE);
	}
}

void CFinalSunDlg::OnEditSingleplayersettings() 
{
	if(m_singleplayersettings.m_hWnd==NULL)
	{		
		if(!m_singleplayersettings.Create(CSingleplayerSettings::IDD, NULL))
		{
			MessageBox(GetLanguageStringACP("Err_CreateErr"), "Error");
		}
	}

	if(m_singleplayersettings.m_hWnd!=NULL)
	{
		m_singleplayersettings.UpdateStrings();
		m_singleplayersettings.UpdateDialog();
		m_singleplayersettings.ShowWindow(SW_SHOW);
		Sound(SOUND_POSITIVE);
	}	
}

void CFinalSunDlg::OnSizing(UINT fwSide, LPRECT pRect) 
{
	CDialog::OnSizing(fwSide, pRect);

		
}

void CFinalSunDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	if(nType==SIZE_MINIMIZED)
	{
		if(!bMiniMapClosedByUser) m_view.m_minimap.ShowWindow(SW_MINIMIZE);
	}
	else if(nType==SIZE_MAXIMIZED)
	{
		if(!bMiniMapClosedByUser) m_view.m_minimap.ShowWindow(SW_RESTORE);
	}

	RecalcLayout();	
}

void CFinalSunDlg::OnEditHouses() 
{
	if(m_houses.m_hWnd==NULL)
	{		
		if(!m_houses.Create(CHouses::IDD, NULL))
		{
			MessageBox(GetLanguageStringACP("Err_CreateErr"), "Error");
		}
	}

	if(m_houses.m_hWnd!=NULL)
	{
		m_houses.UpdateStrings();
		m_houses.UpdateDialog();
		m_houses.ShowWindow(SW_SHOW);
		Sound(SOUND_POSITIVE);
	}	
}

void CFinalSunDlg::OnEditAitriggers() 
{
	if(m_aitriggertypes.m_hWnd==NULL)
	{		
		if(!m_aitriggertypes.Create(CAITriggerTypes::IDD, NULL))
		{
			MessageBox(GetLanguageStringACP("Err_CreateErr"), "Error");
		}
	}

	if(m_aitriggertypes.m_hWnd!=NULL)
	{
		//m_aitriggertypes.UpdateStrings();
		m_aitriggertypes.UpdateDialog();
		m_aitriggertypes.ShowWindow(SW_SHOW);
		Sound(SOUND_POSITIVE);
		
	}
	
}

void CFinalSunDlg::OnEditAitriggerenabling() 
{
	if(m_aitriggertypesenable.m_hWnd==NULL)
	{		
		if(!m_aitriggertypesenable.Create(CAiTriggerTypesEnable::IDD, NULL))
		{
			MessageBox(GetLanguageStringACP("Err_CreateErr"), "Error");
		}
	}

	if(m_aitriggertypesenable.m_hWnd!=NULL)
	{
		//m_aitriggertypesenable.UpdateStrings();
		m_aitriggertypesenable.UpdateDialog();
		m_aitriggertypesenable.ShowWindow(SW_SHOW);
		Sound(SOUND_POSITIVE);
		
	}
}

void CFinalSunDlg::OnEditIniediting() 
{
	if(m_all.m_hWnd==NULL)
	{		
		if(!m_all.Create(CAll::IDD, NULL))
		{
			MessageBox(GetLanguageStringACP("Err_CreateErr"), "Error");
		}
	}

	if(m_all.m_hWnd!=NULL)
	{
		//m_all.UpdateStrings();
		m_all.UpdateDialog();
		m_all.ShowWindow(SW_SHOW);
		Sound(SOUND_POSITIVE);
	}	
}

void CFinalSunDlg::OnEditLighting() 
{
	if(m_lighting.m_hWnd==NULL)
	{		
		if(!m_lighting.Create(CLighting::IDD, NULL))
		{
			MessageBox(GetLanguageStringACP("Err_CreateErr"), "Error");
		}
	}

	if(m_lighting.m_hWnd!=NULL)
	{
		//m_lighting.UpdateStrings();
		m_lighting.UpdateDialog();
		m_lighting.ShowWindow(SW_SHOW);
		Sound(SOUND_POSITIVE);
	}	
	
}

void CFinalSunDlg::OnEditMap() 
{
	if(m_map.m_hWnd==NULL)
	{		
		if(!m_map.Create(CMapD::IDD, NULL))
		{
			MessageBox(GetLanguageStringACP("Err_CreateErr"), "Error");
		}
	}

	if(m_map.m_hWnd!=NULL)
	{
		m_map.UpdateStrings();
		m_map.UpdateDialog();
		m_map.ShowWindow(SW_SHOW);
		Sound(SOUND_POSITIVE);
	}
}

void CFinalSunDlg::OnEditScripts() 
{
	if(m_Scripttypes.m_hWnd==NULL)
	{		
		if(!m_Scripttypes.Create(CScriptTypes::IDD, NULL))
		{
			MessageBox(GetLanguageStringACP("Err_CreateErr"), "Error");
		}
	}

	if(m_Scripttypes.m_hWnd!=NULL)
	{
		//m_Scripttypes.UpdateStrings();
		m_Scripttypes.UpdateDialog();
		m_Scripttypes.ShowWindow(SW_SHOW);
		Sound(SOUND_POSITIVE);
	}
}

void CFinalSunDlg::OnEditSpecialflags() 
{
	if(m_specialflags.m_hWnd==NULL)
	{		
		if(!m_specialflags.Create(CSpecialFlags::IDD, NULL))
		{
			MessageBox(GetLanguageStringACP("Err_CreateErr"), "Error");
		}
	}

	if(m_specialflags.m_hWnd!=NULL)
	{
		//m_specialflags.UpdateStrings();
		m_specialflags.UpdateDialog();
		m_specialflags.ShowWindow(SW_SHOW);
		Sound(SOUND_POSITIVE);
	}
}

void CFinalSunDlg::OnEditTags() 
{
	if(m_tags.m_hWnd==NULL)
	{		
		if(!m_tags.Create(CTags::IDD, NULL))
		{
			MessageBox(GetLanguageStringACP("Err_CreateErr"), "Error");
		}
	}

	if(m_tags.m_hWnd!=NULL)
	{
		//m_tags.UpdateStrings();
		m_tags.UpdateDialog();
		m_tags.ShowWindow(SW_SHOW);
		Sound(SOUND_POSITIVE);
	}
}

void CFinalSunDlg::OnEditTaskforces() 
{
	if(m_taskforces.m_hWnd==NULL)
	{		
		if(!m_taskforces.Create(CTaskForce::IDD, NULL))
		{
			MessageBox(GetLanguageStringACP("Err_CreateErr"), "Error");
		}
	}

	if(m_taskforces.m_hWnd!=NULL)
	{
		//m_taskforces.UpdateStrings();
		m_taskforces.UpdateDialog();
		m_taskforces.ShowWindow(SW_SHOW);
		Sound(SOUND_POSITIVE);
	}
}

void CFinalSunDlg::OnEditTeams() 
{
	if(m_teamtypes.m_hWnd==NULL)
	{		
		if(!m_teamtypes.Create(CTeamTypes::IDD, NULL))
		{
			MessageBox(GetLanguageStringACP("Err_CreateErr"), "Error");
		}
	}

	if(m_teamtypes.m_hWnd!=NULL)
	{
		//m_teamtypes.UpdateStrings();
		m_teamtypes.UpdateDialog();
		m_teamtypes.ShowWindow(SW_SHOW);
		Sound(SOUND_POSITIVE);
	}
}

void CFinalSunDlg::OnEditTriggers() 
{
	if(m_triggers.m_hWnd==NULL)
	{		
		if(!m_triggers.Create(CTriggers::IDD, NULL))
		{
			MessageBox(GetLanguageStringACP("Err_CreateErr"), "Error");
		}
	}

	if(m_triggers.m_hWnd!=NULL)
	{
		//m_triggers.UpdateStrings();
		m_triggers.UpdateDialog();
		m_triggers.ShowWindow(SW_SHOW);
		Sound(SOUND_POSITIVE);
	}
}

void CFinalSunDlg::OnSetFocus(CWnd* pOldWnd) 
{
	CDialog::OnSetFocus(pOldWnd);

	SetForegroundWindow();
}

BOOL CFinalSunDlg::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult) 
{
	//

	
	
	return CDialog::OnChildNotify(message, wParam, lParam, pLResult);
}

void CFinalSunDlg::RecalcLayout()
{
	if(m_bar.m_hWnd==NULL) return;

	RECT r,rr;
	GetClientRect(&r);
	m_bar.GetWindowRect(&rr);
	int h=rr.bottom-rr.top+2;
	m_bar.GetClientRect(&rr);
	rr.right=r.right;
	m_view.SetWindowPos(NULL, 0, h, r.right, r.bottom-h, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_SHOWWINDOW | SWP_DRAWFRAME);
	m_bar.SetWindowPos(NULL, 0,0, rr.right, rr.bottom, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_SHOWWINDOW | SWP_DRAWFRAME);
	m_bar.ShowBand(0, TRUE);
	m_bar.ShowBand(1, TRUE);
}

BOOL CFinalSunDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	NMHDR* pHead=(NMHDR*)lParam;

	if(pHead->hwndFrom==m_bar.m_hWnd && pHead->code==RBN_HEIGHTCHANGE)
	{
		RecalcLayout();
	}

	if(pHead->code==TTN_NEEDTEXT)
	{
		TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pHead;
		UINT_PTR nID =pHead->idFrom;
		if (pTTT->uFlags & TTF_IDISHWND)
		{
			// idFrom ist der HWND des Tools
			nID = ::GetDlgCtrlID((HWND)nID);
		}
		
		if(nID)
		{
			CString s;
			char c[50];
			itoa(nID, c, 10);			
			
			s.LoadString(nID);
			s=TranslateStringACP(s);
			//pTTT->lpszText = s;
			if(s.GetLength()>80) s.SetAt(80, 0);
			strcpy(pTTT->szText, s);
			pTTT->hinst = NULL;
			return(TRUE);
		}
    
	}

	return CDialog::OnNotify(wParam, lParam, pResult);
}

void CFinalSunDlg::OnTerrainHeightenground() 
{
	if(Map->GetIsoSize()==0)
	{
		Sound(SOUND_NEGATIVE);
		return;
	}

	if(AD.mode!=ACTIONMODE_HEIGHTEN)
	{
		m_settingsbar.m_BrushSize=1;
		m_settingsbar.UpdateData(FALSE);
		m_view.m_isoview->m_BrushSize_x=2;
		m_view.m_isoview->m_BrushSize_y=2;
	}

	Sound(SOUND_POSITIVE);

	AD.mode=ACTIONMODE_HEIGHTEN;
	
}

void CFinalSunDlg::OnTerrainLowerground() 
{
	if(Map->GetIsoSize()==0)
	{
		Sound(SOUND_NEGATIVE);
		return;
	}

	if(AD.mode!=ACTIONMODE_LOWER)
	{
		m_settingsbar.m_BrushSize=1;
		m_settingsbar.UpdateData(FALSE);
		m_view.m_isoview->m_BrushSize_x=2;
		m_view.m_isoview->m_BrushSize_y=2;
	}

	Sound(SOUND_POSITIVE);

	AD.mode=ACTIONMODE_LOWER;	
}

//DEL void CFinalSunDlg::OnTerrainPaint() 
//DEL {
//DEL 	if(m_TerrainDlg.m_hWnd==NULL)
//DEL 	{		
//DEL 		if(!m_TerrainDlg.Create(NULL, "Terrain-Browser", WS_OVERLAPPEDWINDOW, CFrameWnd::rectDefault, NULL, NULL, 0))
//DEL 		{
//DEL 			MessageBox(GetLanguageStringACP("Err_CreateErr"), "Error");
//DEL 		}
//DEL 	}
//DEL 
//DEL 	if(m_TerrainDlg.m_hWnd!=NULL)
//DEL 	{
//DEL 		m_TerrainDlg.ShowWindow(SW_SHOW);
//DEL 		m_TerrainDlg.ActivateFrame(SW_SHOW);
//DEL 		m_TerrainDlg.m_bar.Update();		
//DEL 	}
//DEL }

void CFinalSunDlg::OnTerrainLowertile() 
{
	if(Map->GetIsoSize()==0)
	{
		Sound(SOUND_NEGATIVE);
		return;
	}

	if(AD.mode!=ACTIONMODE_LOWERTILE)
	{
		m_settingsbar.m_BrushSize=0;
		m_settingsbar.UpdateData(FALSE);
		m_view.m_isoview->m_BrushSize_x=1;
		m_view.m_isoview->m_BrushSize_y=1;
	
	}
	Sound(SOUND_POSITIVE);

	AD.mode=ACTIONMODE_LOWERTILE;
	
}

void CFinalSunDlg::OnTerrainRaisetile() 
{
	if(Map->GetIsoSize()==0)
	{
		Sound(SOUND_NEGATIVE);
		return;
	}

	if(AD.mode!=ACTIONMODE_HEIGHTENTILE)
	{
		m_settingsbar.m_BrushSize=0;
		m_settingsbar.UpdateData(FALSE);
		m_view.m_isoview->m_BrushSize_x=1;
		m_view.m_isoview->m_BrushSize_y=1;
	}
	Sound(SOUND_POSITIVE);
	AD.mode=ACTIONMODE_HEIGHTENTILE;
	
}

void CFinalSunDlg::OnTerrainFlatten() 
{
	if(Map->GetIsoSize()==0)
	{
		Sound(SOUND_NEGATIVE);
		return;
	}

	if(AD.mode!=ACTIONMODE_FLATTENGROUND)
	{
		m_settingsbar.m_BrushSize=1;
		m_settingsbar.UpdateData(FALSE);
		m_view.m_isoview->m_BrushSize_x=2;
		m_view.m_isoview->m_BrushSize_y=2;
	}
	Sound(SOUND_POSITIVE);

	AD.mode=ACTIONMODE_FLATTENGROUND;	
}

void CFinalSunDlg::OnTerrainCloak() 
{
	if(Map->GetIsoSize()==0)
	{
		Sound(SOUND_NEGATIVE);
		return;
	}

	AD.mode=ACTIONMODE_HIDETILESET;	
	Sound(SOUND_POSITIVE);
}

void CFinalSunDlg::OnTerrainShoweverytile() 
{
	if(!Map->GetIsoSize()) {
		Sound(SOUND_NEGATIVE);
		return;
	}

	m_view.m_isoview->ShowAllTileSets();
	m_view.m_isoview->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	Sound(SOUND_POSITIVE);
}

void CFinalSunDlg::OnTerrainShowallfields() 
{
	if(!Map->GetIsoSize()) {
		Sound(SOUND_NEGATIVE);
		return;
	}

	int i;
	for(i=0;i<Map->GetIsoSize()*Map->GetIsoSize();i++)
	{
		Map->HideField(i, FALSE);
	}

	m_view.m_isoview->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	Sound(SOUND_POSITIVE);
}

void CFinalSunDlg::OnTerrainHidefield() 
{
	if(!Map->GetIsoSize()) {
		Sound(SOUND_NEGATIVE);
		return;
	}

	AD.mode=ACTIONMODE_HIDEFIELD;	
	Sound(SOUND_POSITIVE);
}

void CFinalSunDlg::OnEditTriggereditor() 
{


	if(m_triggereditor.m_hWnd==NULL)
	{		
		if(!m_triggereditor.Create(CTriggerEditorDlg::IDD, NULL))
		{
			MessageBox(GetLanguageStringACP("Err_CreateErr"), "Error");
		}
	}

	if(m_triggereditor.m_hWnd!=NULL)
	{
		//m_all.UpdateStrings();
		m_triggereditor.UpdateDialog();
		m_triggereditor.ShowWindow(SW_SHOW);
		Sound(SOUND_POSITIVE);
	}
}

void CFinalSunDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	
	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CFinalSunDlg::OnMaptoolsChangemapheight() 
{
	if(Map->GetIsoSize()==0)
	{
		Sound(SOUND_NEGATIVE);
		return;
	}

	Sound(SOUND_POSITIVE);

	int vmin=-MAXHEIGHT;
	int vmax=MAXHEIGHT;

	int isosize=Map->GetIsoSize();
	int mapsize=isosize*isosize;
	int mapwidth=Map->GetWidth();
	int mapheight=Map->GetHeight();
	int max=Map->GetIsoSize()*Map->GetIsoSize();
	int i;
	for(i=0;i<max;i++)
	{
		int dwX=i%isosize;
		int dwY=i/isosize;

		//if(dwX==0 || dwY==0 || dwX+dwY<mapwidth+1 || dwX+dwY+1>mapwidth+mapheight*2 || (dwY+1>mapwidth && dwX<dwY-mapwidth+1) || (dwX+1>mapwidth && dwY+mapwidth<dwX+1))
		//if( dwX<2|| dwY<2 || dwX+dwY<mapwidth+2 || dwX+dwY+2>mapwidth+mapheight*2 || (dwY+2>mapwidth && dwX-2<dwY-mapwidth) || (dwX+2>mapwidth && dwY+mapwidth-2<dwX)) 
		if( dwX<1|| dwY<1 || dwX+dwY<mapwidth+1 || dwX+dwY>mapwidth+mapheight*2 || (dwY+1>mapwidth && dwX-1<dwY-mapwidth) || (dwX+1>mapwidth && dwY+mapwidth-1<dwX))
		{

		}
		else
		{
			int v=Map->GetHeightAt(i);
			if(v+vmin <0)
			{
				vmin=-v;
			}
			if(v+vmax>MAXHEIGHT)
			{
				vmax=MAXHEIGHT-v;
			}
		}
	}

	CString p=GetLanguageStringACP("StrChangeHeight");
	char c[50];
	itoa(vmin, c, 10);
	p=TranslateStringVariables(1, p, c);
	itoa(vmax, c, 10);
	p=TranslateStringVariables(2, p, c);

	CString res=InputBox(p,
		GetLanguageStringACP("StrChangeHeightCap"));
	
	if(res.GetLength()<0) return;

	int a=atoi(res);
	if(a==0) return;
	

	
	

	for(i=0;i<max;i++)
	{
		int dwX=i%isosize;
		int dwY=i/isosize;

		//if(dwX==0 || dwY==0 || dwX+dwY<mapwidth+1 || dwX+dwY+1>mapwidth+mapheight*2 || (dwY+1>mapwidth && dwX<dwY-mapwidth+1) || (dwX+1>mapwidth && dwY+mapwidth<dwX+1))
		//if( dwX<2|| dwY<2 || dwX+dwY<mapwidth+2 || dwX+dwY+2>mapwidth+mapheight*2 || (dwY+2>mapwidth && dwX-2<dwY-mapwidth) || (dwX+2>mapwidth && dwY+mapwidth-2<dwX)) 
		if( dwX<1|| dwY<1 || dwX+dwY<mapwidth+1 || dwX+dwY>mapwidth+mapheight*2 || (dwY+1>mapwidth && dwX-1<dwY-mapwidth) || (dwX+1>mapwidth && dwY+mapwidth-1<dwX))
		{

		}
		else
		{
			int v=Map->GetHeightAt(i);
			if(v+a <0 || v+a>MAXHEIGHT)
			{
				MessageBox(GetLanguageStringACP("StrChangeHeightErr"), GetLanguageStringACP("StrChangeHeightErrCap"), MB_ICONSTOP);
				return;
			}
		}
	}

	for(i=0;i<max;i++)
	{
		int v=Map->GetHeightAt(i);
		Map->SetHeightAt(i, v+a);
	}

	this->m_view.m_isoview->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}

void CFinalSunDlg::OnEditGlobalvariables() 
{
	if(Map->GetIsoSize()==0)
	{
		Sound(SOUND_NEGATIVE);
		return;
	}

	Sound(SOUND_POSITIVE);
	CGlobalsDlg dlg;
	dlg.DoModal();
	UpdateDialogs(TRUE);
}

BOOL CFinalSunDlg::PreTranslateMessage(MSG* pMsg)
{
	// CG: Der folgende Block wurde von der Komponente "QuickInfo" hinzugefügt.
	{
		// Diese Nachricht von QuickInfo verarbeiten lassen.
		//m_tooltip.RelayEvent(pMsg);
	}
	return CDialog::PreTranslateMessage(pMsg);	// CG: Dies wurde von der Komponente "QuickInfo" hinzugefügt.
}

void CFinalSunDlg::OnEditUndo() 
{
	if(Map->GetIsoSize()==0)
	{
		Sound(SOUND_NEGATIVE);
		return;
	}

	//int a=0;
	//int b=2/a;
	//memcpy( (void*)0xddffeedd,&a, 1);
	Sound(SOUND_POSITIVE);
	Map->Undo();
	//UpdateDialogs(FALSE);
	m_view.m_isoview->SendMessage(WM_RBUTTONUP);
	m_view.m_isoview->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}

void CFinalSunDlg::OnHelpManual() 
{
	Sound(SOUND_POSITIVE);
	if(ShellExecuteW(0, NULL, (utf8ToUtf16(ExePath)+L"\\HelpManual.pdf").c_str(), NULL, NULL, SW_NORMAL) == 0)
	{
		MessageBox((CString)"Could not open HelpManual.pdf! Try opening "+(CString)ExePath+(CString)"\\HelpManual manually.");
	}
}


LONG __stdcall ExceptionHandler(
  _EXCEPTION_POINTERS *ExceptionInfo   // address of 
                                              // exception info
)
{
	CString s;
	CString s2;
	CString s_add;	
	char adress[50];
	char c[50];
	itoa((std::ptrdiff_t)ExceptionInfo->ExceptionRecord->ExceptionAddress, adress, 16);
	s="Unknown exception";
	switch(ExceptionInfo->ExceptionRecord->ExceptionCode)
	{
	case EXCEPTION_ACCESS_VIOLATION:
		s="EXCEPTION_ACCESS_VIOLATION";
		s2="Thread did not have read or write access to the virtual address.";
		if(ExceptionInfo->ExceptionRecord->ExceptionInformation[0])
		{
			s_add="\nAdditional information: Write access to 0x";
		}
		else
			s_add="\nAdditional information: Read access from 0x";
		
		itoa(ExceptionInfo->ExceptionRecord->ExceptionInformation[1], c, 16);

		s_add+=c;

		break;
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		s="EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
		s2="Thread tried to access an array out of bounds."; 
		break;
	case EXCEPTION_BREAKPOINT :
		s="EXCEPTION_BREAKPOINT";
		s2="Encountered breakpoint.";
		break;
	case EXCEPTION_DATATYPE_MISALIGNMENT:
		s="EXCEPTION_DATATYPE_MISALIGNMENT";
		s2="Access to memory was misaligned for the given datatype.";
		break;
	case EXCEPTION_FLT_DENORMAL_OPERAND:
		s="EXCEPTION_FLT_DENORMAL_OPERAND";
		s2="Denormal operand in floating point operation.";
		break;
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		s="EXCEPTION_FLT_DIVIDE_BY_ZERO";
		s2="Thread divided by zero in a floating point operation.";
		break;
	case EXCEPTION_FLT_INEXACT_RESULT :
		s="EXCEPTION_FLT_INEXACT_RESULT";
		s2="Floating point operation result not representable with exact decimal fraction.";
		break;
	case EXCEPTION_FLT_INVALID_OPERATION:
		s="EXCEPTION_FLT_INVALID_OPERATION";
		s2="Invalid floating point operation.";
		break;
	case EXCEPTION_FLT_OVERFLOW :
		s="EXCEPTION_FLT_OVERFLOW";
		s2="Floating point overflow error.";
		break;
	case EXCEPTION_FLT_STACK_CHECK :
		s="EXCEPTION_FLT_STACK_CHECK";
		s2="Floating point operation caused stack overflow or underflow.";
		break;
	case EXCEPTION_FLT_UNDERFLOW:
		s="EXCEPTION_FLT_UNDERFLOW";
		s2="Floating point underflow error.";
		break;
	case EXCEPTION_ILLEGAL_INSTRUCTION:
		s="EXCEPTION_ILLEGAL_INSTRUCTION";
		s2="Thread executed illegal instruction.";
		break;
	case EXCEPTION_IN_PAGE_ERROR:
		s="EXCEPTION_IN_PAGE_ERROR";
		s2="Thread tried to access a page that could not be retrieved by the system.";
		break;
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		s="EXCEPTION_INT_DIVIDE_BY_ZERO";
		s2="Thread divided by zero in an integer operation.";
		break;
	case EXCEPTION_INT_OVERFLOW:
		s="EXCEPTION_INT_OVERFLOW";
		s2="Integer operation caused overflow.";
		break;
	case EXCEPTION_INVALID_DISPOSITION:
		s="EXCEPTION_INVALID_DISPOSITION";
		s2="Exception handler returned invalid disposition.";
		break;
	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
		s="EXCEPTION_NONCONTINUABLE_EXCEPTION";
		s2="Cannot continue execution after a noncontinuable exception.";
		break;
	case EXCEPTION_PRIV_INSTRUCTION :
		s="EXCEPTION_PRIV_INSTRUCTION";
		s2="Instruction not valid in the current machine mode.";
		break;
	case EXCEPTION_SINGLE_STEP:
		s="EXCEPTION_SINGLE_STEP";
		s2="Instruction step has been executed.";
		break;
	case EXCEPTION_STACK_OVERFLOW:
		s="EXCEPTION_STACK_OVERFLOW";
		s2="Stack overflow.";

	}

	CString s3;
	s3="INTERNAL APPLICATION ERROR\n\nApplication will now try to free memory, save the current map as \"fcrash_backup.map\" in the FinalAlert 2 directory and quit.\n\n\n";

	s3+="Important: If this error has occured while loading graphics, it can very often be fixed by using another system color resolution (16, 24 or 32 bit).";

	s3+="\n\nThe following information is available, please note every line below:\n\n";
	s3+="Last succeeded operation: ";
	itoa(last_succeeded_operation,c,10);
	s3+=c;
	s3+="";
	s3+="\n";
	s3+="\nException data:\n";
	s3+=s;
	s3+="\n";
	s3+=s2;
	s3+="\nAt address: ";
	s3+=adress;
	s3+=s_add;


	errstream << "Exception occured. Current data:" << endl;
	errstream << "Last succeeded operation:" << last_succeeded_operation << endl;
	errstream << "Trying to save current map" << endl;
	errstream.flush();

	

	if(MessageBox(0,s3,"Fatal error",MB_OKCANCEL)==IDOK)
	{
		return EXCEPTION_CONTINUE_EXECUTION;
	}

	std::string file=u8ExePath;
	file+="\\fcrash_backup.map";
	((CFinalSunDlg*)theApp.m_pMainWnd)->SaveMap(file.c_str());
	
	theApp.m_loading->FreeAll();

	CIniFile::Rules.Clear();
	CIniFile::Ai.Clear();
	CIniFile::Art.Clear();
	CIniFile::Temperate.Clear();
	CIniFile::Snow.Clear();
	CIniFile::Urban.Clear();
	Map->UpdateAndGetIniFile().Clear();
	CIniFile::Sound.Clear();
	CIniFile::FAData.Clear();
	CIniFile::FALanguage.Clear();

#ifdef _DEBUG
	return EXCEPTION_CONTINUE_SEARCH;
#endif

	return EXCEPTION_EXECUTE_HANDLER;//EXCEPTION_CONTINUE_SEARCH;//EXCEPTION_EXECUTE_HANDLER;
}

INT_PTR CFinalSunDlg::DoModal()
{
	int res=0;
	SetUnhandledExceptionFilter(ExceptionHandler);

	res=CDialog::DoModal();

	return res;
}

void CFinalSunDlg::OnMaptoolsAutolevel() 
{
	if(Map->GetIsoSize()==0)
	{
		Sound(SOUND_NEGATIVE);
		return;
	}

	Sound(SOUND_POSITIVE);
	MessageBox("This tool will try to automatically raise the terrain using the cliffs.\nIt may take some seconds to execute, as there are masses of data to handle.\nAfter this, you should check your map if everything looks fine. If not, you should use the different height tools, especially flatten ground, to fix any errors. You can use Edit->Undo to undo anything that has been done by using this function.", "Auto Leveler");

	m_view.m_isoview->AutoLevel();	
}

void CFinalSunDlg::OnUpdateEditAitriggerenabling(CCmdUI* pCmdUI) 
{
	
	/*if(Map->GetIsoSize()==0) pCmdUI->Enable(FALSE); 
	else pCmdUI->Enable(TRUE);*/

}

void CFinalSunDlg::OnUpdateEditAitriggers(CCmdUI* pCmdUI) 
{
	/*if(Map->GetIsoSize()==0) pCmdUI->Enable(FALSE); 
	else pCmdUI->Enable(TRUE);*/
}

void CFinalSunDlg::OnUpdateEditBasicsettings(CCmdUI* pCmdUI) 
{
	/*if(Map->GetIsoSize()==0) pCmdUI->Enable(FALSE); 
	else pCmdUI->Enable(TRUE);*/
}

void CFinalSunDlg::OnUpdateEditGlobalvariables(CCmdUI* pCmdUI) 
{
	/*if(Map->GetIsoSize()==0) pCmdUI->Enable(FALSE); 
	else pCmdUI->Enable(TRUE);*/
}

void CFinalSunDlg::OnUpdateEditHouses(CCmdUI* pCmdUI) 
{
	CheckAvail(pCmdUI);
}

void CFinalSunDlg::OnUpdateEditIniediting(CCmdUI* pCmdUI) 
{
	
	CheckAvail(pCmdUI);
	
}

void CFinalSunDlg::OnUpdateEditLighting(CCmdUI* pCmdUI) 
{
	// TODO: Code für die Befehlsbehandlungsroutine zum Aktualisieren der Benutzeroberfläche hier einfügen
	
}

void CFinalSunDlg::OnUpdateEditMap(CCmdUI* pCmdUI) 
{
	// TODO: Code für die Befehlsbehandlungsroutine zum Aktualisieren der Benutzeroberfläche hier einfügen
	
}

void CFinalSunDlg::OnUpdateEditScripts(CCmdUI* pCmdUI) 
{
	// TODO: Code für die Befehlsbehandlungsroutine zum Aktualisieren der Benutzeroberfläche hier einfügen
	
}

void CFinalSunDlg::OnUpdateEditSingleplayersettings(CCmdUI* pCmdUI) 
{
	// TODO: Code für die Befehlsbehandlungsroutine zum Aktualisieren der Benutzeroberfläche hier einfügen
	
}

void CFinalSunDlg::OnUpdateEditSpecialflags(CCmdUI* pCmdUI) 
{
	// TODO: Code für die Befehlsbehandlungsroutine zum Aktualisieren der Benutzeroberfläche hier einfügen
	
}

void CFinalSunDlg::OnUpdateEditTags(CCmdUI* pCmdUI) 
{
	// TODO: Code für die Befehlsbehandlungsroutine zum Aktualisieren der Benutzeroberfläche hier einfügen
	
}

void CFinalSunDlg::OnUpdateEditTaskforces(CCmdUI* pCmdUI) 
{
	// TODO: Code für die Befehlsbehandlungsroutine zum Aktualisieren der Benutzeroberfläche hier einfügen
	
}

void CFinalSunDlg::OnUpdateEditTeams(CCmdUI* pCmdUI) 
{
	// TODO: Code für die Befehlsbehandlungsroutine zum Aktualisieren der Benutzeroberfläche hier einfügen
	
}

void CFinalSunDlg::OnUpdateEditTriggereditor(CCmdUI* pCmdUI) 
{
	// TODO: Code für die Befehlsbehandlungsroutine zum Aktualisieren der Benutzeroberfläche hier einfügen
	
}

void CFinalSunDlg::OnUpdateEditTriggers(CCmdUI* pCmdUI) 
{
	// TODO: Code für die Befehlsbehandlungsroutine zum Aktualisieren der Benutzeroberfläche hier einfügen
	
}

void CFinalSunDlg::OnUpdateEditUndo(CCmdUI* pCmdUI) 
{
	// TODO: Code für die Befehlsbehandlungsroutine zum Aktualisieren der Benutzeroberfläche hier einfügen
	
}

void CFinalSunDlg::OnUpdateFileSave(CCmdUI* pCmdUI) 
{
	// TODO: Code für die Befehlsbehandlungsroutine zum Aktualisieren der Benutzeroberfläche hier einfügen
	
}

void CFinalSunDlg::OnUpdateFileSaveas(CCmdUI* pCmdUI) 
{
	// TODO: Code für die Befehlsbehandlungsroutine zum Aktualisieren der Benutzeroberfläche hier einfügen
	
}

void CFinalSunDlg::OnUpdateFileValidatemap(CCmdUI* pCmdUI) 
{
	// TODO: Code für die Befehlsbehandlungsroutine zum Aktualisieren der Benutzeroberfläche hier einfügen
	
}

void CFinalSunDlg::OnUpdateMaptoolsAutolevel(CCmdUI* pCmdUI) 
{
	// TODO: Code für die Befehlsbehandlungsroutine zum Aktualisieren der Benutzeroberfläche hier einfügen
	
}

void CFinalSunDlg::OnUpdateMaptoolsChangemapheight(CCmdUI* pCmdUI) 
{
	// TODO: Code für die Befehlsbehandlungsroutine zum Aktualisieren der Benutzeroberfläche hier einfügen
	
}

void CFinalSunDlg::OnUpdateOptionsShowminimap(CCmdUI* pCmdUI) 
{
	// TODO: Code für die Befehlsbehandlungsroutine zum Aktualisieren der Benutzeroberfläche hier einfügen
	
}

void CFinalSunDlg::OnUpdateTerrainCloak(CCmdUI* pCmdUI) 
{
	// TODO: Code für die Befehlsbehandlungsroutine zum Aktualisieren der Benutzeroberfläche hier einfügen
	
}

void CFinalSunDlg::OnUpdateTerrainFlatten(CCmdUI* pCmdUI) 
{
	// TODO: Code für die Befehlsbehandlungsroutine zum Aktualisieren der Benutzeroberfläche hier einfügen
	
}

void CFinalSunDlg::OnUpdateTerrainHeightenground(CCmdUI* pCmdUI) 
{
	// TODO: Code für die Befehlsbehandlungsroutine zum Aktualisieren der Benutzeroberfläche hier einfügen
	
}

void CFinalSunDlg::OnUpdateTerrainHidefield(CCmdUI* pCmdUI) 
{
	// TODO: Code für die Befehlsbehandlungsroutine zum Aktualisieren der Benutzeroberfläche hier einfügen
	
}

void CFinalSunDlg::OnUpdateTerrainLowerground(CCmdUI* pCmdUI) 
{
	// TODO: Code für die Befehlsbehandlungsroutine zum Aktualisieren der Benutzeroberfläche hier einfügen
	
}

void CFinalSunDlg::OnUpdateTerrainLowertile(CCmdUI* pCmdUI) 
{
	// TODO: Code für die Befehlsbehandlungsroutine zum Aktualisieren der Benutzeroberfläche hier einfügen
	
}

void CFinalSunDlg::OnUpdateTerrainRaisetile(CCmdUI* pCmdUI) 
{
	// TODO: Code für die Befehlsbehandlungsroutine zum Aktualisieren der Benutzeroberfläche hier einfügen
	
}

void CFinalSunDlg::OnUpdateTerrainShowallfields(CCmdUI* pCmdUI) 
{
	// TODO: Code für die Befehlsbehandlungsroutine zum Aktualisieren der Benutzeroberfläche hier einfügen
	
}

void CFinalSunDlg::OnUpdateTerrainShoweverytile(CCmdUI* pCmdUI) 
{
	// TODO: Code für die Befehlsbehandlungsroutine zum Aktualisieren der Benutzeroberfläche hier einfügen
	
}

void CFinalSunDlg::OnMaptoolsFrontcliff() 
{
	if(Map->GetIsoSize()==0)
	{
		Sound(SOUND_NEGATIVE);
		return;
	}
	if(Map->GetTheater()==THEATER4)
	{
		MessageBox("There are no cliffs in the Lunar theater");
		return;
	}

	Sound(SOUND_POSITIVE);
	AD.mode=ACTIONMODE_CLIFFFRONT;	
}

void CFinalSunDlg::OnMaptoolsBackcliff() 
{
	if(Map->GetIsoSize()==0)
	{
		Sound(SOUND_NEGATIVE);
		return;
	}
	if(Map->GetTheater()==THEATER4)
	{
		MessageBox("There are no cliffs in the Lunar theater");
		return;
	}

	Sound(SOUND_POSITIVE);
	AD.mode=ACTIONMODE_CLIFFBACK;	
}

void CFinalSunDlg::OnEditRedo() 
{
	if(Map->GetIsoSize()==0)
	{
		Sound(SOUND_NEGATIVE);
		return;
	}

	Sound(SOUND_POSITIVE);
	Map->Redo();	
	
	m_view.m_isoview->SendMessage(WM_RBUTTONUP);
	m_view.m_isoview->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}

void CFinalSunDlg::OnMaptoolsAutocreateshores() 
{
	if(Map->GetIsoSize()==0)
	{
		Sound(SOUND_NEGATIVE);
		return;
	}

	Sound(SOUND_POSITIVE);

	Map->TakeSnapshot();

	Map->CreateShore(0,0,Map->GetIsoSize(), Map->GetIsoSize());
	m_view.m_isoview->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

	Map->TakeSnapshot();
	Map->Undo();
	
}

void CFinalSunDlg::OnOptionsDisableautoshore() 
{
	CIniFile Options;
	Options.LoadFile(u8ExePath+"\\FinalAlert.ini");

	if(GetMenu()->GetMenuState(ID_OPTIONS_DISABLEAUTOSHORE, MF_BYCOMMAND) & MF_CHECKED)
	{
		GetMenu()->CheckMenuItem(ID_OPTIONS_DISABLEAUTOSHORE, MF_BYCOMMAND | MF_UNCHECKED);
		theApp.m_Options.bDisableAutoShore=FALSE;		
		Options.sections["UserInterface"].values["DisableAutoShore"]="0";
	}
	else
	{
		GetMenu()->CheckMenuItem(ID_OPTIONS_DISABLEAUTOSHORE, MF_BYCOMMAND | MF_CHECKED);
		theApp.m_Options.bDisableAutoShore=TRUE;
		Options.sections["UserInterface"].values["DisableAutoShore"]="1";
	}

	Options.SaveFile(u8ExePath+"\\FinalAlert.ini");
}





//DEL void CFinalSunDlg::OnNcPaint() 
//DEL {
//DEL 	CBitmap b;
//DEL 	b.LoadBitmap(IDB_TEXTURE1);
//DEL 	CDC dc;
//DEL 	
//DEL 
//DEL 	CDC* target=GetWindowDC();
//DEL 	dc.CreateCompatibleDC(target);
//DEL 	dc.SelectObject(b);
//DEL 
//DEL 	BITMAP bd;
//DEL 	b.GetBitmap(&bd);
//DEL 
//DEL 	RECT r;
//DEL 	GetWindowRect(&r);
//DEL 
//DEL 	int count=(r.right-r.left)/bd.bmWidth+1;
//DEL 	int i;
//DEL 	for(i=0;i<count;i++)
//DEL 	{
//DEL 		target->BitBlt(i*bd.bmWidth,0,bd.bmWidth, bd.bmHeight-1, &dc, 0, 0, SRCCOPY);
//DEL 	}
//DEL 
//DEL 	ReleaseDC(target);
//DEL 	
//DEL 	CMenu* m=GetMenu();
//DEL 	if(m)
//DEL 	{
//DEL 		count=m->GetMenuItemCount();
//DEL 		for(i=0;i<count;i++)
//DEL 		{
//DEL 			DRAWITEMSTRUCT t;
//DEL 			t.CtlType=ODT_MENU;
//DEL 			t.itemID=m->GetMenuItemID(i);
//DEL 			t.itemAction=ODA_DRAWENTIRE;
//DEL 			t.itemState=ODS_DEFAULT;
//DEL 			t.hwndItem=(HWND)m->m_hMenu;
//DEL 			t.rcItem=r;
//DEL 			CString text;
//DEL 			m->GetMenuString(t.itemID, text, MF_BYCOMMAND);
//DEL 			t.itemData=(int)(LPCSTR)text;
//DEL 
//DEL 			m->DrawItem(&t);			
//DEL 		}
//DEL 	}
//DEL 	
//DEL 
//DEL 	
//DEL 	dc.DeleteDC();
//DEL 	b.DeleteObject();
//DEL 
//DEL 	// Kein Aufruf von CDialog::OnNcPaint() für Zeichnungsnachrichten
//DEL }

void CFinalSunDlg::OnOptionsDisableautolat() 
{
	CIniFile Options;
	Options.LoadFile(u8ExePath+"\\FinalAlert.ini");

	if(GetMenu()->GetMenuState(ID_OPTIONS_DISABLEAUTOLAT, MF_BYCOMMAND) & MF_CHECKED)
	{
		GetMenu()->CheckMenuItem(ID_OPTIONS_DISABLEAUTOLAT, MF_BYCOMMAND | MF_UNCHECKED);
		theApp.m_Options.bDisableAutoLat=FALSE;		
		Options.sections["UserInterface"].values["DisableAutoLat"]="0";
	}
	else
	{
		GetMenu()->CheckMenuItem(ID_OPTIONS_DISABLEAUTOLAT, MF_BYCOMMAND | MF_CHECKED);
		theApp.m_Options.bDisableAutoLat=TRUE;
		Options.sections["UserInterface"].values["DisableAutoLat"]="1";
	}

	Options.SaveFile(u8ExePath+"\\FinalAlert.ini");
}

void CFinalSunDlg::OnEditPaste() 
{
	Sound(SOUND_POSITIVE);
	AD.mode=ACTIONMODE_PASTE;
	AD.z_data=0;
}

void CFinalSunDlg::OnEditCopy() 
{
	Sound(SOUND_POSITIVE);
	AD.mode=ACTIONMODE_COPY;	
}

void CFinalSunDlg::OnEditCopywholemap() 
{
	Sound(SOUND_POSITIVE);
	Map->Copy();	
}

void CFinalSunDlg::OnEditPastewholemap() 
{
	Sound(SOUND_POSITIVE);
	Map->Paste(Map->GetIsoSize()/2,Map->GetIsoSize()/2,0);
	m_view.m_isoview->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}

void CFinalSunDlg::OnMarblemadness() 
{
	if(Map->GetIsoSize()==0)
	{
		Sound(SOUND_NEGATIVE);
		return;
	}

	Sound(SOUND_POSITIVE);

	theApp.m_Options.bMarbleMadness=!	theApp.m_Options.bMarbleMadness;

	m_view.m_isoview->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	m_view.m_browser->m_view.SetTileSet(m_view.m_browser->m_view.m_currentTileSet, TRUE);
}

void CFinalSunDlg::CheckAvail(CCmdUI *pCmdUI)
{
	/*if(Map->GetIsoSize()==0) 
	{
		pCmdUI->Enable(FALSE); 

		if(pCmdUI->m_pMenu)
		{
			pCmdUI->m_pMenu->DeleteMenu(pCmdUI->m_nIndex, MF_BYPOSITION);
		}
	}
	else 
	{
		pCmdUI->Enable(TRUE);

		
		if(pCmdUI->m_pMenu)
		{
			pCmdUI->m_pMenu->ModifyMenu(pCmdUI->m_nID, MF_BYCOMMAND | MF_ENABLED);
		}
	}*/
}

void CFinalSunDlg::OnOptionsSounds() 
{
	CIniFile Options;
	Options.LoadFile(u8ExePath+"\\FinalAlert.ini");

	if(GetMenu()->GetMenuState(ID_OPTIONS_SOUNDS, MF_BYCOMMAND) & MF_CHECKED)
	{
		GetMenu()->CheckMenuItem(ID_OPTIONS_SOUNDS, MF_BYCOMMAND | MF_UNCHECKED);
		theApp.m_Options.bNoSounds=TRUE;		
		Options.sections["UserInterface"].values["Sounds"]="0";
	}
	else
	{
		GetMenu()->CheckMenuItem(ID_OPTIONS_SOUNDS, MF_BYCOMMAND | MF_CHECKED);
		theApp.m_Options.bNoSounds=FALSE;
		Options.sections["UserInterface"].values["Sounds"]="1";
	}

	Options.SaveFile(u8ExePath+"\\FinalAlert.ini");
}

void CFinalSunDlg::OnUpdateOptionsSounds(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(theApp.m_Options.bNoSounds ? 0 : 1);	
}

void CFinalSunDlg::OnOptionsDisableslopecorrection() 
{
	CIniFile Options;
	Options.LoadFile(u8ExePath +"\\FinalAlert.ini");

	if(GetMenu()->GetMenuState(ID_OPTIONS_DISABLESLOPECORRECTION, MF_BYCOMMAND) & MF_CHECKED)
	{
		GetMenu()->CheckMenuItem(ID_OPTIONS_DISABLESLOPECORRECTION, MF_BYCOMMAND | MF_UNCHECKED);
		theApp.m_Options.bDisableSlopeCorrection=FALSE;		
		Options.sections["UserInterface"].values["DisableSlopeCorrection"]="0";
	}
	else
	{
		GetMenu()->CheckMenuItem(ID_OPTIONS_DISABLESLOPECORRECTION, MF_BYCOMMAND | MF_CHECKED);
		theApp.m_Options.bDisableSlopeCorrection=TRUE;
		Options.sections["UserInterface"].values["DisableSlopeCorrection"]="1";
	}

	Options.SaveFile(u8ExePath+"\\FinalAlert.ini");
}

void CFinalSunDlg::OnOptionsShowbuildingoutline() 
{
	CIniFile Options;
	Options.LoadFile(u8ExePath+"\\FinalAlert.ini");

	if(GetMenu()->GetMenuState(ID_OPTIONS_SHOWBUILDINGOUTLINE, MF_BYCOMMAND) & MF_CHECKED)
	{
		GetMenu()->CheckMenuItem(ID_OPTIONS_SHOWBUILDINGOUTLINE, MF_BYCOMMAND | MF_UNCHECKED);
		theApp.m_Options.bShowCells=FALSE;		
		Options.sections["UserInterface"].values["ShowBuildingCells"]="0";
	}
	else
	{
		GetMenu()->CheckMenuItem(ID_OPTIONS_SHOWBUILDINGOUTLINE, MF_BYCOMMAND | MF_CHECKED);
		theApp.m_Options.bShowCells=TRUE;
		Options.sections["UserInterface"].values["ShowBuildingCells"]="1";
	}

	m_view.m_isoview->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

	Options.SaveFile(u8ExePath +"\\FinalAlert.ini");
}


void CFinalSunDlg::OnFileFile1() 
{
	if(DoesFileExist(theApp.m_Options.prev_maps[0])) OpenMap(theApp.m_Options.prev_maps[0]);
	
}

void CFinalSunDlg::OnFileFile2() 
{
	if(DoesFileExist(theApp.m_Options.prev_maps[1])) OpenMap(theApp.m_Options.prev_maps[1]);
	
}

void CFinalSunDlg::OnFileFile3() 
{
	if(DoesFileExist(theApp.m_Options.prev_maps[2])) OpenMap(theApp.m_Options.prev_maps[2]);
	
}

void CFinalSunDlg::OnFileFile4() 
{
	if(DoesFileExist(theApp.m_Options.prev_maps[3])) OpenMap(theApp.m_Options.prev_maps[3]);
	
}

// MW 07/20/01: Checks if file already exists in prev. files list. If not, adds it (may delete old ones)
void CFinalSunDlg::InsertPrevFile(CString lpFilename)
{
	int i;

	//int free_at=-1;
	for (i = 0; i < 9; i++)
	{
		CString f = theApp.m_Options.prev_maps[i];
		CString f2 = lpFilename;
		f2.MakeLower();
		f.MakeLower();

		if (f == f2)
		{
			return;
		}
	}

	CIniFile Options;
	Options.LoadFile(u8ExePath + "\\FinalAlert.ini");

	for (i = 8; i > 0; i--)
	{
		theApp.m_Options.prev_maps[i] = theApp.m_Options.prev_maps[i - 1];
		char e[10];
		itoa(i, e, 10);

		Options.sections["Files"].values[e] = theApp.m_Options.prev_maps[i];
	}

	theApp.m_Options.prev_maps[0] = lpFilename;
	Options.sections["Files"].values["0"] = theApp.m_Options.prev_maps[0];

	Options.SaveFile(u8ExePath + "\\FinalAlert.ini");

	UpdateStrings();
}

// MW 07/20/01: New: for files clicked in the file list... copied from OnFileOpenmap();
void CFinalSunDlg::OpenMap(LPCSTR lpFilename)
{
	CString r=GetLanguageStringACP("SAVEDLG_FILETYPES");
    r=TranslateStringVariables(8, r, ";");

	char cuPath[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, cuPath);

	m_PKTHeader.Clear();

	CString fileToOpen=lpFilename;//dlg.GetPathName();
	fileToOpen.MakeLower();	
	CString ext=fileToOpen.Right(fileToOpen.GetLength()-fileToOpen.ReverseFind('.')-1); //dlg.GetFileExt();
	
	ext.MakeLower();
	CIniFile f;
	f.InsertFile(fileToOpen, "Map");
	if((f.sections["Map"].values["Theater"]==THEATER0 && theApp.m_Options.bDoNotLoadTemperateGraphics) || (f.sections["Map"].values["Theater"]==THEATER1 && theApp.m_Options.bDoNotLoadSnowGraphics))
	{
		MessageBox("You have selected to don´t show temperate or snow theater, but this map uses this theater. You cannot load it without restarting FinalSun/FinalAlert 2 with this theater enabled.", "Error");
		return;
	}

	bNoDraw=TRUE;

	CString str = GetLanguageStringACP("MainDialogCaption");
	str+=" (";
	str+=(char*)(LPCTSTR)lpFilename;
	str+=")";

	// MW 07/20/01: Update prev. files
	InsertPrevFile(lpFilename);

	this->SetWindowText(str);

	SetCursor(LoadCursor(NULL, IDC_WAIT));

	errstream << "Map->LoadMap() will be called" << endl;
	errstream.flush();

	Map->LoadMap((char*)(LPCTSTR)fileToOpen);

	BOOL bNoMapFile=FALSE;
	if(!Map->CheckMapPackData())
	{
		int res=MessageBox("This map seems to be corrupt. Do you want to try repairing it? If you click cancel, a empty map will be created, if you click no, it will load the map as it is","Corrupt", MB_YESNOCANCEL);
		if(res==IDCANCEL)
		{
			Map->CreateMap(32,32,THEATER0,0);
			bNoMapFile=TRUE;
		}
		else
		{
			if(res==IDYES) // try repair
			{
				int fielddata_size=Map->GetIsoSize()*Map->GetIsoSize();

				int i;
				for(i=0;i<fielddata_size;i++)
				{
					int gr=Map->GetFielddataAt(i)->wGround;
					if(gr==0xFFFF) gr=0;
					
					if(gr>=(*tiledata_count))
					{
						Map->SetTileAt(i, 0, 0);					
					}
					else
					{				
						if((*tiledata)[gr].wTileCount<=Map->GetFielddataAt(i)->bSubTile)
						{
							Map->SetTileAt(i, 0, 0);
						}
					}
				}

			}
		}
	}
	
	if(!bNoMapFile)
		strcpy(MapPath, fileToOpen);

	Sleep(200);

	SetCursor(m_hArrowCursor);


	bNoDraw=FALSE;

	m_view.m_isoview->UpdateDialog(TRUE);
	UpdateDialogs();

}

void CFinalSunDlg::OnMaptoolsSearchwaypoint() 
{
	if(Map->GetIsoSize()==0) return;

	CSearchWaypointDlg dlg;
	if(dlg.DoModal()==IDCANCEL) return;
	if(dlg.m_WaypointIndex<0) return;

	m_view.m_isoview->FocusWaypoint(dlg.m_WaypointIndex);
}

void CFinalSunDlg::OnMaptoolsToolscripts() 
{
	if(Map->GetIsoSize()==0) return;

	CUserScriptsDlg dlg;
	dlg.DoModal();
	
}


void CFinalSunDlg::OnHelpShowlogs()
{
	// TODO: Fügen Sie hier Ihren Befehlshandlercode ein.
	
	Sound(SOUND_POSITIVE);
	errstream.flush();
	auto f = utf8ToUtf16(theApp.getLogFileName());
	if (ShellExecuteW(0, NULL, f.c_str(), NULL, NULL, SW_NORMAL) == 0)
	{
		MessageBoxW(nullptr, (std::wstring(L"Could not open log! Try opening ") + f + L" manually").c_str(), L"Cannot open log", MB_ICONEXCLAMATION);
	}
}


void CFinalSunDlg::OnOptionsSmoothzoom()
{
	CIniFile Options;
	Options.LoadFile(u8ExePath + "\\FinalAlert.ini");

	if (GetMenu()->GetMenuState(ID_OPTIONS_SMOOTHZOOM, MF_BYCOMMAND) & MF_CHECKED)
	{
		GetMenu()->CheckMenuItem(ID_OPTIONS_SMOOTHZOOM, MF_BYCOMMAND | MF_UNCHECKED);
		theApp.m_Options.viewScaleUseSteps = true;		
	}
	else
	{
		GetMenu()->CheckMenuItem(ID_OPTIONS_SMOOTHZOOM, MF_BYCOMMAND | MF_CHECKED);
		theApp.m_Options.viewScaleUseSteps = false;		
	} 

	Options.sections["UserInterface"].values["ViewScaleUseSteps"] = theApp.m_Options.viewScaleUseSteps ? "1" : "0";
	Options.SaveFile(u8ExePath + "\\FinalAlert.ini");
}


BOOL CFinalSunDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (!CDialog::OnSetCursor(pWnd, nHitTest, message))
		SetCursor(m_hArrowCursor);
	return TRUE;
}



void CFinalSunDlg::OnOptionsUsedefaultmousecursor()
{
	CIniFile Options;
	Options.LoadFile(u8ExePath + "\\FinalAlert.ini");

	if (GetMenu()->GetMenuState(ID_OPTIONS_USEDEFAULTMOUSECURSOR, MF_BYCOMMAND) & MF_CHECKED)
	{
		GetMenu()->CheckMenuItem(ID_OPTIONS_USEDEFAULTMOUSECURSOR, MF_BYCOMMAND | MF_UNCHECKED);
		theApp.m_Options.useDefaultMouseCursor = false;
		m_hArrowCursor = m_hGameCursor;
	}
	else
	{
		GetMenu()->CheckMenuItem(ID_OPTIONS_USEDEFAULTMOUSECURSOR, MF_BYCOMMAND | MF_CHECKED);
		theApp.m_Options.useDefaultMouseCursor = true;
		m_hArrowCursor = LoadCursor(NULL, IDC_ARROW);
	}

	Options.sections["UserInterface"].values["UseDefaultMouseCursor"] = theApp.m_Options.useDefaultMouseCursor ? "1" : "0";
	Options.SaveFile(u8ExePath + "\\FinalAlert.ini");
}
