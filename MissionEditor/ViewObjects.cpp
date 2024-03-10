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

// ViewObjects.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FinalSun.h"
#include "ViewObjects.h"
#include "FinalSunDlg.h"
#include "structs.h"
#include "mapdata.h"
#include "variables.h"
#include "functions.h"
#include "inlines.h"
#include "TubeTool.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewObjects

const int valadded=10000;

IMPLEMENT_DYNCREATE(CViewObjects, CTreeView)

CViewObjects::CViewObjects()
{
}

CViewObjects::~CViewObjects()
{
}


BEGIN_MESSAGE_MAP(CViewObjects, CTreeView)
	//{{AFX_MSG_MAP(CViewObjects)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(TVN_KEYDOWN, OnKeydown)
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


extern int overlay_number[];
extern CString overlay_name[];
extern BOOL overlay_visible[];
extern BOOL overlay_trail[];


extern int overlay_count;


extern ACTIONDATA AD;

/////////////////////////////////////////////////////////////////////////////
// Zeichnung CViewObjects 

void CViewObjects::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// ZU ERLEDIGEN: Code zum Zeichnen hier einfügen
}

/////////////////////////////////////////////////////////////////////////////
// Diagnose CViewObjects

#ifdef _DEBUG
void CViewObjects::AssertValid() const
{
	CTreeView::AssertValid();
}

void CViewObjects::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG

CString GetTheaterLanguageString(LPCSTR lpString)
{
	CString s=lpString;
	CString t=lpString;

	if((tiledata)==&t_tiledata) t+="TEM";
	if((tiledata)==&s_tiledata) t+="SNO";
	if((tiledata)==&u_tiledata) t+="URB";
	if((tiledata)==&un_tiledata) t+="UBN";
	if((tiledata)==&l_tiledata) t+="LUN";
	if((tiledata)==&d_tiledata) t+="DES";

	CString res=GetLanguageStringACP(t);
	if(res.GetLength()==0) res=GetLanguageStringACP(s);

	return res;
}

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CViewObjects 


void CViewObjects::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CIniFile& ini=Map->UpdateAndGetIniFile();
	
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	int val=pNMTreeView->itemNew.lParam;
	if(val<0){ // return;
		if(val==-2) { 
			AD.reset();
			((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); 
		}
		return;
	}

	if(val<valadded)
	{
		// standard selection (maybe erasing etc)
		switch(val)
		{
			case 10: // erase field
			{
				AD.mode=ACTIONMODE_ERASEFIELD;
				break;
			}
			case 20: // waypoint stuff now
			{
				AD.mode=ACTIONMODE_WAYPOINT;
				AD.type=0;
				break;
			}
			case 21:
			{
				AD.mode=3;
				AD.type=1;
				break;
			}
			case 22:
			{
				AD.mode=3;
				AD.type=2;
				break;
			}
			case 23:
			case 24:
			case 25:
			case 26:
			case 27:
			case 28:
			case 29:
			case 30:
			{
				AD.mode=3;
				AD.type=3+val-23;
				break;
			}
			case 36: // celltag stuff
			{
				AD.mode=4;
				AD.type=0;
				break;
			}
			case 37: 
			{
				AD.mode=4;
				AD.type=1;
				break;
			}
			case 38: 
			{
				AD.mode=4;
				AD.type=2;
				break;
			}
			case 40: // node stuff
			{
				AD.mode=5;
				AD.type=0;
				break;
			}
			case 41: 
			{
				AD.mode=5;
				AD.type=1;
				break;
			}
			case 42:
			{
				AD.mode=5;
				AD.type=2;
				break;
			}
			case 50:
			{
				AD.mode=ACTIONMODE_MAPTOOL;
				AD.tool.reset(new AddTubeTool(*Map, *((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview, true));
				break;
			}
			case 51:
			{
				AD.mode = ACTIONMODE_MAPTOOL;
				AD.tool.reset(new ModifyTubeTool(*Map, *((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview, true));
				break;
			}
			case 52:
			{
				AD.mode = ACTIONMODE_MAPTOOL;
				AD.tool.reset(new AddTubeTool(*Map, *((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview, false));
				break;
			}
			case 53:
			{
				AD.mode = ACTIONMODE_MAPTOOL;
				AD.tool.reset(new ModifyTubeTool(*Map, *((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview, false));
				break;
			}
			case 54:
			{
				AD.mode=ACTIONMODE_MAPTOOL;
				AD.tool.reset(new RemoveTubeTool(*Map, *((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview));				
				break;
			}

			case 61:
				if(!tiledata_count) break;
				AD.type=0;
				AD.mode=ACTIONMODE_SETTILE;
				AD.data=0;
				AD.z_data=0;
				HandleBrushSize(0);
				break;
			
			case 62:
				int i;
				if(!tiledata_count) break;
				for(i=0;i<(*tiledata_count);i++)
					if((*tiledata)[i].wTileSet==atoi((*CIniFile::CurrentTheater).sections["General"].values["SandTile"])) break;
				AD.type=i;
				AD.mode=ACTIONMODE_SETTILE;
				AD.data=0;
				AD.z_data=0;
				HandleBrushSize(i);
				break;
			case 63:
				if(!tiledata_count) break;
				for(i=0;i<(*tiledata_count);i++)
					if((*tiledata)[i].wTileSet==atoi((*CIniFile::CurrentTheater).sections["General"].values["RoughTile"])) break;
				AD.type=i;
				AD.mode=ACTIONMODE_SETTILE;
				AD.data=0;
				AD.z_data=0;
				HandleBrushSize(i);
				break;
			case 64:
				if(!tiledata_count) break;
				for(i=0;i<(*tiledata_count);i++)
					if((*tiledata)[i].wTileSet==waterset) break;

				if(((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->m_BrushSize_x<2 ||
				((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->m_BrushSize_y<2)
				{

					((CFinalSunDlg*)theApp.m_pMainWnd)->m_settingsbar.m_BrushSize=1;
					((CFinalSunDlg*)theApp.m_pMainWnd)->m_settingsbar.UpdateData(FALSE);
					((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->m_BrushSize_x=2;
					((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->m_BrushSize_y=2;
				}

				AD.type=i;			
				AD.mode=ACTIONMODE_SETTILE;
				AD.data=1;	 // use water placement logic
				AD.z_data=0;
				break;
			case 65:
				if(!tiledata_count) break;
				for(i=0;i<(*tiledata_count);i++)
					if((*tiledata)[i].wTileSet==atoi((*CIniFile::CurrentTheater).sections["General"].values["GreenTile"])) break;
				AD.type=i;
				AD.mode=ACTIONMODE_SETTILE;
				AD.data=0;
				AD.z_data=0;
				HandleBrushSize(i);
				break;
			case 66:
				if(!tiledata_count) break;
				for(i=0;i<(*tiledata_count);i++)
					if((*tiledata)[i].wTileSet==atoi((*CIniFile::CurrentTheater).sections["General"].values["PaveTile"])) break;
				AD.type=i;
				AD.mode=ACTIONMODE_SETTILE;
				AD.data=0;
				AD.z_data=0;
				HandleBrushSize(i);
				break;
			case 67:
				if(!tiledata_count) break;
				for(i=0;i<(*tiledata_count);i++)
					if((*tiledata)[i].wTileSet==atoi(CIniFile::FAData.sections["NewUrbanInfo"].values["Morphable2"])) break;
				AD.type=i;
				AD.mode=ACTIONMODE_SETTILE;
				AD.data=0;
				AD.z_data=0;
				HandleBrushSize(i);
				break;
				

		}
	}
	else
	{
		int subpos=val%valadded;
		int pos=val/valadded;

		AD.mode=1;
		AD.type=pos;
		AD.data=subpos;

		if(pos==1)
		{
			CString sec="InfantryTypes";
			
			if(subpos<CIniFile::Rules.sections[sec].values.size())
			{
				// standard unit!

				AD.data_s=*CIniFile::Rules.sections[sec].GetValue(subpos);
			}
			else{

				AD.data_s=*ini.sections[sec].GetValue(subpos-CIniFile::Rules.sections[sec].values.size());
			}
		}
		else if(pos==2)
		{
			CString sec="BuildingTypes";
			
			if(subpos<CIniFile::Rules.sections[sec].values.size())
			{
				// standard unit!

				AD.data_s=*CIniFile::Rules.sections[sec].GetValue(subpos);
			}
			else{

				AD.data_s=*ini.sections[sec].GetValue(subpos-CIniFile::Rules.sections[sec].values.size());
			}
		}
		else if(pos==3)
		{
			CString sec="AircraftTypes";
			
			if(subpos<CIniFile::Rules.sections[sec].values.size())
			{
				// standard unit!

				AD.data_s=*CIniFile::Rules.sections[sec].GetValue(subpos);
			}
			else{

				AD.data_s=*ini.sections[sec].GetValue(subpos-CIniFile::Rules.sections[sec].values.size());
			}
		}
		else if(pos==4)
		{
			CString sec="VehicleTypes";
			
			if(subpos<CIniFile::Rules.sections[sec].values.size())
			{
				// standard unit!

				AD.data_s=*CIniFile::Rules.sections[sec].GetValue(subpos);
			}
			else{

				AD.data_s=*ini.sections[sec].GetValue(subpos-CIniFile::Rules.sections[sec].values.size());
			}
		}
		else if(pos==5)
		{
			
			CString sec="TerrainTypes";

			if(subpos==999)
			{
				// TODO: Random Terrain Place
			}
			else
			{
				if(subpos<CIniFile::Rules.sections[sec].values.size())
				{
					// standard unit!

					AD.data_s=*CIniFile::Rules.sections[sec].GetValue(subpos);
				}
				else{

					AD.data_s=*ini.sections[sec].GetValue(subpos-CIniFile::Rules.sections[sec].values.size());
				}
			}
		}
		else if(pos==6)
		{
			if(subpos<100)
			{
				// general overlay functions!
				if(subpos==1)
				{
					AD.data=31;
					AD.data2=atoi(InputBox("Please enter the value (0-255) of the overlay. Don´t exceed this range.","Set overlay manually"));

				}
				else if(subpos==2)
				{
					AD.data=32;
					AD.data2=atoi(InputBox("Please enter the value (0-255) of the overlay-data. Don´t exceed this range.","Set overlay manually"));

				}
				
			}
			else
			{
				AD.data2=subpos%100;
				AD.data=subpos/100;
				if(AD.data>=30) {AD.data=30;AD.data2=subpos%1000;}
			}
		}
		else if(pos==7)
		{
			// set owner
			//if(ini.sections.find(MAPHOUSES)!=ini.sections.end() && ini.sections[MAPHOUSES].values.size()>0)
			if(ini.sections.find(MAPHOUSES)!=ini.sections.end() && ini.sections[MAPHOUSES].values.size()>0)
			{
				AD.data_s=*ini.sections[MAPHOUSES].GetValue(subpos);
			}
			else
			{
				AD.data_s=*CIniFile::Rules.sections[HOUSES].GetValue(subpos);
			}

			currentOwner=AD.data_s;
		}
#ifdef SMUDGE_SUPP
		else if(pos==8)
		{
			
			
			CString sec="SmudgeTypes";

			if(subpos<CIniFile::Rules.sections[sec].values.size())
			{
				// standard unit!

				AD.data_s=*CIniFile::Rules.sections[sec].GetValue(subpos);
			}
			else
			{
				AD.data_s=*ini.sections[sec].GetValue(subpos-CIniFile::Rules.sections[sec].values.size());
			}

		}
#endif


	}

	
	
	*pResult = 0;
}

__inline HTREEITEM TV_InsertItemW(HWND hWnd, WCHAR* lpString, int len, HTREEITEM hInsertAfter, HTREEITEM hParent, int param)
{
	if(!lpString) return NULL;

	TVINSERTSTRUCTW tvis;
	tvis.hInsertAfter=hInsertAfter;
	tvis.hParent=hParent;
	tvis.itemex.mask=TVIF_PARAM | TVIF_TEXT;
	tvis.itemex.cchTextMax=len;
	tvis.itemex.pszText=lpString;
	tvis.itemex.lParam=param;

	// MW 07/17/2001: Updated to use Ascii if Unicode fails:
	HTREEITEM res=(HTREEITEM)::SendMessage(hWnd, TVM_INSERTITEMW, 0,((LPARAM)(&tvis)));	

	if(!res)
	{
		// failed... Probably because of missing Unicode support

		// convert text to ascii, then add it
		BYTE* lpAscii=new(BYTE[len+1]);
		BOOL bUsedDefault;
		memset(lpAscii, 0, len+1);
		WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK,
			lpString, len+1, (LPSTR)lpAscii, len+1,NULL,&bUsedDefault);

		TVINSERTSTRUCT tvis;
		tvis.hInsertAfter=hInsertAfter;
		tvis.hParent=hParent;
		tvis.itemex.mask=TVIF_PARAM | TVIF_TEXT;
		tvis.itemex.cchTextMax=len;
		tvis.itemex.lParam=param;
		tvis.itemex.pszText=(char*)lpAscii;

		res=TreeView_InsertItem(hWnd, &tvis);

		delete[] lpAscii;
	}

	return res;
}

void CViewObjects::UpdateDialog()
{
	Redraw_Initialize();
	Redraw_MainList();
	Redraw_Ground();
	Redraw_Owner();
	Redraw_Infantry();
	Redraw_Vehicle();
	Redraw_Aircraft();
	Redraw_Building();
	Redraw_Terrain();
	Redraw_Smudge();
	Redraw_Overlay();
	Redraw_Waypoint();
	Redraw_Celltag();
	Redraw_Basenode();
	Redraw_Tunnel();
	Redraw_PlayerLocation(); // player location is just waypoints!
	// Redraw_PropertyBrush();
}

int CViewObjects::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	lpCreateStruct->style|=TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_SHOWSELALWAYS;
	if (CTreeView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	
	return 0;
}

void CViewObjects::OnInitialUpdate() 
{
	CTreeView::OnInitialUpdate();
}

void CViewObjects::OnKeydown(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_KEYDOWN* pTVKeyDown = (TV_KEYDOWN*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	
	*pResult = 0;
}

void CViewObjects::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen
	
	// CTreeView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CViewObjects::HandleBrushSize(int iTile)
{
	if(iTile>=*tiledata_count) return;

	int i;
	for(i=0;i<CIniFile::FAData.sections["StdBrushSize"].values.size();i++)
	{
		CString n=*CIniFile::FAData.sections["StdBrushSize"].GetValueName(i);
		if((*CIniFile::CurrentTheater).sections["General"].FindName(n)>=0)
		{
			int tset=atoi((*CIniFile::CurrentTheater).sections["General"].values[n]);
			if(tset==(*tiledata)[iTile].wTileSet)
			{
				int bs=atoi(*CIniFile::FAData.sections["StdBrushSize"].GetValue(i));
				((CFinalSunDlg*)theApp.m_pMainWnd)->m_settingsbar.m_BrushSize=bs-1;
				((CFinalSunDlg*)theApp.m_pMainWnd)->m_settingsbar.UpdateData(FALSE);
				((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->m_BrushSize_x=bs;
				((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->m_BrushSize_y=bs;
			}
		}
	}

}

HTREEITEM CViewObjects::InsertString(const char* pString, DWORD dwItemData, HTREEITEM hParent, HTREEITEM hInsertAfter)
{
	return GetTreeCtrl().InsertItem(TVIF_PARAM | TVIF_TEXT, pString, 0,0,0,0, dwItemData, hParent, hInsertAfter);
}

HTREEITEM CViewObjects::InsertTranslatedString(const char* pOriginString, DWORD dwItemData, HTREEITEM hParent, HTREEITEM hInsertAfter)
{
	return InsertString(GetLanguageStringACP(pOriginString), dwItemData, hParent, hInsertAfter);
}

static int ParseToInt(const char* str, int defaultValue)
{
	int ret;
	if (std::sscanf(str, "%d", &ret) == 1) [[likely]]
        return ret;
	return defaultValue;
}

void CViewObjects::Redraw_Initialize()
{
	for (auto root : ExtNodes)
		root = NULL;
	KnownItem.clear();
	IgnoreSet.clear();
	ForceName.clear();
	Owners.clear();
	this->GetTreeCtrl().DeleteAllItems();

	auto& rules = CIniFile::Rules;
	auto& fadata = CIniFile::FAData;
	auto& doc = Map->GetIniFile();

	auto loadSet = [this](const char* pTypeName, int nType)
	{
		ExtSets[nType].clear();
		auto&& section = MapRules.GetSection(pTypeName);
		for (auto& itr : section)
			ExtSets[nType].insert(itr.second);
	};

	loadSet("BuildingTypes", Set_Building);
	loadSet("InfantryTypes", Set_Infantry);
	loadSet("VehicleTypes", Set_Vehicle);
	loadSet("AircraftTypes", Set_Aircraft);

	if (theApp.m_Configs.ViewObjects_GuessMode == 1)
	{
		auto loadOwner = [this]()
			{
				auto&& sides = MapRules.ParseIndicies("Sides", true);
				for (size_t i = 0, sz = sides.size(); i < sz; ++i)
				{
					for (const auto& owner 
						: std::string_view{ sides[i] } 
						| std::views::split(',') 
						| std::views::transform([](auto rng) {
						return std::string_view(rng);
					}))
						Owners[owner.data()] = i;
				}
			};
		loadOwner();
	}

	if (const auto knownSection = fadata.GetSection("ForceSides"))
	{
		for (const auto& [id, side] : knownSection->values)
		{
			int sideIndex = ParseToInt(side, -1);
			if (sideIndex >= fadata.GetKeyCount("Sides"))
				continue;
			if (sideIndex < -1)
				sideIndex = -1;
			KnownItem[id] = sideIndex;
		}
	}

	if (const auto ignores = fadata.GetSection("IgnoreRA2"))
	{
		for (const auto& [_, id] : ignores->values)
		{
			CString tmp = id;
			tmp.Trim();
			IgnoreSet.emplace(tmp);
		}
	}

	if (const auto forcenames = fadata.GetSection("ForceName"))
	{
		for (const auto& [_, id] : forcenames->values)
		{
			CString tmp = id;
			tmp.Trim();
			ForceName.emplace(tmp);
		}
	}
}

void CViewObjects::Redraw_MainList()
{
	ExtNodes[Root_Nothing] = InsertTranslatedString("NothingObList", -2);
	ExtNodes[Root_Ground] = InsertTranslatedString("GroundObList", 13);
	ExtNodes[Root_Owner] = InsertTranslatedString("ChangeOwnerObList");
	ExtNodes[Root_Infantry] = InsertTranslatedString("InfantryObList", 0);
	ExtNodes[Root_Vehicle] = InsertTranslatedString("VehiclesObList", 1);
	ExtNodes[Root_Aircraft] = InsertTranslatedString("AircraftObList", 2);
	ExtNodes[Root_Building] = InsertTranslatedString("StructuresObList", 3);
	ExtNodes[Root_Terrain] = InsertTranslatedString("TerrainObList", 4);
	ExtNodes[Root_Smudge] = InsertTranslatedString("SmudgesObList", 10);
	ExtNodes[Root_Overlay] = InsertTranslatedString("OverlayObList", 5);
	ExtNodes[Root_Waypoint] = InsertTranslatedString("WaypointsObList", 6);
	ExtNodes[Root_Celltag] = InsertTranslatedString("CelltagsObList", 7);
	ExtNodes[Root_Basenode] = InsertTranslatedString("BaseNodesObList", 8);
	ExtNodes[Root_Tunnel] = InsertTranslatedString("TunnelObList", 9);
	ExtNodes[Root_PlayerLocation] = InsertTranslatedString("StartpointsObList", 12);
	// ExtNodes[Root_PropertyBrush] = InsertTranslatedString("PropertyBrushObList", 14);
	ExtNodes[Root_Delete] = InsertTranslatedString("DelObjObList", 10);
}

void CViewObjects::Redraw_Ground()
{
	HTREEITEM& hGround = ExtNodes[Root_Ground];
	if (hGround == NULL)    return;

	auto& doc = Map->GetIniFile();
	auto theater = doc.GetString("Map", "Theater");
	if (theater == "NEWURBAN")
		theater = "UBN";

	CString suffix;
	if (theater != "")
		suffix = theater.Mid(0, 3);

	this->InsertTranslatedString("GroundClearObList" + suffix, 61, hGround);
	this->InsertTranslatedString("GroundSandObList" + suffix, 62, hGround);
	this->InsertTranslatedString("GroundRoughObList" + suffix, 63, hGround);
	this->InsertTranslatedString("GroundGreenObList" + suffix, 65, hGround);
	this->InsertTranslatedString("GroundPaveObList" + suffix, 66, hGround);
	this->InsertTranslatedString("GroundWaterObList", 64, hGround);

	//if (CIniFile::CurrentTheater)
	//{
	//	int i = 67;
	//	for (auto& morphables : TheaterInfo::CurrentInfo)
	//	{
	//		auto InsertTile = [&](int nTileset)
	//		{
	//			FA2sp::Buffer.Format("TileSet%04d", nTileset);
	//			FA2sp::Buffer = CINI::CurrentTheater->GetString(FA2sp::Buffer, "SetName", FA2sp::Buffer);
	//			ppmfc::CString buffer;
	//			Translations::GetTranslationItem(FA2sp::Buffer, FA2sp::Buffer);
	//			return this->InsertString(FA2sp::Buffer, i++, hGround, TVI_LAST);
	//		};
	//
	//		InsertTile(morphables.Morphable);
	//	}
	//}
}

void CViewObjects::Redraw_Owner()
{
	HTREEITEM& hOwner = ExtNodes[Root_Owner];
	if (hOwner == NULL)    return;

	if (theApp.m_Configs.ViewObjects_SafeHouses)
	{
		if (Map->IsMultiplayer())
		{
			auto&& section = MapRules.GetSection("Countries");
			auto itr = section.begin();
			for (size_t i = 0, sz = section.size(); i < sz; ++i, ++itr)
			{
				if (strcmp(itr->second, "Neutral") == 0 || strcmp(itr->second, "Special") == 0)
					this->InsertString(itr->second, Const_House + i, hOwner);
			}
		}
		else
		{
			auto&& section = MapRules.ParseIndicies("Houses", true);
			for (size_t i = 0, sz = section.size(); i < sz; ++i)
				this->InsertString(section[i], Const_House + i, hOwner);
		}
	}
	else
	{
		if (Map->IsMultiplayer())
		{
			if (auto pSection = CIniFile::Rules.GetSection("Countries"))
			{
				auto& section = pSection->values;
				size_t i = 0;
				for (auto& itr : section)
					this->InsertString(itr.second, Const_House + i++, hOwner);
			}
		}
		else
		{
			if (auto pSection = Map->GetIniFile().GetSection("Houses"))
			{
				auto& section = pSection->values;
				size_t i = 0;
				for (auto& itr : section)
					this->InsertString(itr.second, Const_House + i++, hOwner);
			}
		}
	}
}


void CViewObjects::Redraw_Infantry()
{
	HTREEITEM& hInfantry = ExtNodes[Root_Infantry];
	if (hInfantry == NULL)   return;

	std::map<int, HTREEITEM> subNodes;

	auto& fadata = CIniFile::FAData;

	int i = 0;
	if (auto sides = fadata.GetSection("Sides"))
		for (auto& itr : sides->values)
			subNodes[i++] = this->InsertString(itr.second, -1, hInfantry);
	else
	{
		subNodes[i++] = this->InsertString("Allied", -1, hInfantry);
		subNodes[i++] = this->InsertString("Soviet", -1, hInfantry);
		subNodes[i++] = this->InsertString("Yuri", -1, hInfantry);
	}
	subNodes[-1] = this->InsertTranslatedString("OthObList", -1, hInfantry);

	auto&& infantries = MapRules.GetSection("InfantryTypes");
	for (auto& inf : infantries)
	{
		if (IgnoreSet.find(inf.second) != IgnoreSet.end())
			continue;
		int index = ParseToInt(inf.first, -1);
		if (index == -1)   continue;
		int side = GuessSide(inf.second, Set_Infantry);
		if (subNodes.find(side) == subNodes.end())
			side = -1;
		this->InsertString(
			QueryUIName(inf.second) + " (" + inf.second + ")",
			Const_Infantry + index,
			subNodes[side]
		);
	}

	// Clear up
	if (theApp.m_Configs.ViewObjects_Cleanup)
	{
		for (auto& subnode : subNodes)
		{
			if (!this->GetTreeCtrl().ItemHasChildren(subnode.second))
				this->GetTreeCtrl().DeleteItem(subnode.second);
		}
	}
}

void CViewObjects::Redraw_Vehicle()
{
    HTREEITEM& hVehicle = ExtNodes[Root_Vehicle];
    if (hVehicle == NULL)    return;

    std::map<int, HTREEITEM> subNodes;

    auto& fadata = CIniFile::FAData;

    int i = 0;
    if (auto sides = fadata.GetSection("Sides"))
        for (auto& itr : sides->values)
            subNodes[i++] = this->InsertString(itr.second, -1, hVehicle);
	else
	{
        subNodes[i++] = this->InsertString("Allied", -1, hVehicle);
        subNodes[i++] = this->InsertString("Soviet", -1, hVehicle);
        subNodes[i++] = this->InsertString("Yuri", -1, hVehicle);
    }
    subNodes[-1] = this->InsertTranslatedString("OthObList", -1, hVehicle);

    auto&& vehicles = MapRules.GetSection("VehicleTypes");
	for (auto& veh : vehicles)
	{
        if (IgnoreSet.find(veh.second) != IgnoreSet.end())
            continue;
        int index = ParseToInt(veh.first, -1);
        if (index == -1)   continue;
        int side = GuessSide(veh.second, Set_Vehicle);
        if (subNodes.find(side) == subNodes.end())
            side = -1;
		this->InsertString(
            QueryUIName(veh.second) + " (" + veh.second + ")",
            Const_Vehicle + index,
            subNodes[side]
        );
    }

    // Clear up
	if (theApp.m_Configs.ViewObjects_Cleanup)
	{
		for (auto& subnode : subNodes)
		{
            if (!this->GetTreeCtrl().ItemHasChildren(subnode.second))
                this->GetTreeCtrl().DeleteItem(subnode.second);
        }
    }
}

void CViewObjects::Redraw_Aircraft()
{
    HTREEITEM& hAircraft = ExtNodes[Root_Aircraft];
    if (hAircraft == NULL)    return;

    std::map<int, HTREEITEM> subNodes;

    auto& fadata = CIniFile::FAData;

    int i = 0;
    if (auto sides = fadata.GetSection("Sides"))
        for (auto& itr : sides->values)
            subNodes[i++] = this->InsertString(itr.second, -1, hAircraft);
	else
	{
        subNodes[i++] = this->InsertString("Allied", -1, hAircraft);
        subNodes[i++] = this->InsertString("Soviet", -1, hAircraft);
        subNodes[i++] = this->InsertString("Yuri", -1, hAircraft);
    }
    subNodes[-1] = this->InsertTranslatedString("OthObList", -1, hAircraft);

    auto&& aircrafts = MapRules.GetSection("AircraftTypes");
	for (auto& air : aircrafts)
	{
        if (IgnoreSet.find(air.second) != IgnoreSet.end())
            continue;
        int index = ParseToInt(air.first, -1);
        if (index == -1)   continue;
        int side = GuessSide(air.second, Set_Aircraft);
        if (subNodes.find(side) == subNodes.end())
            side = -1;
		this->InsertString(
            QueryUIName(air.second) + " (" + air.second + ")",
            Const_Aircraft + index,
            subNodes[side]
        );
    }

    // Clear up
	if (theApp.m_Configs.ViewObjects_Cleanup)
	{
		for (auto& subnode : subNodes)
		{
            if (!this->GetTreeCtrl().ItemHasChildren(subnode.second))
                this->GetTreeCtrl().DeleteItem(subnode.second);
        }
    }
}

void CViewObjects::Redraw_Building()
{
    HTREEITEM& hBuilding = ExtNodes[Root_Building];
    if (hBuilding == NULL)    return;

    std::map<int, HTREEITEM> subNodes;

    auto& fadata = CIniFile::FAData;

    int i = 0;
    if (auto sides = fadata.GetSection("Sides"))
        for (auto& itr : sides->values)
            subNodes[i++] = this->InsertString(itr.second, -1, hBuilding);
	else
	{
        subNodes[i++] = this->InsertString("Allied", -1, hBuilding);
        subNodes[i++] = this->InsertString("Soviet", -1, hBuilding);
        subNodes[i++] = this->InsertString("Yuri", -1, hBuilding);
    }
    subNodes[-1] = this->InsertTranslatedString("OthObList", -1, hBuilding);

    auto&& buildings = MapRules.GetSection("BuildingTypes");
	for (auto& bld : buildings)
	{
        if (IgnoreSet.find(bld.second) != IgnoreSet.end())
            continue;
        int index = ParseToInt(bld.first, -1);
        if (index == -1)   continue;
        int side = GuessSide(bld.second, Set_Building);
        if (subNodes.find(side) == subNodes.end())
            side = -1;
		this->InsertString(
            QueryUIName(bld.second) + " (" + bld.second + ")",
            Const_Building + index,
            subNodes[side]
        );
    }

    // Clear up
	if (theApp.m_Configs.ViewObjects_Cleanup)
	{
		for (auto& subnode : subNodes)
		{
            if (!this->GetTreeCtrl().ItemHasChildren(subnode.second))
                this->GetTreeCtrl().DeleteItem(subnode.second);
        }
    }
}

void CViewObjects::Redraw_Terrain()
{
	HTREEITEM& hTerrain = ExtNodes[Root_Terrain];
	if (hTerrain == NULL)   return;

	std::vector<std::pair<HTREEITEM, CString>> nodes;

	if (auto pSection = CIniFile::FAData.GetSection("ObjectBrowser.TerrainTypes"))
	{
		std::map<int, CString> collector;

		for (auto& pair : pSection->value_orig_pos)
			collector[pair.second] = pair.first;

		for (auto& pair : collector)
		{
			const auto& contains = pair.second;
			const auto& translation = pSection->values.find(contains)->second;

			nodes.push_back(std::make_pair(this->InsertTranslatedString(translation, -1, hTerrain), contains));
		}
	}
	HTREEITEM hOther = this->InsertTranslatedString("OthObList", -1, hTerrain);

	auto&& terrains = MapRules.ParseIndicies("TerrainTypes", true);
	for (size_t i = 0, sz = terrains.size(); i < sz; ++i)
	{
		if (IgnoreSet.find(terrains[i]) == IgnoreSet.end())
		{
			auto buffer = QueryUIName(terrains[i]);
			buffer += " (" + terrains[i] + ")";
			bool bNotOther = false;
			for (const auto& node : nodes)
			{
				if (terrains[i].Find(node.second) >= 0)
				{
					this->InsertString(buffer, Const_Terrain + i, node.first);
					bNotOther = true;
				}
			}
			if (!bNotOther)
				this->InsertString(buffer, Const_Terrain + i, hOther);
		}
	}

	this->InsertTranslatedString("RndTreeObList", 50999, hTerrain);
}


void CViewObjects::Redraw_Smudge()
{
	HTREEITEM& hSmudge = ExtNodes[Root_Smudge];
	if (hSmudge == NULL)   return;

	std::vector<std::pair<HTREEITEM, CString>> nodes;

	if (auto pSection = CIniFile::FAData.GetSection("ObjectBrowser.SmudgeTypes"))
	{
		std::map<int, CString> collector;

		for (auto& pair : pSection->value_orig_pos)
			collector[pair.second] = pair.first;

		for (auto& pair : collector)
		{
			const auto& contains = pair.second;
			const auto& translation = pSection->values.find(contains)->second;

			nodes.push_back(std::make_pair(this->InsertTranslatedString(translation, -1, hSmudge), contains));
		}
	}
	HTREEITEM hOther = this->InsertTranslatedString("OthObList", -1, hSmudge);

	auto&& smudges = MapRules.ParseIndicies("SmudgeTypes", true);
	for (size_t i = 0, sz = smudges.size(); i < sz; ++i)
	{
		if (IgnoreSet.find(smudges[i]) == IgnoreSet.end())
		{
			bool bNotOther = false;
			for (const auto& node : nodes)
			{
				if (smudges[i].Find(node.second) >= 0)
				{
					this->InsertString(smudges[i], Const_Smudge + i, node.first);
					bNotOther = true;
				}
			}
			if (!bNotOther)
				this->InsertString(smudges[i], Const_Smudge + i, hOther);
		}
	}
}


void CViewObjects::Redraw_Overlay()
{
	HTREEITEM& hOverlay = ExtNodes[Root_Overlay];
	if (hOverlay == NULL)   return;

	HTREEITEM hTemp;
	hTemp = this->InsertTranslatedString("DelOvrlObList", -1, hOverlay);
	this->InsertTranslatedString("DelOvrl0ObList", 60100, hTemp);
	this->InsertTranslatedString("DelOvrl1ObList", 60101, hTemp);
	this->InsertTranslatedString("DelOvrl2ObList", 60102, hTemp);
	this->InsertTranslatedString("DelOvrl3ObList", 60103, hTemp);

	hTemp = this->InsertTranslatedString("GrTibObList", -1, hOverlay);
	this->InsertTranslatedString("DrawTibObList", 60210, hTemp);
	this->InsertTranslatedString("DrawTib2ObList", 60310, hTemp);

	hTemp = this->InsertTranslatedString("BridgesObList", -1, hOverlay);
	this->InsertTranslatedString("BigBridgeObList", 60500, hTemp);
	this->InsertTranslatedString("SmallBridgeObList", 60501, hTemp);
	this->InsertTranslatedString("BigTrackBridgeObList", 60502, hTemp);
	this->InsertTranslatedString("SmallConcreteBridgeObList", 60503, hTemp);

	// Walls
	HTREEITEM hWalls = this->InsertTranslatedString("OthObList", -1, hOverlay);

	hTemp = this->InsertTranslatedString("AllObList", -1, hOverlay);

	this->InsertTranslatedString("OvrlManuallyObList", 60001, hOverlay);
	this->InsertTranslatedString("OvrlDataManuallyObList", 60002, hOverlay);

	if (nullptr == CIniFile::Rules.GetSection("OverlayTypes"))
		return;

	// a rough support for tracks
	this->InsertTranslatedString("Tracks", Const_Overlay + 39, hOverlay);

	MultimapHelper mmh;
	mmh.AddINI(&CIniFile::Rules);
	auto&& overlays = mmh.ParseIndicies("OverlayTypes", true);
	for (size_t i = 0, sz = std::min<unsigned int>(overlays.size(), 255); i < sz; ++i)
	{
		CString buffer;
		buffer = QueryUIName(overlays[i]);
		buffer += " (" + overlays[i] + ")";
		if (CIniFile::Rules.GetBoolean(overlays[i], "Wall"))
			this->InsertString(
				QueryUIName(overlays[i]),
				Const_Overlay + i,
				hWalls
			);
		if (IgnoreSet.find(overlays[i]) == IgnoreSet.end())
			this->InsertString(buffer, Const_Overlay + i, hTemp);
	}
}


void CViewObjects::Redraw_Waypoint()
{
	HTREEITEM& hWaypoint = ExtNodes[Root_Waypoint];
	if (hWaypoint == NULL)   return;

	this->InsertTranslatedString("CreateWaypObList", 20, hWaypoint);
	this->InsertTranslatedString("DelWaypObList", 21, hWaypoint);
	this->InsertTranslatedString("CreateSpecWaypObList", 22, hWaypoint);
}

void CViewObjects::Redraw_Celltag()
{
	HTREEITEM& hCellTag = ExtNodes[Root_Celltag];
	if (hCellTag == NULL)   return;

	this->InsertTranslatedString("CreateCelltagObList", 36, hCellTag);
	this->InsertTranslatedString("DelCelltagObList", 37, hCellTag);
	this->InsertTranslatedString("CelltagPropObList", 38, hCellTag);
}

void CViewObjects::Redraw_Basenode()
{
	HTREEITEM& hBasenode = ExtNodes[Root_Basenode];
	if (hBasenode == NULL)   return;

	this->InsertTranslatedString("CreateNodeNoDelObList", 40, hBasenode);
	this->InsertTranslatedString("CreateNodeDelObList", 41, hBasenode);
	this->InsertTranslatedString("DelNodeObList", 42, hBasenode);
}

void CViewObjects::Redraw_Tunnel()
{
	HTREEITEM& hTunnel = ExtNodes[Root_Tunnel];
	if (hTunnel == NULL)   return;

	if (CIniFile::FAData.GetBoolean("Debug", "AllowTunnels"))
	{
		this->InsertTranslatedString("NewTunnelObList", 50, hTunnel);
		this->InsertTranslatedString("ModifyTunnelObList", 51, hTunnel);
		this->InsertTranslatedString("NewTunnelSingleObList", 52, hTunnel);
		this->InsertTranslatedString("ModifyTunnelSingleObList", 53, hTunnel);
		this->InsertTranslatedString("DelTunnelObList", 54, hTunnel);
	}
}

void CViewObjects::Redraw_PlayerLocation()
{
	HTREEITEM& hPlayerLocation = ExtNodes[Root_PlayerLocation];
	if (hPlayerLocation == NULL)   return;

	this->InsertTranslatedString("StartpointsDelete", 21, hPlayerLocation);

	if (Map->IsMultiplayer())
	{
		for (int i = 0; i < 8; ++i)
			this->InsertString(std::format("Player {0}", i).c_str(), 23 + i, hPlayerLocation);
	}
}

bool CViewObjects::IsIgnored(const char* pItem)
{
	return IgnoreSet.find(pItem) != IgnoreSet.end();
}

CString CViewObjects::QueryUIName(const char* pRegName, bool bOnlyOneLine)
{
	if (!bOnlyOneLine)
	{
		if (ForceName.find(pRegName) != ForceName.end())
			return MapRules.GetString(pRegName, "Name", pRegName);
		else
			return Map->GetUnitName(pRegName);
	}

	CString buffer;

	if (ForceName.find(pRegName) != ForceName.end())
		buffer = MapRules.GetString(pRegName, "Name", pRegName);
	else
		buffer = Map->GetUnitName(pRegName);

	int idx = buffer.Find('\n');
	return idx == -1 ? buffer : buffer.Mid(0, idx);
}

int CViewObjects::GuessType(const char* pRegName)
{
	if (ExtSets[Set_Building].find(pRegName) != ExtSets[Set_Building].end())
		return Set_Building;
	if (ExtSets[Set_Infantry].find(pRegName) != ExtSets[Set_Infantry].end())
		return Set_Infantry;
	if (ExtSets[Set_Vehicle].find(pRegName) != ExtSets[Set_Vehicle].end())
		return Set_Vehicle;
	if (ExtSets[Set_Aircraft].find(pRegName) != ExtSets[Set_Aircraft].end())
		return Set_Aircraft;
	return -1;
}

int CViewObjects::GuessSide(const char* pRegName, int nType)
{
	auto&& knownIterator = KnownItem.find(pRegName);
	if (knownIterator != KnownItem.end())
		return knownIterator->second;

	int result = -1;
	switch (nType)
	{
	case -1:
	default:
		break;
	case Set_Building:
		result = GuessBuildingSide(pRegName);
		break;
	case Set_Infantry:
		result = GuessGenericSide(pRegName, Set_Infantry);
		break;
	case Set_Vehicle:
		result = GuessGenericSide(pRegName, Set_Vehicle);
		break;
	case Set_Aircraft:
		result = GuessGenericSide(pRegName, Set_Aircraft);
		break;
	}
	KnownItem[pRegName] = result;
	return result;
}

int CViewObjects::GuessBuildingSide(const char* pRegName)
{
	auto& rules = CIniFile::Rules;

	int planning;
	planning = rules.GetInteger(pRegName, "AIBasePlanningSide", -1);
	if (planning >= rules.GetKeyCount("Sides"))
		return -1;
	if (planning >= 0)
		return planning;
	size_t i = 0;
	for (const auto& con : SplitString(rules.GetString("AI", "BuildConst")))
	{
		if (con == pRegName)
            return i;
		++i;
	}
	if (i >= rules.GetKeyCount("Sides"))
		return -1;
	return GuessGenericSide(pRegName, Set_Building);
}

int CViewObjects::GuessGenericSide(const char* pRegName, int nType)
{
	const auto& set = ExtSets[nType];

	if (set.find(pRegName) == set.end())
		return -1;

	switch (theApp.m_Configs.ViewObjects_GuessMode)
	{
	default:
	case 0:
	{
		for (const auto& prep : SplitString(MapRules.GetString(pRegName, "Prerequisite")))
		{
			int guess = -1;
			for (const auto& subprep : SplitString(MapRules.GetString("GenericPrerequisites", prep)))
			{
				if (subprep == pRegName) // Avoid build myself crash
					return -1;
				guess = GuessSide(subprep, GuessType(subprep));
				if (guess != -1)
					return guess;
			}
			if (prep == pRegName) // Avoid build myself crash
				return -1;
			guess = GuessSide(prep, GuessType(prep));
			if (guess != -1)
				return guess;
		}
		return -1;
	}
	case 1:
	{
		auto&& owners = SplitString(MapRules.GetString(pRegName, "Owner"));
		if (owners.size() <= 0)
			return -1;
		auto&& itr = Owners.find(owners[0]);
		if (itr == Owners.end())
			return -1;
		return itr->second;
	}
	}
}