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

#if !defined(AFX_ISOVIEW_H__7FB6D6A0_7B52_11D3_99E1_DA6DFD21E706__INCLUDED_)
#define AFX_ISOVIEW_H__7FB6D6A0_7B52_11D3_99E1_DA6DFD21E706__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// IsoView.h : header file
//



/////////////////////////////////////////////////////////////////////////////
// View CIsoView 

#include <vector>
#include <memory>
#include "Structs.h"

#include "GameScene.h"

class CTube;


class CIsoView : public CView
{
protected:
	DECLARE_DYNCREATE(CIsoView)

private:
	GameScene Scene;
public:
	void InitializeGameScene();

// attributes
public:
	INT m_mapx;
	INT m_mapy;
	int m_FlattenHeight; // for flatten ground
	int m_FlattenLastX;
	int m_FlattenLastY;
	int m_TileChangeCount;
	BOOL bThreadPainting;

private:
	ProjectedVec m_viewOffset;
	Vec2<CSProjected, float> m_viewScale;  // this is used for display and may e.g. be locked to zoom steps
	float m_viewScaleControl;  // you control these with your mouse
	CPoint m_MButtonDown;
	CPoint m_MButtonMoveZooming;

	int rclick_x; // scroll pixel coordinates
	int rclick_y;
	int cur_x_mouse;
	int cur_y_mouse;
	BOOL rscroll;
	BOOL m_bAltCliff;
	bool m_zooming;

// operations
public:
	CIsoView();           
	virtual ~CIsoView();
public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
protected:
	virtual void OnDraw(CDC* pDC);
	virtual void OnInitialUpdate();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

// implementation
protected:
	
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// generated message maps
	//{{AFX_MSG(CIsoView)
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);	
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnDeadChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	// Zoom
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	void Zoom(CPoint& pt, float f);


	void FocusWaypoint(int index);
	RECT m_funcRect;
	Vec2<CSProjected, float> GetViewScale() const
	{
		return m_viewScale;
	}
	ProjectedVec GetViewOffset() const
	{
		return m_viewOffset;
	}
	RECT GetScaledDisplayRect() const;
	void GetScroll(int& xscroll, int& yscroll) const;
	void SetScroll(int xscroll, int yscroll);
	void DrawMap();
	void AutoLevel();
	void FillArea(DWORD dwX, DWORD dwY, DWORD dwID, BYTE bSubTile);
	BOOL m_NoMove;
	void PlaceCurrentObjectAt(int x, int y);
	void PlaceTile(const int x, const int y, const UINT nMouseFlags);
	void ShowAllTileSets();
	void HideTileSet(DWORD dwTileSet);
	int m_BrushSize_x;
	int m_BrushSize_y;
	BOOL ReachableFrom(DWORD dwStart, DWORD dwEnd);
	void __fastcall ChangeTileHeight(DWORD dwPos, DWORD dwNewHeight, BOOL bNonMorphableMove, BOOL bOnlyThisTile=FALSE, BOOL bNoSlopes=FALSE);
	void UpdateOverlayPictures(int id=-1);
	void UpdateStatusBar(int x, int y);
	/// <summary>
	/// Converts from (world) pixel coordinates to logical map coordinates
	/// </summary>
	/// <param name="projCoords">World pixel coordinates</param>
	/// <returns>Logical map coordinates</returns>
	MapCoords GetMapCoordinates(const ProjectedCoords& projCoords, bool bAllowAccessBehindCliffs=false, bool ignoreHideFlags = false) const;
	
	/// <summary>
	/// Converts from view / render target pixel coordinates (0/0 is top left corner of screen) to logical map coordinates 
	/// If you use mouse-move coordinates you need to 
	/// </summary>
	/// <param name="projCoords">Texel coordinates of the backbuffer render target</param>
	/// <returns>Logical map coordinates</returns>
	MapCoords GetMapCoordinatesFromRenderTargetCoordinates(const ProjectedCoords& screenViewCoords, bool bAllowAccessBehindCliffs = false, bool ignoreHideFlags = false) const;

	/// <summary>
	/// Converts from Win32 window client coordinates (0/0 at the top left corner of the view window) to logical map coordinates
	/// </summary>
	/// <param name="clientPt">Client coordinates as given e.g. by window messages</param>
	/// <returns>Logical map coordinates</returns>
	MapCoords GetMapCoordinatesFromClientCoordinates(const CPoint& clientPt, bool bAllowAccessBehindCliffs = false, bool ignoreHideFlags = false) const;

	/// <summary>
	/// Converts from Win32 window client coordinates (0/0 at the top left corner of the view window) to (world) pixel coordinates
	/// </summary>
	/// <param name="clientPt">Client coordinates as given e.g. by window messages</param>
	/// <returns>World pixel coordinates</returns>
	ProjectedCoords GetProjectedCoordinatesFromClientCoordinates(const CPoint& clientPt) const;

	/// <summary>
	/// Converts from logical map coordinates to (world) pixel coordinates
	/// </summary>
	/// <param name="mapCoords">Logical map coordinates</param>
	/// <returns>World pixel coordinates</returns>
	ProjectedCoords GetProjectedCoordinates(const MapCoords& mapCoords) const;

	/// <summary>
	/// Converts from logical map coordinates to (world) pixel coordinates
	/// </summary>
	/// <param name="mapCoords">Logical map coordinates</param>
	/// <param name="mapZ">Fixed logical height</param>
	/// <returns>World pixel coordinates</returns>
	ProjectedCoords GetProjectedCoordinates(const MapCoords& mapCoords, int mapZ) const;

	/// <summary>
	/// Converts from logical map coordinates to view / render target pixel coordinates (screen backbuffer, e.g. currently not yet scaled by view zoom)
	/// </summary>
	/// <param name="mapCoords">Logical map coordinates</param>
	/// <returns>Texel coordinates of the screen backbuffer</returns>
	ProjectedCoords GetRenderTargetCoordinates(const MapCoords& mapCoords) const;

	/// <summary>
	/// Converts from logical map coordinates to view / render target pixel coordinates (screen backbuffer, e.g. currently not yet scaled by view zoom)
	/// </summary>
	/// <param name="mapCoords">Logical map coordinates</param>
	/// <param name="mapZ">Fixed logical height</param>
	/// <returns>Texel coordinates of the screen backbuffer</returns>
	ProjectedCoords GetRenderTargetCoordinates(const MapCoords& mapCoords, int mapZ) const;

	/// <summary>
	/// Converts from logical map coordinates to Win32 window client coordinates (0/0 at the top left corner of the view window)
	/// </summary>
	/// <param name="projCoords">Logical map coordinates</param>
	/// <returns>Texel coordinates of the backbuffer render target</returns>
	CPoint GetClientCoordinates(const MapCoords& mapCoords) const;

	/// <summary>
	/// Converts from (world) pixel coordinates to Win32 window client coordinates (0/0 at the top left corner of the view window)
	/// </summary>
	/// <param name="projCoords">World pixel coordinates</param>
	/// <returns>Client coordinates as given e.g. by window messages</returns>
	CPoint GetClientCoordinatesFromWorld(const ProjectedCoords& projectedCoords) const;

	/// <summary>
	/// Applies the scaling as it is being done when blitting from back to frontbuffer, required e.g. for drawing calls directly to the frontbuffer
	/// If you use mouse-move coordinates you need to 
	/// </summary>
	/// <param name="projCoords">Texel coordinates of the backbuffer render target</param>
	/// <returns>Texel coordinates of the frontbuffer render target</returns>
	ProjectedCoords ScaleBackToFrontBuffer(const ProjectedCoords& backBufferCoords) const;

	void HandleTrail(int x, int y);
	int GetOverlayDirection(int x, int y);
	void SetError(const char* text);
	COLORREF GetColor(const char* house, const char* color = nullptr);
	void HandleProperties(int n, int type);
	void UpdateDialog(BOOL bRepos=TRUE);
	CMenu m_menu;
	BOOL b_IsLoading;
	
private:
	void UpdateScrollRanges();

private:
	RECT m_myRect;
	COLORREF m_linecolor;
	RECT line;
	int m_type;
	int m_id;
	BOOL m_drag;
	BOOL m_moved;
	MapCoords m_cellCursor;
public:
	
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}

#endif // AFX_ISOVIEW_H__7FB6D6A0_7B52_11D3_99E1_DA6DFD21E706__INCLUDED_
