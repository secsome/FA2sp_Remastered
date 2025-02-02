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

#define _HAS_STD_BYTE 0  // see https://developercommunity.visualstudio.com/t/error-c2872-byte-ambiguous-symbol/93889
struct IUnknown;

#include "pack/MissionEditorPackLib.h"
#include <stdexcept>

#define MYASSERT(a, b)             \
  if (!a) {                        \
    FSunPackLib::FSPL_EXCEPTION e; \
    e.err_code = b;                \
    throw(e);                      \
  }

namespace FSunPackLib {
std::wstring utf8ToUtf16(const std::string& utf8) {
  // wstring_convert and codecvt_utf8_utf16 are deprecated in C++17, fallback to Win32
  if (utf8.size() == 0)
    // MultiByteToWideChar does not support passing in cbMultiByte == 0
    return L"";

  // unterminatedCountWChars will be the count of WChars NOT including the terminating zero (due to passing in
  // utf8.size() instead of -1)
  auto utf8Count = utf8.size();
  auto unterminatedCountWChars = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, utf8.data(),
                                                     static_cast<int>(utf8Count), nullptr, 0);
  if (unterminatedCountWChars == 0) {
    throw std::runtime_error("UTF8 -> UTF16 conversion failed");
  }

  std::wstring utf16;
  utf16.resize(unterminatedCountWChars);
  if (MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, utf8.data(), static_cast<int>(utf8Count),
                          utf16.data(), unterminatedCountWChars) == 0) {
    throw std::runtime_error("UTF8 -> UTF16 conversion failed");
  }
  return utf16;
}

// XCC uses CreateFileA to load files which does not support UTF8 filenames if you don't opt in through the app manifest
// (and that's only possible on Win10 1903+) so provide some helpers that open the file through Cwin_handle with
// CreateFileW instead:

template <class F>
int open_write(F& file, const std::string& u8FilePath) {
  return 0;
}

template <class F>
int open_read(F& file, const std::string& u8FilePath) {
  return 0;
}

extern "C" bool _DEBUG_EnableLogs = 0;         // only useable in debug library builds
extern "C" bool _DEBUG_EnableBreakpoints = 0;  // only useable in debug library builds

extern "C" int last_succeeded_operation = 0;

BYTE* EncodeBase64(BYTE* sp, UINT len) {
  // TODO
  return nullptr;
}

// dest should be at least as large as sp
INT ConvertToF80(BYTE* sp, UINT len, BYTE* dest) {
  // TODO
  return 0;
}

INT EncodeF80(BYTE* sp, UINT len, UINT nSections, BYTE** dest) {
  // TODO

  return 0;
}

UINT EncodeIsoMapPack5(BYTE* sp, UINT SourceLength, BYTE** dp) {
  // TODO

  return 0;
}

bool DecodeF80(const BYTE* const sp, const UINT SourceLength, std::vector<BYTE>& dp, const std::size_t max_size) {
  // TODO

  return true;
}

int DecodeBase64(const char* sp, std::vector<BYTE>& dest) {
  // TODO
  return 0;
}

UINT DecodeIsoMapPack5(BYTE* sp, UINT SourceLength, BYTE* dp, HWND hProgressBar, BOOL bDebugMode) {
  // TODO
  return 0;
}

std::int32_t GetFirstPixelColor(IDirectDrawSurface4* pDDS) {
  std::int32_t color = 0;

  DDSURFACEDESC2 desc = {0};
  desc.dwSize = sizeof(DDSURFACEDESC2);

  if (pDDS->Lock(nullptr, &desc, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT | DDLOCK_NOSYSLOCK, nullptr) != DD_OK)
    return color;
  if (desc.lpSurface == nullptr) {
    pDDS->Unlock(nullptr);
    return color;
  } else {
    auto bytes_per_pixel = (desc.ddpfPixelFormat.dwRGBBitCount + 7) / 8;
    memcpy(&color, desc.lpSurface, bytes_per_pixel > 4 ? 4 : bytes_per_pixel);
    pDDS->Unlock(nullptr);

    return color;
  }
}

HRESULT SetColorKey(IDirectDrawSurface4* pDDS, COLORREF rgb) {
  DDPIXELFORMAT pf = {0};
  pf.dwSize = sizeof(DDPIXELFORMAT);
  pDDS->GetPixelFormat(&pf);

  ColorConverter c(pf);
  auto col = rgb == CLR_INVALID ? GetFirstPixelColor(pDDS) : c.GetColor(rgb);
  DDCOLORKEY color_key = {static_cast<DWORD>(col), static_cast<DWORD>(col)};
  return pDDS->SetColorKey(DDCKEY_SRCBLT, &color_key);
}

BOOL XCC_Initialize(BOOL bLoadFromRegistry) { return TRUE; }

HMIXFILE XCC_OpenMix(LPCTSTR szMixFile, HMIXFILE hOwner) {
  // TODO

  return NULL;
}

BOOL XCC_GetMixName(HMIXFILE hOwner, std::string& sMixFile) {
  // TODO

  return TRUE;
}

BOOL XCC_DoesFileExist(LPCSTR szFile, HMIXFILE hOwner) {
  // TODO

  return TRUE;
}

BOOL XCC_CloseMix(HMIXFILE hMixFile) {
  // TODO

  return TRUE;
}

BOOL XCC_ExtractFile(const std::string& szFilename, const std::string& szSaveTo, HMIXFILE hOwner) {
  // TODO
  return TRUE;
}

BOOL XCC_ExtractFile(LPCSTR szFilename, LPCSTR szSaveTo, HMIXFILE hOwner) {
  return XCC_ExtractFile(std::string(szFilename), std::string(szSaveTo), hOwner);
}

BOOL XCC_GetSHPHeader(SHPHEADER* pHeader) {
  // TODO

  return TRUE;
}

/*
Returns the SHP image header of a image in a SHP file
*/
BOOL XCC_GetSHPImageHeader(int iImageIndex, SHPIMAGEHEADER* pImageHeader) {
  // TODO

  return TRUE;
}

BOOL SetCurrentTMP(LPCSTR szTMP, HMIXFILE hOwner) {
  // TODO

  return TRUE;
};

BOOL SetCurrentSHP(LPCSTR szSHP, HMIXFILE hOwner) {
  // TODO

  return TRUE;
};

BOOL XCC_GetTMPTileInfo(int iTile, POINT* lpPos, int* lpWidth, int* lpHeight, BYTE* lpDirection, BYTE* lpTileHeight,
                        BYTE* lpTileType, RGBTRIPLE* lpRgbLeft, RGBTRIPLE* lpRgbRight) {
  // TODO
  return TRUE;
}

BOOL XCC_GetTMPInfo(RECT* lpRect, int* iTileCount, int* iTilesX, int* iTilesY) {
  // TODO
  return TRUE;
}

BOOL LoadSHPImage(int iImageIndex, std::vector<BYTE>& pic) {
  // TODO
  return TRUE;
}

BOOL LoadSHPImage(int iImageIndex, int iCount, BYTE** lpPics) {
  // TODO
  return TRUE;
}

BOOL LoadTMPImageInSurface(IDirectDraw4* pdd, int iStart, int iCount, LPDIRECTDRAWSURFACE4* pdds, HTSPALETTE hPalette) {
  // TODO

  return TRUE;
}

BOOL LoadTMPImage(int iStart, int iCount, BYTE** lpTileArray) {
  // TODO
  return TRUE;
}

const double pi = 3.141592654;

template <class T>
__forceinline void rotate_x(Vec3<T>& v, T a) {
  T l = sqrt(v.y() * v.y() + v.z() * v.z());
  T d_a = atan2(v.y(), v.z()) + a;
  v[1] = l * sin(d_a);
  v[2] = l * cos(d_a);
}

template <class T>
__forceinline void rotate_y(Vec3<T>& v, T a) {
  T l = sqrt(v.x() * v.x() + v.z() * v.z());
  T d_a = atan2(v.x(), v.z()) + a;
  v[0] = l * sin(d_a);
  v[2] = l * cos(d_a);
}

template <class T>
__forceinline void rotate_z(Vec3<T>& v, T a) {
  T l = sqrt(v.x() * v.x() + v.y() * v.y());
  T d_a = atan2(v.x(), v.y()) + a;
  v[0] = l * sin(d_a);
  v[1] = l * cos(d_a);
  ;
}

template <class T>
__forceinline void rotate_zxy(Vec3<T>& v, const Vec3<T>& r) {
  rotate_z(v, r.z());
  rotate_x(v, r.x());
  rotate_y(v, r.y());
}

BOOL SetCurrentVXL(LPCSTR lpVXLFile, HMIXFILE hMixFile) {
  // TODO
  return TRUE;
}

BOOL GetVXLInfo(int* cSections) {
  // TODO
  return TRUE;
}

BOOL GetVXLSectionInfo(int section, VoxelNormalClass& normalClass) {
  // TODO
  return TRUE;
}

void GetVXLSectionBounds(int iSection, const Vec3f& rotation, const Vec3f& modelOffset, Vec3f& minVec, Vec3f& maxVec) {}

BOOL LoadVXLImageInSurface(const VoxelNormalTables& normalTables, Vec3f lightDirection, IDirectDraw4* pdd, int iStart,
                           int iCount, const Vec3f rotation, const Vec3f postHVAOffset, LPDIRECTDRAWSURFACE4* pdds,
                           HTSPALETTE hPalette, int* lpXCenter, int* lpYCenter, int ZAdjust, int* lpXCenterZMax,
                           int* lpYCenterZMax, int i3dCenterX, int i3dCenterY) {
  // TODO
  return TRUE;
}

BOOL LoadVXLImage(const VoxelNormalTables& normalTables, Vec3f lightDirection, const Vec3f rotation,
                  const Vec3f modelOffset, std::vector<BYTE>& image, std::vector<BYTE>& lighting, int* lpXCenter,
                  int* lpYCenter, int ZAdjust, int* lpXCenterZMax, int* lpYCenterZMax, int i3dCenterX, int i3dCenterY,
                  RECT* vxlrect) {
  // TODO
  return TRUE;
}

HTSPALETTE LoadTSPalette(const std::string& szPalette, HMIXFILE hPaletteOwner) {
  // TODO
  return NULL;
}

HTSPALETTE LoadTSPalette(LPCSTR szPalette, HMIXFILE hPaletteOwner) {
  return LoadTSPalette(std::string(szPalette), hPaletteOwner);
}

BOOL SetTSPaletteEntry(HTSPALETTE hPalette, BYTE bIndex, RGBTRIPLE* rgb, RGBTRIPLE* orig) {
  // TODO
  return TRUE;
}

BOOL WriteMixFile(LPCTSTR lpMixFile, LPCSTR* lpFiles, DWORD dwFileCount, Game game) {
  // TODO
  return FALSE;
}

class ColorConverterImpl {
 public:
  ColorConverterImpl(const DDPIXELFORMAT& pf) {}

  int GetColor(int r, int g, int b) const { return 0; }

  int GetColor(int a, int r, int g, int b) const { return 0; }

 private:
};

ColorConverter::ColorConverter(const DDPIXELFORMAT& pf) : m_impl(new ColorConverterImpl(pf)) {}

int ColorConverter::GetColor(int r, int g, int b) const { return m_impl->GetColor(r, g, b); }

int ColorConverter::GetColor(int a, int r, int g, int b) const { return m_impl->GetColor(a, r, g, b); }

int ColorConverter::GetColor(COLORREF col) const {
  return GetColor((LOBYTE((col) >> 24)), GetRValue(col), GetGValue(col), GetBValue(col));
}

};  // namespace FSunPackLib