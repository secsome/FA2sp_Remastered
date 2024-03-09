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

#pragma once

#include <afxcview.h>
#include <array>
#include <set>

class CViewObjects : public CTreeView
{
protected:
	CViewObjects();
	DECLARE_DYNCREATE(CViewObjects)

public:
	void UpdateDialog();

public:
	virtual void OnInitialUpdate();
protected:
	virtual void OnDraw(CDC* pDC);

protected:
	virtual ~CViewObjects();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnKeydown(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	DECLARE_MESSAGE_MAP()
private:
	void HandleBrushSize(int iTile);


private:
    enum {
        Root_Nothing = 0, Root_Ground, Root_Owner, Root_Infantry, Root_Vehicle,
        Root_Aircraft, Root_Building, Root_Terrain, Root_Smudge, Root_Overlay,
        Root_Waypoint, Root_Celltag, Root_Basenode, Root_Tunnel, Root_PlayerLocation,
        Root_PropertyBrush, Root_Delete, Root_Count
    };

    enum {
        Set_Building = 0, Set_Infantry, Set_Vehicle, Set_Aircraft, Set_Count
    };

    enum
    {
        Const_Infantry = 10000, Const_Building = 20000, Const_Aircraft = 30000,
        Const_Vehicle = 40000, Const_Terrain = 50000, Const_Overlay = 63000,
        Const_House = 70000, Const_Smudge = 80000, Const_PropertyBrush = 90000
    };

    std::array<HTREEITEM, Root_Count> ExtNodes;
    std::set<CString> IgnoreSet;
    std::set<CString> ForceName;
    std::set<CString> ExtSets[Set_Count];
    std::map<CString, int> KnownItem;
    std::map<CString, int> Owners;

    HTREEITEM InsertString(const char* pString, DWORD dwItemData = 0,
        HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST);
    HTREEITEM InsertTranslatedString(const char* pOriginString, DWORD dwItemData = 0,
        HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST);

    void Redraw_Initialize();
    void Redraw_MainList();
    void Redraw_Ground();
    void Redraw_Owner();
    void Redraw_Infantry();
    void Redraw_Vehicle();
    void Redraw_Aircraft();
    void Redraw_Building();
    void Redraw_Terrain();
    void Redraw_Smudge();
    void Redraw_Overlay();
    void Redraw_Waypoint();
    void Redraw_Celltag();
    void Redraw_Basenode();
    void Redraw_Tunnel();
    void Redraw_PlayerLocation();

    bool IsIgnored(const char* pItem);
    CString QueryUIName(const char* pRegName, bool bOnlyOneLine = true);

    int GuessType(const char* pRegName);
    int GuessSide(const char* pRegName, int nType);
    int GuessBuildingSide(const char* pRegName);
    int GuessGenericSide(const char* pRegName, int nType);
public:
    
};
