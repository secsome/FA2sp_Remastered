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

#include "stdafx.h"
#include "FinalSun.h"
#include "Loading.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>
#include "resource.h"
#include "mapdata.h"
#include "variables.h"
#include "functions.h"
#include "inlines.h"

#include "PKey.h"
#include "MixFile.h"
#include "Base64.h"

#include <sstream>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CLoading 


CLoading::CLoading(CWnd* pParent /*=NULL*/)
	: CDialog(CLoading::IDD, pParent)
{
	Create(CLoading::IDD, pParent);

	loaded = false;
	current_theater='T';

	m_pic_count=0;

	s_tiledata=NULL;
	t_tiledata=NULL;
	u_tiledata=NULL;
	s_tiledata_count=0;
	t_tiledata_count=0;
	u_tiledata_count=0;
	tiledata=NULL;
	tiledata_count=0;

	errstream << "CLoading::CLoading() called" << endl;
	errstream.flush();
}


void CLoading::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLoading)
	DDX_Control(pDX, IDC_VERSION, m_Version);
	DDX_Control(pDX, IDC_BUILTBY, m_BuiltBy);
	DDX_Control(pDX, IDC_CAP, m_cap);
	DDX_Control(pDX, IDC_PROGRESS1, m_progress);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLoading, CDialog)
	//{{AFX_MSG_MAP(CLoading)
	ON_WM_DESTROY()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// message handlers CLoading 

void CLoading::Load()
{
	m_progress.SetRange(0, 100);

	CString artFile;

	// show a wait cursor
	SetCursor(LoadCursor(NULL, IDC_WAIT));

	// write a log
	errstream << "CLoading::Load() called" << std::endl << std::endl;
	errstream.flush();

	m_cap.SetWindowText(GetLanguageStringACP("LoadExtractStdMixFiles"));

	errstream << "Initializing mix files" << std::endl;
	errstream.flush();
	MEMORYSTATUS ms;
	ms.dwLength=sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&ms);
	int cs=ms.dwAvailPhys+ms.dwAvailPageFile;
	m_progress.SetPos(10); UpdateWindow();
	InitMixFiles();
	errstream << "Loading palettes" << std::endl << std::endl;
	errstream.flush();
	// create a ini file containing some info XCC Mixer needs
	CreateINI();	
	
	// CIniFile::Rules.ini
	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadRules"));
	m_progress.SetPos(30); UpdateWindow();
	LoadTSIni(CIniFile::FAData.GetString("Filenames", "Rules", "rulesmd.ini"), CIniFile::Rules);
	PrepareHouses();
	//HackRules();


	// CIniFile::Art.ini
	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadArt"));
	m_progress.SetPos(35); UpdateWindow();
	LoadTSIni(CIniFile::FAData.GetString("Filenames", "Art", "artmd.ini"), CIniFile::Art);

	// sound.ini
	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadSound"));
	m_progress.SetPos(40); UpdateWindow();
	LoadTSIni(CIniFile::FAData.GetString("Filenames", "Sound", "soundmd.ini"), CIniFile::Sound);


	// eva.INI
	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadEva"));
	m_progress.SetPos(45); UpdateWindow();
	LoadTSIni(CIniFile::FAData.GetString("Filenames", "Eva", "evamd.ini"), CIniFile::Eva);

	// theme.INI
	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadTheme"));
	m_progress.SetPos(50); UpdateWindow();
	LoadTSIni(CIniFile::FAData.GetString("Filenames", "Theme", "thememd.ini"), CIniFile::Theme);


	// AI.INI
	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadAI"));
	m_progress.SetPos(55); UpdateWindow();
	LoadTSIni(CIniFile::FAData.GetString("Filenames", "AI", "aimd.ini"), CIniFile::Ai);

	// Temperat.INI
	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadTemperat"));
	m_progress.SetPos(60); UpdateWindow();
	LoadTSIni(CIniFile::FAData.GetString("Filenames", "Temperate", "temperatmd.ini"), CIniFile::Temperate);

	// Snow.INI
	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadSnow"));
	m_progress.SetPos(65); UpdateWindow();
	LoadTSIni(CIniFile::FAData.GetString("Filenames", "Snow", "snowmd.ini"), CIniFile::Snow);

	// Urban.INI
	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadUrban"));
	m_progress.SetPos(70); UpdateWindow();
	LoadTSIni(CIniFile::FAData.GetString("Filenames", "Urban", "urbanmd.ini"), CIniFile::Urban);

	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadUrbanN"));
	m_progress.SetPos(75); UpdateWindow();
	LoadTSIni(CIniFile::FAData.GetString("Filenames", "NewUrban", "urbannmd.ini"), CIniFile::NewUrban);

	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadLunar"));
	m_progress.SetPos(80); UpdateWindow();
	LoadTSIni(CIniFile::FAData.GetString("Filenames", "Lunar", "lunarmd.ini"), CIniFile::Lunar);

	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadDesert"));
	m_progress.SetPos(85); UpdateWindow();
	LoadTSIni(CIniFile::FAData.GetString("Filenames", "Desert", "desertmd.ini"), CIniFile::Desert);

	
	
	// load Command & Conquer Rules.ini section names
	m_progress.SetPos(90); UpdateWindow();
	LoadStrings();

	// ok now directdraw
	m_cap.SetWindowText(GetLanguageStringACP("LoadInitDDraw"));
	m_progress.SetPos(95); UpdateWindow();
	InitDirectDraw();
	m_progress.SetPos(100); UpdateWindow();
	
	/*errstream << "Now calling InitPics()\n";
	errstream.flush();
	m_cap.SetWindowText(GetLanguageStringACP("LoadInitPics"));
	InitPics();
	errstream << "InitPics() finished\n\n\n";
	errstream.flush();*/

	DestroyWindow();

}

BOOL CLoading::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CString version;
	version.LoadString(IDS_VERSIONTEXT);

	m_Version.SetWindowText(version);
	
	CString builder;
	builder.LoadString(IDS_BUILTBY);
	m_BuiltBy.SetWindowText(builder);

	SetDlgItemText(IDC_LBUILTBY, GetLanguageStringACP("LoadBuiltBy"));
	SetDlgItemText(IDC_LVERSION, GetLanguageStringACP("LoadVersion"));
	SetDlgItemText(IDC_CAP, GetLanguageStringACP("LoadLoading"));
	

	
	UpdateWindow();

	return TRUE;
}

// write a small ini file containing the FinalSun path and version (XCC needs this)
// TODO: this was made for Win9x. It does not work anymore on modern operating systems if you don't run the editor as administrator (which you should not do)
void CLoading::CreateINI()
{
	wchar_t iniFile_[MAX_PATH];
	CIniFile path;
	CString version;
	
	GetWindowsDirectoryW(iniFile_, MAX_PATH);
	std::string iniFile = utf16ToUtf8(iniFile_);
	iniFile += "\\FinalAlert2.ini";

	CString app = "FinalAlert";

	version.LoadString(IDS_VERSION);
	path.sections[app].values["Path"]=ExePath;
	path.sections[app].values["Version"]=version;

	path.SaveFile(iniFile);
}

void CLoading::LoadTSIni(LPCTSTR lpFilename, CIniFile& ini)
{
	CCFileClass ccFile{ lpFilename };

	const auto buffer = ccFile.ReadWholeFile();
	size_t size = ccFile.Size();

	ccFile.Close();

	if (nullptr == buffer)
	{
		std::println(errstream, "Failed to read file {0}", lpFilename);
		return;
	}
	
	if (size == 0)
	{
		std::println(errstream, "File {0} is empty", lpFilename);
		delete[] buffer;
		return;
	}

	std::string_view sv(reinterpret_cast<const char*>(buffer), size);
	std::istringstream is(sv.data());

	if (ini.LoadFile(is, true) != 0)
		std::println(errstream, "Failed to load ini {0}", lpFilename);
	else
		std::println(errstream, "Loaded ini {0}", lpFilename);

	delete[] buffer;
}

bool CLoading::InitMixFiles()
{
	// Initialize PKey
	{
		PKey::FastKey.GetExponent() = BigInt(PKey::FAST_EXPONENT);
		auto pub_str = base64::decode("AihRvNoIbTn85FZRYNZRcT+i6KpU+maCsEqr3Q5q+LDB5tH7Tz2qQ38V");
		PKey::FastKey.Decode_Modulus(pub_str.data());
	}

	// Load Extra Mixes
	if (auto pSection = CIniFile::FAData.GetSection("ExtraMixes"))
	{
		std::map<int, CString> collector;

		for (const auto& [key, index] : pSection->value_orig_pos)
			collector[index] = key;

		CString path;

		for (const auto& [_, key] : collector)
		{
			if (CIniFile::FAData.GetBoolean("ExtraMixes", key))
				path = ExePath;
			else
				path = GamePath;
			path += "\\" + key;
			if (new MFCD(path, &PKey::FastKey))
				std::println(errstream, "[MixLoader][EXTRA] {0} loaded.", path);
			else
				std::println(errstream, "[MixLoader][EXTRA] {0} failed!", path);
		}
	}

	CString Dir = GamePath;
	Dir += "\\";
	auto LoadMixFile = [this, Dir](const char* Mix)
	{
		return nullptr != new MFCD(Mix, &PKey::FastKey);
	};

	
	CString format = "EXPAND" + CIniFile::FAData.GetString("Filenames", "MixExtension", "MD") + "%02d.MIX";
	for (int i = 99; i >= 0; --i)
	{
		CString filename;
		filename.Format(format, i);
		LoadMixFile(filename);
	}

	if (!LoadMixFile("RA2MD.MIX"))		return false;
	if (!LoadMixFile("RA2.MIX"))		return false;
	if (!LoadMixFile("CACHEMD.MIX"))	return false;
	if (!LoadMixFile("CACHE.MIX"))		return false;
	if (!LoadMixFile("LOCALMD.MIX"))	return false;
	if (!LoadMixFile("LOCAL.MIX"))		return false;

	// Init_Expansion_Mixfiles
	// ECACHE and ELOCAL
	WIN32_FIND_DATA fd;
	HANDLE hf = FindFirstFile(Dir + "ECACHE*.MIX", &fd);
	if (hf != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (fd.dwFileAttributes & (FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN))
				continue;
			LoadMixFile(fd.cFileName);
		} while (FindNextFile(hf, &fd));
		FindClose(hf);
	}
	hf = FindFirstFile(Dir + "ELOCAL*.MIX", &fd);
	if (hf != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (fd.dwFileAttributes & (FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN))
				continue;
			LoadMixFile(fd.cFileName);
		} while (FindNextFile(hf, &fd));
		FindClose(hf);
	}

	// Init_Secondary_Mixfiles
	if (!LoadMixFile("CONQMD.MIX"))		return false;
	if (!LoadMixFile("GENERMD.MIX"))	return false;
	if (!LoadMixFile("GENERIC.MIX"))	return false;
	if (!LoadMixFile("ISOGENMD.MIX"))	return false;
	if (!LoadMixFile("ISOGEN.MIX"))		return false;
	if (!LoadMixFile("CONQUER.MIX"))	return false;

	// Init_Theaters
	LoadMixFile("TEMPERATMD.MIX");
	LoadMixFile("ISOTEMMD.MIX");
	LoadMixFile("TEMPERAT.MIX");
	LoadMixFile("ISOTEMP.MIX");
	LoadMixFile("TEM.MIX");

	LoadMixFile("SNOWMD.MIX");
	LoadMixFile("ISOSNOMD.MIX");
	LoadMixFile("SNOW.MIX");
	LoadMixFile("ISOSNOW.MIX");
	LoadMixFile("ISOSNO.MIX");
	LoadMixFile("SNO.MIX");

	LoadMixFile("URBANMD.MIX");
	LoadMixFile("ISOURBMD.MIX");
	LoadMixFile("URBAN.MIX");
	LoadMixFile("ISOURB.MIX");
	LoadMixFile("URB.MIX");

	LoadMixFile("DESERT.MIX");
	LoadMixFile("ISODES.MIX");
	LoadMixFile("ISODESMD.MIX");
	LoadMixFile("DES.MIX");
	LoadMixFile("DESERTMD.MIX");

	LoadMixFile("URBANNMD.MIX");
	LoadMixFile("ISOUBNMD.MIX");
	LoadMixFile("URBANN.MIX");
	LoadMixFile("ISOUBN.MIX");
	LoadMixFile("UBN.MIX");

	LoadMixFile("LUNARMD.MIX");
	LoadMixFile("ISOLUNMD.MIX");
	LoadMixFile("LUNAR.MIX");
	LoadMixFile("ISOLUN.MIX");
	LoadMixFile("LUN.MIX");

	if (LoadMixFile("MARBLE.MIX"))
		theApp.m_Options.bSupportMarbleMadness = true;
	else
	{
		CString FullPath = ExePath;
		FullPath += "\\MARBLE.MIX";
		if (new MFCD(FullPath, &PKey::FastKey))
		{
			std::println(errstream, "[MixLoader] {0} loaded.", FullPath);
			theApp.m_Options.bSupportMarbleMadness = true;
		}
		else
		{
			theApp.m_Options.bSupportMarbleMadness = false;
			std::println(errstream, "Failed to load marble.mix!");
			MessageBox("Failed to load marble.mix! Framework mode won't be able to use!");
		}
	}

	return true;

}

CLoading::~CLoading()
{
	Unload();
}

void CLoading::Unload()
{
}

void CLoading::OnDestroy() 
{
	CDialog::OnDestroy();
}

BOOL CLoading::InitDirectDraw()
{
	last_succeeded_operation=7;

	errstream << "\n\nDirectDrawCreate() will be called now\n";
	errstream.flush();
		
	CIsoView& v=*((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview;
	if(DirectDrawCreate(NULL, &v.dd_1, NULL)!=DD_OK)
	{
		errstream << "DirectDrawCreate() failed\n";
		errstream.flush();
		ShowWindow(SW_HIDE);
		MessageBox("DirectDraw could not be initialized! Quitting...");
		exit(-1);

		return FALSE;
	} 

	errstream << "DirectDrawCreate() successful\n\n";
	errstream.flush();

	errstream << "Now querying the DirectX 7 or 6 interface\n";
	errstream.flush();

	if(v.dd_1->QueryInterface(IID_IDirectDraw7, (void**)&v.dd)!=DD_OK)
	{
		errstream << "QueryInterface() failed -> Using DirectX 6.0\n";
		errstream.flush();
		//ShowWindow(SW_HIDE);
		//MessageBox("You don´t have DirectX 6.0 but an older version. Quitting...");
		//exit(-1);

		//return FALSE;

		if(v.dd_1->QueryInterface(IID_IDirectDraw4, (void**)&v.dd)!=DD_OK)
		{
			MessageBox("You need at least DirectX 6.0 to run this program", "Error");
			exit(-1);
			return FALSE;
		}
	}

	errstream << "QueryInterface() successful\n\nNow setting cooperative level\n";
	errstream.flush();
		
	if(v.dd->SetCooperativeLevel(v.m_hWnd, DDSCL_NORMAL | DDSCL_NOWINDOWCHANGES )!=DD_OK)
	{
		errstream << "SetCooperativeLevel() failed\n";
		errstream.flush();
		ShowWindow(SW_HIDE);
		MessageBox("Cooperative Level could not be set! Quitting...");
		v.dd->Release();
		v.dd=NULL;
		exit(-2);

		return FALSE;
	}

	errstream << "SetCooperativeLevel() successful\n\nCreating primary surface\n";
	errstream.flush();
		
	DDSURFACEDESC2 ddsd;


	memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
	ddsd.dwSize=sizeof(DDSURFACEDESC2);
	ddsd.ddsCaps.dwCaps=DDSCAPS_PRIMARYSURFACE ;
	ddsd.dwFlags=DDSD_CAPS ;

    

	int res=0;
	int trycount=0;
	do
	{
		res=v.dd->CreateSurface(&ddsd, &v.lpds, NULL);
		errstream << "Return code: " << res << endl;
		errstream.flush();

		//if(res!=DD_OK && (res!=DDERR_PRIMARYSURFACEALREADYEXISTS || trycount>100))
		if(res!=DD_OK && trycount>=300)
		{
			
			errstream << "CreateSurface() failed\n";
			
			errstream.flush();
			ShowWindow(SW_HIDE);
			MessageBox("Primary surface could not be initialized! Quitting...");
			v.dd->Release();
			v.dd=NULL;
			exit(-3);

			return FALSE;
		}
		trycount++;
		if(res!=DD_OK)
		{ 
			Sleep(50);
		}

		
	}while(res!=DD_OK);

#ifdef NOSURFACES
	DDPIXELFORMAT pf;
	memset(&pf, 0, sizeof(DDPIXELFORMAT));
	pf.dwSize=sizeof(DDPIXELFORMAT);

	v.lpds->GetPixelFormat(&pf);

	if(!pf.dwBBitMask || !pf.dwRBitMask || !pf.dwGBitMask)
	{
		ShowWindow(SW_HIDE);
		MessageBox("You must not use a palette color mode like 8 bit in order to run FinalSun/FinalAlert 2. Please check readme.txt","Error",MB_OK);

		v.lpds->Release();
		v.lpds=NULL;
		v.dd->Release();
		v.dd=NULL;
		exit(-3);
		return FALSE;
	}
	bpp=(pf.dwRGBBitCount+7)/8;
#endif


	errstream << "CreateSurface() successful\n\nCreating backbuffer surface\n";
	errstream.flush();

	memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
	ddsd.dwSize=sizeof(DDSURFACEDESC2);
	ddsd.dwFlags=DDSD_WIDTH | DDSD_HEIGHT;
	v.lpds->GetSurfaceDesc(&ddsd);
	ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN;


	ddsd.dwFlags=DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;

	
	if(v.dd->CreateSurface(&ddsd, &v.lpdsBack, NULL)!=DD_OK)
	{
		errstream << "CreateSurface() failed\n";
		errstream.flush();
		ShowWindow(SW_HIDE);
		MessageBox("Backbuffer surface could not be initialized! Quitting...");
		v.lpds->Release();
		v.lpds=NULL;
		v.dd->Release();
		v.dd=NULL;
		exit(-4);

		return FALSE;
	}
	if (theApp.m_Options.bHighResUI && v.dd->CreateSurface(&ddsd, &v.lpdsBackHighRes, NULL) != DD_OK)
	{
		errstream << "CreateSurface() failed\n";
		errstream.flush();
		ShowWindow(SW_HIDE);
		MessageBox("Highres Backbuffer surface could not be initialized! Quitting...");
		v.lpdsBack->Release();
		v.lpdsBack = NULL;
		v.lpds->Release();
		v.lpds = NULL;
		v.dd->Release();
		v.dd = NULL;
		exit(-4);

		return FALSE;
	}
	if(v.dd->CreateSurface(&ddsd, &v.lpdsTemp, NULL)!=DD_OK)
	{
		errstream << "CreateSurface() failed\n";
		errstream.flush();
		ShowWindow(SW_HIDE);
		MessageBox("Tempbuffer surface could not be initialized! Quitting...");
		v.lpdsBack->Release();
		v.lpdsBack=NULL;
		if (v.lpdsBackHighRes)
			v.lpdsBackHighRes->Release();
		v.lpdsBackHighRes = nullptr;
		v.lpds->Release();
		v.lpds=NULL;
		v.dd->Release();
		v.dd=NULL;
		exit(-4);

		return FALSE;
	}

	errstream << "CreateSurface() successful\n\nNow creating clipper\n";
	errstream.flush();

	LPDIRECTDRAWCLIPPER ddc;
	if(v.dd->CreateClipper(0, &ddc, NULL)!=DD_OK)
	{
		errstream << "CreateClipper() failed\n";
		errstream.flush();
		ShowWindow(SW_HIDE);
		MessageBox("Clipper could not be created! Quitting...");
		v.lpdsTemp->Release();
		v.lpdsTemp=NULL;
		v.lpdsBack->Release();
		v.lpdsBack=NULL;
		if (v.lpdsBackHighRes)
			v.lpdsBackHighRes->Release();
		v.lpdsBackHighRes = nullptr;
		v.lpds->Release();
		v.lpds=NULL;
		v.dd->Release();
		v.dd=NULL;
		exit(-6);
	}

	errstream << "CreateClipper() successful\n\n";
	errstream.flush();

	v.lpds->SetClipper(ddc);
	
	ddc->SetHWnd(0, v.m_hWnd);

	return TRUE;
}

void CLoading::InitTMPs()
{
	shoreset = atoi((*CIniFile::CurrentTheater).sections["General"].values["ShorePieces"]);
	waterset = atoi((*CIniFile::CurrentTheater).sections["General"].values["WaterSet"]);

	int i, tcount = 0;

	for (i = 0; i < 10000; i++)
	{
		CString tset;
		char c[50];
		itoa(i, c, 10);
		int e;
		for (e = 0; e < 4 - strlen(c); e++)
			tset += "0";
		tset += c;
		CString sec = "TileSet";
		sec += tset;

		if (CIniFile::CurrentTheater->sections.find(sec) == CIniFile::CurrentTheater->sections.end()) break;

		for (e = 0; e < atoi(CIniFile::CurrentTheater->sections[sec].values["TilesInSet"]); e++)
		{
			tcount++;
		}


	}

	if (*tiledata != NULL) delete[] * tiledata;
	*tiledata = new(TILEDATA[tcount]);
	*tiledata_count = tcount;

	DWORD tilecount = 0;
	for (i = 0; i < 10000; i++)
	{
		CString tset;
		char c[50];
		itoa(i, c, 10);
		int e;
		for (e = 0; e < 4 - strlen(c); e++)
			tset += "0";
		tset += c;
		CString sec = "TileSet";
		sec += tset;

		if (CIniFile::CurrentTheater->sections.find(sec) == CIniFile::CurrentTheater->sections.end()) break;

		BOOL bTib, bMorph, bPlace, bMadness;
		bPlace = TRUE;
		bTib = FALSE;
		bMorph = FALSE;
		bMadness = FALSE;
		CIniFile::CurrentTheater->sections[sec].values["AllowTiberium"].MakeLower();
		if (CIniFile::CurrentTheater->sections[sec].values["AllowTiberium"] == "true")
			bTib = TRUE;
		CIniFile::CurrentTheater->sections[sec].values["Morphable"].MakeLower();
		if (CIniFile::CurrentTheater->sections[sec].values["Morphable"] == "true")
			bMorph = TRUE;
		CIniFile::CurrentTheater->sections[sec].values["AllowToPlace"].MakeLower();
		if (CIniFile::CurrentTheater->sections[sec].values["AllowToPlace"] == "no")
			bPlace = FALSE;
		CIniFile::CurrentTheater->sections[sec].values["NonMarbleMadness"].MakeLower();
		if (CIniFile::CurrentTheater->sections[sec].values["NonMarbleMadness"].GetLength() > 0)
			bMadness = TRUE;
		auto tilesetAnimSection = CIniFile::CurrentTheater->GetSection(CIniFile::CurrentTheater->sections[sec].GetString("SetName"));

		tilesets_start[i] = tilecount;


		for (e = 0; e < atoi(CIniFile::CurrentTheater->sections[sec].values["TilesInSet"]); e++)
		{
			std::string sId = std::format("{:02}", e + 1);
			CString filename = CIniFile::CurrentTheater->sections[sec].values["FileName"];
			filename += sId.c_str();

			CString bas_f = filename;

			CString suffix;

			if (CIniFile::CurrentTheater == &CIniFile::Temperate) suffix = ".tem";
			if (CIniFile::CurrentTheater == &CIniFile::Snow) suffix = ".sno";
			if (CIniFile::CurrentTheater == &CIniFile::Urban) suffix = ".urb";
			if (CIniFile::CurrentTheater == &CIniFile::NewUrban) suffix = ".ubn";
			if (CIniFile::CurrentTheater == &CIniFile::Lunar) suffix = ".lun";
			if (CIniFile::CurrentTheater == &CIniFile::Desert) suffix = ".des";

			filename += suffix;
			
			// MW add: use other...
			
			(*tiledata)[tilecount].bAllowTiberium = bTib;
			(*tiledata)[tilecount].bAllowToPlace = bPlace;
			(*tiledata)[tilecount].bMorphable = bMorph;
			(*tiledata)[tilecount].bMarbleMadness = bMadness;
			(*tiledata)[tilecount].wTileSet = i;


			tilecount++;
		}


	}

	tilecount = 0;
	for (i = 0; i < 10000; i++)
	{
		CString tset;
		char c[50];
		itoa(i, c, 10);
		int e;
		for (e = 0; e < 4 - strlen(c); e++)
			tset += "0";
		tset += c;
		CString sec = "TileSet";
		sec += tset;

		if (CIniFile::CurrentTheater->sections.find(sec) == CIniFile::CurrentTheater->sections.end()) break;


		int madnessid = atoi(CIniFile::CurrentTheater->sections[sec].values["MarbleMadness"]);

		for (e = 0; e < atoi(CIniFile::CurrentTheater->sections[sec].values["TilesInSet"]); e++)
		{
			if (madnessid)
			{
				(*tiledata)[tilecount].wMarbleGround = tilesets_start[madnessid] + (tilecount - tilesets_start[i]);
			}
			else
				(*tiledata)[tilecount].wMarbleGround = 0xFFFF;

			tilecount++;

		}


	}
}

void CLoading::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CWnd* poswindow=GetDlgItem(IDC_SAVEOFTEN);

	if(!poswindow) return;

	RECT r1,r2;
	poswindow->GetWindowRect(&r1);
	GetWindowRect(&r2);

	dc.SetTextColor(RGB(255, 0, 0));
	dc.SetBkMode(TRANSPARENT);

	RECT dest;
	dest.top=r1.bottom-r2.top-45;
	dest.bottom=r1.bottom-r2.top+10;
	dest.left=10;
	dest.right=r1.right-r1.left+10;

	CFont f;
	f.CreatePointFont(90, "Tahoma");
	dc.SelectObject(&f);

}

void CLoading::FreeTileSet()
{
	// free the current tileset

	int i;
	for(i=0;i<(*tiledata_count);i++)
	{
		int e;
		int z;
 
		// delete replacements. Replacements themself MUST NOT have replacements
		for(z=0;z<(*tiledata)[i].bReplacementCount;z++)
		{
			TILEDATA& rept=(*tiledata)[i].lpReplacements[z];
			
			for(e=0;e<rept.wTileCount;e++)
			{
#ifdef NOSURFACES
				BYTE* curSur=rept.tiles[e].pic;
				if(curSur) delete[] curSur;
				if(rept.tiles[e].vborder) delete[] rept.tiles[e].vborder;
#else
				LPDIRECTDRAWSURFACE4 curSur=rept.tiles[e].pic;
				if(curSur) curSur->Release();
#endif
			}
			
			delete[] rept.tiles;
			rept.tiles=NULL;
			rept.wTileCount=0;
			rept.bReplacementCount=0;

			ASSERT(!rept.lpReplacements); // make sure we don´t have replacements for this replacement
		}

		for(e=0;e<(*tiledata)[i].wTileCount;e++)
		{
#ifdef NOSURFACES
			BYTE* curSur=(*tiledata)[i].tiles[e].pic;
			if(curSur) delete[] curSur;
			if((*tiledata)[i].tiles[e].vborder) delete[] (*tiledata)[i].tiles[e].vborder;
#else
			LPDIRECTDRAWSURFACE4 curSur=(*tiledata)[i].tiles[e].pic;
			if(curSur) curSur->Release();
#endif
		}

		delete[] ((*tiledata)[i].tiles);
		(*tiledata)[i].tiles=NULL;
		(*tiledata)[i].wTileCount=0;
		(*tiledata)[i].bReplacementCount=0;
		if((*tiledata)[i].lpReplacements) delete[] (*tiledata)[i].lpReplacements;
		(*tiledata)[i].lpReplacements=NULL;
	}
	if(*tiledata) delete[] (*tiledata);
	(*tiledata)=NULL;
	(*tiledata_count)=0;
}

void CLoading::FreeAll()
{
	last_succeeded_operation=14;

	// MW fix: We need to set tiledata and tiledata_count to the old pointers again
	int t=0;
	if(tiledata==&t_tiledata) t=0;
	if(tiledata==&s_tiledata) t=1;
	if(tiledata==&u_tiledata) t=2;
	if(tiledata==&un_tiledata) t=3;
	if(tiledata==&l_tiledata) t=4;
	if(tiledata==&d_tiledata) t=4;

	//try{
	tiledata=&t_tiledata;
	tiledata_count=&t_tiledata_count;
	FreeTileSet();
	tiledata=&s_tiledata;
	tiledata_count=&s_tiledata_count;
	FreeTileSet();
	tiledata=&u_tiledata;
	tiledata_count=&u_tiledata_count;
	FreeTileSet();

	// MW New tilesets:
	tiledata=&un_tiledata;
	tiledata_count=&un_tiledata_count;
	FreeTileSet();
	tiledata=&l_tiledata;
	tiledata_count=&l_tiledata_count;
	FreeTileSet();
	tiledata=&d_tiledata;
	tiledata_count=&d_tiledata_count;
	FreeTileSet();
	/* }
	catch(...)
	{
		errstream << "Exception in FreeTileSet()" << endl;
	}*/

	// MW Reset tiledata & tiledata_count
	if(t==0) { tiledata=&t_tiledata; tiledata_count=&t_tiledata_count; }
	if(t==1) { tiledata=&s_tiledata; tiledata_count=&s_tiledata_count; }
	if(t==2) { tiledata=&u_tiledata; tiledata_count=&u_tiledata_count; }
	if(t==3) { tiledata=&un_tiledata; tiledata_count=&un_tiledata_count; }
	if(t==4) { tiledata=&l_tiledata; tiledata_count=&l_tiledata_count; }
	if(t==5) { tiledata=&d_tiledata; tiledata_count=&d_tiledata_count; }

	map<CString, PICDATA>::iterator i=pics.begin();
	for (int e=0;e<pics.size();e++)
	{
		try{
#ifdef NOSURFACES_OBJECTS			
			if(i->second.bType==PICDATA_TYPE_BMP)
			{
				if(i->second.pic!=NULL) 
				{
					((LPDIRECTDRAWSURFACE4)i->second.pic)->Release();
				}
			}
			else
			{
				if(i->second.pic!=NULL) 
				{
					delete[] i->second.pic;
				}
				if(i->second.vborder) delete[] i->second.vborder;
			}
#else
			if(i->second.pic!=NULL) i->second.pic->Release();
#endif

			i->second.pic=NULL;
		}
		catch(...)
		{
			CString err;
			err="Access violation while trying to release surface ";
			char c[6];
			itoa(e,c,10);
			err+=c;
				
			err+="\n";
			OutputDebugString(err);	
			continue;
		}
			
		i++;
	}



	try{
	CFinalSunDlg* dlg=((CFinalSunDlg*)theApp.m_pMainWnd);
	if(dlg->m_view.m_isoview->lpds!=NULL) dlg->m_view.m_isoview->lpds->Release();
	dlg->m_view.m_isoview->lpds=NULL;
	if(dlg->m_view.m_isoview->lpdsBack!=NULL) dlg->m_view.m_isoview->lpdsBack->Release();
	dlg->m_view.m_isoview->lpdsBack=NULL;
	if (dlg->m_view.m_isoview->lpdsBackHighRes != NULL) dlg->m_view.m_isoview->lpdsBackHighRes->Release();
	dlg->m_view.m_isoview->lpdsBackHighRes = NULL;
	if(dlg->m_view.m_isoview->lpdsTemp!=NULL) dlg->m_view.m_isoview->lpdsTemp->Release();
	dlg->m_view.m_isoview->lpdsTemp=NULL;
	if(dlg->m_view.m_isoview->dd!=NULL) dlg->m_view.m_isoview->dd->Release();
	dlg->m_view.m_isoview->dd=NULL;
	if(dlg->m_view.m_isoview->dd_1!=NULL) dlg->m_view.m_isoview->dd_1->Release();
	dlg->m_view.m_isoview->dd_1=NULL;
	}
	catch(...)
	{
		errstream << "Exception while freeing DirectDraw" << endl;	
	}
} 

void CLoading::PostNcDestroy() 
{

//	delete this; // on stack!
//	CDialog::PostNcDestroy();
}

void CLoading::PrepareHouses()
{
	int i;
	int p=0 ;
	for(i=0;i<CIniFile::Rules.sections["Sides"].values.size();i++)
	{
		int t=0;
		while(GetParam(*CIniFile::Rules.sections["Sides"].GetValue(i), t).GetLength()>0)
		{
			sides[p].name=GetParam(*CIniFile::Rules.sections["Sides"].GetValue(i), t);
			sides[p].orig_n=CIniFile::Rules.sections["Sides"].GetValueOrigPos(i); // mw fix instead of =i
			t++;
			p++;
		}
	}

	for(i=0;i<CIniFile::Rules.sections[HOUSES].values.size();i++)
	{
		if(CIniFile::Rules.sections[HOUSES].GetValueOrigPos(i)<0) continue;

		HOUSEINFO& house=houses[CIniFile::Rules.sections[HOUSES].GetValueOrigPos(i)];
		house.name=*CIniFile::Rules.sections[HOUSES].GetValue(i);
		house.bPlayable=isTrue(CIniFile::Rules.sections[house.name].values["Multiplay"]);
		memset(&house.color, 0, sizeof(RGBTRIPLE));
		int e;
		for(e=0;e<sides.size();e++)
		{
			house.side=NULL;
			if(sides[e].name==CIniFile::Rules.sections[house.name].values["Side"])
				house.side=&sides[e];
		}
		
	}
}


BYTE* Search(BYTE** lpData, BYTE* lpSearched)
{
	BYTE* lpDat=*lpData;
	
	lpDat=(BYTE*)strstr((LPCSTR)lpDat, (LPCSTR)lpSearched)+strlen((LPCSTR)lpSearched);

	return lpDat;
}

class SortDummy2{
public:
	bool operator() (const CString& x, const CString& y) const
	{
		// the length is more important than spelling (numbers!!!)...
		if(x.GetLength()<y.GetLength()) return true;
		if(x.GetLength()==y.GetLength())
		{	
			CString x2=x;
			CString y2=y;
			x2.MakeLower();
			y2.MakeLower();
			if(x2<y2) return true;
		}

		return false;
	
	}
};

extern map<CString, XCString> AllStrings;
void CLoading::LoadStrings()
{
	map<CString, XCString, SortDummy2> strings;

	auto parse_csf = [&](char* buffer)
	{
		char* pos = buffer;

		auto read_int = [&pos](const void* dest)
		{
			memcpy((void*)dest, pos, 4);
			pos += 4;
		};

		// Parse CSF header
		if (std::memcmp(pos, " FSC", 0x4) != 0)
			return false;

		pos += 4; // FSC
		pos += 4; // version
		int _numLabels;
		read_int(&_numLabels);
		pos += 4; // numstrings
		pos += 4; // useless
		pos += 4; // lang
		// Read CSF labels
		for (int i = 0; i < _numLabels; ++i)
		{
			// Read CSF label header
			int identifier;
			read_int(&identifier);
			if (identifier == 'LBL ')
			{
				int numPairs;
				read_int(&numPairs);
				int strLength;
				read_int(&strLength);

				char* labelstr = new char[strLength + 1];
				labelstr[strLength] = '\0';
				memcpy_s(labelstr, strLength, pos, strLength);

				pos += strLength;
				// CSF labels are not case sensitive.
				for (int k = 0; k < strLength; ++k)
					labelstr[k] = tolower(labelstr[k]);

				read_int(&identifier);
				read_int(&strLength);

				char* tmpWideBuffer = new char[(strLength << 1) + 2];
				for (int i = 0; i < strLength << 1; ++i)
					tmpWideBuffer[i] = ~pos[i];
				wchar_t* ptrWideBuffer = reinterpret_cast<wchar_t*>(tmpWideBuffer);
				ptrWideBuffer[strLength] = '\0';

				char* value = nullptr;
				int valueBufferSize = WideCharToMultiByte(CP_ACP, NULL, ptrWideBuffer, strLength, value, 0, NULL, NULL) + 1;
				value = new char[valueBufferSize];
				WideCharToMultiByte(CP_ACP, NULL, ptrWideBuffer, strLength, value, valueBufferSize, NULL, NULL);
				delete[] tmpWideBuffer;
				value[valueBufferSize - 1] = '\0';

				pos += (strLength << 1);
				if (identifier == 'RTSW') // "WSTR"
				{
					read_int(&strLength);
					pos += strLength;
				}

				strings[labelstr].SetString(value);
				AllStrings[labelstr].SetString(value);

				delete[] labelstr;
				delete[] value;

				for (int j = 1; j < numPairs; ++j) // Extra labels will be ignored here
				{
					read_int(&identifier);
					read_int(&strLength);
					pos += (strLength << 1);
					if (identifier == 0x53545257) // "WSTR"
					{
						read_int(&strLength);
						pos += strLength;
					}
				}
			}
			else {
				break;
			}
		}
		return true;
	};

	auto load_csf = [&](LPCSTR filename)
	{
		auto lpData = static_cast<BYTE*>(CCFileClass::ReadWholeFile(filename));
		if (nullptr == lpData)
			return;

		std::println(errstream, "Loading CSF file: {0}... ", filename);
		parse_csf(reinterpret_cast<char*>(lpData));

		delete[] lpData;
	};
	
	load_csf(CIniFile::FAData.GetString("Filenames", "CSF", "ra2md.csf"));
	for (int i = 1; i <= 99; ++i)
	{
		CString filename;
		filename.Format("stringtable%02d.csf", i);
		load_csf(filename);
	}

	for (int i = 0; i < CIniFile::Rules.sections.size(); i++)
	{
		if (CIniFile::Rules.GetSection(i)->FindName("UIName") >= 0)
		{
			if (strings.find(CIniFile::Rules.GetSection(i)->values["UIName"]) != strings.end())
			{
				if (!strings[CIniFile::Rules.GetSection(i)->values["UIName"]].bUsedDefault)
					CCStrings[*CIniFile::Rules.GetSectionName(i)].SetString(strings[CIniFile::Rules.GetSection(i)->values["UIName"]].wString, strings[CIniFile::Rules.GetSection(i)->values["UIName"]].len);
				else
				{
					CCStrings[*CIniFile::Rules.GetSectionName(i)].SetString(strings[CIniFile::Rules.GetSection(i)->values["UIName"]].wString, strings[CIniFile::Rules.GetSection(i)->values["UIName"]].len);
					CCStrings[*CIniFile::Rules.GetSectionName(i)].cString = CIniFile::Rules.GetSection(i)->GetString("Name");
				}
			}
			else
				CCStrings[*CIniFile::Rules.GetSectionName(i)].SetString((LPSTR)(LPCSTR)CIniFile::Rules.GetSection(i)->GetString("Name"));
		}
		else CCStrings[*CIniFile::Rules.GetSectionName(i)].SetString((LPSTR)(LPCSTR)CIniFile::Rules.GetSection(i)->GetString("Name"));
	}
	
	std::println(errstream, "Loaded {0} {1} strings.", AllStrings.size(), CCStrings.size());
}

void CLoading::HackRules()
{
	int i;
	int max_c = 0;
	for (i = 0; i < CIniFile::Rules.sections["BuildingTypes"].values.size(); i++)
	{
		int p = atoi(*CIniFile::Rules.sections["BuildingTypes"].GetValueName(i));
		if (p > max_c) max_c = p;
	}

	char c[50];
	itoa(max_c + 1, c, 10);

	CIniFile::Rules.sections["BuildingTypes"].values[c] = CIniFile::Rules.sections["General"].values["GDIGateOne"];

	// RULES(MD).INI has the incorrect colors set for the following houses, let's remap them to the expected values.
	// Fixup YuriCountry colour
	if (CIniFile::Rules.sections["YuriCountry"].GetString("Color") == "DarkRed") {
		CIniFile::Rules.sections["YuriCountry"].values["Color"] = "Purple";
	}
	// Fixup Allied colors
	std::list<CString> allied_houses;
	allied_houses.push_back("British");
	allied_houses.push_back("French");
	allied_houses.push_back("Germans");
	allied_houses.push_back("Americans");
	allied_houses.push_back("Alliance");
	for (std::list<CString>::iterator it = allied_houses.begin(); it != allied_houses.end(); ++it) {
		if (CIniFile::Rules.sections[*it].GetString("Color") == "Gold") {
			CIniFile::Rules.sections[*it].values["Color"] = "DarkBlue";
		}
	}
	// Fixup Nod color
	if (CIniFile::Rules.sections["Nod"].GetString("Color") == "Gold") {
		CIniFile::Rules.sections["Nod"].values["Color"] = "DarkRed";
	}
}

void CLoading::PrepareBuildingTheaters()
{
	// stub

}
