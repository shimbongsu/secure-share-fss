#ifndef __AERO_SUPPORTS_H__
#define __AERO_SUPPORTS_H__

#include "dwmapi_delegate.h"


#include <winerror.h>
#ifndef __ATLCRACK_H__
#error please include atlcrack.h first
#endif

#ifndef __ATLCTRLS_H__
#include <atlctrls.h>
#endif

#ifndef __ATLSCRL_H__
#include <atlscrl.h>
#endif

#ifndef __ATLMISC_H__
#define _ATL_TMP_NO_CSTRING
#include <atlmisc.h>
#endif

#ifndef __ATLIMAGE_H_
#define __ATLTYPES_H__	// Use the WTL types
#include <atlimage.h>
#endif

// WtlAero.h
//
// WTL::aero namespace: classes and functions supporting the Vista(r) Aero visual style 
//
// Written by Alain Rist (ar@navpoch.com)
// Copyright (c) 2007 Alain Rist.
//
// AR 05.31.2007 CodeProject release
// AR 06.05.2007 CTabView: fixed unthemed environment issues, added DrawMoveMark().
//
// The use and distribution terms for this software are covered by the
// Common Public License 1.0 (http://opensource.org/osi3.0/licenses/cpl1.0.php)
// By using this software in any fashion, you are agreeing to be bound by
// the terms of this license. You must not remove this notice, or
// any other, from this software.
//

#if !defined _WTL_VER || _WTL_VER < 0x800
	#error WtlAero.h requires the Windows Template Library 8.0
#endif

#if _WIN32_WINNT < 0x0600
	#error WtlAero.h requires _WIN32_WINNT >= 0x0600
#elif !defined(NTDDI_VERSION) || (NTDDI_VERSION < NTDDI_LONGHORN)
	#error WtlAero.h requires the Microsoft?Windows?Software Development Kit for Windows Vista?
#endif

#if !defined _UNICODE && !defined UNICODE
	#error WtlAero.h requires Unicode 
#endif


#ifndef __ATLTHEME_H__
	#include "atltheme.h"
#endif



///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// aero::CAeroBase<T> - Base class for Aero translucency (when available)
// aero::CAeroImpl<T> - enables Aero translucency (when available) for any window
// aero::CWindowImpl<T, TBase, TWinTraits> - Implements a Aero window
// aero::CDialogImpl<T, TBase> - dialog implementation of Aero translucency (when available)
// aero::CCtrl<TCtrl> - implementation of Aero drawing for system controls
// aero::CTabCtrl - Aero drawing Tab control
// aero::CReBarCtrl - Aero drawing ReBar control
// aero::CToolBarCtrl - Aero drawing ToolBar Control
// aero::CStatusBarCtrl - Aero drawing StatusBar Control
// aero::CStatic - Aero drawing Static control
// aero::CButton - Aero drawing Button control
// aero::CFrameWindowImpl<T, TBase, TWinTraits> - frame implementation of Aero translucency (when available) 
// aero::CCtrlImpl<T, TCtrlImpl> - implementation of Aero drawing for user and WTL defined controls
// aero::CPrintPreviewWindow - Aero drawing WTL::CPrintPreviewWindow control
// aero::CCommandBarCtrl - Aero drawing WTL::CCommandBarCtrl control
// aero::CTabView - Aero drawing WTL::CTabView control
// aero::CPaneContainer - Aero drawing WTL::CPaneContainer control
// aero::CSplitterImpl<T, t_bVertical> - Aero drawing splitter implementation for any window
// aero::CSplitterWindowImpl<T, t_bVertical, TBase, TWinTraits> - Implements Aero drawing splitter windows
// aero::CSplitterWindowT<t_bVertical> - Aero drawing splitter windows
// aero::CSplitterWindow - Vertical Aero drawing splitter window to be used as is
// aero::CHorSplitterWindow - Horizontal Aero drawing  splitter window to be used as is
//
// namespace helper functions
//
// aero::IsSupported()
// aero::IsComposing()
// aero::IsOpaqueBlend() 
// aero::Subclass()

namespace WTL
{	
namespace aero
{

///////////////////////////////////////////////////////////////////////////////
// WTL::aero helper functions

inline bool IsSupported() 
{
	return CBufferedPaintBase::IsBufferedPaintSupported();
}

inline bool IsComposing() 
{
	BOOL bEnabled = FALSE;
	return IsSupported() ? SUCCEEDED(_DwmIsCompositionEnabled(&bEnabled)) && bEnabled : false;
}    

inline bool IsOpaqueBlend() 
{
	BOOL bOpaqueBlend = FALSE;
	DWORD dwColor;
	return IsSupported() && SUCCEEDED(_DwmGetColorizationColor(&dwColor, &bOpaqueBlend)) ? bOpaqueBlend == TRUE : false;
}    

template <class TCtrl>
inline static BOOL Subclass(TCtrl& Ctrl, HWND hCtrl)
{
	ATLASSERT(::IsWindow(hCtrl));
	return IsSupported() ? Ctrl.SubclassWindow(hCtrl) : FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// aero::CAeroBase - Base class for Aero translucency (when available)

template <class T>
class CAeroBase :
	public WTL::CThemeImpl<T>
{
public:
	CAeroBase(LPCWSTR lpstrThemeClassList = L"globals") 
	{
		SetThemeClassList(lpstrThemeClassList);
	}

	bool IsTheming() const
	{
		return m_hTheme != 0;
	} 

	template <class TCtrl>
	BOOL Subclass(TCtrl& Ctrl, INT idCtrl)
	{
		return aero::Subclass(Ctrl, static_cast<T*>(this)->GetDlgItem(idCtrl));
	}

	bool DrawPartText(HDC dc, int iPartID, int iStateID, LPCTSTR pStr, LPRECT prText, UINT uFormat, DTTOPTS &dto)
	{
		HRESULT hr = S_FALSE;
        if(IsTheming())
			if (IsSupported())
				hr = DrawThemeTextEx (dc, iPartID, iStateID, pStr, -1, uFormat, prText, &dto );
			else
				hr = DrawThemeText(dc, iPartID, iStateID, pStr, -1, uFormat, 0, prText);
		else
			hr = CDCHandle(dc).DrawText(pStr, -1, prText, uFormat) != 0 ? S_OK : S_FALSE;

		return SUCCEEDED(hr);
	}

	 bool DrawPartText(HDC dc, int iPartID, int iStateID, LPCTSTR pStr, LPRECT prText, UINT uFormat, 
		DWORD dwFlags = DTT_COMPOSITED, int iGlowSize = 0)
	{
		DTTOPTS dto = {sizeof(DTTOPTS)};
		dto.dwFlags = dwFlags;
		dto.iGlowSize = iGlowSize;
		return DrawPartText(dc, iPartID, iStateID, pStr, prText, uFormat, dto);
	}

	bool DrawText(HDC dc, LPCTSTR pStr, LPRECT prText, UINT uFormat, DTTOPTS &dto)
	{
		return DrawPartText(dc, 1, 1, pStr, prText, uFormat, dto);
	}

	bool DrawText(HDC dc, LPCTSTR pStr, LPRECT prText, UINT uFormat, DWORD dwFlags = DTT_COMPOSITED, int iGlowSize = 0)
	{
		return DrawPartText(dc, 1, 1, pStr, prText, uFormat, dwFlags, iGlowSize);
	}

};

///////////////////////////////////////////////////////////////////////////////
// aero::CAeroImpl - implementation of Aero translucency (when available)

template <class T>
class CAeroImpl :
	public WTL::CBufferedPaintImpl<T>,
	public CAeroBase<T>
{
public:
	CAeroImpl(LPCWSTR lpstrThemeClassList = L"globals") : CAeroBase<T>(lpstrThemeClassList)
	{
		m_PaintParams.dwFlags = BPPF_ERASE;
		MARGINS m = {-1};
		m_Margins = m;
	}

	MARGINS m_Margins;

	bool SetMargins(MARGINS& m)
	{
		m_Margins = m;
		T* pT = static_cast<T*>(this);
		return pT->IsWindow() && IsComposing() ? SUCCEEDED(_DwmExtendFrameIntoClientArea(pT->m_hWnd, &m_Margins)) : true;
	}

	bool SetOpaque(bool bOpaque = true)
	{
		MARGINS m = {bOpaque - 1};
		return SetMargins(m);
	}

	bool SetOpaque(RECT &rOpaque)
	{
		T* pT = static_cast<T*>(this);
		RECT rClient;
		pT->GetClientRect(&rClient);
		MARGINS m = {rOpaque.left, rClient.right - rOpaque.right, rOpaque.top, rClient.bottom - rOpaque.bottom};
		return SetMargins(m);
	}

	bool SetOpaqueUnder(ATL::CWindow wChild)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(wChild.IsWindow());
		ATLASSERT(pT->IsChild(wChild));

		RECT rChild;
		wChild.GetWindowRect(&rChild);
		pT->ScreenToClient(&rChild);

		return SetOpaque(rChild);
	}

	bool SetOpaqueUnder(UINT uID)
	{
		return SetOpaqueUnder(static_cast<T*>(this)->GetDlgItem(uID));
	}

// implementation
	void DoPaint(CDCHandle dc, RECT& rDest)
	{
		T* pT = static_cast<T*>(this);

		RECT rClient;
		pT->GetClientRect(&rClient);

		RECT rView = {rClient.left + m_Margins.cxLeftWidth, rClient.top + m_Margins.cyTopHeight, 
			rClient.right - m_Margins.cxRightWidth, rClient.bottom - m_Margins.cyBottomHeight};

		if (!IsComposing())
			if (IsTheming())
				pT->DrawThemeBackground(dc, WP_FRAMEBOTTOM, pT->m_hWnd == GetFocus() ? FS_ACTIVE : FS_INACTIVE, &rClient, &rDest);
			else
				dc.FillSolidRect(&rClient, ::GetSysColor(COLOR_MENUBAR));

		if ((m_Margins.cxLeftWidth != -1) && !::IsRectEmpty(&rView))
		{
			dc.FillSolidRect(&rView, ::GetSysColor(COLOR_WINDOW));
			if (!m_BufferedPaint.IsNull())
				m_BufferedPaint.MakeOpaque(&rView);
		}
		else 
			::SetRectEmpty(&rView);

		pT->Paint(dc, rClient, rView, rDest);
	}

	// Overrideables
	void Paint(CDCHandle /*dc*/, RECT& /*rClient*/, RECT& /*rView*/, RECT& /*rDest*/)
	{}

	void OnComposition()
	{}

	void OnColorization()
	{}

	BEGIN_MSG_MAP(CAeroImpl)
		CHAIN_MSG_MAP(CThemeImpl<T>)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
        MESSAGE_HANDLER(WM_DWMCOMPOSITIONCHANGED, OnCompositionChanged)
        MESSAGE_HANDLER(WM_DWMCOLORIZATIONCOLORCHANGED, OnColorizationChanged)
		CHAIN_MSG_MAP(CBufferedPaintImpl<T>)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if (IsThemingSupported())
			OpenThemeData();

		if (IsComposing())
			::_DwmExtendFrameIntoClientArea(static_cast<T*>(this)->m_hWnd, &m_Margins);
		return bHandled = FALSE;
	}

	LRESULT OnActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if (!IsComposing() && IsTheming())
			static_cast<T*>(this)->Invalidate(FALSE);
		return bHandled = FALSE;
	}

	LRESULT OnCompositionChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if (IsComposing())
			SetMargins(m_Margins);
		static_cast<T*>(this)->OnComposition();
		return bHandled = FALSE;
	}

	LRESULT OnColorizationChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		static_cast<T*>(this)->OnColorization();
		return bHandled = FALSE;
	}
};

///////////////////////////////////////////////////////////////////////////////
//  aero::CWindowImpl - Implements a Aero window

template <class T, class TBase = ATL::CWindow, class TWinTraits = ATL::CControlWinTraits>
class ATL_NO_VTABLE CWindowImpl : 
	public ATL::CWindowImpl< T, TBase, TWinTraits >, 
	public CAeroImpl<T>
{
public:
	DECLARE_WND_CLASS_EX(NULL, CS_DBLCLKS, -1)

	CWindowImpl(LPCWSTR lpstrThemeClassList = L"window") : CAeroImpl<T>(lpstrThemeClassList)
	{}

	typedef CAeroImpl<T> _baseAero;

	void Paint(CDCHandle dc, RECT& rClient, RECT& rView, RECT& rDest)
	{}

	BEGIN_MSG_MAP(CWindowImpl)
		CHAIN_MSG_MAP(_baseAero)
	END_MSG_MAP()
};

///////////////////////////////////////////////////////////////////////////////
// aero::CDialogImpl - dialog implementation of Aero translucency (when available)

template <class T, class TBase  = ATL::CWindow>
class ATL_NO_VTABLE CDialogImpl : 
	public ATL::CDialogImpl<T, TBase>, 
	public CAeroImpl<T>
{
public:
	CDialogImpl(LPCWSTR lpstrThemeClassList = L"dialog") : CAeroImpl(lpstrThemeClassList)
	{}

	void Paint(CDCHandle dc, RECT& /*rClient*/, RECT& rView, RECT& rDest)
	{
		if (!::IsRectEmpty(&rView))
		{
			if (IsTheming())
				DrawThemeBackground(dc, WP_DIALOG, 1, &rView, &rDest);
			else
				dc.FillSolidRect(&rView, GetSysColor(COLOR_BTNFACE));
		}
	}

	BEGIN_MSG_MAP(CDialogImpl)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CAeroImpl<T>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		SendMessage(WM_CREATE);
		if (IsThemingSupported())
			EnableThemeDialogTexture(ETDT_ENABLE);
		return bHandled = FALSE;
	}

};

#ifdef __ATLCTRLS_H__

///////////////////////////////////////////////////////////////////////////////
// aero::CCtrl -  implementation of Aero drawing for system controls
// Note: This class is intended for system themed control specializations

template <class TBase>
class CCtrl : 
	public WTL::CBufferedPaintWindowImpl<CCtrl<TBase>, TBase>,
	public CAeroBase<CCtrl<TBase> >
{
public:
	typedef CAeroBase<CCtrl<TBase> > baseAero;
	typedef WTL::CBufferedPaintWindowImpl<CCtrl<TBase>, TBase> baseWindow;

	DECLARE_WND_SUPERCLASS(NULL, TBase::GetWndClassName())

	// creation and initilization
	CCtrl(LPCWSTR lpstrThemeClassList = GetThemeName()) : baseAero(lpstrThemeClassList)
	{
		m_PaintParams.dwFlags = BPPF_ERASE;
	}

	CCtrl<TBase>& operator =(HWND hWnd)
	{
		TBase::m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		TBase baseCtrl;
		if (baseCtrl.Create(hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam) != NULL)
			SubclassWindow(baseCtrl.m_hWnd);
		return m_hWnd;
	}

	BOOL SubclassWindow(HWND hWnd)
	{
		//ATLASSERT(IsSupported());
		if(baseWindow::SubclassWindow(hWnd))
			OpenThemeData();
		return m_hWnd != NULL;
	}

	// specializables
	static LPCWSTR GetThemeName()
	{
		return TBase::GetWndClassName();
	}

	virtual void CtrlPaint(HDC hdc, RECT& /*rCtrl*/, RECT& rPaint)
	{
		DefCtrlPaint(hdc, rPaint);
	}

	// operations
	void DefCtrlPaint(HDC hdc, RECT& rPaint, bool bEraseBkGnd = false)
	{
		if (bEraseBkGnd)
			DefWindowProc(WM_ERASEBKGND, (WPARAM)hdc, NULL);
		DefWindowProc(WM_PAINT, (WPARAM)hdc, 0);
		if( !m_BufferedPaint.IsNull() )
		{
			m_BufferedPaint.MakeOpaque(&rPaint);
		}
	}

	BOOL DrawCtrlBackground(HDC hdc, int nPartID, int nStateID, RECT &rCtrl, RECT &rPaint)
	{
		return SUCCEEDED(DrawThemeBackground(hdc, nPartID, nStateID, &rCtrl, &rPaint));
	}

	BOOL DrawCtrlEdge(HDC hdc, int nPartID, int nStateID, RECT &rCtrl, UINT uEdge = EDGE_ETCHED, UINT uFlags = BF_RECT, LPRECT pContentRect = NULL)
	{
		return SUCCEEDED(DrawThemeEdge(hdc, nPartID, nStateID, &rCtrl, uEdge, uFlags, pContentRect));
	}

	BOOL DrawCtrlText(CDCHandle dc, int nPartID, int nStateID, UINT uFormat, RECT &rCtrl, HFONT hFont = 0,
		DWORD dwFlags = DTT_COMPOSITED, int iGlowSize = 0)
	{
		HRESULT hr;
		RECT rText;
		hr = GetThemeBackgroundContentRect(dc, nPartID, nStateID, &rCtrl, &rText);
		MARGINS m = {0};
		hr = GetThemeMargins(dc, nPartID, nStateID, TMT_CONTENTMARGINS, &rText, &m);
		rText.left += m.cxLeftWidth;
		rText.right -= m.cxRightWidth;
		int iLength = GetWindowTextLength();
		if (iLength > 0)
		{
			CTempBuffer<WCHAR> sText(++iLength);
			GetWindowText(sText, iLength);

			HFONT hf = dc.SelectFont(hFont == 0 ? GetFont() : hFont);
			hr = DrawPartText(dc, nPartID, nStateID, sText,  &rText , uFormat, dwFlags, iGlowSize);
			dc.SelectFont(hf);
		}
		return SUCCEEDED(hr) && iLength > 0;
	}

	// implementation
	void DoBufferedPaint(HDC hdc, RECT& rPaint)
	{
		HDC hDCPaint = NULL;
		RECT rCtrl;
		GetClientRect(&rCtrl);
		if( IsComposing() )
		{
			m_BufferedPaint.Begin(hdc, &rCtrl, m_dwFormat, &m_PaintParams, &hDCPaint);
			//ATLASSERT(hDCPaint != NULL);
			CtrlPaint(hDCPaint, rCtrl, rPaint);
			m_BufferedPaint.End();
		}else
		{
			CtrlPaint(hDCPaint, rCtrl, rPaint);
		}
	}

	void DoPaint(HDC /*hdc*/, RECT& /*rCtrl*/)
	{
		DefWindowProc();
	}

	BEGIN_MSG_MAP(CCtrl)
		MESSAGE_HANDLER(WM_PAINT, OnPaintMsg)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgndMsg)
		CHAIN_MSG_MAP(baseAero)
		CHAIN_MSG_MAP(baseWindow)
	END_MSG_MAP()

	LRESULT OnPaintMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(!IsComposing())
			return DefWindowProc();
		else
			return bHandled = FALSE;
	}

	virtual LRESULT OnEraseBkgndMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(!IsComposing())
			return DefWindowProc();
		else
			return bHandled = FALSE;
	}
};

///////////////////////////////////////////////////////////////////////////////
// Macro declaring and subclassing a control (static declaration to use in OnCreate or OnInitDialog members)

#define AERO_CONTROL(type, name, id)\
	static aero::type name;\
	WTL::aero::Subclass(name, id);

///////////////////////////////////////////////////////////////////////////////
// aero::CEdit - Aero drawing Edit control

class CEdit : public CCtrl<WTL::CEdit> 
{
public:
	BEGIN_MSG_MAP(CEdit)
		MESSAGE_HANDLER(EM_SETSEL, OnRedraw)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnRedraw)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnRedraw)
		MESSAGE_HANDLER(WM_KEYDOWN, OnRedraw)
		MESSAGE_HANDLER(WM_PASTE, OnRedraw)
		CHAIN_MSG_MAP(CCtrl<WTL::CEdit>)
	END_MSG_MAP()
	
	LRESULT OnRedraw(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		DefWindowProc();
		Invalidate(FALSE);
		return 0;
	}

	void CtrlPaint(HDC hdc, RECT& rCtrl, RECT& rPaint)
	{
		DefCtrlPaint( hdc, rPaint, true );
	}

	BOOL DrawCtrlBackground(HDC hdc, int nPartID, int nStateID, RECT &rCtrl, RECT &rPaint)
	{
		//return SUCCEEDED(DrawThemeBackground(hdc, nPartID, nStateID, &rCtrl, &rPaint));
		return TRUE;
	}

};



///////////////////////////////////////////////////////////////////////////////
// aero::CTabCtrl - Aero drawing Tab control

typedef CCtrl<WTL::CTabCtrl> CTabCtrl;

inline LPCWSTR CTabCtrl::GetThemeName()
{
	return L"TAB";
};

inline LRESULT CTabCtrl::OnEraseBkgndMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	if(!IsComposing())
		return DefWindowProc();
	else
	{
		//CPaintDC dc(m_hWnd);
		//RECT rClient;
		//GetClientRect(&rClient);
		//RECT rCtrl;
		//GetWindowRect(&rCtrl);
		//POINT pt;
		//pt.x = rCtrl.left;
		//pt.y = rCtrl.top;
		//ScreenToClient( &pt );
		//dc.FillSolidRect( &rClient, RGB(255,255,0));
		return bHandled = FALSE;
	}
}

inline void CTabCtrl::CtrlPaint(HDC hdc, RECT& rCtrl, RECT& rPaint)
{
	
	RECT rTab;
	//TCITEM TcItem;
	//RECT rClient;
	//CPaintDC dc(m_hWnd);
	//GetClientRect(&rClient);
	//GetWindowRect(&rClient);
	////dc.FillSolidRect( &rCtrl, RGB(255,255,0));
	//CDC dd(hdc);
	//dd.FillSolidRect( &rPaint, RGB(0,0,0));
	//DefWindowProc(WM_PAINT, (WPARAM)hdc, NULL);
	

	DefCtrlPaint(hdc, rPaint);


	if( !m_BufferedPaint.IsNull() )
	{
		m_BufferedPaint.MakeOpaque(&rPaint);
		for (int nTab = 0; GetItemRect(nTab, &rTab); nTab++)
		{
			//if( GetItem( nTab, &TcItem ) )
			//{
			//	DrawCtrlText(hdc, 1, 1, TcItem.pszText, &rTab, DT_CENTER);
			//}
			//rTab.right -= 3;
			//rTab.bottom -= 3;
			//DrawThemeBackground(hdc, 1, 1, &rTab, &rPaint);
			m_BufferedPaint.MakeOpaque(&rTab);
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
// aero::CToolBarCtrl - Aero translucent ToolBar Control

typedef CCtrl<WTL::CToolBarCtrl> CToolBarCtrl;

inline LPCWSTR CToolBarCtrl::GetThemeName()
{
	return L"TOOLBAR";
};

inline void CToolBarCtrl::CtrlPaint(HDC hdc, RECT& /*rCtrl*/, RECT& rPaint)
{
	int nBtn = GetButtonCount(), 
		iHot = GetHotItem();

	CImageList img = GetImageList();

	int cx, cy;
	img.GetIconSize(cx, cy);

	RECT rBtn;
	TOOLBARPARTS part;
	TOOLBARSTYLESTATES state;

	for (int i = 0; i < nBtn; i++)
	{
		TBBUTTONINFO tbbi = {sizeof(TBBUTTONINFO), TBIF_BYINDEX | TBIF_STATE | TBIF_STYLE | TBIF_IMAGE};
		GetButtonInfo(i, &tbbi);

		part = tbbi.fsStyle & BTNS_SEP ? TP_SEPARATOR : TP_BUTTON;
		state = tbbi.fsState & TBSTATE_ENABLED ? tbbi.fsState & TBSTATE_CHECKED ? TS_CHECKED : TS_NORMAL : TS_DISABLED;
		if (i == iHot)
			state = tbbi.fsState & TBSTATE_PRESSED ? TS_PRESSED : TS_HOT;


		GetItemRect(i, &rBtn);
		DrawThemeBackground(hdc, part, state, &rBtn, &rPaint);

		if (part != TP_SEPARATOR)
		{
			RECT rbmp;
			GetThemeBackgroundContentRect(hdc, part, state, &rBtn, &rbmp);
			int x = rbmp.left + (rbmp.right - rbmp.left - cx) / 2;
			int y = rbmp.top + (rbmp.bottom - rbmp.top - cy) / 2;
			IMAGELISTDRAWPARAMS ildp = {sizeof(IMAGELISTDRAWPARAMS), img, tbbi.iImage, hdc, x, y, 0, 0, 0, 0,
				CLR_NONE, CLR_NONE, ILD_PRESERVEALPHA, SRCCOPY, (state & TS_DISABLED)? ILS_SATURATE : ILS_NORMAL}; 
			img.DrawIndirect(&ildp);
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
// aero::CStatusBarCtrl - Aero drawing StatusBar Control

typedef CCtrl<WTL::CStatusBarCtrl> CStatusBarCtrl;

inline LPCWSTR CStatusBarCtrl::GetThemeName()
{
	return L"STATUS";
};

inline void CStatusBarCtrl::CtrlPaint(HDC hdc, RECT& /*rCtrl*/, RECT& rPaint)
{
	DefCtrlPaint(hdc, rPaint, true);
}

///////////////////////////////////////////////////////////////////////////////
// aero::CListBox - Aero drawing ListBox control

typedef CCtrl<WTL::CListBox> CListBox;

inline void CListBox::CtrlPaint(HDC hdc, RECT& /*rCtrl*/, RECT& rPaint)
{
	DefCtrlPaint(hdc, rPaint, true);
}

///////////////////////////////////////////////////////////////////////////////
// aero::CComboBox - Aero drawing ComboBox control

inline void CCtrl<WTL::CComboBox>::CtrlPaint(HDC hdc, RECT& /*rCtrl*/, RECT& rPaint)
{
	DefCtrlPaint(hdc, rPaint, true);
}

class CComboBoxEx : public CCtrl<WTL::CComboBoxEx>
{
public:
	aero::CEdit m_AE;

	BOOL SubclassWindow(HWND hWnd)
	{
		//ATLASSERT(IsSupported());
		if(baseWindow::SubclassWindow(hWnd))
		{
			OpenThemeData();
			HWND hwndEdit= (HWND) SendMessage( hWnd, CBEM_GETEDITCONTROL, (WPARAM)0, (LPARAM)0 );
			if (hwndEdit != NULL)
			{
				aero::Subclass(m_AE, hwndEdit);
			}else
			{
				HWND hwndCombo= (HWND) SendMessage( hWnd, CBEM_GETCOMBOCONTROL, (WPARAM)0, (LPARAM)0 );
				if( hwndCombo != NULL )
				{
					aero::Subclass(m_AE, hwndCombo);
				}
			}
		}
		return m_hWnd != NULL;
	}
};

class CComboBox : public CCtrl<WTL::CComboBox>
{
public:
	aero::CEdit m_AE;

	BOOL SubclassWindow(HWND hWnd)
	{
		ATLASSERT(IsSupported());
		if(baseWindow::SubclassWindow(hWnd))
		{
			OpenThemeData();
			COMBOBOXINFO cbi = {sizeof(COMBOBOXINFO)};
			GetComboBoxInfo(&cbi);
			if (cbi.hwndItem != NULL)
				ATLVERIFY(aero::Subclass(m_AE, cbi.hwndItem));
		}
		return m_hWnd != NULL;
	}
};


///////////////////////////////////////////////////////////////////////////////
// aero::CStatic - Aero drawing Static control

typedef CCtrl<WTL::CStatic> CStatic;

inline LPCWSTR CStatic::GetThemeName()
{
	return L"static::globals";
};

inline void CStatic::CtrlPaint(HDC hdc, RECT& rCtrl, RECT& rPaint)
{
	DWORD dwStyle = GetStyle();

	UINT uFormat = dwStyle & 0x3L;
	if ((dwStyle | SS_SIMPLE) == SS_SIMPLE)
		uFormat |= DT_SINGLELINE; 
	else
		uFormat |= DT_WORDBREAK; 


	switch (dwStyle & 0xfL)
	{
	case SS_ICON:
	case SS_BITMAP:
	case SS_ENHMETAFILE:
	case SS_BLACKRECT:
	case SS_GRAYRECT:
	case SS_WHITERECT:
	case SS_BLACKFRAME:
	case SS_GRAYFRAME:
	case SS_WHITEFRAME:
	case SS_OWNERDRAW:
		DefCtrlPaint(hdc, rPaint);
		break;
	default:
		DrawCtrlText(hdc, 1, 1, uFormat, rCtrl, 0, DTT_COMPOSITED | DTT_GLOWSIZE, 10);
	}
}

///////////////////////////////////////////////////////////////////////////////
// aero::CButton - Aero drawing Button control

class CButtonThemeHelper
{
public:
	CButtonThemeHelper(DWORD dwStyle, DWORD dwState, BOOL bEnabled)
	{
		m_bp = _GetPart(dwStyle);
		m_bs = _GetState(m_bp, dwState, bEnabled);
		m_fmt = _GetFormat(m_bp, dwStyle);
	}

	enum BUTTONPARTS m_bp;
	int m_bs;
	UINT m_fmt;

	static int _GetFormat(enum BUTTONPARTS bp, DWORD dwStyle)
	{
		int fmt = dwStyle & BS_MULTILINE ? DT_WORDBREAK : DT_SINGLELINE;
		switch (dwStyle & BS_CENTER)
		{
		case BS_LEFT: 
			fmt |= DT_LEFT;
			break;
		case BS_RIGHT: 
			fmt |= DT_RIGHT;
			break;
		case BS_CENTER: 
			fmt |= DT_CENTER; 
			break;
		default:
			fmt |= bp == BP_PUSHBUTTON ? DT_CENTER : DT_LEFT; 
		}

		switch (dwStyle & BS_VCENTER)
		{
		case BS_TOP: 
			fmt |= DT_TOP;
			break;
		case BS_BOTTOM: 
			fmt |= DT_BOTTOM;
			break;
		case BS_VCENTER: 
			fmt |= DT_VCENTER; 
			break;
		default:
			if (fmt ^ BS_MULTILINE)
				fmt |= bp == BP_GROUPBOX ? DT_TOP : DT_VCENTER; 
		}

		return fmt;
	}

	static int _GetBaseButtonState(DWORD dwState, BOOL bEnabled)
	{
		if (!bEnabled)
			return PBS_DISABLED;
		if (dwState & BST_PUSHED)
			return PBS_PRESSED;
		if (dwState & BST_HOT)
			return PBS_HOT;
		return PBS_NORMAL;
	}

	static int _GetState(enum BUTTONPARTS bp, DWORD dwState, BOOL bEnabled)
	{
		int bs = _GetBaseButtonState(dwState, bEnabled);
		switch (bp)
		{
		case BP_PUSHBUTTON:
		case BP_COMMANDLINK:
			if ((bs == PBS_NORMAL) && (dwState & BST_FOCUS)) 
				bs = PBS_DEFAULTED_ANIMATING;
			break;
		case BP_CHECKBOX:
			if (dwState & BST_INDETERMINATE)
			{
				bs += 8;
				break;
			}
		case BP_RADIOBUTTON:
			if (dwState & BST_CHECKED)
				bs += 4;
		}
		return bs;
	}

	static enum BUTTONPARTS _GetPart(DWORD dwStyle)
	{
		switch(dwStyle & 0xf)
		{
		case BS_CHECKBOX:
		case BS_AUTOCHECKBOX:
		case BS_3STATE:
		case BS_AUTO3STATE:
			return BP_CHECKBOX;
		case BS_RADIOBUTTON:
		case BS_AUTORADIOBUTTON:
			return BP_RADIOBUTTON;
		case BS_GROUPBOX:
			return BP_GROUPBOX;
		case BS_USERBUTTON:
		case BS_OWNERDRAW:
			return BP_USERBUTTON;
		case BS_COMMANDLINK:
		case BS_DEFCOMMANDLINK:
			return BP_COMMANDLINK;
		default:
			return BP_PUSHBUTTON;
		}
	}
};

typedef CCtrl<WTL::CButton> CButton;

inline void CButton::CtrlPaint(HDC hdc, RECT& rCtrl, RECT& rPaint)
{
		HRESULT hr;
		CButtonThemeHelper helper(GetButtonStyle(), GetState(), IsWindowEnabled());
		switch (helper.m_bp)
		{
		case BP_USERBUTTON:
			DefCtrlPaint(hdc, rPaint);
			return;

		case BP_GROUPBOX:
			DefWindowProc(WM_PAINT, (WPARAM)hdc, 0);
			{
				int iLength = GetWindowTextLength();
				if (iLength > 0)
				{
					RECT rText;
					CDCHandle dc(hdc);
					CTempBuffer<WCHAR> sText(++iLength);
					GetWindowText(sText, iLength);
					HFONT hf = dc.SelectFont(GetFont());
					hr = GetThemeTextExtent(dc, helper.m_bp, helper.m_bs, sText, -1, helper.m_fmt, &rCtrl, &rText);
					ATLASSERT(SUCCEEDED(hr));
					OffsetRect(&rText, 10, 0);
					m_BufferedPaint.Clear(&rText);
					hr = DrawPartText(dc, helper.m_bp, helper.m_bs,  (LPCWSTR)sText, &rText , helper.m_fmt, DTT_COMPOSITED | DTT_GLOWSIZE, 10);
					ATLASSERT(SUCCEEDED(hr));
					dc.SelectFont(hf);
				}
			}
			return;
			
		case BP_RADIOBUTTON:
		case BP_CHECKBOX:
			{
				CBitmapHandle bm;
				SIZE s;
				RECT rBitmap = rCtrl;
				hr = GetThemeBitmap(helper.m_bp, helper.m_bs, 0, GBF_DIRECT, bm.m_hBitmap);
				ATLASSERT(SUCCEEDED(hr));
				bm.GetSize(s);
				if (GetButtonStyle() & BS_RIGHTBUTTON)
					rCtrl.right = rBitmap.left = rBitmap.right - s.cx - s.cx / 2;
				else
					rCtrl.left = rBitmap.right = rBitmap.left + s.cx + s.cx / 2;
				hr = DrawThemeBackground(hdc, helper.m_bp, helper.m_bs, &rBitmap, NULL);
				ATLASSERT(SUCCEEDED(hr));
			}
			break;

		default:
			hr = DrawCtrlBackground(hdc, helper.m_bp, helper.m_bs, rCtrl, rPaint);
			ATLASSERT(SUCCEEDED(hr));
		}
		hr = DrawCtrlText(hdc, helper.m_bp, helper.m_bs, helper.m_fmt, rCtrl, 0);
		ATLASSERT(SUCCEEDED(hr));
}

#ifdef __ATLDLGS_H__

///////////////////////////////////////////////////////////////////////////////
// aero::CPropertySheetImpl - Property Sheet implementation of Aero translucency (when available)

template <class T, class TBase  = WTL::CPropertySheetWindow>
class ATL_NO_VTABLE CPropertySheetImpl : 
	public WTL::CPropertySheetImpl<T, TBase>, 
	public CAeroImpl<T>
{
	typedef WTL::CPropertySheetImpl<T, TBase> basePS;
public:

	CPropertySheetImpl(ATL::_U_STRINGorID title = (LPCTSTR)NULL, UINT uStartPage = 0, HWND hWndParent = NULL):
		basePS(title, uStartPage, hWndParent) , CAeroImpl(L"window")
	{}

	aero::CTabCtrl m_Atab;
	aero::CButton m_btns[PSBTN_MAX];

	void Paint(CDCHandle dc, RECT& /*rClient*/, RECT& rView, RECT& rDest)
	{
		if (!::IsRectEmpty(&rView))
		{
			if (IsTheming())
				DrawThemeBackground(dc, WP_DIALOG, 1, &rView, &rDest);
			else
				dc.FillSolidRect(&rView, GetSysColor(COLOR_WINDOW));
		}
	}

	void InitPS()
	{
		aero::Subclass(m_Atab, GetTabControl());
		for (int idBtn = 0 ; idBtn < PSBTN_MAX; idBtn++)
			if (HWND hBtn = GetDlgItem(idBtn))
				aero::Subclass(m_btns[idBtn], hBtn);
		SendMessage(WM_CREATE);
		if (IsThemingSupported())
			EnableThemeDialogTexture(ETDT_ENABLE);
	}

// Callback function
	static int CALLBACK PropSheetCallback(HWND hWnd, UINT uMsg, LPARAM lParam)
	{
		lParam;   // avoid level 4 warning
		int nRet = 0;

		if(uMsg == PSCB_INITIALIZED)
		{
			ATLASSERT(hWnd != NULL);
			T* pT = (T*)ModuleHelper::ExtractCreateWndData();
			// subclass the sheet window
			pT->SubclassWindow(hWnd);
			// remove page handles array
			pT->_CleanUpPages();

			pT->InitPS();
			pT->OnSheetInitialized();
		}

		return nRet;
	}

	BEGIN_MSG_MAP(CPropertySheetImpl)
		CHAIN_MSG_MAP(CAeroImpl<T>)
		CHAIN_MSG_MAP(basePS)
	END_MSG_MAP()
};

///////////////////////////////////////////////////////////////////////////////
// aero::CPropertyPageImpl - Property page implementation of Aero translucency (when available)

template <class T, class TBase = WTL::CPropertyPageWindow>
class ATL_NO_VTABLE CPropertyPageImpl : 
	public WTL::CPropertyPageImpl<T, TBase>, 
	public CAeroImpl<T>
{
public:
	typedef WTL::CPropertyPageImpl<T, TBase> basePP;

	CPropertyPageImpl(ATL::_U_STRINGorID title = (LPCTSTR)NULL) :
		basePP(title),
		CAeroImpl<T>(L"dialog")
	{}

	void Paint(CDCHandle dc, RECT& /*rClient*/, RECT& rView, RECT& rDest)
	{
		if (!::IsRectEmpty(&rView))
		{
			if (IsTheming())
				DrawThemeBackground(dc, WP_DIALOG, 1, &rView, &rDest);
			else
				dc.FillSolidRect(&rView, GetSysColor(COLOR_WINDOW));
		}
	}

	BEGIN_MSG_MAP(CPropertyPageImpl)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CAeroImpl<T>)
		CHAIN_MSG_MAP(basePP)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		SendMessage(WM_CREATE);
		if (IsThemingSupported())
			EnableThemeDialogTexture(ETDT_ENABLE);
		return bHandled = FALSE;
	}
};
#endif // __ATLDLGS_H__


#ifdef __ATLFRAME_H__

///////////////////////////////////////////////////////////////////////////////
// aero::CFrameWindowImpl - Aero frame implementation

template <class T, class TBase = ATL::CWindow, class TWinTraits = ATL::CFrameWinTraits>
class ATL_NO_VTABLE CFrameWindowImpl : 
	public WTL::CFrameWindowImpl<T, TBase, TWinTraits>,
	public CAeroImpl<T>
{
	typedef WTL::CFrameWindowImpl<T, TBase, TWinTraits> _baseClass;

public:
	CFrameWindowImpl(LPCWSTR lpstrThemeClassList = L"window") : CAeroImpl(lpstrThemeClassList)
	{}

	void UpdateLayout(BOOL bResizeBars = TRUE)
	{
		RECT rect = { 0 };
		GetClientRect(&rect);

		// position margins
		if (m_Margins.cxLeftWidth != -1)
		{
			rect.left += m_Margins.cxLeftWidth;
			rect.top += m_Margins.cyTopHeight;
			rect.right -= m_Margins.cxRightWidth;
			rect.bottom -= m_Margins.cyBottomHeight;
		}

		// position bars and offset their dimensions
		UpdateBarsPosition(rect, bResizeBars);

		// resize client window
		if(m_hWndClient != NULL)
			::SetWindowPos(m_hWndClient, NULL, rect.left, rect.top,
				rect.right - rect.left, rect.bottom - rect.top,
				SWP_NOZORDER | SWP_NOACTIVATE);

		Invalidate(FALSE);
	}

	void UpdateBarsPosition(RECT& rect, BOOL bResizeBars = TRUE)
	{
		// resize toolbar
		if(m_hWndToolBar != NULL && ((DWORD)::GetWindowLong(m_hWndToolBar, GWL_STYLE) & WS_VISIBLE))
		{
			RECT rectTB = { 0 };
			::GetWindowRect(m_hWndToolBar, &rectTB);
			if(bResizeBars)
			{
				::SetWindowPos(m_hWndToolBar, NULL, rect.left, rect.top,
					rect.right - rect.left, rectTB.bottom - rectTB.top,
					SWP_NOZORDER | SWP_NOACTIVATE);
				::InvalidateRect(m_hWndToolBar, NULL, FALSE);
			}
			rect.top += rectTB.bottom - rectTB.top;
		}

		// resize status bar
		if(m_hWndStatusBar != NULL && ((DWORD)::GetWindowLong(m_hWndStatusBar, GWL_STYLE) & WS_VISIBLE))
		{
			RECT rectSB = { 0 };
			::GetWindowRect(m_hWndStatusBar, &rectSB);
			rect.bottom -= rectSB.bottom - rectSB.top;
			if(bResizeBars)
				::SetWindowPos(m_hWndStatusBar, NULL, rect.left, rect.bottom,
					rect.right - rect.left, rectSB.bottom - rectSB.top,
					SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}
	
	aero::CStatusBarCtrl m_ASB;
	aero::CToolBarCtrl m_ATB;

	BOOL CreateSimpleStatusBar(LPCTSTR lpstrText, DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS , UINT nID = ATL_IDW_STATUS_BAR)
	{
		ATLASSERT(!::IsWindow(m_hWndStatusBar));
		m_hWndStatusBar = ::CreateStatusWindow(dwStyle | CCS_NOPARENTALIGN , lpstrText, m_hWnd, nID);
		aero::Subclass(m_ASB, m_hWndStatusBar);
		return (m_hWndStatusBar != NULL);
	}

	BOOL CreateSimpleStatusBar(UINT nTextID = ATL_IDS_IDLEMESSAGE, DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS , UINT nID = ATL_IDW_STATUS_BAR)
	{
		const int cchMax = 128;   // max text length is 127 for status bars (+1 for null)
		TCHAR szText[cchMax];
		szText[0] = 0;
		::LoadString(ModuleHelper::GetResourceInstance(), nTextID, szText, cchMax);
		return CreateSimpleStatusBar(szText, dwStyle, nID);
	}

	HWND CreateAeroToolBarCtrl(HWND hWndParent, UINT nResourceID, BOOL bInitialSeparator = FALSE, 
			DWORD dwStyle = ATL_SIMPLE_TOOLBAR_STYLE, UINT nID = ATL_IDW_TOOLBAR)
	{
		HWND hWndToolBar = _baseClass::CreateSimpleToolBarCtrl(hWndParent, nResourceID, bInitialSeparator, dwStyle, nID);
		if (hWndToolBar != NULL)
			aero::Subclass(m_ATB, hWndToolBar);
		return hWndToolBar;
	}

	HWND CreateSimpleReBarCtrl(HWND hWndParent, DWORD dwStyle = ATL_SIMPLE_REBAR_STYLE, UINT nID = ATL_IDW_TOOLBAR)
	{
		HWND hRB = _baseClass::CreateSimpleReBarCtrl(hWndParent, dwStyle | CCS_NOPARENTALIGN, nID);
		return hRB;
	}

	BOOL CreateSimpleReBar(DWORD dwStyle = ATL_SIMPLE_REBAR_STYLE, UINT nID = ATL_IDW_TOOLBAR)
	{
		ATLASSERT(!::IsWindow(m_hWndToolBar));
		m_hWndToolBar = CreateSimpleReBarCtrl(m_hWnd, dwStyle | CCS_NOPARENTALIGN, nID);
		return (m_hWndToolBar != NULL);
	}

	BEGIN_MSG_MAP(CFrameWindowImpl)
		CHAIN_MSG_MAP(CAeroImpl<T>)
		CHAIN_MSG_MAP(_baseClass)
	END_MSG_MAP()
};
#endif // __ATLFRAME_H__
#endif // __ATLCTRLS_H__

///////////////////////////////////////////////////////////////////////////////
// aero::CCtrlImpl -  implementation of Aero drawing for user and WTL defined controls
// Note: This class is intended for derivation

template <class T, class TCtrlImpl, bool t_bOpaque = false>
class ATL_NO_VTABLE CCtrlImpl : 
	public TCtrlImpl,
	public CAeroImpl<T>
{
public:
	DECLARE_WND_SUPERCLASS(NULL, TCtrlImpl::GetWndClassName())

	CCtrlImpl(LPCWSTR lpstrThemeClassList = L"window") : CAeroImpl(lpstrThemeClassList)
	{
		m_PaintParams.dwFlags = BPPF_ERASE;
	}

	void DoPaint(HDC hdc, RECT& rect)
	{
		BOOL bHandled = TRUE;
		TCtrlImpl::OnPaint(WM_PAINT, (WPARAM) hdc, NULL, bHandled);
		if (t_bOpaque)
		{
			if( !m_BufferedPaint.IsNull() )
			m_BufferedPaint.MakeOpaque(&rect);
		}
	}

	BEGIN_MSG_MAP(CCtrlImpl)
		CHAIN_MSG_MAP(CAeroImpl<T>)
		CHAIN_MSG_MAP(TCtrlImpl)
	END_MSG_MAP()

};

#ifdef __ATLCTRLW_H__

///////////////////////////////////////////////////////////////////////////////
// aero::CCommandBarCtrl - Aero drawing WTL::CCommandBarCtrl control

class CCommandBarCtrl :
	public CCtrlImpl<CCommandBarCtrl, WTL::CCommandBarCtrlImpl<CCommandBarCtrl> >
{
public:
	
	DECLARE_WND_SUPERCLASS(_T("WTL_AeroCommandBar"), GetWndClassName())

	CCommandBarCtrl() : CCtrlImpl(L"MENU")
	{}

	void DoPaint(HDC hdc, RECT&)
	{
		DefWindowProc(WM_PAINT, (WPARAM) hdc, NULL);
	}

	BEGIN_MSG_MAP(CCommandBarCtrl)
		CHAIN_MSG_MAP(CThemeImpl<CCommandBarCtrl>)
		CHAIN_MSG_MAP(CBufferedPaintImpl<CCommandBarCtrl>)
		CHAIN_MSG_MAP(WTL::CCommandBarCtrlImpl<CCommandBarCtrl>)
	ALT_MSG_MAP(1)   // Parent window messages
		NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW, OnParentCustomDraw)
		CHAIN_MSG_MAP_ALT(WTL::CCommandBarCtrlImpl<CCommandBarCtrl>, 1)
	ALT_MSG_MAP(3)   // Message hook messages
		CHAIN_MSG_MAP_ALT(WTL::CCommandBarCtrlImpl<CCommandBarCtrl>, 3)
		END_MSG_MAP()

	LRESULT OnParentCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		if (!IsTheming())
			return WTL::CCommandBarCtrlImpl<CCommandBarCtrl>::OnParentCustomDraw(idCtrl, pnmh, bHandled);

		LRESULT lRet = CDRF_DODEFAULT;
		bHandled = FALSE;
		if(pnmh->hwndFrom == m_hWnd)
		{
			LPNMTBCUSTOMDRAW lpTBCustomDraw = (LPNMTBCUSTOMDRAW)pnmh;
			CDCHandle dc = lpTBCustomDraw->nmcd.hdc;
			RECT& rc = lpTBCustomDraw->nmcd.rc;

			if(lpTBCustomDraw->nmcd.dwDrawStage == CDDS_PREPAINT)
			{
				DrawThemeBackground(dc, MENU_BARBACKGROUND, m_bParentActive ? MB_ACTIVE : MB_INACTIVE, &rc);
				lRet = CDRF_NOTIFYITEMDRAW;
				bHandled = TRUE;
			}
			else if(lpTBCustomDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
			{
				bool bHot = ((lpTBCustomDraw->nmcd.uItemState & CDIS_HOT) == CDIS_HOT);
				bool bPushed = ((lpTBCustomDraw->nmcd.uItemState & CDIS_SELECTED) == CDIS_SELECTED);
				bool bDisabled = ((lpTBCustomDraw->nmcd.uItemState & CDIS_DISABLED) == CDIS_DISABLED);

				int bis = bPushed ? MBI_PUSHED : bHot ? MBI_HOT : MBI_NORMAL;

				if(m_bFlatMenus)
				{
					if (bDisabled || (!m_bParentActive && !bPushed && !bHot))
						bis += 3;
					if ((bHot || bPushed) && !bDisabled)
						DrawThemeBackground(dc, MENU_POPUPITEM, MBI_HOT, &rc);
				}
				else
				{
					if (!m_bParentActive ||  bDisabled)
						bis += 3;
					DrawThemeBackground(dc, MENU_BARITEM, bis, &rc);
				}

				CLogFont lf;
				HRESULT hr = GetThemeSysFont(TMT_MENUFONT, &lf);
				CFont hFont = (hr == S_OK) ? lf.CreateFontIndirect() : GetFont();
				HFONT hFontOld = NULL;
				if(hFont != NULL)
					hFontOld = dc.SelectFont(hFont);

				CTempBuffer<WCHAR> szText(200);
				TBBUTTONINFO tbbi = {sizeof(TBBUTTONINFO), TBIF_TEXT};
				tbbi.pszText = szText;
				tbbi.cchText = 200;
				GetButtonInfo((int)lpTBCustomDraw->nmcd.dwItemSpec, &tbbi);
				
				hr = DrawPartText(dc, MENU_BARITEM, bis, szText, &rc, 
					DT_SINGLELINE | DT_CENTER | DT_VCENTER | (m_bShowKeyboardCues ? 0 : DT_HIDEPREFIX));

				if(hFont != NULL)
					dc.SelectFont(hFontOld);
				lRet = CDRF_SKIPDEFAULT;
				bHandled = TRUE;
			}
		}
		return lRet;
	}

};
#endif // __ATLCTRLW_H__

#ifdef __ATLPRINT_H__

///////////////////////////////////////////////////////////////////////////////
// aero::CPrintPreviewWindow - Aero drawing WTL::CPrintPreviewWindow control

class CPrintPreviewWindow : public aero::CCtrlImpl<CPrintPreviewWindow, WTL::CPrintPreviewWindow>
{
public:
	DECLARE_WND_CLASS_EX(_T("WTL_AeroPrintPreview"), CS_VREDRAW | CS_HREDRAW, -1)

	void DoPaint(CDCHandle dc, RECT& rc)
	{
		RECT rcClient = { 0 };
		GetClientRect(&rcClient);
		RECT rcArea = rcClient;
		::InflateRect(&rcArea, -m_cxOffset, -m_cyOffset);
		if (rcArea.left > rcArea.right)
			rcArea.right = rcArea.left;
		if (rcArea.top > rcArea.bottom)
			rcArea.bottom = rcArea.top;
		GetPageRect(rcArea, &rc);
		if (!aero::IsComposing())
		{
			CRgn rgn1, rgn2;
			rgn1.CreateRectRgnIndirect(&rc);
			rgn2.CreateRectRgnIndirect(&rcClient);
			rgn2.CombineRgn(rgn1, RGN_DIFF);
			dc.SelectClipRgn(rgn2);
			if (IsTheming())
				DrawThemeBackground(dc, WP_FRAMEBOTTOM, m_hWnd == GetFocus() ? FS_ACTIVE : FS_INACTIVE, 
					&rcClient);
			else
				dc.FillRect(&rcClient, ::GetSysColor(COLOR_BTNSHADOW));
			dc.SelectClipRgn(NULL);
		}
		dc.FillRect(&rc, (HBRUSH)::GetStockObject(WHITE_BRUSH));

		WTL::CPrintPreviewWindow::DoPaint(dc, rc);
		if( !m_BufferedPaint.IsNull() )
			m_BufferedPaint.MakeOpaque(&rc);
	}

};

#endif // __ATLPRINT_H__

//#ifdef __ATLCTRLX_H__

///////////////////////////////////////////////////////////////////////////////
// aero::CTabView - Aero drawing WTL::CTabView control

class CTabView : 
	public CCtrlImpl<CTabView, WTL::CTabViewImpl<CTabView> >
{
public:
	DECLARE_WND_CLASS_EX(_T("WTL_AeroTabView"), 0, COLOR_APPWORKSPACE)

	aero::CTabCtrl m_Atab;

	bool CreateTabControl()
	{
		if (aero::IsSupported())
		{
			m_Atab.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TCS_TOOLTIPS, 0, m_nTabID);
			m_tab.SubclassWindow(m_Atab);
		}
		else
			m_tab.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TCS_TOOLTIPS, 0, m_nTabID);

		ATLASSERT(m_tab.m_hWnd != NULL);
		if(m_tab.m_hWnd == NULL)
			return false;

		m_tab.SetFont(AtlGetDefaultGuiFont());
		m_tab.SetItemExtra(sizeof(TABVIEWPAGE));
		m_cyTabHeight = CalcTabHeight();

		return true;
	}

	void GetMoveMarkRect(RECT& rect) const
	{
		m_tab.GetClientRect(&rect);

		RECT rcItem = { 0 };
		m_tab.GetItemRect(m_nInsertItem, &rcItem);

		int cxMoveMark = IsComposing() ? 0 : m_cxMoveMark / 2;

		if(m_nInsertItem <= m_nActivePage)
		{
			rect.left = rcItem.left - cxMoveMark - 1;
			rect.right = rcItem.left + cxMoveMark + 1;
		}
		else
		{
			rect.left = rcItem.right - cxMoveMark - 1;
			rect.right = rcItem.right + cxMoveMark + 1;
		}
	}

	void DrawMoveMark(int nItem)
	{
		if (!IsComposing())
			return WTL::CTabViewImpl<CTabView>::DrawMoveMark(nItem);

		RECT rect = { 0 };
		if(m_nInsertItem != -1)
		{
			GetMoveMarkRect(rect);
			m_tab.InvalidateRect(&rect);
		}

		m_nInsertItem = nItem;

		if(m_nInsertItem != -1)
		{
			GetMoveMarkRect(rect);

			CClientDC dcTab(m_tab.m_hWnd);
			if( IsComposing() )
			{
				HDC hDCPaint = NULL;
				m_BufferedPaint.Begin(dcTab, &rect, m_dwFormat, &m_PaintParams, &hDCPaint);
				ATLASSERT(hDCPaint != NULL);
				CDC dc(hDCPaint);

				CPen pen;
				pen.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_HOTLIGHT));
				CBrush brush;
				brush.CreateSolidBrush(::GetSysColor(COLOR_HOTLIGHT));

				HPEN hPenOld = dc.SelectPen(pen);
				HBRUSH hBrushOld = dc.SelectBrush(brush);

				dc.Rectangle(&rect);

				dc.SelectPen(hPenOld);
				dc.SelectBrush(hBrushOld);
				if( !m_BufferedPaint.IsNull() )
					m_BufferedPaint.MakeOpaque(&rect);
				m_BufferedPaint.End();
			}else
			{
				CDC dc(dcTab.m_hDC);

				CPen pen;
				pen.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_HOTLIGHT));
				CBrush brush;
				brush.CreateSolidBrush(::GetSysColor(COLOR_HOTLIGHT));

				HPEN hPenOld = dc.SelectPen(pen);
				HBRUSH hBrushOld = dc.SelectBrush(brush);

				dc.Rectangle(&rect);

				dc.SelectPen(hPenOld);
				dc.SelectBrush(hBrushOld);

			}
		}
	}

	void DoPaint(HDC hdc, RECT& rDest)
	{
		if (!IsComposing())
		{
			RECT rClient;
			GetClientRect(&rClient);
			if (IsTheming())
				DrawThemeBackground(hdc, WP_FRAMEBOTTOM, FS_INACTIVE, &rClient, &rDest);
			else
			{
				DefWindowProc(WM_ERASEBKGND, (WPARAM)hdc, NULL);
				if (!m_BufferedPaint.IsNull())
					m_BufferedPaint.MakeOpaque(&rDest);
			}
		}

		DefWindowProc(WM_PAINT, (WPARAM)hdc, NULL);
	}

// Message map and handlers
	BEGIN_MSG_MAP(CTabView)
		CHAIN_MSG_MAP(CAeroImpl<CTabView>)
		CHAIN_MSG_MAP(WTL::CTabViewImpl<CTabView>)
	ALT_MSG_MAP(1)   // tab control
		CHAIN_MSG_MAP_ALT(WTL::CTabViewImpl<CTabView>, 1)
	END_MSG_MAP()

};

///////////////////////////////////////////////////////////////////////////////
// aero::CPaneContainer - Aero drawing WTL::CPaneContainer control

class CPaneContainer : 
	public CCtrlImpl<CPaneContainer, WTL::CPaneContainerImpl<CPaneContainer> >
{
	// not using aero::CToolBarCtrl
	class CPaneToolBar : public WTL::CToolBarCtrl
	{};
	CCtrl<CPaneToolBar> m_Atb;

public:
	void CreateCloseButton()
	{
		WTL::CPaneContainerImpl<CPaneContainer>::CreateCloseButton();
		aero::Subclass(m_Atb, m_tb.m_hWnd);
	}

	void DrawPaneTitle(CDCHandle dc)
	{
		WTL::CPaneContainerImpl<CPaneContainer>::DrawPaneTitle(dc);

		if (IsComposing() && !m_BufferedPaint.IsNull())
		{
			RECT rect = { 0 };
			GetClientRect(&rect);
			if(IsVertical())
				rect.right = rect.left + m_cxyHeader;
			else
				rect.bottom = rect.top + m_cxyHeader;

			if( !m_BufferedPaint.IsNull() )
				m_BufferedPaint.MakeOpaque(&rect);
		}
	}

	// called only if pane is empty
	void DrawPane(CDCHandle dc)
	{
		RECT rect = { 0 };
		GetClientRect(&rect);
		if(IsVertical())
			rect.left += m_cxyHeader;
		else
			rect.top += m_cxyHeader;

		if((GetExStyle() & WS_EX_CLIENTEDGE) == 0)
				dc.DrawEdge(&rect, EDGE_SUNKEN, BF_RECT | BF_ADJUST);
		if (!IsComposing())
				dc.FillRect(&rect, COLOR_APPWORKSPACE);
	}

};

//#endif // __ATLCTRLX_H__

#ifdef __ATLSPLIT_H__

///////////////////////////////////////////////////////////////////////////////
// aero::CSplitterImpl<T, t_bVertical> - Provides Aero drawing splitter support to any window

// Aero splitter extended style
#define SPLIT_NONTRANSPARENT		0x00000008

template <class T, bool t_bVertical = true>
class CSplitterImpl : 
	public WTL::CSplitterImpl<T, t_bVertical>
{
public:
	typedef WTL::CSplitterImpl<T, t_bVertical> baseSplit;

	// called only if pane is empty
	void DrawSplitterPane(CDCHandle dc, int nPane)
	{
		T* pT = static_cast<T*>(this);
		RECT rect;
		if(GetSplitterPaneRect(nPane, &rect))
		{
			if((pT->GetExStyle() & WS_EX_CLIENTEDGE) == 0)
				dc.DrawEdge(&rect, EDGE_SUNKEN, BF_RECT | BF_ADJUST);

			if (!aero::IsComposing())
				dc.FillRect(&rect, COLOR_APPWORKSPACE);
		}
	}

	void DrawSplitterBar(CDCHandle dc)
	{
		T* pT = static_cast<T*>(this);
		if (aero::IsComposing())
		{
			if (GetSplitterExtendedStyle() & SPLIT_NONTRANSPARENT)
			{
				RECT rect;
				GetSplitterBarRect(&rect);
				pT->DrawThemeBackground(dc, WP_FRAMEBOTTOM, pT->m_hWnd == GetFocus() ? FS_ACTIVE : FS_INACTIVE, &rect) ;
			}
		}
		else
			baseSplit::DrawSplitterBar(dc);
	}
};

///////////////////////////////////////////////////////////////////////////////
// aero::CSplitterWindowImpl - Implements an Aero drawing splitter window

template <class T, bool t_bVertical = true, class TBase = ATL::CWindow, class TWinTraits = ATL::CControlWinTraits>
class ATL_NO_VTABLE CSplitterWindowImpl : 
	public aero::CWindowImpl<T, TBase, TWinTraits>, 
	public aero::CSplitterImpl<T, t_bVertical>
{
public:
	typedef aero::CWindowImpl<T, TBase, TWinTraits> baseClass;

	DECLARE_WND_CLASS_EX(NULL, CS_DBLCLKS, COLOR_WINDOW)

	void DoPaint(HDC hdc, RECT&)
	{
		if(m_nSinglePane == SPLIT_PANE_NONE && m_xySplitterPos == -1)
			SetSplitterPos();
		DrawSplitter(hdc);
	}

	BEGIN_MSG_MAP(CSplitterWindowImpl)
		CHAIN_MSG_MAP(baseClass)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		CHAIN_MSG_MAP(baseSplit)
		FORWARD_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(wParam != SIZE_MINIMIZED)
			SetSplitterRect();

		return bHandled = FALSE;
	}
};

///////////////////////////////////////////////////////////////////////////////
// aero::CSplitterWindow - Implements an Aero drawing splitter window to be used as is

template <bool t_bVertical = true>
class CSplitterWindowT : public aero::CSplitterWindowImpl<CSplitterWindowT<t_bVertical>, t_bVertical>
{
public:
	DECLARE_WND_CLASS_EX(_T("WTL_AeroSplitterWindow"), CS_DBLCLKS, COLOR_WINDOW)
};

typedef CSplitterWindowT<true>    CSplitterWindow;
typedef CSplitterWindowT<false>   CHorSplitterWindow;



template <class T, class TBase = ATL::CWindow>
class ATL_NO_VTABLE CAeroHyperLinkImpl : 
	public CCtrl<TBase>
{
	typedef CCtrl<TBase> baseWindow;

public:
	LPTSTR m_lpstrLabel;
	LPTSTR m_lpstrHyperLink;

	HCURSOR m_hCursor;
	HFONT m_hFont;
	HFONT m_hFontNormal;

	RECT m_rcLink;
#ifndef _WIN32_WCE
	CToolTipCtrl m_tip;
#endif // !_WIN32_WCE

	COLORREF m_clrLink;
	COLORREF m_clrVisited;

	DWORD m_dwExtendedStyle;   // Hyper Link specific extended styles

	bool m_bPaintLabel:1;
	bool m_bVisited:1;
	bool m_bHover:1;
	bool m_bInternalLinkFont:1;


// Constructor/Destructor
	CAeroHyperLinkImpl(DWORD dwExtendedStyle = HLINK_UNDERLINED) : 
			m_lpstrLabel(NULL), m_lpstrHyperLink(NULL),
			m_hCursor(NULL), m_hFont(NULL), m_hFontNormal(NULL),
			m_clrLink(RGB(0, 0, 255)), m_clrVisited(RGB(128, 0, 128)),
			m_dwExtendedStyle(dwExtendedStyle),
			m_bPaintLabel(true), m_bVisited(false),
			m_bHover(false), m_bInternalLinkFont(false),
			baseWindow( _T("static::globals") )
	{
		::SetRectEmpty(&m_rcLink);
	}

	~CAeroHyperLinkImpl()
	{
		delete [] m_lpstrLabel;
		delete [] m_lpstrHyperLink;
		if(m_bInternalLinkFont && m_hFont != NULL)
			::DeleteObject(m_hFont);
#if (WINVER < 0x0500) && !defined(_WIN32_WCE)
		// It was created, not loaded, so we have to destroy it
		if(m_hCursor != NULL)
			::DestroyCursor(m_hCursor);
#endif // (WINVER < 0x0500) && !defined(_WIN32_WCE)
	}

// Attributes
	DWORD GetHyperLinkExtendedStyle() const
	{
		return m_dwExtendedStyle;
	}

	DWORD SetHyperLinkExtendedStyle(DWORD dwExtendedStyle, DWORD dwMask = 0)
	{
		DWORD dwPrevStyle = m_dwExtendedStyle;
		if(dwMask == 0)
			m_dwExtendedStyle = dwExtendedStyle;
		else
			m_dwExtendedStyle = (m_dwExtendedStyle & ~dwMask) | (dwExtendedStyle & dwMask);
		return dwPrevStyle;
	}

	bool GetLabel(LPTSTR lpstrBuffer, int nLength) const
	{
		if(m_lpstrLabel == NULL)
			return false;
		ATLASSERT(lpstrBuffer != NULL);
		if(nLength <= lstrlen(m_lpstrLabel))
			return false;

		SecureHelper::strcpy_x(lpstrBuffer, nLength, m_lpstrLabel);

		return true;
	}

	bool SetLabel(LPCTSTR lpstrLabel)
	{
		delete [] m_lpstrLabel;
		m_lpstrLabel = NULL;
		int cchLen = lstrlen(lpstrLabel) + 1;
		ATLTRY(m_lpstrLabel = new TCHAR[cchLen]);
		if(m_lpstrLabel == NULL)
			return false;

		SecureHelper::strcpy_x(m_lpstrLabel, cchLen, lpstrLabel);
		T* pT = static_cast<T*>(this);
		pT->CalcLabelRect();

		if(m_hWnd != NULL)
			SetWindowText(lpstrLabel);   // Set this for accessibility

		return true;
	}

	bool GetHyperLink(LPTSTR lpstrBuffer, int nLength) const
	{
		if(m_lpstrHyperLink == NULL)
			return false;
		ATLASSERT(lpstrBuffer != NULL);
		if(nLength <= lstrlen(m_lpstrHyperLink))
			return false;

		SecureHelper::strcpy_x(lpstrBuffer, nLength, m_lpstrHyperLink);

		return true;
	}

	bool SetHyperLink(LPCTSTR lpstrLink)
	{
		delete [] m_lpstrHyperLink;
		m_lpstrHyperLink = NULL;
		int cchLen = lstrlen(lpstrLink) + 1;
		ATLTRY(m_lpstrHyperLink = new TCHAR[cchLen]);
		if(m_lpstrHyperLink == NULL)
			return false;

		SecureHelper::strcpy_x(m_lpstrHyperLink, cchLen, lpstrLink);
		if(m_lpstrLabel == NULL)
		{
			T* pT = static_cast<T*>(this);
			pT->CalcLabelRect();
		}
#ifndef _WIN32_WCE
		if(m_tip.IsWindow())
		{
			m_tip.Activate(TRUE);
			m_tip.AddTool(m_hWnd, m_lpstrHyperLink, &m_rcLink, 1);
		}
#endif // !_WIN32_WCE
		return true;
	}

	HFONT GetLinkFont() const
	{
		return m_hFont;
	}

	void SetLinkFont(HFONT hFont)
	{
		if(m_bInternalLinkFont && m_hFont != NULL)
		{
			::DeleteObject(m_hFont);
			m_bInternalLinkFont = false;
		}
		m_hFont = hFont;
	}

	int GetIdealHeight() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		if(m_lpstrLabel == NULL && m_lpstrHyperLink == NULL)
			return -1;
		if(!m_bPaintLabel)
			return -1;

		CClientDC dc(m_hWnd);
		RECT rect = { 0 };
		GetClientRect(&rect);
		HFONT hFontOld = dc.SelectFont(m_hFontNormal);
		RECT rcText = rect;
		dc.DrawText(_T("NS"), -1, &rcText, DT_LEFT | DT_WORDBREAK | DT_CALCRECT);
		dc.SelectFont(m_hFont);
		RECT rcLink = rect;
		dc.DrawText(_T("NS"), -1, &rcLink, DT_LEFT | DT_WORDBREAK | DT_CALCRECT);
		dc.SelectFont(hFontOld);
		return max(rcText.bottom - rcText.top, rcLink.bottom - rcLink.top);
	}

	bool GetIdealSize(SIZE& size) const
	{
		int cx = 0, cy = 0;
		bool bRet = GetIdealSize(cx, cy);
		if(bRet)
		{
			size.cx = cx;
			size.cy = cy;
		}
		return bRet;
	}

	bool GetIdealSize(int& cx, int& cy) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		if(m_lpstrLabel == NULL && m_lpstrHyperLink == NULL)
			return false;
		if(!m_bPaintLabel)
			return false;

		CClientDC dc(m_hWnd);
		RECT rcClient = { 0 };
		GetClientRect(&rcClient);
		RECT rcAll = rcClient;

		if(IsUsingTags())
		{
			// find tags and label parts
			LPTSTR lpstrLeft = NULL;
			int cchLeft = 0;
			LPTSTR lpstrLink = NULL;
			int cchLink = 0;
			LPTSTR lpstrRight = NULL;
			int cchRight = 0;

			const T* pT = static_cast<const T*>(this);
			pT->CalcLabelParts(lpstrLeft, cchLeft, lpstrLink, cchLink, lpstrRight, cchRight);

			// get label part rects
			HFONT hFontOld = dc.SelectFont(m_hFontNormal);
			RECT rcLeft = rcClient;
			dc.DrawText(lpstrLeft, cchLeft, &rcLeft, DT_LEFT | DT_WORDBREAK | DT_CALCRECT);

			dc.SelectFont(m_hFont);
			RECT rcLink = { rcLeft.right, rcLeft.top, rcClient.right, rcClient.bottom };
			dc.DrawText(lpstrLink, cchLink, &rcLink, DT_LEFT | DT_WORDBREAK | DT_CALCRECT);

			dc.SelectFont(m_hFontNormal);
			RECT rcRight = { rcLink.right, rcLink.top, rcClient.right, rcClient.bottom };
			dc.DrawText(lpstrRight, cchRight, &rcRight, DT_LEFT | DT_WORDBREAK | DT_CALCRECT);

			dc.SelectFont(hFontOld);

			int cyMax = max(rcLeft.bottom, max(rcLink.bottom, rcRight.bottom));
			::SetRect(&rcAll, rcLeft.left, rcLeft.top, rcRight.right, cyMax);
		}
		else
		{
			HFONT hOldFont = NULL;
			if(m_hFont != NULL)
				hOldFont = dc.SelectFont(m_hFont);
			LPTSTR lpstrText = (m_lpstrLabel != NULL) ? m_lpstrLabel : m_lpstrHyperLink;
			DWORD dwStyle = GetStyle();
			int nDrawStyle = DT_LEFT;
			if (dwStyle & SS_CENTER)
				nDrawStyle = DT_CENTER;
			else if (dwStyle & SS_RIGHT)
				nDrawStyle = DT_RIGHT;
			dc.DrawText(lpstrText, -1, &rcAll, nDrawStyle | DT_WORDBREAK | DT_CALCRECT);
			if(m_hFont != NULL)
				dc.SelectFont(hOldFont);
			if (dwStyle & SS_CENTER)
			{
				int dx = (rcClient.right - rcAll.right) / 2;
				::OffsetRect(&rcAll, dx, 0);
			}
			else if (dwStyle & SS_RIGHT)
			{
				int dx = rcClient.right - rcAll.right;
				::OffsetRect(&rcAll, dx, 0);
			}
		}

		cx = rcAll.right - rcAll.left;
		cy = rcAll.bottom - rcAll.top;

		return true;
	}

	// for command buttons only
	bool GetToolTipText(LPTSTR lpstrBuffer, int nLength) const
	{
		ATLASSERT(IsCommandButton());
		return GetHyperLink(lpstrBuffer, nLength);
	}

	bool SetToolTipText(LPCTSTR lpstrToolTipText)
	{
		ATLASSERT(IsCommandButton());
		return SetHyperLink(lpstrToolTipText);
	}

// Operations
	BOOL SubclassWindow(HWND hWnd)
	{
		ATLASSERT(m_hWnd == NULL);
		ATLASSERT(::IsWindow(hWnd));
		typedef CCtrl<TBase>   _baseClass;
		BOOL bRet = _baseClass::SubclassWindow(hWnd);
		if(bRet)
		{
			T* pT = static_cast<T*>(this);
			pT->Init();
			//OpenThemeData();
		}
		return bRet;
	}

	bool Navigate()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		bool bRet = true;
		if(IsNotifyButton())
		{
			NMHDR nmhdr = { m_hWnd, GetDlgCtrlID(), NM_CLICK };
			::SendMessage(GetParent(), WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&nmhdr);
		}
		else if(IsCommandButton())
		{
			::SendMessage(GetParent(), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), BN_CLICKED), (LPARAM)m_hWnd);
		}
		else
		{
			ATLASSERT(m_lpstrHyperLink != NULL);
#ifndef _WIN32_WCE
			DWORD_PTR dwRet = (DWORD_PTR)::ShellExecute(0, _T("open"), m_lpstrHyperLink, 0, 0, SW_SHOWNORMAL);
			bRet = (dwRet > 32);
#else // CE specific
			SHELLEXECUTEINFO shExeInfo = { sizeof(SHELLEXECUTEINFO), 0, 0, L"open", m_lpstrHyperLink, 0, 0, SW_SHOWNORMAL, 0, 0, 0, 0, 0, 0, 0 };
			::ShellExecuteEx(&shExeInfo);
			DWORD_PTR dwRet = (DWORD_PTR)shExeInfo.hInstApp;
			bRet = (dwRet == 0) || (dwRet > 32);
#endif // _WIN32_WCE
			ATLASSERT(bRet);
			if(bRet)
			{
				m_bVisited = true;
				Invalidate();
			}
		}
		return bRet;
	}

// Message map and handlers
	BEGIN_MSG_MAP(CAeroHyperLinkImpl)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
#ifndef _WIN32_WCE
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_RANGE_HANDLER(WM_MOUSEFIRST, WM_MOUSELAST, OnMouseMessage)
#endif // !_WIN32_WCE
		//MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		//MESSAGE_HANDLER(WM_PAINT, OnPaint)
#ifndef _WIN32_WCE
		//MESSAGE_HANDLER(WM_PRINTCLIENT, OnPaint)
#endif // !_WIN32_WCE
		MESSAGE_HANDLER(WM_SETFOCUS, OnFocus)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnFocus)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
#ifndef _WIN32_WCE
		MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
#endif // !_WIN32_WCE
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
		MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
		MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
		MESSAGE_HANDLER(WM_ENABLE, OnEnable)
		MESSAGE_HANDLER(WM_GETFONT, OnGetFont)
		MESSAGE_HANDLER(WM_SETFONT, OnSetFont)
		MESSAGE_HANDLER(WM_UPDATEUISTATE, OnUpdateUiState)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		CHAIN_MSG_MAP(baseWindow)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		T* pT = static_cast<T*>(this);
		pT->Init();
		OpenThemeData();
		return 0;
	}

#ifndef _WIN32_WCE
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(m_tip.IsWindow())
		{
			m_tip.DestroyWindow();
			m_tip.m_hWnd = NULL;
		}
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnMouseMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		MSG msg = { m_hWnd, uMsg, wParam, lParam };
		if(m_tip.IsWindow() && IsUsingToolTip())
			m_tip.RelayEvent(&msg);
		bHandled = FALSE;
		return 1;
	}
#endif // !_WIN32_WCE

	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return 1;   // no background painting needed (we do it all during WM_PAINT)
	}

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		RECT rcClient = { 0 };
		GetClientRect(&rcClient);
		if(!m_bPaintLabel)
		{
			bHandled = FALSE;
			return 1;
		}


		

		T* pT = static_cast<T*>(this);
		if(wParam != NULL)
		{
			DefCtrlPaint( (HDC)wParam, rcClient, TRUE );
			pT->DoEraseBackground((HDC)wParam);
			pT->DoPaint((HDC)wParam, rcClient);
		}
		else
		{
			CPaintDC dc(m_hWnd);
			pT->DoEraseBackground(dc.m_hDC);
			DefCtrlPaint( dc.m_hDC, rcClient, TRUE );
			pT->DoPaint(dc.m_hDC, rcClient);
		}

		return 0;
	}

	LRESULT OnFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(m_bPaintLabel)
			Invalidate();
		else
			bHandled = FALSE;
		return 0;
	}

	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		if((m_lpstrHyperLink != NULL  || IsCommandButton()) && ::PtInRect(&m_rcLink, pt))
		{
			::SetCursor(m_hCursor);
			if(IsUnderlineHover())
			{
				if(!m_bHover)
				{
					m_bHover = true;
					InvalidateRect(&m_rcLink);
					UpdateWindow();
#ifndef _WIN32_WCE
					StartTrackMouseLeave();
#endif // !_WIN32_WCE
				}
			}
		}
		else
		{
			if(IsUnderlineHover())
			{
				if(m_bHover)
				{
					m_bHover = false;
					InvalidateRect(&m_rcLink);
					UpdateWindow();
				}
			}
			bHandled = FALSE;
		}
		return 0;
	}

#ifndef _WIN32_WCE
	LRESULT OnMouseLeave(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		if(IsUnderlineHover() && m_bHover)
		{
			m_bHover = false;
			InvalidateRect(&m_rcLink);
			UpdateWindow();
		}
		return 0;
	}
#endif // !_WIN32_WCE

	LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		if(::PtInRect(&m_rcLink, pt))
		{
			SetFocus();
			SetCapture();
		}
		return 0;
	}

	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		if(GetCapture() == m_hWnd)
		{
			ReleaseCapture();
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			if(::PtInRect(&m_rcLink, pt))
			{
				T* pT = static_cast<T*>(this);
				pT->Navigate();
			}
		}
		return 0;
	}

	LRESULT OnChar(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		if(wParam == VK_RETURN || wParam == VK_SPACE)
		{
			T* pT = static_cast<T*>(this);
			pT->Navigate();
		}
		return 0;
	}

	LRESULT OnGetDlgCode(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return DLGC_WANTCHARS;
	}

	LRESULT OnSetCursor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		POINT pt = { 0, 0 };
		GetCursorPos(&pt);
		ScreenToClient(&pt);
		if((m_lpstrHyperLink != NULL  || IsCommandButton()) && ::PtInRect(&m_rcLink, pt))
		{
			return TRUE;
		}
		bHandled = FALSE;
		return FALSE;
	}

	LRESULT OnEnable(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		Invalidate();
		UpdateWindow();
		return 0;
	}

	LRESULT OnGetFont(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return (LRESULT)m_hFontNormal;
	}

	LRESULT OnSetFont(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		m_hFontNormal = (HFONT)wParam;
		if((BOOL)lParam)
		{
			Invalidate();
			UpdateWindow();
		}
		return 0;
	}

	LRESULT OnUpdateUiState(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// If the control is subclassed or superclassed, this message can cause
		// repainting without WM_PAINT. We don't use this state, so just do nothing.
		return 0;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		T* pT = static_cast<T*>(this);
		pT->CalcLabelRect();
		pT->Invalidate();
		return 0;
	}

// Implementation
	void Init()
	{
		ATLASSERT(::IsWindow(m_hWnd));

		// Check if we should paint a label
		const int cchBuff = 8;
		TCHAR szBuffer[cchBuff] = { 0 };
		if(::GetClassName(m_hWnd, szBuffer, cchBuff))
		{
			if(lstrcmpi(szBuffer, _T("static")) == 0)
			{
				ModifyStyle(0, SS_NOTIFY);   // we need this
				DWORD dwStyle = GetStyle() & 0x000000FF;
#ifndef _WIN32_WCE
				if(dwStyle == SS_ICON || dwStyle == SS_BLACKRECT || dwStyle == SS_GRAYRECT || 
						dwStyle == SS_WHITERECT || dwStyle == SS_BLACKFRAME || dwStyle == SS_GRAYFRAME || 
						dwStyle == SS_WHITEFRAME || dwStyle == SS_OWNERDRAW || 
						dwStyle == SS_BITMAP || dwStyle == SS_ENHMETAFILE)
#else // CE specific
				if(dwStyle == SS_ICON || dwStyle == SS_BITMAP)
#endif // _WIN32_WCE
					m_bPaintLabel = false;
			}
		}

		// create or load a cursor
#if (WINVER >= 0x0500) || defined(_WIN32_WCE)
		m_hCursor = ::LoadCursor(NULL, IDC_HAND);
#else
		m_hCursor = ::CreateCursor(ModuleHelper::GetModuleInstance(), _AtlHyperLink_CursorData.xHotSpot, _AtlHyperLink_CursorData.yHotSpot, _AtlHyperLink_CursorData.cxWidth, _AtlHyperLink_CursorData.cyHeight, _AtlHyperLink_CursorData.arrANDPlane, _AtlHyperLink_CursorData.arrXORPlane);
#endif
		ATLASSERT(m_hCursor != NULL);

		// set font
		if(m_bPaintLabel)
		{
			ATL::CWindow wnd = GetParent();
			m_hFontNormal = wnd.GetFont();
			if(m_hFontNormal == NULL)
				m_hFontNormal = (HFONT)::GetStockObject(SYSTEM_FONT);
			if(m_hFontNormal != NULL && m_hFont == NULL)
			{
				LOGFONT lf = { 0 };
				CFontHandle font = m_hFontNormal;
				font.GetLogFont(&lf);
				if(IsUsingTagsBold())
					lf.lfWeight = FW_BOLD;
				else if(!IsNotUnderlined())
					lf.lfUnderline = TRUE;
				m_hFont = ::CreateFontIndirect(&lf);
				m_bInternalLinkFont = true;
				ATLASSERT(m_hFont != NULL);
			}
		}

#ifndef _WIN32_WCE
		// create a tool tip
		m_tip.Create(m_hWnd);
		ATLASSERT(m_tip.IsWindow());
#endif // !_WIN32_WCE

		// set label (defaults to window text)
		if(m_lpstrLabel == NULL)
		{
			int nLen = GetWindowTextLength();
			if(nLen > 0)
			{
				CTempBuffer<TCHAR, _WTL_STACK_ALLOC_THRESHOLD> buff;
				LPTSTR lpstrText = buff.Allocate(nLen + 1);
				ATLASSERT(lpstrText != NULL);
				if((lpstrText != NULL) && (GetWindowText(lpstrText, nLen + 1) > 0))
					SetLabel(lpstrText);
			}
		}

		T* pT = static_cast<T*>(this);
		pT->CalcLabelRect();

		// set hyperlink (defaults to label), or just activate tool tip if already set
		if(m_lpstrHyperLink == NULL && !IsCommandButton())
		{
			if(m_lpstrLabel != NULL)
				SetHyperLink(m_lpstrLabel);
		}
#ifndef _WIN32_WCE
		else
		{
			m_tip.Activate(TRUE);
			m_tip.AddTool(m_hWnd, m_lpstrHyperLink, &m_rcLink, 1);
		}
#endif // !_WIN32_WCE

		// set link colors
		if(m_bPaintLabel)
		{
			ATL::CRegKey rk;
			LONG lRet = rk.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Internet Explorer\\Settings"));
			if(lRet == 0)
			{
				const int cchValue = 12;
				TCHAR szValue[cchValue] = { 0 };
#if (_ATL_VER >= 0x0700)
				ULONG ulCount = cchValue;
				lRet = rk.QueryStringValue(_T("Anchor Color"), szValue, &ulCount);
#else
				DWORD dwCount = cchValue * sizeof(TCHAR);
				lRet = rk.QueryValue(szValue, _T("Anchor Color"), &dwCount);
#endif
				if(lRet == 0)
				{
					COLORREF clr = pT->_ParseColorString(szValue);
					ATLASSERT(clr != CLR_INVALID);
					if(clr != CLR_INVALID)
						m_clrLink = clr;
				}

#if (_ATL_VER >= 0x0700)
				ulCount = cchValue;
				lRet = rk.QueryStringValue(_T("Anchor Color Visited"), szValue, &ulCount);
#else
				dwCount = cchValue * sizeof(TCHAR);
				lRet = rk.QueryValue(szValue, _T("Anchor Color Visited"), &dwCount);
#endif
				if(lRet == 0)
				{
					COLORREF clr = pT->_ParseColorString(szValue);
					ATLASSERT(clr != CLR_INVALID);
					if(clr != CLR_INVALID)
						m_clrVisited = clr;
				}
			}
		}
	}

	static COLORREF _ParseColorString(LPTSTR lpstr)
	{
		int c[3] = { -1, -1, -1 };
		LPTSTR p = NULL;
		for(int i = 0; i < 2; i++)
		{
			for(p = lpstr; *p != _T('\0'); p = ::CharNext(p))
			{
				if(*p == _T(','))
				{
					*p = _T('\0');
					c[i] = T::_xttoi(lpstr);
					lpstr = &p[1];
					break;
				}
			}
			if(c[i] == -1)
				return CLR_INVALID;
		}
		if(*lpstr == _T('\0'))
			return CLR_INVALID;
		c[2] = T::_xttoi(lpstr);

		return RGB(c[0], c[1], c[2]);
	}

	bool CalcLabelRect()
	{
		if(!::IsWindow(m_hWnd))
			return false;
		if(m_lpstrLabel == NULL && m_lpstrHyperLink == NULL)
			return false;

		CClientDC dc(m_hWnd);
		RECT rcClient = { 0 };
		GetClientRect(&rcClient);
		m_rcLink = rcClient;
		if(!m_bPaintLabel)
			return true;

		if(IsUsingTags())
		{
			// find tags and label parts
			LPTSTR lpstrLeft = NULL;
			int cchLeft = 0;
			LPTSTR lpstrLink = NULL;
			int cchLink = 0;
			LPTSTR lpstrRight = NULL;
			int cchRight = 0;

			T* pT = static_cast<T*>(this);
			pT->CalcLabelParts(lpstrLeft, cchLeft, lpstrLink, cchLink, lpstrRight, cchRight);
			ATLASSERT(lpstrLink != NULL);
			ATLASSERT(cchLink > 0);

			// get label part rects
			HFONT hFontOld = dc.SelectFont(m_hFontNormal);

			RECT rcLeft = rcClient;
			if(lpstrLeft != NULL)
				dc.DrawText(lpstrLeft, cchLeft, &rcLeft, DT_LEFT | DT_WORDBREAK | DT_CALCRECT);

			dc.SelectFont(m_hFont);
			RECT rcLink = rcClient;
			if(lpstrLeft != NULL)
				rcLink.left = rcLeft.right;
			dc.DrawText(lpstrLink, cchLink, &rcLink, DT_LEFT | DT_WORDBREAK | DT_CALCRECT);

			dc.SelectFont(hFontOld);

			m_rcLink = rcLink;
		}
		else
		{
			HFONT hOldFont = NULL;
			if(m_hFont != NULL)
				hOldFont = dc.SelectFont(m_hFont);
			LPTSTR lpstrText = (m_lpstrLabel != NULL) ? m_lpstrLabel : m_lpstrHyperLink;
			DWORD dwStyle = GetStyle();
			int nDrawStyle = DT_LEFT;
			if (dwStyle & SS_CENTER)
				nDrawStyle = DT_CENTER;
			else if (dwStyle & SS_RIGHT)
				nDrawStyle = DT_RIGHT;
			dc.DrawText(lpstrText, -1, &m_rcLink, nDrawStyle | DT_WORDBREAK | DT_CALCRECT);
			if(m_hFont != NULL)
				dc.SelectFont(hOldFont);
			if (dwStyle & SS_CENTER)
			{
				int dx = (rcClient.right - m_rcLink.right) / 2;
				::OffsetRect(&m_rcLink, dx, 0);
			}
			else if (dwStyle & SS_RIGHT)
			{
				int dx = rcClient.right - m_rcLink.right;
				::OffsetRect(&m_rcLink, dx, 0);
			}
		}

		return true;
	}

	void CalcLabelParts(LPTSTR& lpstrLeft, int& cchLeft, LPTSTR& lpstrLink, int& cchLink, LPTSTR& lpstrRight, int& cchRight) const
	{
		lpstrLeft = NULL;
		cchLeft = 0;
		lpstrLink = NULL;
		cchLink = 0;
		lpstrRight = NULL;
		cchRight = 0;

		LPTSTR lpstrText = (m_lpstrLabel != NULL) ? m_lpstrLabel : m_lpstrHyperLink;
		int cchText = lstrlen(lpstrText);
		bool bOutsideLink = true;
		for(int i = 0; i < cchText; i++)
		{
			if(lpstrText[i] != _T('<'))
				continue;

			if(bOutsideLink)
			{
				if(::CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, &lpstrText[i], 3, _T("<A>"), 3) == CSTR_EQUAL)
				{
					if(i > 0)
					{
						lpstrLeft = lpstrText;
						cchLeft = i;
					}
					lpstrLink = &lpstrText[i + 3];
					bOutsideLink = false;
				}
			}
			else
			{
				if(::CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, &lpstrText[i], 4, _T("</A>"), 4) == CSTR_EQUAL)
				{
					cchLink = i - 3 - cchLeft;
					if(lpstrText[i + 4] != 0)
					{
						lpstrRight = &lpstrText[i + 4];
						cchRight = cchText - (i + 4);
						break;
					}
				}
			}
		}

	}

	void DoEraseBackground(CDCHandle dc)
	{
		HBRUSH hBrush = (HBRUSH)::SendMessage(GetParent(), WM_CTLCOLORSTATIC, (WPARAM)dc.m_hDC, (LPARAM)m_hWnd);
		if(hBrush != NULL)
		{
			RECT rect = { 0 };
			GetClientRect(&rect);
			dc.FillRect(&rect, hBrush);
			dc.FillSolidRect( &rect, RGB(0,0,0));
		}
		return;
	}

	virtual void CtrlPaint(HDC hdc, RECT& /*rCtrl*/, RECT& rPaint)
	{
		RECT rcClient = { 0 };
		GetClientRect(&rcClient);		
		T* pT = static_cast<T*>(this);
		//CPaintDC dc(m_hWnd);
		DefWindowProc();
		pT->DoEraseBackground(hdc);
		pT->DoPaint(hdc, rcClient);
		//DefCtrlPaint( dc.m_hDC, rPaint, false );
		if( !m_BufferedPaint.IsNull() )
		{
			//m_BufferedPaint.MakeOpaque(&rPaint);
		}
		//DefCtrlPaint(hdc, rPaint);
	}


	//void Paint(CDCHandle dc, RECT& rClient, RECT& rView, RECT& rDest)
	void DoPaint(CDCHandle dc, RECT& rDest)
	{
		if(IsUsingTags())
		{
			// find tags and label parts
			LPTSTR lpstrLeft = NULL;
			int cchLeft = 0;
			LPTSTR lpstrLink = NULL;
			int cchLink = 0;
			LPTSTR lpstrRight = NULL;
			int cchRight = 0;

			T* pT = static_cast<T*>(this);
			pT->CalcLabelParts(lpstrLeft, cchLeft, lpstrLink, cchLink, lpstrRight, cchRight);

			// get label part rects
			RECT rcClient = { 0 };
			GetClientRect(&rcClient);

			dc.SetBkMode(TRANSPARENT);
			HFONT hFontOld = dc.SelectFont(m_hFontNormal);

			if(lpstrLeft != NULL)
			{
				this->DrawText( dc, lpstrLeft,  &rcClient, DT_LEFT | DT_WORDBREAK);
				//dc.DrawText(lpstrLeft, cchLeft, &rcClient, DT_LEFT | DT_WORDBREAK);
			}

			COLORREF clrOld = dc.SetTextColor(IsWindowEnabled() ? (m_bVisited ? m_clrVisited : m_clrLink) : (::GetSysColor(COLOR_GRAYTEXT)));
			if(m_hFont != NULL && (!IsUnderlineHover() || (IsUnderlineHover() && m_bHover)))
				dc.SelectFont(m_hFont);
			else
				dc.SelectFont(m_hFontNormal);

			//DrawCtrlText(dc, int nPartID, int nStateID, UINT uFormat, rDest);

			this->DrawText( dc, lpstrLink,  &m_rcLink, DT_LEFT | DT_WORDBREAK);
			//dc.DrawText(lpstrLink, cchLink, &m_rcLink, DT_LEFT | DT_WORDBREAK);

			dc.SetTextColor(clrOld);
			dc.SelectFont(m_hFontNormal);
			if(lpstrRight != NULL)
			{
				RECT rcRight = { m_rcLink.right, m_rcLink.top, rcClient.right, rcClient.bottom };
				dc.DrawText(lpstrRight, cchRight, &rcRight, DT_LEFT | DT_WORDBREAK);
			}

			if(GetFocus() == m_hWnd)
				dc.DrawFocusRect(&m_rcLink);

			dc.SelectFont(hFontOld);
		}
		else
		{
			dc.SetBkMode(TRANSPARENT);

			COLORREF clrOld = dc.SetTextColor(IsWindowEnabled() ? (m_bVisited ? m_clrVisited : m_clrLink) : (::GetSysColor(COLOR_GRAYTEXT)));

			HFONT hFontOld = NULL;
			if(m_hFont != NULL && (!IsUnderlineHover() || (IsUnderlineHover() && m_bHover)))
				hFontOld = dc.SelectFont(m_hFont);
			else
				hFontOld = dc.SelectFont(m_hFontNormal);

			LPTSTR lpstrText = (m_lpstrLabel != NULL) ? m_lpstrLabel : m_lpstrHyperLink;

			DWORD dwStyle = GetStyle();
			int nDrawStyle = DT_LEFT;
			if (dwStyle & SS_CENTER)
				nDrawStyle = DT_CENTER;
			else if (dwStyle & SS_RIGHT)
				nDrawStyle = DT_RIGHT;

			//DrawPartText();
			//this->DrawText( dc, lpstrText,  &m_rcLink, nDrawStyle | DT_WORDBREAK | DT_SINGLELINE);
			DrawPartText(dc, TEXT_HYPERLINKTEXT, TS_HYPERLINK_NORMAL, lpstrText, &m_rcLink, nDrawStyle | DT_WORDBREAK | DT_SINGLELINE, 
				DTT_COMPOSITED, 0);
			/*HRESULT hr;
			HFONT hFont = NULL;
			RECT rText = {0};
			int PartsId = TEXT_HYPERLINKTEXT;
			int StateId = TS_HYPERLINK_NORMAL;
			hr = GetThemeBackgroundContentRect(dc, PartsId, StateId, &m_rcLink, &m_rcLink);
			MARGINS m = {0};
			hr = GetThemeMargins(dc, PartsId, StateId, TMT_CONTENTMARGINS, &m_rcLink, &m);
			rText.left += m.cxLeftWidth;
			rText.right -= m.cxRightWidth;
			int iLength = GetWindowTextLength();
			if (iLength > 0)
			{
				CTempBuffer<WCHAR> sText(++iLength);
				GetWindowText(sText, iLength);

				HFONT hf = dc.SelectFont(hFont == 0 ? GetFont() : hFont);
				hr = DrawPartText(dc, PartsId, StateId, sText,  &rText , DT_SINGLELINE, 0, 0);
				dc.SelectFont(hf);
			}*/
		//return SUCCEEDED(hr) && iLength > 0;

			//dc.DrawText(lpstrText, -1, &m_rcLink, nDrawStyle | DT_WORDBREAK);

			if(GetFocus() == m_hWnd)
				dc.DrawFocusRect(&m_rcLink);

			dc.SetTextColor(clrOld);
			dc.SelectFont(hFontOld);
		}
	}

#ifndef _WIN32_WCE
	BOOL StartTrackMouseLeave()
	{
		TRACKMOUSEEVENT tme = { 0 };
		tme.cbSize = sizeof(tme);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = m_hWnd;
		return _TrackMouseEvent(&tme);
	}
#endif // !_WIN32_WCE

// Implementation helpers
	bool IsUnderlined() const
	{
		return ((m_dwExtendedStyle & (HLINK_NOTUNDERLINED | HLINK_UNDERLINEHOVER)) == 0);
	}

	bool IsNotUnderlined() const
	{
		return ((m_dwExtendedStyle & HLINK_NOTUNDERLINED) != 0);
	}

	bool IsUnderlineHover() const
	{
		return ((m_dwExtendedStyle & HLINK_UNDERLINEHOVER) != 0);
	}

	bool IsCommandButton() const
	{
		return ((m_dwExtendedStyle & HLINK_COMMANDBUTTON) != 0);
	}

	bool IsNotifyButton() const
	{
		return ((m_dwExtendedStyle & HLINK_NOTIFYBUTTON) == HLINK_NOTIFYBUTTON);
	}

	bool IsUsingTags() const
	{
		return ((m_dwExtendedStyle & HLINK_USETAGS) != 0);
	}

	bool IsUsingTagsBold() const
	{
		return ((m_dwExtendedStyle & HLINK_USETAGSBOLD) == HLINK_USETAGSBOLD);
	}

	LPCWSTR GetThemeName()
	{
		return L"globals";
	};

	bool IsUsingToolTip() const
	{
		return ((m_dwExtendedStyle & HLINK_NOTOOLTIP) == 0);
	}

	static int _xttoi(const TCHAR* nptr)
	{
#ifndef _ATL_MIN_CRT
		return _ttoi(nptr);
#else // _ATL_MIN_CRT
		while(*nptr == _T(' '))   // skip spaces
			++nptr;

		int c = (int)(_TUCHAR)*nptr++;
		int sign = c;   // save sign indication
		if (c == _T('-') || c == _T('+'))
			c = (int)(_TUCHAR)*nptr++;   // skip sign

		int total = 0;
		while((TCHAR)c >= _T('0') && (TCHAR)c <= _T('9'))
		{
			total = 10 * total + ((TCHAR)c - _T('0'));   // accumulate digit
			c = (int)(_TUCHAR)*nptr++;        // get next char
		}

		// return result, negated if necessary
		return ((TCHAR)sign != _T('-')) ? total : -total;
#endif // _ATL_MIN_CRT
	}
};


//class CAeroHyperLink : public CCtrl<CStatic>
//{
//public:
//	DECLARE_WND_CLASS(_T("Aero_HyperLink"))
//};

class CAeroHyperLink : public  CAeroHyperLinkImpl<CAeroHyperLink> 
{
public:
	DECLARE_WND_CLASS(_T("Aero_HyperLink"))
};


#endif // __ATLSPLIT_H__

}; // namespace aero
}; // namespace WTL



#endif
