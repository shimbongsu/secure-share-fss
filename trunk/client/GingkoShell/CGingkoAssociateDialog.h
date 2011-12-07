#ifndef __GINGKO_ASSOCIATE_DIALOG_H__
#define __GINGKO_ASSOCIATE_DIALOG_H__
#include "Extends\PictureBox.h"
#include <GingkoLibrary.h>
#include "AeroSupports.h"
#include <vector>
#include <math.h>

#define VERTICAL_SPACE		30
#define HORIZONTAL_SPACE	80
#define	BOX_WIDTH			130
#define	BOX_HEIGHT			30
#define BOX_LEFT			10
#define BOX_TOP				10

typedef struct __AssociateBox
{
	TCHAR* gingkoId;
	TCHAR* assignedBy;
	TCHAR* Name;
	BOOL	Holder;
	BOOL	Owner;
	BOOL	Actived;
	BOOL	Readable;
	BOOL	Writable;
	BOOL	Printable;
	BOOL	Deletable;

	int	   Left;
	int	   Top;
	int	   Right;
	int	   Bottom;
	int	   Color;
	struct __AssociateBox *Parent;	 ///Parent
	struct __AssociateBox *Next;	 ///Next Level
	struct __AssociateBox *Direct;	 ///Direct Level
} AssociateBox;

class CGingkoToolTip :
	public CWindowImpl<CGingkoToolTip, CToolTipCtrl>,
	public CCustomDraw<CGingkoToolTip> 
{
	
public:
	
	typedef CWindowImpl<CGingkoToolTip, CToolTipCtrl> baseClass;

	CGingkoToolTip(HWND hWnd)
	{
		m_hWnd = hWnd;
	}

	//DECLARE_WND_CLASS( NULL )

	DECLARE_WND_SUPERCLASS( NULL, GetWndClassName() )

	BEGIN_MSG_MAP(CGingkoToolTip)
		MSG_WM_CREATE(OnCreate)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	HWND Create(LPCTSTR lpstrWndClass, HWND hWndParent, _U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL) throw()
	{
		HWND hWnd = CWindow::Create( lpstrWndClass, hWndParent, rect, szWindowName, dwStyle,
			dwExStyle, MenuOrID, lpCreateParam );
		//SubclassWindow( hWnd );
		return hWnd;
	}

public:
	LRESULT OnCreate(LPCREATESTRUCT sr)
	{
		return 0;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return 0;
	}

	void OnPaint(CDCHandle dc)
	{
		dc.TextOut( 0, 0, _T("Test CDC. ") );
		//baseClass::OnPaint(dc);
	}

	DWORD OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
	{
		return 0;
	}

	DWORD OnPostPaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
	{
		return 0;
	}

	DWORD OnPreErase(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
	{
		return 0;
	}

	DWORD OnPostErase(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
	{
		return 0;
	}

	DWORD OnItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
	{
		return 0;
	}

	DWORD OnItemPostPaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
	{
		return 0;
	}

	DWORD OnItemPreErase(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
	{
		return 0;
	}

	DWORD OnItemPostErase(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
	{
		return 0;
	}

};

class CAssociatePaint
{
private:
	GingkoDigitalPack*			m_DigitalPack;
	CPictureBox*				m_PictureBox;
	DWORD						m_dwThreadId;
	Gdiplus::Graphics*			m_ThisGraphic;
	AssociateBox*				m_AssociateBox;  ///Set the AbstractOwner (who uploaded this digital).
	std::vector<AssociateBox*>  m_BoxVector;
	int							m_ConvsHeight;
	int							m_ConvsWidth;
	HBITMAP						m_hBitmap;
	HBITMAP						m_hOldBitmap;
	HDC							m_MemDC;
	HTHEME						m_hTheme;
	Gdiplus::Font*				m_Font;

public:
	CAssociatePaint( )
	{
		m_DigitalPack = NULL;

		m_AssociateBox = (AssociateBox*) malloc( sizeof(AssociateBox) );
		if(m_AssociateBox)
		{
			memset( m_AssociateBox, 0, sizeof(AssociateBox) );
			m_AssociateBox->Left = BOX_LEFT;
			m_AssociateBox->Top = BOX_TOP;
			m_AssociateBox->Right = BOX_WIDTH + BOX_LEFT;
			m_AssociateBox->Bottom = BOX_TOP + BOX_HEIGHT;
			m_AssociateBox->Color = 0xFF0F8034; //RGB(15,80,34);
		}

		m_hBitmap = NULL;
		m_ConvsHeight = 240;
		m_ConvsWidth  = 320;

		m_Font = NULL;

		InitFont();
	}

	~CAssociatePaint()
	{
		//if( m_hBitmap != NULL )
		//{
		//	delete m_hBitmap;
		//}
		//if( m_hOldBitmap != NULL )
		//{
		//	delete m_hOldBitmap;
		//}
		Destroy();
		delete m_ThisGraphic;
		m_ThisGraphic = NULL;

		if( m_Font )
		{
			delete m_Font;
		}

	
	}

	void InitFont()
	{
		Gdiplus::FontFamily ff( _T("Arial Unicode MS") );
		//Gdiplus::FontFamily ff( _T("Arial") );
		Gdiplus::Font* font = NULL;
		if( ff.GetLastStatus() == Ok )
		{
			font = new Gdiplus::Font( &ff, 16.0f, FontStyleRegular, Gdiplus::UnitPixel);
		}else
		{
			m_hTheme = OpenThemeData( GetDesktopWindow(), _T("globals") );
			LOGFONT fnt;
			if( S_OK == GetThemeSysFont(m_hTheme, TMT_MSGBOXFONT, &fnt ) )
			{
				Gdiplus::FontFamily SysFamily( fnt.lfFaceName );
				font = new Gdiplus::Font( &SysFamily, 16.0f, FontStyleRegular,Gdiplus::UnitPixel  );
			}
		}

		if( font != NULL )
		{
			m_Font = font;
		}
	}


	void FreeAssociateBox( AssociateBox *pBox )
	{
		if( pBox != NULL )
		{
			if( pBox->Name )
			{
				free( pBox->Name);
			}
			if( pBox->gingkoId )
			{
				free( pBox->gingkoId);
			}
			if( pBox->assignedBy )
			{
				free( pBox->assignedBy);
			}
			free( pBox );
		}
	}

	void Destroy()
	{
		AssociateBox* pBox = m_AssociateBox;
		AssociateBox* pNext;
		while ( pBox != NULL )
		{
			pNext = pBox->Next;
			AssociateBox* pDirect = NULL;
			AssociateBox* pDirectNext = NULL;
			pDirect = pBox->Direct;
			while( pDirect != NULL )
			{
				pDirectNext = pDirect->Direct;
				FreeAssociateBox( pDirect );
				pDirect = pDirectNext;
			}
			FreeAssociateBox( pBox );
			pBox = pNext;
		}
	}


	void DrawArrow(Gdiplus::Graphics *graphic, Gdiplus::Pen *pen, Gdiplus::Point& ptLeft, Gdiplus::Point& ptRight)
	{
	  double slopy , cosy , siny;
	  double Par = 10.0;  //length of Arrow (>)

	  slopy = atan2( (double)( ptLeft.Y - ptRight.Y ), (double)( ptLeft.X - ptRight.X ) );
	  cosy = cos( slopy );
	  siny = sin( slopy ); //need math.h for these functions


	  graphic->DrawLine( pen, ptLeft, ptRight );

	  //here is the tough part - actually drawing the arrows
	  //a total of 6 lines drawn to make the arrow shape
	  graphic->DrawLine( pen, ptLeft.X, ptLeft.Y, ptLeft.X + int(- Par * cosy - ( Par / 2.0 * siny ) ),  ptLeft.Y + int( - Par * siny + ( Par / 2.0 * cosy ) ) );
	  graphic->DrawLine( pen, ptLeft.X, ptLeft.Y, ptLeft.X + int( - Par * cosy + ( Par / 2.0 * siny ) ), ptLeft.Y - int( Par / 2.0 * cosy + Par * siny ) );
	}


	
	void ResizeConvs(int dx, int dy)
	{
		int width = m_ConvsWidth;
		int height = m_ConvsHeight;

		m_ConvsWidth += dx;
		m_ConvsHeight += dy;
		//if( m_ThisGraphic )
		//{
		//	m_ThisGraphic->Flush();
		//	SelectObject( m_ThisGraphic->GetHDC(), m_hOldBitmap );
		//	m_ThisGraphic->ReleaseHDC( m_MemDC );
		//	delete m_ThisGraphic;
		//	m_ThisGraphic = NULL;
		//}

		//HDC hDC = GetDC( NULL );
		//HDC TempDC = CreateCompatibleDC( hDC );
		HBITMAP hBitmap = CreateCompatibleBitmap( m_MemDC, m_ConvsWidth, m_ConvsHeight );
		//ReleaseDC( NULL, hDC );
		if( hBitmap != NULL )
		{
			if( m_ThisGraphic )
			{
				m_ThisGraphic->Flush();
				m_ThisGraphic->ReleaseHDC( m_MemDC );
				delete m_ThisGraphic;
				m_ThisGraphic = NULL;
			}

			m_hBitmap = (HBITMAP)SelectObject( m_MemDC, hBitmap );

			Gdiplus::Graphics* g = new Gdiplus::Graphics(m_MemDC);
			
			g->SetSmoothingMode( SmoothingModeHighQuality );
			g->SetCompositingMode( Gdiplus::CompositingModeSourceOver );
			g->SetCompositingQuality( CompositingQualityHighQuality);
			g->SetInterpolationMode( InterpolationModeHighQuality);
			g->Clear( Gdiplus::Color::White );
			//m_ThisGraphic->Clear( Gdiplus::Color::CornflowerBlue );
			
			Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap(m_hBitmap,  NULL);
			g->DrawImage( bitmap, 0.0f, 0.0f );
			//g->Flush();
			delete bitmap;
			DeleteObject( m_hBitmap );
			m_hBitmap = NULL;

			m_ThisGraphic = g;

			m_hBitmap = hBitmap;
		}
		//DeleteDC( TempDC );
	}

	void WINAPI DoPaint( )
	{
		if( m_PictureBox == NULL )
		{
			return;
		}

		HDC hDC = GetDC( NULL );
		m_MemDC = CreateCompatibleDC( hDC );
		m_hBitmap = CreateCompatibleBitmap( hDC, m_ConvsWidth, m_ConvsHeight );
		m_hOldBitmap = (HBITMAP) SelectObject( m_MemDC, m_hBitmap );
		ReleaseDC( NULL, hDC );

		m_ThisGraphic = new Gdiplus::Graphics(m_MemDC);
		///Gdiplus::Pen pen( Gdiplus::Color(123,123,122) );
		m_ThisGraphic->SetSmoothingMode( SmoothingModeHighQuality );
		m_ThisGraphic->SetCompositingMode( CompositingModeSourceCopy );
		m_ThisGraphic->SetCompositingQuality( CompositingQualityHighQuality);
		m_ThisGraphic->SetInterpolationMode( InterpolationModeHighQuality);
		m_ThisGraphic->Clear( Gdiplus::Color::White );

		if( m_DigitalPack != NULL )
		{
			findAssignedPermission( m_DigitalPack->unitId, m_DigitalPack->digitalId, (LPVOID)this, FetchPermission ); 
			RelinkAssociateBoxes();
		}
		///m_ThisGraphic->DrawLine( &pen, 0, 0, 2000, 2000 );
		m_hBitmap = (HBITMAP) SelectObject( m_MemDC, m_hOldBitmap );
		//DeleteObject( hOldBitmap );
		//DeleteObject( m_hOldBitmap );
		DeleteDC( m_MemDC );
		m_PictureBox->SetBitmap( m_hBitmap, false );
		m_PictureBox->CenterPicture( true );
		m_PictureBox->UseMenu( true );
		m_PictureBox->UpdateScrollView();
		m_PictureBox->Invalidate();
		delete m_ThisGraphic;
		m_ThisGraphic = NULL;
		//DeleteObject( hBitmap );
		
	}

	void DrawAssociateBox( AssociateBox *pThisBox )
	{
		if( pThisBox->Left < 0 || pThisBox->Top < 0 || pThisBox->Right > m_ConvsWidth || pThisBox->Bottom > m_ConvsHeight )
		{
			int dx = 0;
			int dy = 0;
			if( pThisBox->Right > m_ConvsWidth )
			{
				dx =  ((pThisBox->Right - m_ConvsWidth) / (BOX_WIDTH + HORIZONTAL_SPACE) + 1) * (BOX_WIDTH + HORIZONTAL_SPACE);
			}

			if( pThisBox->Bottom > m_ConvsHeight )
			{
				dy = ((pThisBox->Bottom - m_ConvsHeight) /  (BOX_HEIGHT + VERTICAL_SPACE) + 1) * (BOX_HEIGHT + VERTICAL_SPACE);
			}

			ResizeConvs( dx, dy);
		}
		Gdiplus::GraphicsContainer gc = m_ThisGraphic->BeginContainer();
		Gdiplus::Pen pen( Gdiplus::Color( pThisBox->Color ) );
		//m_ThisGraphic->DrawString( pThisBox->Name ); 
		Gdiplus::SolidBrush brush( Gdiplus::Color( pThisBox->Color ) );
		Gdiplus::SolidBrush darkBrush( Gdiplus::Color::DarkGray );
		m_ThisGraphic->FillRectangle( &darkBrush,  pThisBox->Left + 2, pThisBox->Top + 2, pThisBox->Right - pThisBox->Left + 2, pThisBox->Bottom - pThisBox->Top + 2 );
		m_ThisGraphic->FillRectangle( &brush,  pThisBox->Left, pThisBox->Top, pThisBox->Right - pThisBox->Left, pThisBox->Bottom - pThisBox->Top );
		m_ThisGraphic->DrawRectangle( &pen, pThisBox->Left, pThisBox->Top, pThisBox->Right - pThisBox->Left, pThisBox->Bottom - pThisBox->Top );
		///Draw Link Arrow
		if( pThisBox->Parent != NULL )
		{
			Gdiplus::Point point( pThisBox->Left, pThisBox->Top + ( pThisBox->Bottom - pThisBox->Top ) / 2 );
			Gdiplus::Point parentPoint( pThisBox->Parent->Right, pThisBox->Parent->Top + ( pThisBox->Parent->Bottom - pThisBox->Parent->Top ) / 2 );
			DrawArrow( m_ThisGraphic, &pen, point, parentPoint );
		}

		Gdiplus::SolidBrush textBrush( Gdiplus::Color::White );
		Gdiplus::StringFormat sf;
		sf.SetAlignment( Gdiplus::StringAlignmentCenter );
		sf.SetLineAlignment( Gdiplus::StringAlignmentCenter );
		sf.SetTrimming( Gdiplus::StringTrimmingWord );
		
		Gdiplus::PointF pf( (float)( pThisBox->Left + (pThisBox->Right - pThisBox->Left) / 2 ), (float)(pThisBox->Top +  (pThisBox->Bottom-pThisBox->Top) / 2) );
		//Gdiplus::FontFamily ff( _T("Arial Unicode MS") );
		
		m_ThisGraphic->DrawString( pThisBox->Name, (int) _tcslen( pThisBox->Name ), 
			m_Font, 
			pf, 
			&sf, 
			&textBrush );
		m_ThisGraphic->EndContainer( gc );	
	}

	void DrawPermission( const GingkoPermissionPack *pPerms )
	{
		if( pPerms != NULL && pPerms->assignedBy != NULL )
		{
			if( _tcscmp( pPerms->assignedBy, pPerms->gingkoId ) == 0 )
			{
				///This is a AbstractOwner.
				m_AssociateBox->assignedBy = _tcsdup( pPerms->assignedBy );
				m_AssociateBox->gingkoId = _tcsdup( pPerms->gingkoId );
				m_AssociateBox->Name = _tcsdup( pPerms->displayName );
				DrawAssociateBox( m_AssociateBox );
				return;
			}else
			{
				AssociateBox* pBox = m_AssociateBox;
				while( pBox )
				{
					if( pBox->gingkoId != NULL && _tcscmp( pPerms->assignedBy, pBox->gingkoId ) == 0 )
					{
						///If the assigned by is this box.
						///Add new Box to next level
						AddAssociateBox( pBox, pBox, pBox->Next, pPerms );
						return;
					}else 
					{
						if( DirectDrawPermission( pBox, pBox->Direct, pPerms ) )
						{
							return; ///Find this in the direct GingkoPermissionPack;
						};
					}
					pBox = pBox->Next;
				}

				///Add this to unchecked AssociateBox;
				AddAssociateBox( pPerms );
			}
		}
	}
	
	void AddAssociateBox(const GingkoPermissionPack *pPerms)
	{
		AssociateBox *pThisBox = (AssociateBox*) malloc( sizeof(AssociateBox) );

		if( !pThisBox )
		{
			///Failed directly return.
			return;
		}

		memset( pThisBox, 0, sizeof(AssociateBox) );

		pThisBox->gingkoId = _tcsdup( pPerms->gingkoId );
		pThisBox->Name = _tcsdup( pPerms->displayName );
		pThisBox->assignedBy = _tcsdup( pPerms->assignedBy );
		pThisBox->Parent = NULL;  //Assigned by Parent.
		pThisBox->Holder = pPerms->holder;
		pThisBox->Owner = pPerms->owner;
		pThisBox->Actived = !pPerms->deleted;
		pThisBox->Readable = pPerms->readable;
		pThisBox->Writable = pPerms->writable;
		pThisBox->Printable = pPerms->printable;
		pThisBox->Deletable = pPerms->deletable;
		m_BoxVector.push_back( pThisBox );

	}

	void RelinkAssociateBoxes()
	{
		while( m_BoxVector.size() > 0 )
		{
			std::vector<AssociateBox*>::const_iterator itr = m_BoxVector.begin();
			while( itr != m_BoxVector.end() )
			{
				AssociateBox* pBox = (*itr);
				if( RelinkAssociateBox( pBox ) )
				{
					m_BoxVector.erase( itr ); 
					break;
				}
				itr ++;
			}
		}
	}

	void UpdateAssociateBoxDirect( AssociateBox *pParent, AssociateBox *pNext )
	{
		AssociateBox *pTemp = pParent;
		while( pTemp != NULL )
		{
			pTemp->Next = pNext;
			pTemp = pTemp->Direct;
		}
	}

	BOOL RelinkAssociateBox( AssociateBox* pBox )
	{
		///By Next
		AssociateBox* pGenNext = m_AssociateBox;
		while( pGenNext != NULL )
		{
			///fetch direct
			AssociateBox *pDirect = pGenNext;
			AssociateBox *pShouldNext = pGenNext->Next;
			while( pDirect != NULL )
			{
				if( _tcscmp( pDirect->gingkoId , pBox->assignedBy ) == 0 )
				{
					pBox->Parent = pDirect;
					pBox->Left	 = pDirect->Right + HORIZONTAL_SPACE;
					pBox->Right  = pBox->Left + BOX_WIDTH;
					pBox->Top	 = pDirect->Top;
					pBox->Bottom = pDirect->Bottom;
					pBox->Color	 = pDirect->Color;

					if( pShouldNext != NULL )
					{
						pBox->Next = pShouldNext; ///all will link to next
						//fetch direction.
						AssociateBox *pTemp = pDirect->Next;
						while( pTemp != NULL )
						{
							if( pTemp->Direct == NULL )
							{
								pTemp->Direct = pBox;
								pBox->Direct = NULL;
								pBox->Top = pTemp->Bottom + VERTICAL_SPACE;
								pBox->Bottom = pBox->Top + BOX_HEIGHT;
								break;
							}
							pTemp = pTemp->Direct;
						}
					}else{
						pBox->Next = NULL;
						pDirect->Next = pBox;
						pBox->Direct = NULL;
						if( pGenNext != NULL )
						{
							pBox->Top = pGenNext->Top;
							pBox->Bottom = pGenNext->Bottom;
						}
						UpdateAssociateBoxDirect( pGenNext, pBox );
					}
					DrawAssociateBox( pBox );
					return TRUE;
				}
				pDirect = pDirect->Direct;
			}

			pGenNext = pGenNext->Next;
		}
		return FALSE;
	}

	void AddAssociateBox( AssociateBox *pParent, AssociateBox *LevelParent, AssociateBox *pDirect, const GingkoPermissionPack *pPerms )
	{
		AssociateBox *pThisBox = (AssociateBox*) malloc( sizeof(AssociateBox) );

		if( !pThisBox )
		{
			///Failed directly return.
			return;
		}

		memset( pThisBox, 0, sizeof(AssociateBox) );

		pThisBox->gingkoId = _tcsdup( pPerms->gingkoId );
		pThisBox->Name = _tcsdup( pPerms->displayName );
		pThisBox->assignedBy = _tcsdup( pPerms->assignedBy );
		pThisBox->Parent = pParent;  //Assigned by Parent.
		pThisBox->Holder = pPerms->holder;
		pThisBox->Owner = pPerms->owner;
		pThisBox->Actived = !pPerms->deleted;
		pThisBox->Readable = pPerms->readable;
		pThisBox->Writable = pPerms->writable;
		pThisBox->Printable = pPerms->printable;
		pThisBox->Deletable = pPerms->deletable;

		if( pDirect == NULL )
		{
			LevelParent->Next = pThisBox;
			pThisBox->Left = LevelParent->Right + HORIZONTAL_SPACE;
			pThisBox->Top = LevelParent->Top;
			pThisBox->Right = pThisBox->Left + BOX_WIDTH;
			pThisBox->Bottom = pThisBox->Top + BOX_HEIGHT;
			pThisBox->Color = LevelParent->Color;
			pThisBox->Direct = NULL;
			pThisBox->Next = NULL;
			DrawAssociateBox( pThisBox );
			return;
		}

		if( pDirect != NULL )
		{
			///Find the position into this level.
			AssociateBox* pDirectNext = pDirect;
			while( pDirectNext != NULL )
			{
				if( pDirectNext->Direct == NULL )
				{
					///Add thisbox here.
					pDirectNext->Direct = pThisBox;
					pThisBox->Next = pDirect ? pDirect->Next : NULL;
					pThisBox->Direct = NULL;

					pThisBox->Left = LevelParent->Right + HORIZONTAL_SPACE;
					pThisBox->Top = pDirectNext->Bottom + VERTICAL_SPACE;
					pThisBox->Right = pThisBox->Left + BOX_WIDTH;
					pThisBox->Bottom = pThisBox->Top + BOX_HEIGHT;
					pThisBox->Color = LevelParent->Color;
					DrawAssociateBox( pThisBox );
					return;
				}
				pDirectNext = pDirectNext->Direct;
			}
		}
	}

	const AssociateBox* HitTest( int x, int y )
	{
		AssociateBox *pBox = m_AssociateBox;
		while( pBox != NULL )
		{
			AssociateBox *pDirect = pBox;
			if( pBox->Right >= x )
			{
				while( pDirect != NULL )
				{
					if( pDirect->Left <= x && pDirect->Right >= x && pDirect->Top <= y && pDirect->Bottom >= y )
					{
						return pDirect;
					}

					if( pDirect->Top > y )
					{
						break;
					}
					
					pDirect = pDirect->Direct;
				}
			}
			pBox = pBox->Next;
		}
		return NULL;
	}

	BOOL DirectDrawPermission(AssociateBox* pPal, AssociateBox* pDirect, const GingkoPermissionPack *pPerms )
	{
		AssociateBox* pDirectNext = pDirect;
		while( pDirectNext != NULL )
		{
			if( pDirectNext->gingkoId != NULL && _tcscmp( pPerms->gingkoId, pDirectNext->gingkoId ) == 0 )
			{
				AddAssociateBox( pDirectNext, pPal, pDirectNext->Next, pPerms );
				return TRUE;
			}
			pDirectNext = pDirectNext->Direct;
		}
		return FALSE;
	}

	static BOOL GINGKO_API FetchPermission( LPVOID pCaller, const GingkoPermissionPack *pPerms )
	{
		CAssociatePaint *Paint = (CAssociatePaint*) pCaller;
		Paint->DrawPermission( pPerms );
		return TRUE;
	}


	void WINAPI StartAssociate(CPictureBox *PictureBox, GingkoDigitalPack *pDigitalPack)
	{
		m_PictureBox	= PictureBox;
		m_DigitalPack	= pDigitalPack;
		m_dwThreadId	= StartThread( ThreadDrawAssociateMap, (LPVOID)this );
	}

	static DWORD WINAPI ThreadDrawAssociateMap( LPVOID pCaller )
	{
		CAssociatePaint* Paint = (CAssociatePaint*)pCaller;
		Paint->DoPaint( );
		return 0;
	}

};


class CAssociatePictureBox : public CPictureBox, public IPrintJobInfo
{

public:
	CAssociatePaint* m_Paint;
	CToolTipCtrl	 m_CtrlTip;
	//CToolInfo		 m_ToolInfo;
	const AssociateBox *m_pOldBox;

public:
	CAssociatePictureBox()
	{
		m_FillWhiteBoard = TRUE;
	}

	~CAssociatePictureBox()
	{

	}


	virtual LRESULT	OnCreate(LPCREATESTRUCT st)
	{
		CPictureBox::OnCreate( st );
		m_CtrlTip.Create(m_hWnd);
		m_CtrlTip.Activate(FALSE);
		m_CtrlTip.AddTool(m_hWnd);
		m_CtrlTip.SetMaxTipWidth(260); // Set the max tip width to 260 characters
		m_CtrlTip.SetDelayTime(TTDT_AUTOPOP, 10000); // Set auto pop delay time to 10s
		CPictureBox::CenterPicture( true );
		CPictureBox::ClippedPicture( false );

		//m_Menu.ModifyMenu(PICTUREBOX_MENU_CENTER, MF_STRING | MF_UNCHECKED,  (UINT_PTR)0, _T("Center"));
		//m_Menu.ModifyMenu(PICTUREBOX_MENU_STRETCH, MF_STRING| MF_UNCHECKED, (UINT_PTR)0, _T("Stretch"));
		//m_Menu.ModifyMenu(PICTUREBOX_MENU_CLIPPED, MF_STRING| MF_UNCHECKED, (UINT_PTR)0, _T("Clipp"));

		SetMsgHandled( false );

		return 0;
	}

	virtual BOOL OnCreateMenuItem(UINT nPosMenu, CString& MenuText )
	{
		switch( nPosMenu )
		{
		case PICTUREBOX_MENU_CENTER:
			MenuText.Format( GetGlobalCaption( CAPTION_PICTURE_MEMU_CENTER, _T("Center Image") ) );
			break;
		case PICTUREBOX_MENU_STRETCH:
			MenuText.Format( GetGlobalCaption( CAPTION_PICTURE_MEMU_STRETCH, _T("Stretch Image") ) );
			break;
		case PICTUREBOX_MENU_CLIPPED:
			MenuText.Format( GetGlobalCaption( CAPTION_PICTURE_MEMU_CLIPPED, _T("Clipped Image") ) );
			break;
		}
		return TRUE;
	}

	virtual void OnMouseMove(UINT nFlags, CPoint ptPos)
	{
		POINT ptOffset;
		POINT ctOffset = GetCenterOffset();

		if( IsStretch() )
		{
			return;
		}

		GetScrollOffset( ptOffset );
		
		const AssociateBox* pBox = m_Paint->HitTest( ptPos.x + ptOffset.x - ctOffset.x, ptPos.y + ptOffset.y - ctOffset.y );

		if( pBox != NULL )
		{
			if( m_pOldBox != pBox )
			{
				ShowTooltip( pBox );
				m_pOldBox = pBox;
			}
			//::MessageBox( NULL, _T("Test"), _T("TEST"), 0 );
		}else
		{
			m_CtrlTip.Activate( FALSE );
		}
		//return 0;
	}

	virtual LRESULT OnMouseMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		MSG msg = { m_hWnd, uMsg, wParam, lParam };
		if (m_CtrlTip.IsWindow())
		{
			m_CtrlTip.RelayEvent(&msg);
		}

		bHandled = FALSE; // Leave the message to next case
		
		return 0;
	}



	void ShowTooltip( const AssociateBox *pBox )
	{
		POINT pt;
		GetCursorPos(&pt);
		
		//m_CtrlTip.TrackPosition( pt.x, pt.y );
		m_CtrlTip.DelTool( m_hWnd );
		m_CtrlTip.AddTool( m_hWnd, pBox->Name );
		m_CtrlTip.Activate(TRUE);
	}


	int m_PrinterWidth;
	int m_PrinterHeight;
	UINT m_PrintPages;
///Print Job Function 
	virtual void BeginPrintJob(HDC hDC)
	{
		m_PrinterWidth = GetDeviceCaps( hDC, HORZRES );
		m_PrinterHeight = GetDeviceCaps( hDC, VERTRES );
		///m_PrintPages = (m_Image.GetWidth() / m_PrinterWidth) * (m_Image.GetHeight() / m_PrinterHeight);
	}

	virtual void EndPrintJob(HDC hDC, bool bAborted) 
	{
		m_PrinterWidth = 0;
		m_PrinterHeight = 0;
		m_PrintPages = 0;
	}

	virtual void PrePrintPage(UINT nPage, HDC hDC)
	{
	}

	virtual bool PrintPage(UINT nPage, HDC hDC)
	{
		int p = (m_Image.GetWidth() / m_PrinterWidth);
		int w = p == 0 ? 0 : (nPage % p);
		int h = p == 0 ? 0 : nPage / p;

		CMemDC mDC( hDC );
		CDCHandle dcBitmap(m_Image.GetDC());
		int srcWidth = (m_PrinterWidth * (w + 1)) > m_Image.GetWidth() ?  m_Image.GetWidth() : (m_PrinterWidth * (w + 1));
		int srcHeight = (m_PrinterHeight * (w + 1)) > m_Image.GetHeight() ?  m_Image.GetHeight() : (m_PrinterHeight * (h + 1));
		mDC.StretchBlt( 0, 0, m_PrinterWidth, m_PrinterHeight, dcBitmap, 
			m_PrinterWidth * w, m_PrinterHeight * h, 
			srcWidth, 
			srcHeight, 
			SRCCOPY);

		m_Image.ReleaseDC();
		return true;
	}

	virtual void PostPrintPage(UINT nPage, HDC hDC)
	{
	}
	// If you want per page devmodes, return the DEVMODE* to use for nPage.
	// You can optimize by only returning a new DEVMODE* when it is different
	// from the one for nLastPage, otherwise return NULL.
	// When nLastPage==0, the current DEVMODE* will be the default passed to
	// StartPrintJob.
	// Note: During print preview, nLastPage will always be "0".
	virtual DEVMODE* GetNewDevModeForPage(UINT nLastPage, UINT nPage)
	{
		return NULL;
	}

	virtual bool IsValidPage(UINT nPage)
	{
		int w =  m_Image.GetWidth() / m_PrinterWidth;
		int h =  m_Image.GetHeight() / m_PrinterHeight; //height
		
		if( w == 0 && h == 0  )
		{
			if( nPage == 0 )
			{
				return true;
			}else{
				return false;
			}
		}

		int v = w == 0 ? 0 : nPage / w; ///How many line?
		int x = h == 0 ? 0 : nPage / h;

		if( (x * m_PrinterHeight) > m_Image.GetHeight() )
		{
			return false;
		}

		if( (v * m_PrinterWidth) > m_Image.GetWidth() )
		{
			return false;
		}

		return true;
	}

};


class CGingkoAssociateDialog : 
	public CResizableDialogImplT<CGingkoAssociateDialog, CWindow, aero::CDialogImpl<CGingkoAssociateDialog> >,
		public CMessageFilter, public CIdleHandler
{
	typedef CResizableDialogImplT<CGingkoAssociateDialog, CWindow, aero::CDialogImpl<CGingkoAssociateDialog> > baseClass;

private:
	///CPictureBox			m_PictureBox;
	CAssociatePictureBox m_PictureBox;
	CAssociatePaint		m_Paint;
	GingkoDigitalPack*	m_DigitalPack;

public:
	enum { IDD = IDD_ASSOCIATE_DIALOG };

	CGingkoAssociateDialog( GingkoDigitalPack *pPack )
	{
		m_DigitalPack = pPack;
		m_EnabledSizeGrip = FALSE;
	}

	~CGingkoAssociateDialog()
	{
	}
 
	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		return FALSE;
	}


	BEGIN_MSG_MAP(CGingkoAssociateDialog)
		CHAIN_MSG_MAP(baseClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER( WM_PAINT, OnPaint )
		COMMAND_ID_HANDLER(IDOK,	OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnOK)
		COMMAND_ID_HANDLER(IDC_PRINT, OnPrint)
		COMMAND_ID_HANDLER(IDC_SAVEAS, OnSave)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// unregister message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveMessageFilter(this);
		pLoop->RemoveIdleHandler(this);
		return 0;
	}

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if( ::IsCompositionEnabled() )
		{
			CRect rcGlassArea;
			CPaintDC dc(m_hWnd);
			GetClientRect( rcGlassArea );
			//rcGlassArea.bottom = 150;
			dc.FillSolidRect(rcGlassArea, RGB(0,0,0));
		}
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// center the dialog on the screen
		
		
		
		UpdateDialogUI( *this, IDD_ASSOCIATE_DIALOG );
		
		CWindow picBox = GetDlgItem( IDC_MAIN_CONTAINER );
		RECT rc;
		picBox.GetClientRect( &rc );
		picBox.ShowWindow( SW_HIDE );
		

		///m_PictureBox.SubclassWindow( GetDlgItem(IDC_MAIN_CONTAINER) );
		
		m_PictureBox.Create( *this, rc, NULL,
				WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0);

		m_PictureBox.CenterPicture( true );
		m_PictureBox.ClippedPicture( false );
		m_PictureBox.StretchPicture( false );

		AddAnchor( m_PictureBox,	TOP_LEFT, BOTTOM_RIGHT );
		AddAnchor( IDOK,		BOTTOM_RIGHT, BOTTOM_RIGHT );
		AddAnchor( IDC_PRINT,	BOTTOM_LEFT, BOTTOM_LEFT );
		AddAnchor( IDC_SAVEAS,	BOTTOM_LEFT, BOTTOM_LEFT );

		m_PictureBox.m_Paint = &m_Paint;

		m_Paint.StartAssociate( &m_PictureBox, m_DigitalPack );

		AERO_CONTROL( CButton, _btnClose, GetDlgItem(IDOK) );
		AERO_CONTROL( CButton, _btnPrint, GetDlgItem(IDC_PRINT) );
		AERO_CONTROL( CButton, _btnSaveAs, GetDlgItem(IDC_SAVEAS) );

		if( IsCompositionEnabled() )
		{
			MARGINS mar = {-1};
			_DwmExtendFrameIntoClientArea ( m_hWnd, &mar );
		}

		HWND hParent = GetParent();
		RECT rcParent;
		if( hParent == NULL )
		{
			hParent = GetDesktopWindow();
		}

		if( hParent != NULL )
		{
			if( ::GetWindowRect( hParent, &rcParent ) )
			{
				MoveWindow( rcParent.left, rcParent.top, (rcParent.right - rcParent.left) * 4 / 5, 
					(rcParent.bottom - rcParent.top) * 4 / 5,  false );
			}

		}
		//SetOpaqueUnder(m_PictureBox);

		SetOpaqueUnder(m_PictureBox);

		CenterWindow();

		HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), 
											  MAKEINTRESOURCE(IDI_GINGKOSHELL),
											  IMAGE_ICON, 
											  ::GetSystemMetrics(SM_CXSMICON), 
											  ::GetSystemMetrics(SM_CYSMICON), 
											  LR_DEFAULTCOLOR);

		SetIcon(hIconSmall, FALSE );

		HICON hIconBig = (HICON)::LoadImage(_Module.GetResourceInstance(), 
											  MAKEINTRESOURCE(IDI_GINGKOSHELL),
											  IMAGE_ICON, 
											  ::GetSystemMetrics(SM_CXICON), 
											  ::GetSystemMetrics(SM_CYICON), 
											  LR_DEFAULTCOLOR);

		SetIcon(hIconBig, TRUE );
		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);

		///UIAddChildWindowContainer(m_hWnd);

		return TRUE;
	}

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog( wID );
		return 0;
	}

	LRESULT OnSave(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		UINT num = 0;
		UINT size = 0;
		if( Ok != Gdiplus::GetImageEncodersSize( &num, &size ) )
		{
			return 0;
		}

		ImageCodecInfo* pImageCodecInfo;

		pImageCodecInfo = (ImageCodecInfo*)(malloc(size));

		if( pImageCodecInfo == NULL )
		{
			return 0;
		}

		
		if( Ok != GetImageEncoders(num, size, pImageCodecInfo) )
		{
			free( pImageCodecInfo);
			return 0;
		}

		TCHAR*	_ExtBuffer = NULL;
		int _ExtBufferSize = sizeof(TCHAR) * num * MAX_PATH;
		_ExtBuffer = (TCHAR*) malloc( _ExtBufferSize );
		if( _ExtBuffer == NULL )
		{
			free( pImageCodecInfo);
			return 0;
		}

		memset( _ExtBuffer, 0, _ExtBufferSize );

		TCHAR* _pTempBuff = _ExtBuffer;
		for( UINT i = 0; i < num; i ++ )
		{
			int TStrSize = (int) _tcslen(pImageCodecInfo[i].FormatDescription) + (int) _tcslen(pImageCodecInfo[i].FilenameExtension) + 3; 
			wnsprintf( _pTempBuff, _ExtBufferSize, _T("%s(%s)"),  pImageCodecInfo[i].FormatDescription, pImageCodecInfo[i].FilenameExtension);
			_pTempBuff += TStrSize;
			_ExtBufferSize -= TStrSize;
			wnsprintf( _pTempBuff, _ExtBufferSize, _T("%s"), pImageCodecInfo[i].FilenameExtension );
			TStrSize = (int) _tcslen(pImageCodecInfo[i].FilenameExtension) + 1;
			_pTempBuff += TStrSize;
			_ExtBufferSize -= TStrSize;
		}

		CFileDialog fileDialog( FALSE, _T("*.*"), NULL, 4 | 2, _ExtBuffer, *this );

		if( IDOK == fileDialog.DoModal() )
		{
			 fileDialog.m_szFileName;
			 CString szFilename( fileDialog.m_szFileName );
			 int ext = szFilename.ReverseFind( _T('.') );
			 CString szExt = szFilename.Right( szFilename.GetLength() - ext );
			 szExt.Insert(0, _T('*') );
			 CLSID   *pFormatClsId = NULL;
			for( UINT i = 0; i < num; i ++ )
			{
				CString szFilenameExt(pImageCodecInfo[i].FilenameExtension);
				int find = szFilenameExt.Find( _T(";" ) );
				BOOL wasFound = FALSE;
				int bef = 0;
				while( find >= 0 )
				{
					CString szPartExt = szFilenameExt.Mid( bef, find - bef );
					if( szExt.CompareNoCase( szPartExt ) == 0 )
					{
						wasFound = TRUE;
						break;
					}
					bef = find + 1;
					find =  szFilenameExt.Find( _T(";" ), find + 1 );
				}

				if( find < 0 )
				{
					CString szPartExt = szFilenameExt.Right( szFilenameExt.GetLength() - bef );
					if( szExt.CompareNoCase( szPartExt ) == 0 )
					{
						wasFound = TRUE;
					}
				}


				if( wasFound )
				{
					pFormatClsId = &(pImageCodecInfo[i].Clsid);
					break;
				}
			}

			if( pFormatClsId == NULL )
			{
				MessageBox( GetGlobalCaption( CAPTION_CANNOT_SAVE_AS_FORMAT, _T("Can not save the image as this format. Please choose another file.") ), MESSAGE_WARNING_TITLE );
			}else{
				Gdiplus::Bitmap bmp( m_PictureBox.GetSaveBitmap(), NULL );
				bmp.Save( fileDialog.m_szFileName, pFormatClsId );
			}
		}

		if( pImageCodecInfo != NULL )
		{
			free( pImageCodecInfo );
		}

		if( _ExtBuffer )
		{
			free(_ExtBuffer);
		}
		return 0;
	}

	LRESULT OnPrint(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		//WTL::CPrintPreview cpp;
		//WTL::CPrintPreviewWindow cppw;
		
		CPrintDialogEx cpd;
		CString szFilename;
		INT_PTR nRet = cpd.DoModal();

		if( nRet != IDCANCEL && nRet != IDCLOSE )
		{
			
			if( cpd.PrintToFile() )
			{
				CFileDialog ofd(false);
				if( IDOK == ofd.DoModal() )
				{
					szFilename.Format( ofd.m_szFileName );
				}
			}

			CPrintJob job;
			CPrinter  printer;
			int fromPage = 0;
			int toPage = -1;

			printer.OpenPrinter( cpd.GetDeviceName(), cpd.GetDevMode() );
			job.StartPrintJob( false, printer, cpd.GetDevMode(),
				&m_PictureBox, _T("AssociateBox"), fromPage, toPage, 
				cpd.PrintToFile() ? true : false, szFilename );

			//m_PictureBox.DoPrint( hdc, rc );
		}
		return 0;
	}

};


#endif
