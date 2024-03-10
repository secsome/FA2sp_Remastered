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

#ifndef FSUNPACKLIB_INCLUDED
#define FSUNPACKLIB_INCLUDED

#include <windows.h>
#include <ddraw.h>
#include <memory>
#include <vector>
#include <string>
#include "Vec3.h"

class VoxelNormalTables;

typedef DWORD HMIXFILE;
typedef DWORD HTSPALETTE;

struct SHPHEADER
{
	__int16 zero;
	__int16 cx;
	__int16 cy;
	__int16 c_images;
};

struct SHPIMAGEHEADER
{
	__int16 x;
	__int16 y;
	__int16 cx;
	__int16 cy;
	__int32 compression;
	__int32 unknown;
	__int32 zero;
	__int32 offset;
};

namespace FSunPackLib
{
	enum Game
	{
		RA2,
		RA2_YR,
		TS,
		TS_FS
	};

	enum class VoxelNormalClass: std::uint8_t
	{
		Unknown = 0,
		Gen1 = 1,
		TS = 2,
		Gen3 = 3,
		RA2 = 4,		
	};

	extern "C" extern bool _DEBUG_EnableLogs; // only useable in debug library builds
	extern "C" extern bool _DEBUG_EnableBreakpoints; // only useable in debug library builds

	struct FSPL_EXCEPTION
	{
		int err_code;
	};

	extern "C" extern int last_succeeded_operation;

	class ColorConverterImpl;
	class ColorConverter
	{
	public:
		ColorConverter(const DDPIXELFORMAT& pf);
		int GetColor(int a, int r, int g, int b) const;
		int GetColor(int r, int g, int b) const;
		int GetColor(COLORREF col) const;

	private:
		std::shared_ptr<ColorConverterImpl> m_impl;
	};

	bool XCC_Initialize(bool bLoadFromRegistry);

	HMIXFILE XCC_FindFileInMix(LPCSTR lpFilename);

	HMIXFILE XCC_OpenMix(LPCTSTR szMixFile, HMIXFILE hOwner);

	BOOL XCC_GetMixName(HMIXFILE hOwner, std::string& sMixFile);


	BOOL XCC_DoesFileExist(LPCSTR szFile, HMIXFILE hOwner);


	BOOL XCC_CloseMix(HMIXFILE hMixFile);

	BOOL XCC_ExtractFile(const std::string& szFilename, const std::string& szSaveTo, HMIXFILE hOwner);
	BOOL XCC_ExtractFile(LPCSTR szFilename, LPCSTR szSaveTo, HMIXFILE hOwner);
	
	void* XCC_ReadWholeFile(LPCSTR lpFilename, HMIXFILE hOwner, size_t* pSize = nullptr);

	bool XCC_LoadPalette(const void* data, HTSPALETTE& hPal);

	BOOL XCC_GetSHPHeader(SHPHEADER* pHeader);


/*
Returns the SHP image header of a image in a SHP file
*/
	BOOL XCC_GetSHPImageHeader(int iImageIndex, SHPIMAGEHEADER* pImageHeader);



	BOOL SetCurrentTMP(LPCSTR szTMP, HMIXFILE hOwner);


	BOOL SetCurrentSHP(LPCSTR szSHP, HMIXFILE hOwner);


	BOOL XCC_GetTMPTileInfo(int iTile, POINT* lpPos, int* lpWidth, int* lpHeight, BYTE* lpDirection, BYTE* lpTileHeight, BYTE* lpTileType, RGBTRIPLE* lpRgbLeft, RGBTRIPLE* lpRgbRight);


	BOOL XCC_GetTMPInfo(RECT* lpRect, int* iTileCount, int* iTilesX, int* iTilesY);



	BOOL LoadTMPImageInSurface(IDirectDraw4* pdd, int iStart, int iCount, LPDIRECTDRAWSURFACE4* pdds, HTSPALETTE hPalette);
	BOOL LoadTMPImage(int iStart, int iCount, BYTE** lpTileArray);

	BOOL LoadSHPImageInSurface(IDirectDraw4* pdd, HTSPALETTE hPalette, int iImageIndex, int iCount, LPDIRECTDRAWSURFACE4* pdds);
	BOOL LoadSHPImage(int iImageIndex, int iCount, BYTE** lpPics);
	BOOL LoadSHPImage(int iImageIndex, std::vector<BYTE>& pic);

	HTSPALETTE LoadTSPalette(LPCSTR szPalette, HMIXFILE hPaletteOwner);
	HTSPALETTE LoadTSPalette(const std::string& szPalette, HMIXFILE hPaletteOwner);


	BOOL SetTSPaletteEntry(HTSPALETTE hPalette, BYTE bIndex, RGBTRIPLE* rgb, RGBTRIPLE* orig);


	BOOL SetCurrentVXL(LPCSTR lpVXLFile, HMIXFILE hMixFile);


	BOOL GetVXLInfo(int* cSections);

	BOOL GetVXLSectionInfo(int section, VoxelNormalClass& normalClass);

	BOOL LoadVXLImageInSurface(const VoxelNormalTables& normalTables, Vec3f lightDirection, IDirectDraw4* pdd, int iStart, int iCount, Vec3f rotation, Vec3f modelOffset, LPDIRECTDRAWSURFACE4* pdds, HTSPALETTE hPalette, int* lpXCenter = NULL, int* lpYCenter = NULL, int ZAdjust = 0, int* lpXCenterZMax = NULL, int* lpYCenterZMax = NULL, int i3dCenterX = -1, int i3dCenterY = -1);

	// modelOffset is applied before VXL/HVA translates and scales and before model-to-world rotation
	BOOL LoadVXLImage(const VoxelNormalTables& normalTables, Vec3f lightDirection, Vec3f rotation, Vec3f modelOffset, std::vector<BYTE>& image, std::vector<BYTE>& lighting, int* lpXCenter = NULL, int* lpYCenter = NULL, int ZAdjust = 0, int* lpXCenterZMax = NULL, int* lpYCenterZMax = NULL, int i3dCenterX = -1, int i3dCenterY = -1, RECT* vxlrect = NULL);


	BOOL WriteMixFile(LPCTSTR lpMixFile, LPCSTR* lpFiles, DWORD dwFileCount, Game game);

	HRESULT SetColorKey(IDirectDrawSurface4* pDDS, COLORREF rgb);

};

#endif