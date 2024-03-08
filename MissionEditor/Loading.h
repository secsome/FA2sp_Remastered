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
#include "MissionEditorPackLib.h"
#include <memory>
#include <optional>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Loading.h : header file
//

class VoxelNormalTables;

struct EXPANDMIX
{
	HMIXFILE hExpand; // NULL if expansion mix does not exist
	HMIXFILE hECache; // NULL if no ECache
	HMIXFILE hConquer; // NULL if no Conquer
	HMIXFILE hLocal;
	HMIXFILE hIsoSnow; // NULL if no IsoSnow
	HMIXFILE hIsoTemp; // NULL if no IsoTemp
	HMIXFILE hIsoUrb;
	HMIXFILE hIsoGen;
	HMIXFILE hIsoLun;
	HMIXFILE hIsoDes;
	HMIXFILE hIsoUbn;
	HMIXFILE hIsoGenMd;
	HMIXFILE hIsoLunMd;
	HMIXFILE hIsoDesMd;
	HMIXFILE hIsoUbnMd;
	HMIXFILE hTemperat; // NULL if no Temperat
	HMIXFILE hSnow;
	HMIXFILE hUrban;
	HMIXFILE hLunar;
	HMIXFILE hUrbanN;
	HMIXFILE hDesert;
	HMIXFILE hGeneric;
	HMIXFILE hTem;
	HMIXFILE hSno;
	HMIXFILE hUrb;
	HMIXFILE hLun;
	HMIXFILE hDes;
	HMIXFILE hUbn;
	HMIXFILE hBuildings;
	HMIXFILE hMarble;
	EXPANDMIX() {memset(this, 0, sizeof(EXPANDMIX));};
};

class CFinalSunDlg;


/////////////////////////////////////////////////////////////////////////////
// dialog field CLoading 


struct FindShpResult
{
	FindShpResult(HMIXFILE mixfile_, TheaterChar mixfile_theater_, CString filename_, TheaterChar theat_, HTSPALETTE palette_): mixfile(mixfile_), mixfile_theater(mixfile_theater_), filename(filename_), theat(theat_), palette(palette_) { }
	HMIXFILE mixfile;
	TheaterChar mixfile_theater;
	CString filename;
	TheaterChar theat;
	HTSPALETTE palette;
};

class CLoading : public CDialog
{
// Construction
public:
	void CreateConvTable(RGBTRIPLE* pal, int* iPal);
	void FetchPalettes();
	void PrepareUnitGraphic(LPCSTR lpUnittype);
	void LoadStrings();
	void FreeAll();
	void FreeTileSet();
	BOOL InitDirectDraw();
	
	void InitTMPs(CProgressCtrl* prog=NULL);
	void InitPalettes();
	
	void* ReadWholeFile(LPCSTR lpFilename, DWORD* pSize = NULL);

	~CLoading();
	void Unload();
	bool InitMixFiles();
	void InitSHPs(CProgressCtrl* prog=NULL);
	void LoadTSIni(LPCTSTR lpFilename, CIniFile& ini);
	void CreateINI();
	CLoading(CWnd* pParent = NULL);   // Standardconstructor
	void InitPics(CProgressCtrl* prog=NULL);
	void Load();
	BOOL LoadUnitGraphic(LPCTSTR lpUnittype);
	void LoadBuildingSubGraphic(const CString& subkey, const CIniFileSection& artSection, BOOL bAlwaysSetChar, char theat, HMIXFILE hShpMix, SHPHEADER& shp_h, BYTE*& shp);
	void LoadOverlayGraphic(LPCTSTR lpOvrlName, int iOvrlNum);
	void InitVoxelNormalTables();
	HTSPALETTE GetIsoPalette(char theat);
	HTSPALETTE GetUnitPalette(char theat);
	std::optional<FindShpResult> FindUnitShp(const CString& image, char preferred_theat, const CIniFileSection& artSection);
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
	void CalcPicCount();
	int m_pic_count;
	int m_bmp_count;
	BOOL LoadTile(LPCSTR lpFilename, HMIXFILE hOwner, HTSPALETTE hPalette, DWORD dwID, BOOL bReplacement);
	
	HTSPALETTE PAL_ISOTEM;
	HTSPALETTE PAL_ISOSNO;
	HTSPALETTE PAL_ISOURB;
	HTSPALETTE PAL_ISOLUN;
	HTSPALETTE PAL_ISODES;
	HTSPALETTE PAL_ISOUBN;
	HTSPALETTE PAL_UNITTEM;
	HTSPALETTE PAL_UNITSNO;
	HTSPALETTE PAL_UNITURB;
	HTSPALETTE PAL_UNITLUN;
	HTSPALETTE PAL_UNITDES;
	HTSPALETTE PAL_UNITUBN;
	HTSPALETTE PAL_TEMPERAT;
	HTSPALETTE PAL_SNOW;
	HTSPALETTE PAL_URBAN;
	HTSPALETTE PAL_LUNAR;
	HTSPALETTE PAL_DESERT;
	HTSPALETTE PAL_URBANN;
	HTSPALETTE PAL_LIBTEM;

	HMIXFILE FindFileInMix(LPCTSTR lpFilename, TheaterChar* pTheaterChar=NULL);
	bool loaded;

	std::unique_ptr<VoxelNormalTables> m_voxelNormalTables;
	
	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_LOADING_H__5D5C3284_8962_11D3_B63B_AAA51FD322E3__INCLUDED_
