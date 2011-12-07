//////////////////////////////////////////////////////////////////////////
// Name:	    PictureBox Header
// Version:		1.0
// Date:	    24-7-2004
// Author:	    Trilobyte
// Copyright:   (c) Trilobyte-Solutions 2004-2008
// Desciption:
// Defines CPictureBox and CMemDC in the WTL namespace
//////////////////////////////////////////////////////////////////////////
#ifndef __PICTUREBOX_H
#define __PICTUREBOX_H

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

#define UM_TIMER_GIFNEXTFRAME (WM_USER+0x01)

#include "dwmapi_delegate.h"

namespace WTL
{
	// CMemDC is an assistant for drawing
#ifndef _MEMDC
#define _MEMDC
	class CMemDC: public CDC
	{
	public:
		CDCHandle     m_hOwnerDC;	// Owner DC
		CBitmap       m_bitmap;		// Offscreen bitmap
		CBitmapHandle m_hOldBitmap;	// Originally selected bitmap
		RECT          m_rcOwner;	// Rectangle of drawing area
		

		CMemDC(HDC hDC, LPRECT pRect = NULL)
		{
			ATLASSERT(hDC!=NULL);
			m_hOwnerDC = hDC;
			if( pRect != NULL )
				m_rcOwner = *pRect; 
			else
				m_hOwnerDC.GetClipBox(&m_rcOwner);

			CreateCompatibleDC(m_hOwnerDC);
			::LPtoDP(m_hOwnerDC, (LPPOINT) &m_rcOwner, sizeof(RECT)/sizeof(POINT));
			m_bitmap.CreateCompatibleBitmap(m_hOwnerDC, m_rcOwner.right - m_rcOwner.left, m_rcOwner.bottom - m_rcOwner.top);
			m_hOldBitmap = SelectBitmap(m_bitmap);
			::DPtoLP(m_hOwnerDC, (LPPOINT) &m_rcOwner, sizeof(RECT)/sizeof(POINT));
			SetWindowOrg(m_rcOwner.left, m_rcOwner.top);
		}
		~CMemDC()
		{
			// Copy the offscreen bitmap onto the screen.
			m_hOwnerDC.BitBlt(m_rcOwner.left, m_rcOwner.top, m_rcOwner.right - m_rcOwner.left, m_rcOwner.bottom - m_rcOwner.top, m_hDC, m_rcOwner.left, m_rcOwner.top, SRCCOPY);
			//Swap back the original bitmap.
			SelectBitmap(m_hOldBitmap);
		}
	};
#endif
    // Picture box Styles and class
#define PICTUREBOX_MENU_CENTER	WM_APP + 1
#define PICTUREBOX_MENU_STRETCH	WM_APP + 2
#define PICTUREBOX_MENU_CLIPPED WM_APP + 3

	enum PICTUREBOX
	{
		PICTUREBOX_CENTER	= 0x0001,
		PICTUREBOX_STRETCH	= 0x0002,
		PICTUREBOX_MENU		= 0x0004,
		PICTUREBOX_OWNER	= 0x0008,
		PICTUREBOX_CLIPPED  = 0x0010,
	};


	class CPictureBox:	public WTL::CScrollWindowImpl<CPictureBox>
	{
	protected:
		POINT				m_CenterOffset;
		Gdiplus::Bitmap		*m_Bitmap;
		int					iphotoframecurrent;
		int					iphotoframecount;
		int					iphotoframeindex;
		GUID*				pphotoframeDimensionIDs;
		PropertyItem*		pphotoframeItem;
		BOOL				m_Clipped;
		BOOL				m_FillWhiteBoard;
	public:
		DWORD				m_dwStyle;
		ATL::CImage			m_Image;
		WTL::CMenu			m_Menu;
	public:
		// Constructor/Destructor
				CPictureBox()
				{
					pphotoframeItem = NULL;
					pphotoframeDimensionIDs = NULL;
					iphotoframecurrent = 0;
					iphotoframecount = 0;
					m_Bitmap = NULL;
					m_Clipped = FALSE;
					m_FillWhiteBoard = FALSE;
					m_dwStyle = PICTUREBOX_CENTER | PICTUREBOX_MENU | PICTUREBOX_OWNER;
				}

				~CPictureBox()
				{
					if (!m_Image.IsNull() && m_dwStyle & PICTUREBOX_OWNER)
						m_Image.Destroy();
					if (m_Menu.IsMenu())
						m_Menu.DestroyMenu();

					if( m_Bitmap )
					{
						delete m_Bitmap;
					}

					if( pphotoframeDimensionIDs )
					{
						delete[] pphotoframeDimensionIDs;
					}

					if( pphotoframeItem )
					{
						free( pphotoframeItem );
					}
				}

		// ATL Message map stuff
		DECLARE_WND_CLASS(NULL)
		BEGIN_MSG_MAP(CPictureBox)
			MSG_WM_CREATE(OnCreate)
			MSG_WM_ERASEBKGND(OnEraseBkGnd)
			MESSAGE_RANGE_HANDLER(WM_MOUSEFIRST, WM_MOUSELAST, OnMouseMessage)
			MSG_WM_RBUTTONDOWN(OnRMouseDown)
			MSG_WM_MOUSEMOVE(OnMouseMove)
			MSG_WM_MOUSEHOVER(OnMouseHover)
			MSG_WM_TIMER(OnTimer)
			COMMAND_ID_HANDLER_EX(PICTUREBOX_MENU_CENTER, OnCenterImage)
			COMMAND_ID_HANDLER_EX(PICTUREBOX_MENU_STRETCH, OnStretchImage)
			COMMAND_ID_HANDLER_EX(PICTUREBOX_MENU_CLIPPED, OnClippedImage)
			CHAIN_MSG_MAP(WTL::CScrollWindowImpl<CPictureBox>)
			CHAIN_MSG_MAP_ALT(WTL::CScrollWindowImpl<CPictureBox>, 1)
			DEFAULT_REFLECTION_HANDLER()
		END_MSG_MAP()
		// Message handlers
		virtual LRESULT	OnCreate(LPCREATESTRUCT)
		{
			SetMsgHandled(false);

			// Create the menu
			m_Menu.CreatePopupMenu();

			CString szCenterImage(_T("Center Image"));
			CString szStretchImage(_T("Stretch Image"));
			CString szClippedImage(_T("Clipp Image"));
			if( OnCreateMenuItem( PICTUREBOX_MENU_CENTER, szCenterImage ) )
			{
				m_Menu.AppendMenu(MF_STRING, PICTUREBOX_MENU_CENTER, szCenterImage);
			}

			if( OnCreateMenuItem( PICTUREBOX_MENU_STRETCH, szStretchImage ) )
			{
				m_Menu.AppendMenu(MF_STRING, PICTUREBOX_MENU_STRETCH,szStretchImage );
			}

			if( OnCreateMenuItem( PICTUREBOX_MENU_CLIPPED, szClippedImage ) )
			{
				m_Menu.AppendMenu(MF_STRING, PICTUREBOX_MENU_CLIPPED,szClippedImage );
			}
			//if( IsCompositionEnabled() )
			//{
			//	MARGINS mar = {-1};
			//	_DwmExtendFrameIntoClientArea ( m_hWnd, &mar );
			//}

			// Update the scroll view
			UpdateScrollView();
			UpdateMenu();

			return 0;
		}


		virtual BOOL OnCreateMenuItem(UINT nPosMenu, CString& MenuText )
		{
			return TRUE;
		}

		LRESULT	OnEraseBkGnd(HDC)
		{
			SetMsgHandled(false);
			if( ::IsCompositionEnabled() )
			{
				CRect rcGlassArea;
				CPaintDC dc(m_hWnd);
				GetClientRect( rcGlassArea );
				//rcGlassArea.bottom = 150;
				dc.FillSolidRect(rcGlassArea, RGB(0,0,0));
			}else
			{
				CRect rcGlassArea;
				CPaintDC dc(m_hWnd);
				GetClientRect( rcGlassArea );
				//rcGlassArea.bottom = 150;
				dc.FillSolidRect(rcGlassArea, RGB(0xFF,0xFF,0xFF));
			}
			return 0;
		}

		virtual LRESULT OnMouseHover(WPARAM wParam, CPoint ptPos)
		{
			return 0;
		}

		virtual void OnMouseMove(UINT nFlags, CPoint point)
		{
			
		}

		virtual LRESULT OnMouseMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			bHandled = FALSE;
			return 0;
		}

		void	OnRMouseDown(UINT, CPoint)
		{
			if (m_Menu.IsMenu() && (m_dwStyle & PICTUREBOX_MENU))
			{
				POINT ptPoint;
				GetCursorPos(&ptPoint);
				m_Menu.TrackPopupMenu(TPM_RIGHTALIGN, ptPoint.x, ptPoint.y, m_hWnd);
			}
		}
		void	OnCenterImage(UINT, int, HWND)
		{
			if (m_dwStyle & PICTUREBOX_CENTER)
				CenterPicture(false);
			else
				CenterPicture(true);
		}
		
		void	OnStretchImage(UINT, int, HWND)
		{
			if (m_dwStyle & PICTUREBOX_STRETCH)
				StretchPicture(false);
			else
				StretchPicture(true);
		}

		void	OnClippedImage(UINT, int, HWND)
		{
			if (m_dwStyle & PICTUREBOX_CLIPPED)
				ClippedPicture(false);
			else
				ClippedPicture(true);
		}

		void ClippedPicture( bool clipped )
		{
			if (clipped)
				m_dwStyle |= PICTUREBOX_CLIPPED;
			else
				m_dwStyle &= ~PICTUREBOX_CLIPPED;
			
			UpdateScrollView();

			UpdateMenu();
			
			Invalidate();
		}

		void CheckImageAsGif()
		{
			iphotoframeindex = m_Bitmap->GetFrameDimensionsCount();
			
			if( pphotoframeDimensionIDs )
			{
				delete[] pphotoframeDimensionIDs;
			}

			pphotoframeDimensionIDs = (GUID*)new GUID[iphotoframeindex];
			
			m_Bitmap->GetFrameDimensionsList(pphotoframeDimensionIDs, iphotoframeindex);
			
			WCHAR strGuid[64] = {0};
			
			StringFromGUID2(pphotoframeDimensionIDs[0], strGuid, 64);
			
			iphotoframecount = m_Bitmap->GetFrameCount( &pphotoframeDimensionIDs[0] );

			if (iphotoframecount==1)	//not dynamic GIF, only one frame
			{
				return;
			}

			UINT TotalBuffer = m_Bitmap->GetPropertyItemSize( PropertyTagFrameDelay );
			
			if( pphotoframeItem != NULL )
			{
				free( pphotoframeItem );
			}

			if( TotalBuffer <= 0 )
			{
				return;
			}

			pphotoframeItem = (PropertyItem*) malloc( TotalBuffer );
			
			m_Bitmap->GetPropertyItem( PropertyTagFrameDelay, TotalBuffer, pphotoframeItem);

			iphotoframecurrent = 0;
			
			GUID Guid = FrameDimensionTime;

			if( Ok != m_Bitmap->SelectActiveFrame(&Guid,iphotoframecurrent) ) // Get current frame
			{
				return;
			}
			
			int delay = ((UINT*)pphotoframeItem[0].value)[iphotoframecurrent];
			
			if (0==delay)							// No delay time, use default 100ms
			{
				delay = 100;
			}

			SetTimer(	UM_TIMER_GIFNEXTFRAME, delay, NULL );

			++ iphotoframecurrent;
			//g_bGIFStoped = FALSE;
		}


		void OnTimer(UINT_PTR nIDEvent)
		{
			if( nIDEvent == UM_TIMER_GIFNEXTFRAME )
			{
				KillTimer(nIDEvent);

				if ( iphotoframecount == 1 )
				{
					return;
				}

				GUID Guid = FrameDimensionTime;
				
				if( Ok == m_Bitmap->SelectActiveFrame( &Guid, iphotoframecurrent ) )
				{
				
					HBITMAP hBitmap = NULL;
					
					m_Bitmap->GetHBITMAP( Color(0,0,0), &hBitmap );
					
					if( hBitmap != NULL )
					{
						//m_Image.Attach( hBitmap );
						SetBitmap( hBitmap );
					}

					int delay = ((UINT*) pphotoframeItem[0].value )[ iphotoframecurrent ] * 10;	// i also want to know why mul 10 here??
					
					if( 0 == delay)
							delay = 100;


					SetTimer( UM_TIMER_GIFNEXTFRAME, delay, NULL); 
					
					iphotoframecurrent = ( ++ iphotoframecurrent) % iphotoframecount;
				}

			}
		}

		
		BOOL IsStretch()
		{
			return m_dwStyle & PICTUREBOX_STRETCH;
		}

		POINT& GetCenterOffset()
		{
			return m_CenterOffset;
		}

		void DoPrint( HDC hPrintDC, CRect& rcClient  )
		{
			CMemDC mDC( hPrintDC );
			CDCHandle dcBitmap(m_Image.GetDC());
			mDC.StretchBlt(rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), dcBitmap, 0, 0, m_Image.GetWidth(), m_Image.GetHeight(), SRCCOPY);
			m_Image.ReleaseDC();
		}

		// Overloaded from CScrollWindowImpl<>
		void	DoPaint(CDCHandle hDC)
		{
			CMemDC		mDC(hDC.m_hDC);
			CRect		rcClient;
			POINT		ptScrollOffset;

			GetClientRect(&rcClient);
			GetScrollOffset(ptScrollOffset);
			rcClient.OffsetRect(ptScrollOffset);
			if( m_FillWhiteBoard )
			{
				mDC.FillRect(&rcClient, AtlGetStockBrush(WHITE_BRUSH));
			}else if( !IsCompositionEnabled() )
			{
				DWORD dwFacesColor = GetSysColor( COLOR_3DFACE );
				HBRUSH hBrush = CreateSolidBrush( dwFacesColor );
				mDC.FillRect(&rcClient, hBrush );
				DeleteObject( hBrush );
			}

			m_CenterOffset.x = 0;
			m_CenterOffset.y = 0;

			if (!m_Image.IsNull())
			{
				if (m_dwStyle & PICTUREBOX_STRETCH)
				{
					CDCHandle dcBitmap(m_Image.GetDC());
					mDC.StretchBlt(rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), dcBitmap, 0, 0, m_Image.GetWidth(), m_Image.GetHeight(), SRCCOPY);
					m_Image.ReleaseDC();
				}
				else
				{
					CDCHandle	dcBitmap(m_Image.GetDC());
					CRect		mRect(0, 0, m_Image.GetWidth(), m_Image.GetHeight());

					if (m_dwStyle & PICTUREBOX_CENTER)
					{
						int x = rcClient.Width() / 2 - m_Image.GetWidth() / 2;
						int y = rcClient.Height() / 2 - m_Image.GetHeight() / 2;

						if (x < 0)
							x = 0;
						if (y < 0)
							y = 0;

						
						m_CenterOffset.x = x;
						m_CenterOffset.y = y;

						mRect.right = x + mRect.Width();
						mRect.bottom = y + mRect.Height();
						mRect.left = x;
						mRect.top = y;
					}

					if( m_dwStyle & PICTUREBOX_CLIPPED )
					{	
						mDC.BitBlt(mRect.left, mRect.top, rcClient.Width(), rcClient.Height(), dcBitmap, 0, 0, SRCCOPY);
					}else{
						mDC.BitBlt(mRect.left, mRect.top, mRect.Width(), mRect.Height(), dcBitmap, 0, 0, SRCCOPY);
					}
					m_Image.ReleaseDC();
				}
			}
		}

		int		ScrollWindowEx(int, int, UINT)
		{
			Invalidate();
			return 0;
		}

		// Picture functions
		HBITMAP	GetBitmap()
		{
			m_dwStyle &= ~PICTUREBOX_OWNER;
			return m_Image.Detach();
		}

		HBITMAP	GetSaveBitmap()const
		{
			return m_Image;
		}

		void	SetBitmap(HBITMAP hBitmap, bool bOwner = false)
		{
			if (!m_Image.IsNull())
			{
				if (m_dwStyle & PICTUREBOX_OWNER)
					m_Image.Destroy();
				else
					ATLTRACE(_T("Old bitmap not destroyed, resource leak"));
			}

			//if (bOwner)
			//	m_dwStyle |= PICTUREBOX_OWNER;
			//else
			//	m_dwStyle &= PICTUREBOX_OWNER;

			m_Image.Attach(hBitmap);

			//UseMenu( true );

			if (IsWindow())
			{
				//if( !(m_dwStyle & PICTUREBOX_CLIPPED) && !(m_dwStyle & PICTUREBOX_STRETCH) )
				//{
				//UpdateScrollView();
				//}

				//UpdateScrollView();
				Invalidate(FALSE);
				
				if( (m_dwStyle & PICTUREBOX_MENU) )
				{
					UpdateMenu();
				}
				//::InvalidateRect( *this, NULL, TRUE );
				//SendMessage( *this, WM_PAINT, 0, 0 );
				//PostMessage(WM_PAINT, 0, 0 );
			}
		}


		bool LoadBitmapFromFile(LPCTSTR pszFileName)
		{
			if (!m_Image.IsNull())
			{
				if (m_dwStyle & PICTUREBOX_OWNER)
					m_Image.Destroy();
				else
					ATLTRACE(_T("Old bitmap not destroyed, resource leak"));
			}

			Gdiplus::Bitmap *bitmap = Gdiplus::Bitmap::FromFile( pszFileName );

			if( bitmap != NULL )
			{
				HBITMAP hBitmap = NULL;

				if( m_Bitmap != NULL )
				{
					delete m_Bitmap;
				}
				m_Bitmap = bitmap;

				CheckImageAsGif();

				m_Bitmap->GetHBITMAP( Color(0,0,0), &hBitmap );
				
				if( hBitmap != NULL )
				{
					m_Image.Attach( hBitmap );
				}

			}else{
				if (m_Image.Load(pszFileName) == S_OK)
				{
					m_dwStyle |= PICTUREBOX_OWNER;
				}
			}

			if (IsWindow())
			{
				UpdateScrollView();
				Invalidate();
			}

			return true;
			//return false;
		}

		bool	LoadBitmapFromID(HINSTANCE hInstance, UINT uiIDResource)
		{

			if (!m_Image.IsNull())
			{
				if (m_dwStyle & PICTUREBOX_OWNER)
					m_Image.Destroy();
				else
					ATLTRACE(_T("Old bitmap not destroyed, resource leak"));
			}
			
			Gdiplus::Bitmap *bitmap = Gdiplus::Bitmap::FromResource( hInstance, MAKEINTRESOURCE(uiIDResource) );

			if( bitmap != NULL )
			{
				HBITMAP hBitmap = NULL;
				bitmap->GetHBITMAP( Gdiplus::Color(0,0,0), &hBitmap );
				if( hBitmap != NULL )
				{
					m_Image.Attach( hBitmap );
				}
				delete bitmap;
			}else{
				m_Image.LoadFromResource(hInstance, uiIDResource);
				m_dwStyle |= PICTUREBOX_OWNER;
			}

			if (IsWindow())
			{
				UpdateScrollView();
				Invalidate();
			}
			
			return true;
		}

		void	CenterPicture(bool bCenter)
		{
			if (bCenter)
				m_dwStyle |= PICTUREBOX_CENTER;
			else
				m_dwStyle &= ~PICTUREBOX_CENTER;

			UpdateMenu();
			Invalidate();
		}
		void	StretchPicture(bool bStretch)
		{
			if (bStretch)
				m_dwStyle |= PICTUREBOX_STRETCH;
			else
				m_dwStyle &= ~PICTUREBOX_STRETCH;

			UpdateMenu();
			UpdateScrollView();
			Invalidate();
		}
		// Misc functions
		void	UseMenu(bool bUseMenu)
		{
			if (bUseMenu)
				m_dwStyle |= PICTUREBOX_MENU;
			else
				m_dwStyle &= ~PICTUREBOX_MENU;
		}
	
		// Misc functions
		void	UpdateScrollView()
		{
			int iWidth = 0;
			int iHeight = 0;
			if (!m_Image.IsNull())
			{
				iWidth = m_Image.GetWidth();
				iHeight = m_Image.GetHeight();
			}

			if (m_dwStyle & PICTUREBOX_STRETCH || m_dwStyle & PICTUREBOX_CLIPPED)
			{
				iWidth = 0;
				iHeight = 0;
			}

			if (iWidth < 1)
				iWidth = 1;
			if (iHeight < 1)
				iHeight = 1;

			RECT	rcClient;
			POINT	ptScrollOffset;
			SIZE	szSize;

			GetClientRect(&rcClient);
			GetScrollOffset(ptScrollOffset);

			// Calculate the new virtual size
			SetScrollSize(iWidth, iHeight);
			SetScrollLine(100, 100);
			SetScrollPage(rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);

			// Prevent the window to be larger then the virtual view
			GetScrollSize(szSize);
			if (ptScrollOffset.y + rcClient.bottom - rcClient.top > szSize.cy)
			{
				if (rcClient.bottom - rcClient.top > szSize.cy)
					ptScrollOffset.y = 0;
				else
					ptScrollOffset.y = szSize.cy - rcClient.bottom - rcClient.top;
			}
			if (ptScrollOffset.x + rcClient.right - rcClient.left > szSize.cx)
			{
				if (rcClient.right - rcClient.left > szSize.cx)
					ptScrollOffset.x = 0;
				else
					ptScrollOffset.x = szSize.cx - rcClient.right - rcClient.left;
			}	
			SetScrollOffset(ptScrollOffset);
		}
		void	UpdateMenu()
		{
			if (m_Menu.IsMenu())
			{
				if (m_dwStyle & PICTUREBOX_CENTER)
					m_Menu.CheckMenuItem(PICTUREBOX_MENU_CENTER, MF_BYCOMMAND|MF_CHECKED);
				else
					m_Menu.CheckMenuItem(PICTUREBOX_MENU_CENTER, MF_BYCOMMAND|MF_UNCHECKED);

				if (m_dwStyle & PICTUREBOX_CLIPPED)
					m_Menu.CheckMenuItem(PICTUREBOX_MENU_CLIPPED, MF_BYCOMMAND|MF_CHECKED);
				else
					m_Menu.CheckMenuItem(PICTUREBOX_MENU_CLIPPED, MF_BYCOMMAND|MF_UNCHECKED);

				if (m_dwStyle & PICTUREBOX_STRETCH)
				{
					m_Menu.CheckMenuItem(PICTUREBOX_MENU_STRETCH, MF_BYCOMMAND|MF_CHECKED);
					m_Menu.EnableMenuItem(PICTUREBOX_MENU_CENTER, MF_BYCOMMAND|MF_GRAYED);
					m_Menu.EnableMenuItem(PICTUREBOX_MENU_CLIPPED, MF_BYCOMMAND|MF_GRAYED);
				}
				else
				{
					m_Menu.CheckMenuItem(PICTUREBOX_MENU_STRETCH, MF_BYCOMMAND|MF_UNCHECKED);
					m_Menu.EnableMenuItem(PICTUREBOX_MENU_CENTER, MF_BYCOMMAND|MF_ENABLED);
					m_Menu.EnableMenuItem(PICTUREBOX_MENU_CLIPPED, MF_BYCOMMAND|MF_ENABLED);
				}
			}
		}
		// Variables

	};



// Notes:
// - HLINK_USETAGS and HLINK_USETAGSBOLD are always left-aligned
// - When HLINK_USETAGSBOLD is used, the underlined styles will be ignored

template <class T, class TBase = ATL::CWindow, class TWinTraits = ATL::CControlWinTraits>
class ATL_NO_VTABLE CStaticLabelImpl : public ATL::CWindowImpl< T, TBase, TWinTraits >
{
public:
	LPTSTR m_lpstrLabel;

	HCURSOR m_hCursor;
	HFONT m_hFont;
	HFONT m_hFontNormal;

	RECT m_rcLink;
#ifndef _WIN32_WCE
	CToolTipCtrl m_tip;
#endif // !_WIN32_WCE

	//COLORREF m_clrLink;
	COLORREF m_clrVisited;

	DWORD m_dwExtendedStyle;   // Hyper Link specific extended styles

	bool m_bPaintLabel:1;
	bool m_bVisited:1;
	bool m_bHover:1;
	bool m_bInternalLinkFont:1;


// CStaticLabelImpl/Destructor
	CStaticLabelImpl(DWORD dwExtendedStyle = HLINK_UNDERLINED) : 
			m_lpstrLabel(NULL), 
			m_hCursor(NULL), m_hFont(NULL), m_hFontNormal(NULL),
			m_clrVisited(RGB(128, 0, 128)),
			m_dwExtendedStyle(dwExtendedStyle),
			m_bPaintLabel(true), m_bVisited(false),
			m_bHover(false), m_bInternalLinkFont(false)
	{
		::SetRectEmpty(&m_rcLink);
	}

	~CStaticLabelImpl()
	{
		delete [] m_lpstrLabel;
		//delete [] m_lpstrHyperLink;
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
#if (_MSC_VER >= 1300)
		BOOL bRet = ATL::CWindowImpl< T, TBase, TWinTraits>::SubclassWindow(hWnd);
#else // !(_MSC_VER >= 1300)
		typedef ATL::CWindowImpl< T, TBase, TWinTraits>   _baseClass;
		BOOL bRet = _baseClass::SubclassWindow(hWnd);
#endif // !(_MSC_VER >= 1300)
		if(bRet)
		{
			T* pT = static_cast<T*>(this);
			pT->Init();
		}
		return bRet;
	}


// Message map and handlers
	BEGIN_MSG_MAP(CStaticLabelImpl)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
#ifndef _WIN32_WCE
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_RANGE_HANDLER(WM_MOUSEFIRST, WM_MOUSELAST, OnMouseMessage)
#endif // !_WIN32_WCE
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
#ifndef _WIN32_WCE
		MESSAGE_HANDLER(WM_PRINTCLIENT, OnPaint)
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
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		T* pT = static_cast<T*>(this);
		pT->Init();
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
		if(!m_bPaintLabel)
		{
			bHandled = FALSE;
			return 1;
		}

		T* pT = static_cast<T*>(this);
		if(wParam != NULL)
		{
			pT->DoEraseBackground((HDC)wParam);
			pT->DoPaint((HDC)wParam);
		}
		else
		{
			CPaintDC dc(m_hWnd);
			pT->DoEraseBackground(dc.m_hDC);
			pT->DoPaint(dc.m_hDC);
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
		return 0;
	}

#ifndef _WIN32_WCE
	LRESULT OnMouseLeave(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return 0;
	}
#endif // !_WIN32_WCE

	LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		return 0;
	}

	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		return 0;
	}

	LRESULT OnChar(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
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
		if(m_lpstrLabel == NULL ) //&& m_lpstrHyperLink == NULL)
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
			LPTSTR lpstrText = m_lpstrLabel; //; != NULL) ? m_lpstrLabel : m_lpstrHyperLink;
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

		LPTSTR lpstrText = m_lpstrLabel; //(m_lpstrLabel != NULL) ? m_lpstrLabel : m_lpstrHyperLink;
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
		}
	}

	void DoPaint(CDCHandle dc)
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
				dc.DrawText(lpstrLeft, cchLeft, &rcClient, DT_LEFT | DT_WORDBREAK);

			COLORREF clrOld = dc.SetTextColor(IsWindowEnabled() ? (m_bVisited ? m_clrVisited : m_clrVisited) : (::GetSysColor(COLOR_GRAYTEXT)));
			if(m_hFont != NULL && (!IsUnderlineHover() || (IsUnderlineHover() && m_bHover)))
				dc.SelectFont(m_hFont);
			else
				dc.SelectFont(m_hFontNormal);

			dc.DrawText(lpstrLink, cchLink, &m_rcLink, DT_LEFT | DT_WORDBREAK);

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
			COLORREF clrOld = dc.SetTextColor(IsWindowEnabled() ? (m_bVisited ? m_clrVisited : m_clrVisited) : (::GetSysColor(COLOR_GRAYTEXT)));

			HFONT hFontOld = NULL;
			if(m_hFont != NULL && (!IsUnderlineHover() || (IsUnderlineHover() && m_bHover)))
				hFontOld = dc.SelectFont(m_hFont);
			else
				hFontOld = dc.SelectFont(m_hFontNormal);

			LPTSTR lpstrText = m_lpstrLabel;// (m_lpstrLabel != NULL) ? m_lpstrLabel : m_lpstrHyperLink;

			DWORD dwStyle = GetStyle();
			int nDrawStyle = DT_LEFT;
			if (dwStyle & SS_CENTER)
				nDrawStyle = DT_CENTER;
			else if (dwStyle & SS_RIGHT)
				nDrawStyle = DT_RIGHT;

			dc.DrawText(lpstrText, -1, &m_rcLink, nDrawStyle | DT_WORDBREAK);

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


class CStaticLabel : public CStaticLabelImpl<CStaticLabel>
{
public:
	DECLARE_WND_CLASS(_T("WTL_StaticLabel"))
};

}
#endif