//
//			CBalloonHelp WTL version
//

// Copyright 2001, Joshua Heyer
// Copyright 2002, Maximilian Hänel (WTL version)
//  You are free to use this code for whatever you want, provided you
// give credit where credit is due.
//  I'm providing this code in the hope that it is useful to someone, as i have
// gotten much use out of other peoples code over the years.
//  If you see value in it, make some improvements, etc., i would appreciate it 
// if you sent me some feedback.

//
// Ported to WTL by Maximilian Hänel (max_haenel@smjh.de) 2002
//
// Revision History:
//
// 28.05.2002:	Version 1.0 (Maximilian Hänel)
//				Initial Version (WTL port)
//
#ifndef __XPBALLOONDLG__H__
#define __XPBALLOONDLG__H__

#pragma once

//
// Don't forget to add BalloonHelp.cpp to your project :-)
//

#ifndef __cplusplus
	#error WTL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLWIN_H__
	#error BalloonHelp.H requires atlwin.h to be included first
#endif

#ifndef __ATLMISC_H__
	#error BalloonHelp.H requires atlmisc.h to be included first
#endif

#ifndef __ATLCRACK_H__
	#error BalloonHelp.H requires atlcrack.h to be included first
#endif

#ifndef __ATLCTRLS_H__
	#error BalloonHelp.H requires atlctrls.h to be included first
#endif

#ifndef __ATLGDI_H__
	#error BalloonHelp.H requires atlgdi.h to be included first
#endif

#ifndef BALLOON_HELP_NO_NAMESPACE
namespace WTL
{
#endif


//
//			class CBalloonAnchor
//
class CBalloonAnchor
{
public:
	
	CBalloonAnchor();

	CPoint GetAnchorPoint();
	
	void SetAnchorPoint(const CPoint& pt);
	void SetAnchorPoint(HWND hWndCenter);
	void SetFollowMe(HWND hWndFollow, POINT* pptAnchor);
	BOOL IsFollowMeMode(){return m_wndFollow.IsWindow();}
	BOOL AnchorPointChanged();
		
protected:

	CPoint	m_ptAnchor;
	CSize	m_sizeOffset;
	CWindow	m_wndFollow;

};// class CBalloonAnchor

typedef int (*CloseWindowCallback)(PVOID pSrc);

//
//			class CBalloonHelp (WTL version)
//
class CBalloonHelp:
	public CWindowImpl<CBalloonHelp>,
	//public _KeybHookThunk<CBalloonHelp>,
	//public _MouseHookThunk<CBalloonHelp>,
	//public _CallWndRetHookThunk<CBalloonHelp>,
	public CMessageFilter, public CIdleHandler
{
private:
	
	typedef CBalloonHelp thisClass;
	typedef CWindowImpl<CBalloonHelp>		WindowBase;
	//typedef _KeybHookThunk<thisClass>		KeybHook;
	//typedef _MouseHookThunk<thisClass>		MouseHook;
	//typedef _CallWndRetHookThunk<thisClass>	CallWndRetHook;

public:

	DECLARE_WND_CLASS_EX(_T("XPBalloonHelp"),CS_HREDRAW|CS_VREDRAW,COLOR_BTNFACE)
	//DECLARE_WND_CLASS_EX(_T("XPBalloonHelp"),CS_HREDRAW|CS_VREDRAW|CS_SAVEBITS|CS_DBLCLKS,COLOR_BTNFACE)

	

	CloseWindowCallback __CloseWindowCallback;

	CBalloonHelp();
	CBalloonHelp(ULONG dwPermission);
	~CBalloonHelp();

	enum BallonOptions{
		BOShowCloseButton		= 0x0001,	// Shows close button in upper right.
		BOShowInnerShadow		= 0x0002,	// Draw inner shadow in balloon.
		BOShowTopMost			= 0x0004,	// Place balloon above all other windows.

		BOCloseOnLButtonDown	= 0x0008,	// Closes window on WM_LBUTTONDOWN.
		BOCloseOnMButtonDown	= 0x0010,	// Closes window on WM_MBUTTONDOWN.
		BOCloseOnRButtonDown	= 0x0020,	// Closes window on WM_RBUTTONDOWN.
		BOCloseOnButtonDown		= BOCloseOnLButtonDown|BOCloseOnMButtonDown|BOCloseOnRButtonDown,

		BOCloseOnLButtonUp		= 0x0040,	// Closes window on WM_LBUTTONUP.
		BOCloseOnMButtonUp		= 0x0080,	// Closes window on WM_MBUTTONUP.
		BOCloseOnRButtonUp		= 0x0100,	// Closes window on WM_RBUTTONUP.
		BOCloseOnButtonUp		= BOCloseOnLButtonUp|BOCloseOnMButtonUp|BOCloseOnRButtonUp,

		BOCloseOnMouseMove		= 0x0200,	// Closes window when user moves mouse past threshhold.
		BOCloseOnKeyDown		= 0x0400,	// Closes window on the next keypress message sent to this thread.
		BOCloseOnAppDeactivate	= 0x0800,	// Closes window when the application is being deactivated.
		
		BODeleteThisOnClose		= 0x1000,	// Deletes object when window is closed.  Use only if the object is created on the heap.

		BODisableFadeIn			= 0x2000,	// Disable the fade-in effect (overrides system and user settings).
		BODisableFadeOut		= 0x4000,	// Disable the fade-out effect (overrides system and user settings).
		BODisableFade			= BODisableFadeIn|BODisableFadeOut,

		BONoShow				= 0x8000,	// Dont't show balloon after creating. Usefull if you want to animate the balloon.

		BODefault=BODisableFadeOut|BOCloseOnButtonDown|BOCloseOnKeyDown|BOCloseOnAppDeactivate,
	};

	
	static BOOL Show(HWND hWndOwner, UINT nIdTitle, UINT nIdContent,
				HWND hWndAnchor, LPCTSTR pszIcon=IDI_EXCLAMATION, DWORD dwOptions=BODefault, 
				LPCTSTR pszURL=NULL, UINT uTimeout=0);

	static BOOL Show(HWND hWndOwner, LPCTSTR pszTitle, LPCTSTR pszContent,
				HWND hWndAnchor, LPCTSTR pszIcon=IDI_EXCLAMATION, DWORD dwOptions=BODefault,
				LPCTSTR pszURL=NULL, UINT uTimeout=0);

	static BOOL Show(HWND hWndOwner, UINT nIdTitle, UINT nIdContent, 
				const CPoint& ptAnchor, LPCTSTR pszIcon=IDI_EXCLAMATION, DWORD dwOptions=BODefault,
				LPCTSTR pszURL=NULL, UINT uTimeout=0);

	static BOOL Show(HWND hWndOwner, LPCTSTR pszTitle, LPCTSTR pszContent,
				const CPoint& ptAnchor, LPCTSTR pszIcon=IDI_EXCLAMATION, DWORD dwOptions=BODefault,
				LPCTSTR pszURL=NULL, UINT uTimeout=0);
	
	BOOL Create(HWND hWndOwner, LPCTSTR pszTitle, 
				LPCTSTR pszContent, const POINT* pptAnchor=NULL, DWORD dwOptions=BODefault,  
				LPCTSTR pszURL=NULL, UINT uTimeout=0);

	enum{OleSysColorBits=0x80000000};

	void SetTitleFont(HFONT hFont);
	void SetContentFont(HFONT hFont);
	void SetIcon(LPCTSTR pszIcon);
	void SetIcon(HICON hIcon);
	void SetIcon(HBITMAP hBitmap, COLORREF crMask);
	void SetIcon(HBITMAP hBitmap, HBITMAP hMask);
	void SetIcon(HIMAGELIST hImgList, int nIconIndex);
	void SetTimeout(UINT nTimeout);
	void SetURL(LPCTSTR pszUrl);
	void SetAnchorPoint(CPoint ptAnchor);
	void SetAnchorPoint(HWND hWndAnchor);
	void SetFollowMe(HWND hWndFollow, POINT* pptAnchor);
	void SetTitle(LPCTSTR pszTitle);
	void SetContent(LPCTSTR pszContent);
	void SetForeGroundColor(OLE_COLOR clr);
	void SetBackgroundColor(OLE_COLOR clr);
	void SetMouseMoveTolerance(int nTolerance);
	void SetCloseCallback(CloseWindowCallback _CloseCallbackFun, PVOID pObj);
	void SetYesNoIcon( HBITMAP hYesPassBitmap, HBITMAP hNoPassBitmap );

	inline
	static COLORREF OleTranslateColor(OLE_COLOR ole_clr)
	{
		COLORREF clr;
		::OleTranslateColor(ole_clr,NULL,&clr);
		return clr;
	}

	virtual BOOL OnIdle()
	{
		return FALSE;
	}

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage( pMsg );
	}

protected:
	PVOID __pObject;
	enum BalloonQuadrant{BQTopRight, BQTopLeft, BQBottomLeft, BQBottomRight};
	BalloonQuadrant GetBalloonQuadrant();
	void GetAnchorScreenBounds(CRect& rcBounds);

	enum{nTipBound = 4, nTipMargin = 8, nTipTail = 20, nCXCloseBtn = 14,nCYCloseBtn = 14, nAnimDuration=200};
	CSize DrawHeader(HDC hdc, bool bDraw=true);
	CSize DrawContent(HDC hdc, int nTop, bool bDraw=true);
	CSize CalcHeaderSize(HDC hdc){return DrawHeader(hdc,false);}
	CSize CalcContentSize(HDC hdc){return DrawContent(hdc,0,false);}
	CSize CalcClientSize();
	CSize CalcWindowSize();
	
	void DrawBkgnd(HDC hdc);
	void Draw(HDC hdc);
	void DrawNc(HDC hdc, BOOL bExcludeClient=TRUE);

	void PositionWindow();
	void ShowWindow();
	void CloseWindow();
	void Animate(BOOL bShow);
	
	enum {IdTimerClose=1,IdTimerHotTrack=2,TimerHotTrackElapse=100};
	BEGIN_MSG_MAP_EX(thisClass)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_NCPAINT(OnNcPaint)
		MSG_WM_NCCALCSIZE(OnNcCalcSize)
		MSG_WM_CAPTURECHANGED(OnCaptureChanged)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_MOUSEACTIVATE(OnMouseActivate)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_ACTIVATEAPP(OnActivateApp)
		MSG_WM_PRINT(OnPrint)
	END_MSG_MAP()

protected:
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnDestroy();
	bool OnEraseBkgnd(HDC){return true;}
	void OnPaint(HDC hdc);
	void OnNcPaint(HRGN);
	LRESULT OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam);
	void OnCaptureChanged(HWND);
	void OnMouseMove(UINT, CPoint pt);
	void OnLButtonDown(UINT, CPoint pt);
	void OnLButtonUp(UINT, CPoint pt);
	int	 OnMouseActivate(HWND hWnd, UINT nHitTest,UINT nMsg){return MA_NOACTIVATE;}
	// This is the correct version. Fixed in WTL 7.0
	void OnTimer(UINT nIDEvent);
	void OnActivateApp(BOOL bActivate, DWORD dwTask);
	void OnFinalMessage(HWND);
	void OnPrint(HDC hdc,UINT);
	

	//
	//			Hook related functions
	//

	//			WH_KEYBOARD
	void SetKeyboardHook();
	void RemoveKeyboardHook();
	HHOOK GetKeybHookHandle(){return m_hKeybHook;}
	
	LRESULT KeyboardHookProc(int code, WPARAM wParam, LPARAM lParam);

	//			WH_MOUSE
	void SetMouseHook();
	void RemoveMouseHook();
	HHOOK GetMouseHookHandle(){return m_hMouseHook;}
	
	LRESULT MouseHookProc(int code, WPARAM wParam, LPARAM lParam);
	
	//			WH_CALLWNDPROCRET
	void SetCallWndRetHook();
	void RemoveCallWndRetHook();
	HHOOK GetCallWndRetHandle(){return m_hCallWndRetHook;}
	
	LRESULT CallWndRetProc(int code, WPARAM wParam, LPARAM lParam);
	
private:
	
	HHOOK m_hKeybHook;
	HHOOK m_hMouseHook;
	HHOOK m_hCallWndRetHook;
	
protected:
	
	DWORD			m_dwOptions;
	UINT			m_uTimeout;
	INT				m_nMouseMoveTolerance;

	CFont			m_TitleFont;
	CFont			m_ContentFont;
	
	OLE_COLOR		m_clrForeground;
	OLE_COLOR		m_clrBackground;
	
	CString			m_strContent;
	CString			m_strURL;
	CString			m_szTitle;
	CBalloonAnchor	m_Anchor;
	CRect			m_rcScreen;

	CImageList		m_ilIcon;
	CImageList		m_ilYesNo;
	CPoint			m_ptMouseOrg;
	UINT			m_uCloseState;
	CRgn			m_rgnComplete;
	//HBITMAP			m_YesBitmap;
	//HBITMAP			m_NoBitmap;

public:
	ULONG			m_Permission;

};// class CBalloonHelp (WTL version)

#ifndef BALLOON_HELP_NO_NAMESPACE
}// namespace WTL
#endif

#endif // #ifndef __XPBALLOONDLG__H__