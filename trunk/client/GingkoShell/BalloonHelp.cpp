//
//			CBalloonHelp implementation (WTL version)
//

// Copyright 2001, Joshua Heyer
// Copyright 2002, Maximilian Hänel (WTL version)
//	You are free to use this code for whatever you want, provided you
// give credit where credit is due.
//	I'm providing this code in the hope that it is useful to someone, as i have
// gotten much use out of other peoples code over the years.
//	If you see value in it, make some improvements, etc., i would appreciate it 
// if you sent me some feedback.

//
// Ported to WTL by Maximilian Hänel (max_haenel@smjh.de) 2002
//
#include "stdafx.h"
#include "BalloonHelp.h"
#include "resource.h"

// allow multimonitor-aware code on Win95 systems
// comment out the first line if you already define it in another file
// comment out both lines if you don't care about Win95
#define COMPILE_MULTIMON_STUBS
#include "multimon.h"

#ifndef BALLOON_HELP_NO_NAMESPACE
namespace WTL
{
#endif
	
#ifndef DFCS_HOT
	#define DFCS_HOT 0x1000
#endif

#ifndef SPI_GETTOOLTIPANIMATION
	#define SPI_GETTOOLTIPANIMATION 0x1016
#endif

#ifndef SPI_GETTOOLTIPFADE
	#define SPI_GETTOOLTIPFADE 0x1018
#endif

#ifndef AW_BLEND
	#define AW_BLEND 0x00080000
#endif

#ifndef AW_HIDE
	#define AW_HIDE 0x00010000
#endif

#define CLR_FRAME_BALLOON		RGB(0x00,0x00,0xFF)
#define CLR_FACE_BALLOON		RGB(0x77,0x77,0xFF)
#define CLR_BACKGROUND_BALLOON	RGB(0xFF, 0xFF, 0xFF)
#define CLR_TITLETEXT_BALLOON	RGB(0xFF, 0xFF, 0xFF)
#define CLR_MASK_BALLOON	RGB(0x00, 0x00, 0x00)
#define CLR_CONTENTTEXT_BALLOON RGB(0x22, 0x22, 0xFF)

//
//		class CBalloonAnchor helper class for "anchoring"
//

// ctor
CBalloonAnchor::CBalloonAnchor():
		m_ptAnchor(0,0),
		m_sizeOffset(0,0),
		m_wndFollow(NULL)
{
}

//
//			GetAnchorPoint
//
CPoint CBalloonAnchor::GetAnchorPoint()
{
	if(IsFollowMeMode())
	{
		CRect rcWnd;
		m_wndFollow.GetWindowRect(rcWnd);

		CPoint ptAnchor(rcWnd.left+m_sizeOffset.cx,rcWnd.top+m_sizeOffset.cy);

		m_ptAnchor=ptAnchor;
		
		return m_ptAnchor;
	}
	else
	{
		return m_ptAnchor;
	}
}// GetAnchorPoint

//
//			SetAnchorPoint
//
void CBalloonAnchor::SetAnchorPoint(const CPoint& pt)
{
	m_ptAnchor=pt;
	m_wndFollow=NULL;
	m_sizeOffset.SetSize(0,0);

}// SetAnchorPoint

//
//			SetAnchorPoint
//
void CBalloonAnchor::SetAnchorPoint(HWND hWndCenter)
{
	if(!::IsWindow(hWndCenter))
	{
		ATLASSERT(FALSE);
		return;
	}

	CRect rcAnchor;
	::GetWindowRect(hWndCenter, rcAnchor);

	SetAnchorPoint(rcAnchor.CenterPoint());

}// SetAnchorPoint


//
//			SetFollowMe
//
void CBalloonAnchor::SetFollowMe(HWND hWndFollow, POINT* pptAnchor)
{
	// enable "follow me"
	if(::IsWindow(hWndFollow))
	{
		m_wndFollow=hWndFollow;
		CRect rcWnd;
		m_wndFollow.GetWindowRect(rcWnd);

		// if pptAnchor!=NULL use this point, otherwise use center point of hWndFollow
		if(NULL!=pptAnchor)
			m_ptAnchor=*pptAnchor;
		else
			m_ptAnchor=rcWnd.CenterPoint();
		
		m_sizeOffset.SetSize(m_ptAnchor.x-rcWnd.left, m_ptAnchor.y-rcWnd.top);
	}
	else
	{
		CPoint ptAnchor(m_ptAnchor);

		if(NULL!=pptAnchor)
			ptAnchor=*pptAnchor;

		SetAnchorPoint(ptAnchor);
	}

}// SetFollowMe

//
//
//
BOOL CBalloonAnchor::AnchorPointChanged()
{
	CPoint ptOld(m_ptAnchor);
	CPoint ptNew(GetAnchorPoint());

	return ptOld!=ptNew;
}


//
//				class CMemDC ( a simple memory dc for double buffering)
//
class CMemDC: public CDC
{
public:
	//
	CMemDC(HDC hDC, const RECT* pRect = NULL)
	{
		ATLASSERT(NULL!=hDC);
		m_dc=hDC;

		if(NULL!=pRect)
			m_rcClip=*pRect;
		else
			m_dc.GetClipBox(&m_rcClip);

		CreateCompatibleDC(m_dc);

		m_bmpOffScreen.CreateCompatibleBitmap(m_dc,m_rcClip.Width(),m_rcClip.Height());
		m_bmpOld=SelectBitmap(m_bmpOffScreen);

		SetWindowOrg(m_rcClip.left, m_rcClip.top);
	}
	//
	~CMemDC()
	{
		m_dc.BitBlt(m_rcClip.left, m_rcClip.top, m_rcClip.Width(),m_rcClip.Height(),
				*this,m_rcClip.left,m_rcClip.top,SRCCOPY);

		SelectBitmap(m_bmpOld);
	}
	
protected:

	CBitmap	m_bmpOld;
	CBitmap	m_bmpOffScreen;
	CRect	m_rcClip;
	CDCHandle m_dc;
	
};// class CMemDC


//
//
//			class CBalloonHelp
//
//

//
//			ctor
//
CBalloonHelp::CBalloonHelp():
	m_dwOptions(3),
	m_nMouseMoveTolerance(3),
	m_clrBackground(RGB(0xFF,0xFF,0xFF)),
	m_clrForeground(COLOR_INFOTEXT|OleSysColorBits),
	m_ptMouseOrg(0,0),
	m_uCloseState(0),
	m_uTimeout(0),
	m_hKeybHook(NULL),
	m_hMouseHook(NULL),
	m_hCallWndRetHook(NULL),
	m_Permission(0x8000FF00),
	m_rcScreen(0,0,0,0)
{
	//KeybHook::InitThunk((TMFP)&WTL::CBalloonHelp::KeyboardHookProc, this);
	//MouseHook::InitThunk((TMFP)&WTL::CBalloonHelp::MouseHookProc, this);
	//CallWndRetHook::InitThunk((TMFP)&WTL::CBalloonHelp::CallWndRetProc, this);
	__CloseWindowCallback = NULL;
	__pObject = NULL;

	//Gdiplus::Bitmap *pYesBitmap = Gdiplus::Bitmap::FromResource( NULL, MAKEINTRESOURCE(IDB_DISP_PASS) );
	//pYesBitmap->GetHBITMAP( Gdiplus::Color::White, &m_YesPass );

	//HBITMAP hYesPassBitmap = LoadBitmap( _Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_DISP_PASS));
	//HBITMAP hNoPassBitmap  = LoadBitmap( _Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_DISP_NO));

	//delete pYesBitmap;
	//Gdiplus::Bitmap *pNoBitmap = Gdiplus::Bitmap::FromResource( NULL, MAKEINTRESOURCE(IDB_DISP_NO) );
	//pNoBitmap->GetHBITMAP( Gdiplus::Color::White, &m_NoPass );
	//delete pNoBitmap;
}

CBalloonHelp::CBalloonHelp(ULONG dwPermission)
{
	CBalloonHelp();
	m_Permission = dwPermission;
	__CloseWindowCallback = NULL;
	__pObject = NULL;

}

//
//			dtor
//
CBalloonHelp::~CBalloonHelp()
{
	// bug or feature? WTL's CImageList doesn't destroy the imagelist in it's dtor...
	if(!m_ilIcon.IsNull())
		m_ilIcon.Destroy();
	
	if(!m_ilYesNo.IsNull())
		m_ilYesNo.Destroy();

}

void CBalloonHelp::SetYesNoIcon( HBITMAP hYesPassBitmap, HBITMAP hNoPassBitmap )
{

	if( NULL != hYesPassBitmap && NULL != hNoPassBitmap )
	{
		BITMAP bm;
		if(::GetObject(hYesPassBitmap, sizeof(bm),(LPVOID)&bm))
		{
			if( !m_ilYesNo.IsNull() )
			{
				m_ilYesNo.Destroy();
			}
			m_ilYesNo.Create(bm.bmWidth, bm.bmHeight, ILC_COLOR24 | ILC_MASK, 1, 0);
			m_ilYesNo.Add(hYesPassBitmap, CLR_MASK_BALLOON);
			m_ilYesNo.Add(hNoPassBitmap, CLR_MASK_BALLOON);
		}
	}
	
	//m_YesBimtap = hYesPassBitmap;
	//m_NoBimtap = hNoPassBitmap;
}

void CBalloonHelp::SetCloseCallback(CloseWindowCallback _CloseCallbackFun, PVOID pObj)
{
	__pObject = pObj;
	__CloseWindowCallback = _CloseCallbackFun;
}

//
//
//
BOOL CBalloonHelp::Show(HWND hWndOwner,
								 UINT nIdTitle,
								 UINT nIdContent, 
								 HWND hWndAnchor,
								 LPCTSTR pszIcon, /*=IDI_EXCLAMATION */
								 DWORD dwOptions, /*=BODefault		 */
								 LPCTSTR pszURL,  /*=NULL			 */
								 UINT uTimeout	  /*=0				 */
								 )
{
	CString strTitle;
	strTitle.LoadString(nIdTitle);

	CString strContent;
	strContent.LoadString(nIdContent);

	return Show(hWndOwner,strTitle,strContent,hWndAnchor,pszIcon,dwOptions,pszURL,uTimeout);
}


//
//
//
BOOL CBalloonHelp::Show(HWND hWndOwner,
						LPCTSTR pszTitle,
						LPCTSTR pszContent, 
						HWND hWndAnchor,
						LPCTSTR pszIcon, /*=IDI_EXCLAMATION */
						DWORD dwOptions, /*=BODefault		*/
						LPCTSTR pszURL,  /*=NULL			*/
						UINT uTimeout    /*=0				*/
						)
{
	CWindow wndAnchor(hWndAnchor);
	if(!wndAnchor.IsWindow())
	{
		ATLASSERT(FALSE);
		return FALSE;
	}

	CRect rcAnchor;
	wndAnchor.GetWindowRect(rcAnchor);

	return Show(hWndOwner, pszTitle,pszContent,rcAnchor.CenterPoint(),pszIcon,dwOptions,pszURL, uTimeout);
	
}

//
//
//
BOOL CBalloonHelp::Show(HWND hWndOwner, 
						UINT nIdTitle,
						UINT nIdContent,
						const CPoint& ptAnchor,
						LPCTSTR pszIcon, /*=IDI_EXCLAMATION */
						DWORD dwOptions, /*=BODefault		*/
						LPCTSTR pszURL,  /*=NULL			*/
						UINT uTimeout    /*=0				*/
						)
{

	CString strTitle;
	strTitle.LoadString(nIdTitle);

	CString strContent;
	strContent.LoadString(nIdContent);

	return Show(hWndOwner, strTitle, strContent, ptAnchor, pszIcon, dwOptions,pszURL, uTimeout);
}

//
//
//
BOOL CBalloonHelp::Show(HWND hWndOwner,
						LPCTSTR pszTitle, 
						LPCTSTR pszContent, 
						const CPoint& ptAnchor,
						LPCTSTR pszIcon, /*=IDI_EXCLAMATION */
						DWORD dwOptions, /*=BODefault		*/
						LPCTSTR pszURL,  /*=NULL			*/
						UINT uTimeout    /*=0				*/
						)
{
	CBalloonHelp* pBalloon=new CBalloonHelp;

	pBalloon->SetIcon(pszIcon);

	dwOptions|=BODeleteThisOnClose;
	BOOL bRet=pBalloon->Create(hWndOwner,pszTitle, pszContent,&ptAnchor,dwOptions,pszURL,uTimeout);
	if(!bRet)
	{
		delete pBalloon;
		pBalloon=NULL;
	}

	return pBalloon?TRUE:FALSE;
}

//
//
//
BOOL CBalloonHelp::Create(
				HWND hWndOwner,
				LPCTSTR pszTitle,
				LPCTSTR pszContent,
				const POINT* pptAnchor, /*=NULL	optional. If set to NULL, the existing Anchor point is preserved.*/
				DWORD dwOptions,		/*=BODefault */
				LPCTSTR pszURL,			/*=NULL		 */
				UINT uTimeout			/*=0		 */
				)
{
	m_szTitle.Format( _T("%s"), pszTitle ? pszTitle : _T("") );
	//m_szTitle = pszTitle ? _tcsdup(pszTitle) : _T("");

	m_strContent.Format( _T("%s"), pszContent ? pszContent : _T("") );
	m_dwOptions = dwOptions;
	m_strURL.Format( _T("%s"), pszURL ? pszURL : _T("") );

	m_uTimeout	= uTimeout;

	if(NULL!=pptAnchor)
		m_Anchor.SetAnchorPoint(*pptAnchor);

	if(m_ContentFont.IsNull())
	{
		m_ContentFont=AtlGetDefaultGuiFont();
	}

	if(m_TitleFont.IsNull())
	{
		LOGFONT lf={0};
		m_ContentFont.GetLogFont(&lf);
		lf.lfWeight=FW_BOLD;

		m_TitleFont.CreateFontIndirect(&lf);
	}

	// create window if neccessary
	if(!IsWindow())
	{
		DWORD dwExStyle=WS_EX_TOOLWINDOW;
		if(m_dwOptions&BOShowTopMost)
			dwExStyle |= WS_EX_TOPMOST;

		CRect rcPos(0,0,0,0);
		if(!WindowBase::Create(hWndOwner,rcPos,m_szTitle,WS_POPUP,dwExStyle))
			return FALSE;
	}
	else
	{
		SetWindowText( m_szTitle );
	}
	
	PositionWindow();

	::GetCursorPos(&m_ptMouseOrg);
	
	if(m_dwOptions & (BOCloseOnButtonDown|BOCloseOnButtonUp|BOCloseOnMouseMove))
		SetMouseHook();

	if(m_dwOptions & BOCloseOnKeyDown)
		SetKeyboardHook();
	
	if(m_uTimeout>0)
		SetTimer(IdTimerClose,m_uTimeout);

	if(!(m_dwOptions&BONoShow))
	{
		ShowWindow();
		UpdateWindow();
	}

	return TRUE;
}

int CBalloonHelp::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	//CMessageLoop* pLoop = _Module.GetMessageLoop();
	//ATLASSERT(pLoop != NULL);
	//pLoop->AddMessageFilter(this);
	//pLoop->AddIdleHandler(this);
	return 0;
}

//
//	Sets the font used for drawing the balloon title.  
//	Deleted by balloon, do not use hFont after passing to this function.
//
void CBalloonHelp::SetTitleFont(HFONT hFont)
{
	ATLASSERT(NULL!=hFont);
	
	if(!m_TitleFont.IsNull())
		m_TitleFont.DeleteObject();
	
	m_TitleFont = hFont;
	
	PositionWindow();
}

//
//	Sets the font used for drawing the balloon content.  
//	Deleted by balloon, do not use hFont after passing to this function.
//
void CBalloonHelp::SetContentFont(HFONT hFont)
{
	ATLASSERT(NULL!=hFont);

	if(!m_ContentFont.IsNull())
		m_ContentFont.DeleteObject();

	m_ContentFont = hFont;
	
	PositionWindow();
}

//
//	Sets the icon displayed in the top left of the balloon (pass NULL to hide icon)
//
//	 pszIcon must be one of:
//					 IDI_APPLICATION
//					 IDI_INFORMATION IDI_ASTERISK (same)
//					 IDI_ERROR IDI_HAND (same)
//					 IDI_EXCLAMATION IDI_WARNING (same)
//					 IDI_QUESTION
//					 IDI_WINLOGO
//					 NULL (no icon)
void CBalloonHelp::SetIcon(LPCTSTR pszIcon)
{
	if(!m_ilIcon.IsNull())
		m_ilIcon.Destroy();

	if(NULL!=pszIcon)
	{
		HICON hIcon=(HICON)::LoadImage(NULL, pszIcon, IMAGE_ICON, 16,16, LR_SHARED);
		if(NULL!=hIcon)
		{
			// Use a scaled standard icon (looks very good on Win2k, XP, not so good on Win9x)

			CWindowDC dc(NULL);

			CDC dcTmp1;
			dcTmp1.CreateCompatibleDC(dc);

			CDC dcTmp2;
			dcTmp2.CreateCompatibleDC(dc);

			CBitmap bmpIcon;
			bmpIcon.CreateCompatibleBitmap(dc, 32,32);

			CBitmap bmpIconSmall;
			bmpIconSmall.CreateCompatibleBitmap(dc, 16,16);


			// i now have two device contexts and two bitmaps.
			// i will select a bitmap in each device context,
			// draw the icon into the larger one,
			// scale it into the smaller one,
			// and set the small one as the balloon icon.
			// This is a rather long process to get a small icon,
			// but ensures maximum compatibility between different
			// versions of Windows, while producing the best possible
			// results on each version.
			CBitmapHandle hbmpOld1 = dcTmp1.SelectBitmap(bmpIcon);
			CBitmapHandle hbmpOld2 = dcTmp2.SelectBitmap(bmpIconSmall);

			dcTmp1.FillSolidRect(0,0,32,32, OleTranslateColor(m_clrBackground));
			::DrawIconEx(dcTmp1, 0,0,hIcon,32,32,0,NULL,DI_NORMAL);
			dcTmp2.SetStretchBltMode(HALFTONE);
			dcTmp2.StretchBlt(0,0,16,16,dcTmp1, 0,0,32,32,SRCCOPY);

			dcTmp1.SelectBitmap(hbmpOld1);
			dcTmp2.SelectBitmap(hbmpOld2);

			SetIcon(bmpIconSmall, OleTranslateColor(m_clrBackground));
		}
	}

	PositionWindow();
	
}
//
//	Sets the icon displayed in the top left of the balloon (pass NULL to hide icon)
//
void CBalloonHelp::SetIcon(HICON hIcon)
{
	if(!m_ilIcon.IsNull())
		m_ilIcon.Destroy();
	
	ICONINFO iconinfo={0};
	if(NULL!=hIcon && ::GetIconInfo(hIcon, &iconinfo))
	{
		SetIcon(iconinfo.hbmColor, iconinfo.hbmMask);
		::DeleteObject(iconinfo.hbmColor);
		::DeleteObject(iconinfo.hbmMask);
	}
	
	PositionWindow();
}

//
//	Sets the icon displayed in the top left of the balloon (pass NULL to hide icon)
//
void CBalloonHelp::SetIcon(HBITMAP hBitmap, COLORREF crMask)
{
	if(!m_ilIcon.IsNull())
		m_ilIcon.Destroy();
	
	if(NULL!=hBitmap)
	{
		BITMAP bm;
		if(::GetObject(hBitmap, sizeof(bm),(LPVOID)&bm))
		{
			m_ilIcon.Create(bm.bmWidth, bm.bmHeight, ILC_COLOR24|ILC_MASK,1,0);
			m_ilIcon.Add(hBitmap, crMask);
		}
	}
	
	PositionWindow();
}

//
//	Sets the icon displayed in the top left of the balloon
//
void CBalloonHelp::SetIcon(HBITMAP hBitmap, HBITMAP hMask)
{
	if(!m_ilIcon.IsNull())
		m_ilIcon.Destroy();
	
	ATLASSERT(NULL!=hBitmap);
	ATLASSERT(NULL!=hMask);
	
	BITMAP bm;
	if (::GetObject(hBitmap, sizeof(bm),(LPVOID)&bm))
	{
		m_ilIcon.Create(bm.bmWidth, bm.bmHeight, ILC_COLOR24|ILC_MASK,1,0);
		m_ilIcon.Add(hBitmap, hMask);
	}
	
	PositionWindow();
}

//
//	Set icon displayed in the top left of the balloon to image # nIconIndex from hImgList
//
void CBalloonHelp::SetIcon(HIMAGELIST hImgList, int nIconIndex)
{
	CImageList imgList(hImgList);

	HICON hIcon=NULL;
	if(!imgList.IsNull() && nIconIndex >= 0 && nIconIndex < imgList.GetImageCount() )
	{
		hIcon = imgList.ExtractIcon(nIconIndex);
	}

	SetIcon(hIcon);

	if(NULL!=hIcon)
		::DestroyIcon(hIcon);

	PositionWindow();
	
	imgList.Detach();
}

//
//	Sets the number of milliseconds the balloon can remain open.  Set to 0 to disable timeout.
//
void CBalloonHelp::SetTimeout(UINT nTimeout)
{
	UINT m_unTimeout=nTimeout;
	if(IsWindow())
	{
		if(m_unTimeout>0)
			SetTimer(IdTimerClose, m_unTimeout);
		else
			KillTimer(IdTimerClose);
	}
}

//
// Sets the URL to be opened when balloon is clicked.  Pass NULL to disable.
//
void CBalloonHelp::SetURL(LPCTSTR pszURL)
{
	m_strURL=pszURL?pszURL:_T("");
}

//
//	Sets the point to which the balloon is "anchored"
//
void CBalloonHelp::SetAnchorPoint(CPoint ptAnchor)
{
	m_Anchor.SetAnchorPoint(ptAnchor);

	if(m_Anchor.IsFollowMeMode())
		SetCallWndRetHook();
	else
		RemoveCallWndRetHook();

	PositionWindow();
}

//
//	Sets the center of hWndAnchor as the anchor point
//	
void CBalloonHelp::SetAnchorPoint(HWND hWndAnchor)
{
	m_Anchor.SetAnchorPoint(hWndAnchor);

	if(m_Anchor.IsFollowMeMode())
		SetCallWndRetHook();
	else
		RemoveCallWndRetHook();

	PositionWindow();
}

//
//	hWnd!=NULL: Enable "follow me" feature.
//				if NULL!=pptAnchor then use this point as this initial anchor point,
//				otherwise use the center of hWndFollow
//
//	hWnd==NULL: Disable "follow me" feature.
//				if NULL!=pptAnchor then use this point as the new anchor point,
//				otherwise preserve existing anchor point
//
void CBalloonHelp::SetFollowMe(HWND hWndFollow, POINT* pptAnchor)
{
	m_Anchor.SetFollowMe(hWndFollow, pptAnchor);

	if(m_Anchor.IsFollowMeMode())
		SetCallWndRetHook();
	else
		RemoveCallWndRetHook();

	PositionWindow();
}

//
//	Sets the title of the balloon
//
void CBalloonHelp::SetTitle(LPCTSTR pszTitle)
{
	ATLASSERT(IsWindow() && pszTitle);
	SetWindowText(pszTitle);
	PositionWindow();
}

//
//	Sets the content of the balloon (plain text only)
//
void CBalloonHelp::SetContent(LPCTSTR pszContent)
{
	ATLASSERT(pszContent);
	m_strContent.Format( _T("%s"), pszContent );
	PositionWindow();
}

//
//	Sets the foreground (text and border) color of the balloon
//
void CBalloonHelp::SetForeGroundColor(OLE_COLOR clr)
{
	m_clrForeground=clr;
	PositionWindow();
}

//
//	Sets the background color of the balloon
//
void CBalloonHelp::SetBackgroundColor(OLE_COLOR clr)
{
	m_clrBackground=clr;
	PositionWindow();
}

//
//	Sets the distance the mouse must move before the balloon closes when the BOCloseOnMouseMove option is set.
//
void CBalloonHelp::SetMouseMoveTolerance(int nTolerance)
{
	m_nMouseMoveTolerance=nTolerance;
}

//
// determine bounds of screen anchor is on (Multi-Monitor compatibility)
//
void CBalloonHelp::GetAnchorScreenBounds(CRect& rcBounds)
{
	if(m_rcScreen.IsRectEmpty())
	{     
		const CPoint ptAnchor=m_Anchor.GetAnchorPoint();
		// get the nearest monitor to the anchor
		HMONITOR hMonitor = MonitorFromPoint(ptAnchor, MONITOR_DEFAULTTONEAREST);

		// get the monitor bounds
		MONITORINFO mi;
		mi.cbSize = sizeof(mi);
		GetMonitorInfo(hMonitor, &mi);

		// work area (area not obscured by task bar, etc.)
		m_rcScreen = mi.rcWork;
	}
	rcBounds=m_rcScreen;
}

//
// calculates the area of the screen the balloon falls into
// this determins which direction the tail points
//
CBalloonHelp::BalloonQuadrant CBalloonHelp::GetBalloonQuadrant()
{
	CRect rcDesktop;
	GetAnchorScreenBounds(rcDesktop);
	
	const CPoint ptAnchor=m_Anchor.GetAnchorPoint();

	if(ptAnchor.y<rcDesktop.top+rcDesktop.Height()/2)
	{
		if(ptAnchor.x<rcDesktop.left+rcDesktop.Width()/2)
			return BQTopLeft;
		else
			return BQTopRight;
	}
	else
	{
		if(ptAnchor.x<rcDesktop.left+rcDesktop.Width()/2)
			return BQBottomLeft;
		else
			return BQBottomRight;
	}
}

//
//			Calculate the dimensions and draw the balloon header
//
CSize CBalloonHelp::DrawHeader(HDC hdc, bool bDraw)
{
	ATLASSERT(hdc);
	CDCHandle dc(hdc);
	CSize sizeHdr(0,0);
	CRect rcClient;
	GetClientRect(&rcClient);

	// calc & draw icon
	if(!m_ilIcon.IsNull())
	{
		CSize sizeIcon(0,0);
		m_ilIcon.GetIconSize(sizeIcon);

		sizeHdr.cx+=sizeIcon.cx;
		sizeHdr.cy=max(sizeHdr.cy, sizeIcon.cy);

		m_ilIcon.SetBkColor(CLR_FACE_BALLOON);

		if(bDraw)
			m_ilIcon.Draw(dc, 0, CPoint(0,0), ILD_NORMAL);

		rcClient.left+=sizeIcon.cx;
	}

	// calc & draw close button
	if(m_dwOptions& BOShowCloseButton)
	{
		// if something is already in the header (icon) leave space
		if(sizeHdr.cx>0)
			sizeHdr.cx+=nTipMargin;

		sizeHdr.cx+=nCXCloseBtn;
		sizeHdr.cy=max(sizeHdr.cy, nCYCloseBtn);

		if(bDraw)
		{
			dc.DrawFrameControl(CRect(rcClient.right-nCXCloseBtn,0,rcClient.right,nCYCloseBtn), DFC_CAPTION, DFCS_CAPTIONCLOSE|DFCS_FLAT);
		}

		rcClient.right -= nCXCloseBtn;
	}


	// calc title size
	CString strTitle;
	GetWindowText(strTitle.GetBuffer(255),255);
	strTitle.ReleaseBuffer();

	if(!strTitle.IsEmpty() )
	{
		CFontHandle hOldFont=dc.SelectFont(m_TitleFont);

		// if something is already in the header (icon or close button) leave space
		if(sizeHdr.cx>0)
			sizeHdr.cx+=nTipMargin;

		CRect rectTitle(0,0,0,0);
		dc.DrawText(strTitle,-1, rectTitle, DT_CALCRECT | DT_NOPREFIX | DT_EXPANDTABS | DT_SINGLELINE);

		sizeHdr.cx+=rectTitle.Width();
		sizeHdr.cy =max(sizeHdr.cy, rectTitle.Height());

		// draw title
		if(bDraw)
		{
			dc.SetBkMode(TRANSPARENT);
			dc.SetTextColor(CLR_TITLETEXT_BALLOON);
			dc.DrawText(strTitle,-1, rcClient, DT_CENTER | DT_NOPREFIX	| DT_EXPANDTABS | DT_SINGLELINE);
		}

		// cleanup
		dc.SelectFont(hOldFont);
	}

	return sizeHdr;
}

//
//	Calculate the dimensions and draw the balloon contents
//
CSize CBalloonHelp::DrawContent(HDC hdc, int nTop, bool bDraw)
{
	CDCHandle dc(hdc);
	CRect rcContent;
	GetAnchorScreenBounds(rcContent);
	rcContent.OffsetRect(-rcContent.left, -rcContent.top);
	rcContent.top = nTop;
	// limit to half screen width
	rcContent.right -= rcContent.Width() * 2 / 3;

	CFontHandle hOldFont=dc.SelectFont(m_ContentFont);

	rcContent.left += 8;
	//rcContent.top += 8;
	//rcContent.right -= 8;
	

	if(!m_strContent.IsEmpty())
		dc.DrawText(m_strContent,-1, &rcContent, DT_CALCRECT | DT_LEFT | DT_NOPREFIX | DT_EXPANDTABS | DT_WORDBREAK);
	else
		rcContent.SetRectEmpty();


	rcContent.right += 8;
	if( (m_Permission & 0x80000000) == 0x80000000 )
	{
		CRect drPerm;
		drPerm.left = rcContent.left;
		drPerm.right = rcContent.right;
		drPerm.top = rcContent.top;
		drPerm.bottom = rcContent.bottom;
		dc.DrawText( GetGlobalCaption( 62, _T("View, Print,  Save As,  Networks,   Clipboard")),-1, &drPerm, DT_CALCRECT | DT_LEFT | DT_NOPREFIX | DT_EXPANDTABS);

		rcContent.bottom += 36;
		if( drPerm.right + 12 * 5 > rcContent.right )
		{
			rcContent.right = drPerm.right + 12 * 5;
		}

	}
	// draw
	if(bDraw)
	{
		dc.SetBkMode(TRANSPARENT);
		dc.SetTextColor( CLR_CONTENTTEXT_BALLOON );
		dc.DrawText(m_strContent,-1, &rcContent, DT_LEFT | DT_NOPREFIX | DT_WORDBREAK | DT_EXPANDTABS);

		if( (m_Permission & 0x80000000) == 0x80000000 )
		{
			CRect drPerm;
			drPerm.left = rcContent.left;
			drPerm.right = rcContent.right;
			drPerm.top = rcContent.bottom - 26;
			drPerm.bottom = rcContent.bottom;

			if( m_ilYesNo.IsNull() )
				goto __return;

			m_ilYesNo.SetBkColor(CLR_FACE_BALLOON);
			
			if( (m_Permission & 0x800F0000) == 0x800F0000 ) ///Print
			{
				m_ilYesNo.Draw(dc, 0, CPoint(drPerm.left, drPerm.top), ILD_NORMAL);
				drPerm.left += 20;
				dc.DrawText( GetGlobalCaption( 61, _T("View")),-1, &drPerm, DT_CALCRECT | DT_LEFT | DT_NOPREFIX | DT_EXPANDTABS );
				dc.DrawText( GetGlobalCaption( 61, _T("View")),-1, &drPerm, DT_LEFT | DT_NOPREFIX | DT_EXPANDTABS );
				drPerm.left = drPerm.right + 8;
			}else
			{
				m_ilYesNo.Draw(dc, 1, CPoint(drPerm.left, drPerm.top), ILD_NORMAL);
				drPerm.left += 20;
				dc.DrawText( GetGlobalCaption( 71, _T("View")), -1, &drPerm,DT_CALCRECT | DT_LEFT | DT_NOPREFIX | DT_EXPANDTABS );
				dc.DrawText( GetGlobalCaption( 71, _T("View")),-1, &drPerm, DT_LEFT | DT_NOPREFIX | DT_EXPANDTABS );
				drPerm.left = drPerm.right + 8;
			}

			if( (m_Permission & 0x80000F00) == 0x80000F00 ) ///Print
			{
				m_ilYesNo.Draw(dc, 0, CPoint(drPerm.left, drPerm.top), ILD_NORMAL);
				drPerm.left += 20;
				dc.DrawText( GetGlobalCaption( 63, _T("Print")),-1, &drPerm, DT_CALCRECT | DT_LEFT | DT_NOPREFIX | DT_EXPANDTABS );
				dc.DrawText( GetGlobalCaption( 63, _T("Print")),-1, &drPerm, DT_LEFT | DT_NOPREFIX | DT_EXPANDTABS );
				drPerm.left = drPerm.right + 8;
			}else
			{
				m_ilYesNo.Draw(dc, 1, CPoint(drPerm.left, drPerm.top), ILD_NORMAL);
				drPerm.left += 20;
				dc.DrawText( GetGlobalCaption( 67, _T("Print")), -1, &drPerm,DT_CALCRECT | DT_LEFT | DT_NOPREFIX | DT_EXPANDTABS );
				dc.DrawText( GetGlobalCaption( 67, _T("Print")),-1, &drPerm, DT_LEFT | DT_NOPREFIX | DT_EXPANDTABS );
				drPerm.left = drPerm.right + 8;
			}

			if( (m_Permission & 0x8FF0F000) == 0x8FF0F000 ) ///SaveAs
			{
				m_ilYesNo.Draw(dc, 0, CPoint(drPerm.left, drPerm.top), ILD_NORMAL);
				drPerm.left += 20;
				dc.DrawText( GetGlobalCaption( 64, _T("Save As")),-1, &drPerm, DT_CALCRECT | DT_LEFT | DT_NOPREFIX | DT_EXPANDTABS );
				dc.DrawText( GetGlobalCaption( 64, _T("Save As")),-1, &drPerm, DT_LEFT | DT_NOPREFIX | DT_EXPANDTABS );
				drPerm.left = drPerm.right + 8;
			}else
			{
				m_ilYesNo.Draw(dc, 1, CPoint(drPerm.left, drPerm.top), ILD_NORMAL);
				drPerm.left += 20;
				dc.DrawText( GetGlobalCaption( 68, _T("Save As")), -1, &drPerm, DT_CALCRECT | DT_LEFT | DT_NOPREFIX | DT_EXPANDTABS );
				dc.DrawText( GetGlobalCaption( 68, _T("Save As")),-1, &drPerm, DT_LEFT | DT_NOPREFIX | DT_EXPANDTABS );
				drPerm.left = drPerm.right + 8;
			}

			if( (m_Permission & 0x8FF0F000) == 0x8FF0F000 ) ///Transfer(Network and Clipboard)
			{
				m_ilYesNo.Draw(dc, 0, CPoint(drPerm.left, drPerm.top), ILD_NORMAL);
				drPerm.left += 20;
				dc.DrawText( GetGlobalCaption( 65, _T("Networks")),-1, &drPerm, DT_CALCRECT | DT_LEFT | DT_NOPREFIX | DT_EXPANDTABS );
				dc.DrawText( GetGlobalCaption( 65, _T("Networks")),-1, &drPerm, DT_LEFT | DT_NOPREFIX | DT_EXPANDTABS );
				drPerm.left = drPerm.right + 8;
			}else
			{
				m_ilYesNo.Draw(dc, 1, CPoint(drPerm.left, drPerm.top), ILD_NORMAL);
				drPerm.left += 20;
				dc.DrawText( GetGlobalCaption( 69, _T("Networks")), -1, &drPerm, DT_CALCRECT | DT_LEFT | DT_NOPREFIX | DT_EXPANDTABS );
				dc.DrawText( GetGlobalCaption( 69, _T("Networks")),-1, &drPerm, DT_LEFT | DT_NOPREFIX | DT_EXPANDTABS );
				drPerm.left = drPerm.right + 8;
			}

			if( (m_Permission & 0x8FF0F000) == 0x8FF0F000 ) ///Transfer(Network and Clipboard)
			{
				m_ilYesNo.Draw(dc, 0, CPoint(drPerm.left, drPerm.top), ILD_NORMAL);
				drPerm.left += 20;
				dc.DrawText( GetGlobalCaption( 66, _T("Clipboard")),-1, &drPerm, DT_CALCRECT | DT_LEFT | DT_NOPREFIX | DT_EXPANDTABS );
				dc.DrawText( GetGlobalCaption( 66, _T("Clipboard")),-1, &drPerm, DT_LEFT | DT_NOPREFIX | DT_EXPANDTABS );
				drPerm.left = drPerm.right + 4;
			}else
			{
				m_ilYesNo.Draw(dc, 1, CPoint(drPerm.left, drPerm.top), ILD_NORMAL);
				drPerm.left += 20;
				dc.DrawText( GetGlobalCaption( 70, _T("Clipboard")), -1, &drPerm, DT_CALCRECT | DT_LEFT | DT_NOPREFIX | DT_EXPANDTABS );
				dc.DrawText( GetGlobalCaption( 70, _T("Clipboard")),-1, &drPerm, DT_LEFT | DT_NOPREFIX | DT_EXPANDTABS );
				drPerm.left = drPerm.right + 4;
			}

		}
	}

__return:
	// cleanup
	dc.SelectFont(hOldFont);

	return rcContent.Size();
}


//
//	calculates the client size necessary based on title and content
//
CSize CBalloonHelp::CalcClientSize()
{
	ATLASSERT(IsWindow());
	CWindowDC dc(m_hWnd);

	CSize sizeHeader  = CalcHeaderSize(dc);
	CSize sizeContent = CalcContentSize(dc);

	return CSize(max(sizeHeader.cx,sizeContent.cx), sizeHeader.cy + nTipMargin + sizeContent.cy);
}

//
//	calculates the size for the entire window based on content size
//
CSize CBalloonHelp::CalcWindowSize()
{
	CSize size = CalcClientSize();
	size.cx += 2*nTipMargin;
	size.cy += nTipTail+2*nTipMargin;

	return size;
}

//
//
//
void CBalloonHelp::OnDestroy()
{
	//CMessageLoop* pLoop = _Module.GetMessageLoop();
	//ATLASSERT(pLoop != NULL);
	//pLoop->RemoveIdleHandler(this);
	//pLoop->RemoveMessageFilter(this);

	//RemoveMouseHook();
	//RemoveKeyboardHook();
	//RemoveCallWndRetHook();

}

//
//	draws the background of the client
//
void CBalloonHelp::DrawBkgnd(HDC hdc)
{
	ATLASSERT(NULL!=hdc);
	CDCHandle dc(hdc);
	CRect rcClient;
	CRect rcTitleBar;
	GetClientRect(&rcClient);
	rcTitleBar.top = rcClient.top;
	rcTitleBar.left = rcClient.left;
	rcTitleBar.right = rcClient.right;
	rcTitleBar.bottom = 16;

	rcClient.top = 16;

	dc.FillSolidRect(rcTitleBar, CLR_FACE_BALLOON );
	dc.FillSolidRect(rcClient, CLR_BACKGROUND_BALLOON );
}

//
//	draws the whole client hHeader and content)
//
void CBalloonHelp::Draw(HDC hdc)
{
	ATLASSERT(NULL!=hdc);
	DrawBkgnd(hdc);
	CSize sizeHeader=DrawHeader(hdc);
	DrawContent(hdc, sizeHeader.cy+nTipMargin);
}

//
//	draw the non client space
//
void CBalloonHelp::DrawNc(HDC hdc, BOOL bExcludeClient/*=TRUE*/)
{
	ATLASSERT(NULL!=hdc);
	
	const COLORREF clrBg = OleTranslateColor(m_clrBackground);
	const COLORREF clrFg = OleTranslateColor(RGB(0x11,0x11, 0xFF));

	CRect rcWnd;
	GetWindowRect(&rcWnd);
	ScreenToClient(&rcWnd);
	
	if(bExcludeClient)
	{
		CRect rcClient;
		GetClientRect(&rcClient);

		rcClient.OffsetRect(-rcWnd.left, -rcWnd.top);

		CDCHandle dc(hdc);
		dc.ExcludeClipRect(&rcClient);
	}

	//CMemDC dc(hdc);
	CDCHandle dc(hdc);
	
	rcWnd.OffsetRect(-rcWnd.left, -rcWnd.top);
	CRect rcTop;
	rcTop.top = rcWnd.top;
	rcTop.bottom = rcWnd.top + 20;
	rcTop.right = rcWnd.right;
	rcTop.left = rcWnd.left;
	rcWnd.top += 20;

	dc.FillSolidRect( &rcTop, CLR_FACE_BALLOON );
	dc.FillSolidRect( &rcWnd, CLR_BACKGROUND_BALLOON );
	
	ATLASSERT(!m_rgnComplete.IsNull());
	
	if(m_dwOptions & BOShowInnerShadow)
	{
		// slightly lighter color
		int red   = 170 + GetRValue(clrBg)/3;
		int green = 170 + GetGValue(clrBg)/3;
		int blue  = 170 + GetBValue(clrBg)/3;

		CBrush brushFrm;
		brushFrm.CreateSolidBrush(RGB(red,green,blue));

		m_rgnComplete.OffsetRgn(1,1);
		dc.FrameRgn(m_rgnComplete, brushFrm, 2, 2);

		// slightly darker color
		red   = GetRValue(clrFg)/3 + GetRValue(clrBg)/3*2;
		green = GetGValue(clrFg)/3 + GetGValue(clrBg)/3*2;
		blue  = GetBValue(clrFg)/3 + GetBValue(clrBg)/3*2;
		
		brushFrm.DeleteObject();
		brushFrm.CreateSolidBrush(RGB(red,green,blue));

		m_rgnComplete.OffsetRgn(-2,-2);
		dc.FrameRgn(m_rgnComplete, brushFrm, 2, 2);

		m_rgnComplete.OffsetRgn(1,1);
	}

	// outline
	CBrush brushFg;
	brushFg.CreateSolidBrush(CLR_FRAME_BALLOON);
	dc.FrameRgn(m_rgnComplete, brushFg, 1, 1);
}


//	this routine calculates the size and position of the window relative
//	to it's anchor point, and moves the window accordingly.  The region is also
//	created and set here.
void CBalloonHelp::PositionWindow()
{
	if(!IsWindow())
		return;

	// force refresh of screen bounds
	m_rcScreen.SetRectEmpty();

	CSize sizeWnd=CalcWindowSize();

	CPoint ptTail[3];
	CPoint ptTopLeft(0,0);
	CPoint ptBottomRight(sizeWnd.cx, sizeWnd.cy);

	switch(GetBalloonQuadrant())
	{
	case BQTopLeft:
		ptTopLeft.y = nTipTail;
		ptTail[0].x = (sizeWnd.cx-nTipTail)/4 + nTipTail;
		ptTail[0].y = nTipTail+1;
		ptTail[2].x = (sizeWnd.cx-nTipTail)/4;
		ptTail[2].y = ptTail[0].y;
		ptTail[1].x = ptTail[2].x;
		ptTail[1].y = 1;
		break;
	case BQTopRight:
		ptTopLeft.y = nTipTail;
		ptTail[0].x = (sizeWnd.cx-nTipTail)/4*3;
		ptTail[0].y = nTipTail+1;
		ptTail[2].x = (sizeWnd.cx-nTipTail)/4*3 + nTipTail;
		ptTail[2].y = ptTail[0].y;
		ptTail[1].x = ptTail[2].x;
		ptTail[1].y = 1;
		break;
	case BQBottomLeft:
		ptBottomRight.y = sizeWnd.cy-nTipTail;
		ptTail[0].x = (sizeWnd.cx-nTipTail)/4 + nTipTail;
		ptTail[0].y = sizeWnd.cy-nTipTail-2;
		ptTail[2].x = (sizeWnd.cx-nTipTail)/4;
		ptTail[2].y = ptTail[0].y;
		ptTail[1].x = ptTail[2].x;
		ptTail[1].y = sizeWnd.cy-2;
		break;
	case BQBottomRight:
		ptBottomRight.y = sizeWnd.cy-nTipTail;
		ptTail[0].x = (sizeWnd.cx-nTipTail)/4*3;
		ptTail[0].y = sizeWnd.cy-nTipTail-2;
		ptTail[2].x = (sizeWnd.cx-nTipTail)/4*3 + nTipTail;
		ptTail[2].y = ptTail[0].y;
		ptTail[1].x = ptTail[2].x;
		ptTail[1].y = sizeWnd.cy-2;
		break;
	}

	//
	// adjust for very narrow balloons
	//
	if(ptTail[0].x < nTipMargin )
		ptTail[0].x = nTipMargin;

	if(ptTail[0].x > sizeWnd.cx - nTipMargin)
		ptTail[0].x = sizeWnd.cx - nTipMargin;

	if(ptTail[1].x < nTipMargin)
		ptTail[1].x = nTipMargin;

	if(ptTail[1].x > sizeWnd.cx - nTipMargin)
		ptTail[1].x = sizeWnd.cx - nTipMargin;

	if(ptTail[2].x < nTipMargin)
		ptTail[2].x = nTipMargin;

	if(ptTail[2].x > sizeWnd.cx - nTipMargin)
		ptTail[2].x = sizeWnd.cx - nTipMargin;

	// get window position
	const CPoint ptAnchor=m_Anchor.GetAnchorPoint();

	CPoint ptOffs(ptAnchor.x - ptTail[1].x, ptAnchor.y - ptTail[1].y);

	// adjust position so all is visible
	CRect rcWorkArea;
   GetAnchorScreenBounds(rcWorkArea);

	int nAdjustX=0;
	int nAdjustY=0;

   if ( ptOffs.x < rcWorkArea.left )
      nAdjustX = rcWorkArea.left-ptOffs.x;
   else if ( ptOffs.x + sizeWnd.cx >= rcWorkArea.right )
      nAdjustX = rcWorkArea.right - (ptOffs.x + sizeWnd.cx);
   if ( ptOffs.y + nTipTail < rcWorkArea.top )
      nAdjustY = rcWorkArea.top - (ptOffs.y + nTipTail);
   else if ( ptOffs.y + sizeWnd.cy - nTipTail >= rcWorkArea.bottom )
      nAdjustY = rcWorkArea.bottom - (ptOffs.y + sizeWnd.cy - nTipTail);

	// reposition tail
	// uncomment two commented lines below to move entire tail
	// instead of just anchor point

	//ptTail[0].x -= nAdjustX;
	ptTail[1].x -= nAdjustX;
	//ptTail[2].x -= nAdjustX;
	ptOffs.x	+= nAdjustX;
	ptOffs.y	+= nAdjustY;

	// place window
	MoveWindow(ptOffs.x, ptOffs.y, sizeWnd.cx, sizeWnd.cy, TRUE);

	// apply region
	CRgn rgnTail;
	rgnTail.CreatePolygonRgn(&ptTail[0], 3, ALTERNATE);

	CRgn rgnRound;
	rgnRound.CreateRoundRectRgn(ptTopLeft.x,ptTopLeft.y,ptBottomRight.x,ptBottomRight.y,nTipBound*3,nTipBound*3);

	CRgn rgnComplete;
	rgnComplete.CreateRectRgn(0,0,1,1);
	//rgnComplete.CombineRgn(rgnTail, rgnRound, RGN_OR);
	//rgnComplete.CombineRgn(rgnRound, RGN_COPY);

	rgnComplete.CopyRgn( rgnRound );

	bool bRegionChanged=true;
	if(m_rgnComplete.IsNull())
	{
		m_rgnComplete.CreateRectRgn(0,0,1,1);
	}
	else if(m_rgnComplete.EqualRgn(rgnComplete))
	{
		bRegionChanged=false;
	}
	

	if(bRegionChanged)
	{
		m_rgnComplete.CopyRgn(rgnComplete);

		SetWindowRgn((HRGN)rgnComplete.Detach(), TRUE);
	}	
	UpdateWindow();
}


//
//
//
void CBalloonHelp::ShowWindow()
{
	Animate(TRUE);
}

//
//
//
void CBalloonHelp::CloseWindow()
{
	if(IsWindow())
	{
		Animate(FALSE);
		PostMessage(WM_CLOSE);
	}

	m_Permission = 0;

	if( __CloseWindowCallback )
	{
		__CloseWindowCallback(__pObject);
	}

}

//
//
//
void CBalloonHelp::Animate(BOOL bShow)
{
	typedef BOOL (WINAPI* FnAnimateWindow)(IN HWND hWnd,IN DWORD dwTime,IN DWORD dwFlags);

	ATLASSERT(IsWindow());
	
	BOOL bAnimate=bShow?(!(m_dwOptions & BODisableFadeIn)):(!(m_dwOptions & BODisableFadeOut));
	if(bAnimate)
	{
		BOOL bFade=FALSE;
		::SystemParametersInfo(SPI_GETTOOLTIPANIMATION, 0, &bFade, 0);
		if(bFade)
			::SystemParametersInfo(SPI_GETTOOLTIPFADE, 0, &bFade, 0);

		if(bFade)
		{
			HMODULE hModule=::LoadLibrary(_T("User32.dll"));
			FnAnimateWindow AnimateWnd=(FnAnimateWindow)::GetProcAddress(hModule,"AnimateWindow");
			
			if(NULL!=AnimateWnd)
			{
				DWORD dwFlags=AW_BLEND;
				if(!bShow)
					dwFlags|=AW_HIDE;

				if(0!=AnimateWnd(m_hWnd,nAnimDuration,dwFlags))
					return;
			}
		}
	}

	WindowBase::ShowWindow(SW_SHOWNA);
}



//
//
//
void CBalloonHelp::OnPaint(HDC hdc)
{
	if(NULL==hdc)
	{
		CPaintDC dc(m_hWnd);
		{
			//CMemDC dcMem(dc);
			Draw(dc);
		}
	}
	else
	{
		Draw(hdc);
	}
}

//
//	draw balloon shape & boarder
//
void CBalloonHelp::OnNcPaint(HRGN)
{
	CWindowDC dc(m_hWnd);
	DrawNc(dc);
}

//
//
//
LRESULT CBalloonHelp::OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam)
{
	NCCALCSIZE_PARAMS* pncsp=(NCCALCSIZE_PARAMS*)lParam;

	// nTipMargin pixel margin on all sides
	::InflateRect(&pncsp->rgrc[0], -4, -4);

	// nTipTail pixel "tail" on side closest to anchor
	switch (GetBalloonQuadrant())
	{
	case BQTopRight:
	case BQTopLeft:
		pncsp->rgrc[0].top += nTipTail;
		break;
	case BQBottomRight:
	case BQBottomLeft:
		pncsp->rgrc[0].bottom -= nTipTail;
		break;
	}

	// sanity: ensure rect does not have negative size
	if( pncsp->rgrc[0].right < pncsp->rgrc[0].left)
		pncsp->rgrc[0].right = pncsp->rgrc[0].left;
	if( pncsp->rgrc[0].bottom < pncsp->rgrc[0].top)
		pncsp->rgrc[0].bottom = pncsp->rgrc[0].top;

	return 0;
}


//
//	OnCaptureChanged
//
void CBalloonHelp::OnCaptureChanged(HWND)
{
	if(m_dwOptions & BOShowCloseButton)
	{
		m_uCloseState=0;

		CClientDC dc(m_hWnd);

		CRect rcCloseBtn;
		GetClientRect(&rcCloseBtn);
		rcCloseBtn.left	  = rcCloseBtn.right-nCXCloseBtn;
		rcCloseBtn.bottom = rcCloseBtn.top  +nCYCloseBtn;
		
		dc.DrawFrameControl(rcCloseBtn, DFC_CAPTION, DFCS_CAPTIONCLOSE|DFCS_FLAT);
	}

}// OnCaptureChanged


//
//	do mouse tracking:
//	Close on mouse move;
//	Tracking for close button;
//
void CBalloonHelp::OnMouseMove(UINT, CPoint pt)
{
	if(m_dwOptions & BOShowCloseButton)
	{
		CRect rcCloseBtn;
		GetClientRect(&rcCloseBtn);
		rcCloseBtn.left	  = rcCloseBtn.right-nCXCloseBtn;
		rcCloseBtn.bottom = rcCloseBtn.top  +nCYCloseBtn;

		UINT uState = DFCS_CAPTIONCLOSE;
		BOOL bPushed= m_uCloseState&DFCS_PUSHED;

		m_uCloseState &= ~DFCS_PUSHED;

		if(rcCloseBtn.PtInRect(pt))
		{
			uState |= DFCS_HOT;

			if(bPushed)
				uState |= DFCS_PUSHED;

			SetTimer(IdTimerHotTrack, TimerHotTrackElapse);
		}
		else
		{
			uState |= DFCS_FLAT;
		}

		if(uState!=m_uCloseState )
		{
			CClientDC dc(m_hWnd);
			dc.DrawFrameControl(rcCloseBtn, DFC_CAPTION, uState);
			m_uCloseState = uState;
		}

		if(bPushed)
			m_uCloseState |= DFCS_PUSHED;
	}
}

//
//	Close button handler
//
void CBalloonHelp::OnLButtonDown(UINT, CPoint pt) 
{
	if(m_dwOptions & BOShowCloseButton)
	{
		CRect rcClient;
		GetClientRect(&rcClient);

		rcClient.left  = rcClient.right-nCXCloseBtn;
		rcClient.bottom= rcClient.top+nCYCloseBtn;

		if(rcClient.PtInRect(pt))
		{
			m_uCloseState|=DFCS_PUSHED;
			SetCapture();
			OnMouseMove(0, pt);
		}
	}
}


//
//	Close button handler
//
void CBalloonHelp::OnLButtonUp(UINT, CPoint pt) 
{
	if( (m_dwOptions & BOShowCloseButton) && (m_uCloseState & DFCS_PUSHED))
	{
		ReleaseCapture();
		m_uCloseState&=~DFCS_PUSHED;

		CRect rcClient;
		GetClientRect(&rcClient);

		rcClient.left  = rcClient.right-nCXCloseBtn;
		rcClient.bottom= rcClient.top  +nCYCloseBtn;

		if(rcClient.PtInRect(pt))
			CloseWindow();
	}
	else if ( !m_strURL.IsEmpty() )
	{
		CloseWindow();
		::ShellExecute(NULL, NULL, m_strURL, NULL, NULL, SW_SHOWNORMAL);
	}
}

	// This is the correct version. Fixed in WTL 7.0
	void CBalloonHelp::OnTimer(UINT nIDEvent)
	{
	if(IdTimerHotTrack==nIDEvent)
	{
		KillTimer(IdTimerHotTrack);

		CPoint pt;
		::GetCursorPos(&pt);
		ScreenToClient(&pt);

		OnMouseMove(0, pt);
	}
	else if(IdTimerClose==nIDEvent)
	{
		KillTimer(IdTimerClose);
		CloseWindow();
	}
	else
	{
		SetMsgHandled(FALSE);
	}
}

//
//
//
void CBalloonHelp::OnActivateApp(BOOL bActivate, DWORD dwTask)
{
	if(!bActivate && (m_dwOptions & BOCloseOnAppDeactivate))
		CloseWindow();
}


//
//	AnimateWindow needs this
//
void CBalloonHelp::OnPrint(HDC hdc,UINT Flags)
{
	ATLASSERT(hdc);

	CDCHandle dc(hdc);

	CPoint ptOrg;
	dc.GetViewportOrg(&ptOrg);

	if(PRF_ERASEBKGND & Flags)
	{
		DrawBkgnd(dc);
	}

	if(PRF_NONCLIENT & Flags)
	{
		DrawNc(dc, FALSE);

		CRect rcWnd;
		GetWindowRect(rcWnd);
		rcWnd.OffsetRect(-rcWnd.TopLeft());

		CRect rc(rcWnd);
		OnNcCalcSize(FALSE,(LPARAM)&rc);

		dc.SetViewportOrg(rc.left-rcWnd.left,rc.top-rcWnd.top);
	}

	if(PRF_CLIENT & Flags)
	{
		Draw(dc);
	}

	dc.SetViewportOrg(ptOrg);
}

//
//
//
void CBalloonHelp::OnFinalMessage(HWND)
{
	if(m_dwOptions & BODeleteThisOnClose)
	{
		delete this;
	}
}

//
//
//
void CBalloonHelp::SetKeyboardHook()
{
	//if(NULL==m_hKeybHook)
	//{
	//	m_hKeybHook=::SetWindowsHookEx(
	//		WH_KEYBOARD,
	//		(HOOKPROC)KeybHook::GetThunk(),
	//		NULL,
	//		::GetCurrentThreadId());
	//}
}

//
//
//
void CBalloonHelp::RemoveKeyboardHook()
{
	//if(NULL!=m_hKeybHook)
	//{
	//	::UnhookWindowsHookEx(m_hKeybHook);
	//	m_hKeybHook=NULL;
	//}
}

//
//
//
LRESULT CBalloonHelp::KeyboardHookProc( int code, WPARAM wParam, LPARAM lParam)
{
	// Skip if the key was released or if it's a repeat
	// Bit 31:	Specifies the transition state. The value is 0 if the key  
	//			is being pressed and 1 if it is being released (see MSDN).
	if(code>=0 && !(lParam&0x80000000))
	{
		CloseWindow();
	}
	return ::CallNextHookEx(GetKeybHookHandle(), code, wParam, lParam);
}


//
//
//
void CBalloonHelp::SetMouseHook()
{
	//if(NULL==m_hMouseHook)
	//{
	//	m_hMouseHook=::SetWindowsHookEx(
	//		WH_MOUSE,
	//		(HOOKPROC)MouseHook::GetThunk(),
	//		NULL,
	//		::GetCurrentThreadId());
	//}
}

//
//
//
void CBalloonHelp::RemoveMouseHook()
{
	//if(NULL!=m_hMouseHook)
	//{
	//	::UnhookWindowsHookEx(m_hMouseHook);
	//	m_hMouseHook=NULL;
	//}
}
//
//
//
LRESULT CBalloonHelp::MouseHookProc(int code, WPARAM wParam, LPARAM lParam)
{
	if(code>=0)
	{
		const UINT uMsg=(UINT)wParam;

		if( (uMsg==WM_MOUSEMOVE) )
		{
			if((m_dwOptions & BOCloseOnMouseMove))
			{
				CPoint pt;
				::GetCursorPos(&pt);
				if((abs(pt.x-m_ptMouseOrg.x) > m_nMouseMoveTolerance || abs(pt.y-m_ptMouseOrg.y) > m_nMouseMoveTolerance) )
					CloseWindow();
			}
		}
		else if( (uMsg==WM_NCLBUTTONDOWN || uMsg==WM_LBUTTONDOWN) )
		{
			if((m_dwOptions & BOCloseOnLButtonDown))
				CloseWindow();
		}
		else if( (uMsg==WM_NCMBUTTONDOWN || uMsg==WM_MBUTTONDOWN) )
		{
			if((m_dwOptions & BOCloseOnMButtonDown))
				CloseWindow();
		}
		else if( (uMsg==WM_NCRBUTTONDOWN || uMsg==WM_RBUTTONDOWN) )
		{
			if((m_dwOptions& BOCloseOnRButtonDown))
				CloseWindow();
		}
		else if( (uMsg==WM_NCLBUTTONUP || uMsg==WM_LBUTTONUP) )
		{
			if((m_dwOptions & BOCloseOnLButtonUp))
				CloseWindow();
		}
		else if( (uMsg==WM_NCMBUTTONUP || uMsg==WM_MBUTTONUP) )
		{
			if((m_dwOptions & BOCloseOnMButtonUp))
				CloseWindow();
		}
		else if( (uMsg==WM_NCRBUTTONUP || uMsg==WM_RBUTTONUP) )
		{
			if((m_dwOptions & BOCloseOnRButtonUp))
				CloseWindow();
		}
	}

	return ::CallNextHookEx(GetMouseHookHandle(), code, wParam, lParam);
}

//
//
//
void CBalloonHelp::SetCallWndRetHook()
{
	if(NULL==m_hCallWndRetHook)
	{
		//m_hCallWndRetHook=::SetWindowsHookEx(
		//	WH_CALLWNDPROCRET,
		//	(HOOKPROC)CallWndRetHook::GetThunk(),
		//	NULL,
		//	::GetCurrentThreadId());
	}
}

//
//
//
void CBalloonHelp::RemoveCallWndRetHook()
{
	//if(NULL!=m_hCallWndRetHook)
	//{
	//	::UnhookWindowsHookEx(m_hCallWndRetHook);
	//	m_hCallWndRetHook=NULL;
	//}
}

//
//
//	
LRESULT CBalloonHelp::CallWndRetProc(int code, WPARAM wParam, LPARAM lParam)
{
	if(code>=0)
	{
		static BOOL bInCall=FALSE;

		CWPRETSTRUCT* pcwpr=(CWPRETSTRUCT*)lParam;
		
		if(WM_MOVE==pcwpr->message && !bInCall)
		{
			bInCall=TRUE;

			if(m_Anchor.AnchorPointChanged())
				PositionWindow();

			bInCall=FALSE;
		}
	}
	
	return ::CallNextHookEx(GetCallWndRetHandle(), code, wParam, lParam);
}

#ifndef BALLOON_HELP_NO_NAMESPACE
}// namespace WTL
#endif

// eof