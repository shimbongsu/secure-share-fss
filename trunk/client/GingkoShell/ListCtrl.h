/////////////////////////////////////////////////////////////////////////////
// 
// CListCtrl - A WTL list control with Windows Vista style item selection.
//
// Revision:      1.5
// Last modified: 9th April 2006
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <set>
#include <algorithm>
#include <vector>
#include <atltheme.h>
#include <atlctrls.h>
#include <atlctrlx.h>


using namespace std;
/** List Types **/

#define NULL_COLUMN					-1
#define NULL_ITEM					-1
#define NULL_SUBITEM				-1
#define ITEM_HEIGHT_MARGIN			6
#define ITEM_WIDTH_MARGIN			20
#define ITEM_SCROLL_OFFSET			5
#define DRAG_HEADER_OFFSET			4
#define DRAG_ITEM_OFFSET			3
#define ITEM_EDIT_MARGIN			5
#define TOOLTIP_TOOL_ID				1000
#define RESIZE_COLUMN_TIMER			1001
#define RESIZE_COLUMN_PERIOD		20
#define ITEM_VISIBLE_TIMER			1002
#define ITEM_VISIBLE_PERIOD			250
#define SEARCH_PERIOD				800
#define ITEM_AUTOSCROLL_TIMER		1003
#define ITEM_SCROLL_TIMER			1004
#define ITEM_SCROLL_PERIOD			20
#define ITEM_SCROLL_UNIT_MAX		8
#define ITEM_SCROLL_UNIT_MIN		4

#define ITEM_FORMAT_NONE			0
#define ITEM_FORMAT_EDIT			1
#define ITEM_FORMAT_DATETIME		2
#define ITEM_FORMAT_COMBO			3
#define ITEM_FORMAT_CHECKBOX		4
#define ITEM_FORMAT_CHECKBOX_3STATE	5
#define ITEM_FORMAT_HYPERLINK		6
#define ITEM_FORMAT_PROGRESS		7
#define ITEM_FORMAT_CUSTOM			8

#define ITEM_FLAGS_NONE				0x0000
#define ITEM_FLAGS_LEFT				0x0001
#define ITEM_FLAGS_RIGHT			0x0002
#define ITEM_FLAGS_CENTRE			0x0004
#define ITEM_FLAGS_READ_ONLY		0x0008
#define ITEM_FLAGS_EDIT_UPPER		0x0010
#define ITEM_FLAGS_EDIT_NUMBER		0x0020
#define ITEM_FLAGS_EDIT_FLOAT		0x0040
#define ITEM_FLAGS_EDIT_NEGATIVE	0x0080
#define ITEM_FLAGS_EDIT_OPERATOR	0x0100
#define ITEM_FLAGS_COMBO_EDIT		0x0200
#define ITEM_FLAGS_DATE_ONLY		0x0400
#define ITEM_FLAGS_TIME_ONLY		0x0800
#define ITEM_FLAGS_DATETIME_NONE	0x1000
#define ITEM_FLAGS_PROGRESS_SOLID	0x2000

#define ITEM_IMAGE_NONE				-1
#define ITEM_IMAGE_DOWN				0
#define ITEM_IMAGE_UP				1
#define ITEM_IMAGE_CHECK_OFF		2
#define ITEM_IMAGE_CHECK_ON			3
#define ITEM_IMAGE_3STATE_UNDEF		ITEM_IMAGE_CHECK_OFF
#define ITEM_IMAGE_3STATE_ON		4
#define ITEM_IMAGE_3STATE_OFF		5
#define ITEM_IMAGE_LOCK				6
#define ITEM_IMAGE_ATTACHMENT		7
#define ITEM_IMAGE_3STATE			8
#define ITEM_IMAGE_CHECKBOX			9

#define HITTEST_FLAG_NONE			0x0000
#define HITTEST_FLAG_HEADER_DIVIDER	0x0001
#define HITTEST_FLAG_HEADER_LEFT	0x0002
#define HITTEST_FLAG_HEADER_RIGHT	0x0004

#define LCN_FIRST					( 0U - 1500U )
#define LCN_LAST					( 0U - 1600U )

#define LCN_SELECTED				( LCN_FIRST - 1 )
#define LCN_LEFTCLICK				( LCN_FIRST - 2 )
#define LCN_RIGHTCLICK				( LCN_FIRST - 3 )
#define LCN_DBLCLICK				( LCN_FIRST - 4 )
#define LCN_ENDEDIT					( LCN_FIRST - 5 )
#define LCN_MODIFIED				( LCN_FIRST - 6 )
#define LCN_HYPERLINK				( LCN_FIRST - 7 )

struct CListNotify
{
	NMHDR m_hdrNotify;
	int m_nItem;
	int m_nSubItem;
	TCHAR m_nExitChar;
	LPCTSTR m_lpszItemText;
	LPSYSTEMTIME m_lpItemDate;
};

template <class T, class TEqual = CSimpleArrayEqualHelper< T > >
class CListArray : public CSimpleArray< T, TEqual >
{
public:
	void Reverse()
	{
		reverse( m_aT, m_aT + m_nSize );
	}

	void Sort()
	{
		sort( m_aT, m_aT + m_nSize );
	}
	
	template< class Pred > inline
	void Sort( Pred Compare )
	{
		sort( m_aT, m_aT + m_nSize, Compare );
	}
	
	BOOL IsEmpty()
	{
		return ( GetSize() == 0 );
	}
	
	BOOL InsertAt( int nIndex, const T& t )
	{
		if ( nIndex < 0 )
			return FALSE;
	
		if ( nIndex >= m_nSize )
			return ( Add( t ) != -1 );
		
		if ( m_nSize == m_nAllocSize )
		{
			int nNewAllocSize = ( m_nAllocSize == 0 ) ? 1 : ( m_nSize * 2 );
			T* aT = (T*)realloc( m_aT, nNewAllocSize * sizeof( T ) );
			if ( aT == NULL )
				return FALSE;
			m_nAllocSize = nNewAllocSize;
			m_aT = aT;
		}
		MoveMemory( (LPVOID)( m_aT + nIndex + 1 ), (LPVOID)( m_aT + nIndex ), ( m_nSize - nIndex ) * sizeof( T ) );
		InternalSetAtIndex( nIndex, t );
		m_nSize++;
		return TRUE;
	}
};

/** DragDrop support **/
class CEnumFormatEtc : public IEnumFORMATETC
{
public:
	CEnumFormatEtc( const vector < FORMATETC >& vFormatEtc )
	{
		m_nRefCount = 0;
		m_nIndex = 0;
		m_vFormatEtc = vFormatEtc;
	}
	
protected:
	vector < FORMATETC > m_vFormatEtc;
	int m_nRefCount;
	int m_nIndex;

public:
	// IUnknown members
	STDMETHOD(QueryInterface)( REFIID refiid, void FAR* FAR* ppvObject )
	{
		*ppvObject = ( refiid == IID_IUnknown || refiid == IID_IEnumFORMATETC ) ? this : NULL;
		
		if ( *ppvObject != NULL )
			( (LPUNKNOWN)*ppvObject )->AddRef();
		
		return *ppvObject == NULL ? E_NOINTERFACE : S_OK;
	}
	
	STDMETHOD_(ULONG, AddRef)( void )
	{
		return ++m_nRefCount;
	}
	
	STDMETHOD_(ULONG, Release)( void )
	{
		int nRefCount = --m_nRefCount;
		if ( nRefCount == 0 )
			delete this;
		return nRefCount;
	}
	
	// IEnumFORMATETC members
	STDMETHOD(Next)( ULONG celt, LPFORMATETC lpFormatEtc, ULONG FAR *pceltFetched )
	{
		if ( pceltFetched != NULL )
			*pceltFetched=0;

		ULONG cReturn = celt;

		if ( celt <= 0 || lpFormatEtc == NULL || m_nIndex >= (int)m_vFormatEtc.size() )
			return S_FALSE;

		if ( pceltFetched == NULL && celt != 1 ) // pceltFetched can be NULL only for 1 item request
			return S_FALSE;

		while ( m_nIndex < (int)m_vFormatEtc.size() && cReturn > 0 )
		{
			*lpFormatEtc++ = m_vFormatEtc[ m_nIndex++ ];
			cReturn--;
		}
		
		if ( pceltFetched != NULL )
			*pceltFetched = celt - cReturn;

		return cReturn == 0 ? S_OK : S_FALSE;
	}
	
	STDMETHOD(Skip)( ULONG celt )
	{
		if ( ( m_nIndex + (int)celt ) >= (int)m_vFormatEtc.size() )
			return S_FALSE;
		m_nIndex += celt;
		return S_OK;
	}
	
	STDMETHOD(Reset)( void )
	{
		m_nIndex = 0;
		return S_OK;
	}
	
	STDMETHOD(Clone)( IEnumFORMATETC FAR * FAR* ppCloneEnumFormatEtc )
	{
		if ( ppCloneEnumFormatEtc == NULL )
			return E_POINTER;

		*ppCloneEnumFormatEtc = new CEnumFormatEtc( m_vFormatEtc );
		( (CEnumFormatEtc*)*ppCloneEnumFormatEtc )->AddRef();
		( (CEnumFormatEtc*)*ppCloneEnumFormatEtc )->m_nIndex = m_nIndex;
		
		return S_OK;
	}
};

class CDropSource : public IDropSource
{
public:
	CDropSource()
	{
		m_nRefCount = 0;
	}

protected:
	int m_nRefCount;
	
public:
	// IUnknown members
	STDMETHOD(QueryInterface)( REFIID refiid, void FAR* FAR* ppvObject )
	{
		*ppvObject = ( refiid == IID_IUnknown || refiid == IID_IDropSource ) ? this : NULL;
		
		if ( *ppvObject != NULL )
			( (LPUNKNOWN)*ppvObject )->AddRef();
		
		return *ppvObject == NULL ? E_NOINTERFACE : S_OK;
	}
	
	STDMETHOD_(ULONG, AddRef)( void )
	{
		return ++m_nRefCount;
	}
	
	STDMETHOD_(ULONG, Release)( void )
	{
		int nRefCount = --m_nRefCount;
		if ( nRefCount == 0 )
			delete this;
		return nRefCount;
	}
	
	// IDropSource members
	STDMETHOD(QueryContinueDrag)( BOOL bEscapePressed, DWORD dwKeyState )
	{
		if ( bEscapePressed )
			return DRAGDROP_S_CANCEL;
			
		if ( !( dwKeyState & ( MK_LBUTTON | MK_RBUTTON ) ) )
			return DRAGDROP_S_DROP;
		
		return S_OK;
	}
	
    STDMETHOD(GiveFeedback)( DWORD dwEffect )
    {
		return DRAGDROP_S_USEDEFAULTCURSORS;
    }
};

class CDataObject : public IDataObject
{
public:
	CDataObject( CDropSource *pDropSource )
	{
		m_nRefCount = 0;
		m_pDropSource = pDropSource;
		m_bSwappedButtons = GetSystemMetrics( SM_SWAPBUTTON );
	}
	
	virtual ~CDataObject()
	{
		for ( vector < STGMEDIUM >::iterator posStgMedium = m_vStgMedium.begin(); posStgMedium != m_vStgMedium.end(); posStgMedium++ )
			ReleaseStgMedium( &( *posStgMedium ) );
	}

protected:
	CDropSource *m_pDropSource;
	int m_nRefCount;
	BOOL m_bSwappedButtons;
	
	vector < FORMATETC > m_vFormatEtc;
	vector < STGMEDIUM > m_vStgMedium;

public:
	// IUnknown members
	STDMETHOD(QueryInterface)( REFIID refiid, void FAR* FAR* ppvObject )
	{
		*ppvObject = ( refiid == IID_IUnknown || refiid == IID_IDataObject ) ? this : NULL;
		
		if ( *ppvObject != NULL )
			( (LPUNKNOWN)*ppvObject )->AddRef();
		
		return *ppvObject == NULL ? E_NOINTERFACE : S_OK;
	}
	
	STDMETHOD_(ULONG, AddRef)( void )
	{
		return ++m_nRefCount;
	}
	
	STDMETHOD_(ULONG, Release)( void )
	{
		int nRefCount = --m_nRefCount;
		if ( nRefCount == 0 )
			delete this;
		return nRefCount;
	}
	
	// IDataObject members
	STDMETHOD(GetData)( FORMATETC __RPC_FAR *pformatetcIn, STGMEDIUM __RPC_FAR *pmedium )
	{
		if ( pformatetcIn == NULL || pmedium == NULL )
			return E_INVALIDARG;

		ZeroMemory( pmedium, sizeof( STGMEDIUM ) );
		
		for ( int nFormatEtc = 0; nFormatEtc < (int)m_vFormatEtc.size(); nFormatEtc++ )
		{
			if ( pformatetcIn->tymed & m_vFormatEtc[ nFormatEtc ].tymed &&
				 pformatetcIn->dwAspect == m_vFormatEtc[ nFormatEtc ].dwAspect &&
				 pformatetcIn->cfFormat == m_vFormatEtc[ nFormatEtc ].cfFormat )
			{
				if ( m_vStgMedium[ nFormatEtc ].tymed == TYMED_NULL )
					return OnRenderData( m_vFormatEtc[ nFormatEtc ], pmedium, ( GetAsyncKeyState( m_bSwappedButtons ? VK_RBUTTON : VK_LBUTTON ) >= 0 ) ) ? S_OK : DV_E_FORMATETC;
				
				CopyMedium( pmedium, m_vStgMedium[ nFormatEtc ], m_vFormatEtc[ nFormatEtc ] );
				return S_OK;
			}
		}
		
		return DV_E_FORMATETC;
	}
	
	STDMETHOD(GetDataHere)( FORMATETC __RPC_FAR *pformatetc, STGMEDIUM __RPC_FAR *pmedium )
	{
		return E_NOTIMPL;	
	}
	
	STDMETHOD(QueryGetData)( FORMATETC __RPC_FAR *pformatetc )
	{
		if ( pformatetc == NULL )
			return E_INVALIDARG;

		if ( !( pformatetc->dwAspect & DVASPECT_CONTENT ) )
			return DV_E_DVASPECT;
		
		HRESULT hResult = DV_E_TYMED;
		for ( int nFormatEtc = 0; nFormatEtc < (int)m_vFormatEtc.size(); nFormatEtc++ )
		{
			if ( !( pformatetc->tymed & m_vFormatEtc[ nFormatEtc ].tymed ) )
			{
				hResult = DV_E_TYMED;
				continue;
			}
			
			if ( pformatetc->cfFormat == m_vFormatEtc[ nFormatEtc ].cfFormat )
				return S_OK;
			
			hResult = DV_E_CLIPFORMAT;
		}

		return hResult;
	}
	
	STDMETHOD(GetCanonicalFormatEtc)( FORMATETC __RPC_FAR *pformatectIn, FORMATETC __RPC_FAR *pformatetcOut )
	{
		return pformatetcOut == NULL ? E_INVALIDARG : DATA_S_SAMEFORMATETC;
	}
    
    STDMETHOD(SetData)( FORMATETC __RPC_FAR *pformatetc, STGMEDIUM __RPC_FAR *pmedium, BOOL bRelease )
    {
		if ( pformatetc == NULL || pmedium == NULL )
			return E_INVALIDARG;

		m_vFormatEtc.push_back( *pformatetc );
		
		STGMEDIUM StgMedium = *pmedium;
		 
		if ( !bRelease )
			CopyMedium( &StgMedium, *pmedium, *pformatetc );

		m_vStgMedium.push_back( StgMedium );

		return S_OK;
    }
    
    STDMETHOD(EnumFormatEtc)( DWORD dwDirection, IEnumFORMATETC __RPC_FAR *__RPC_FAR *ppenumFormatEtc )
    {
		if ( ppenumFormatEtc == NULL )
			return E_POINTER;

		switch ( dwDirection )
		{
			case DATADIR_GET:	*ppenumFormatEtc = new CEnumFormatEtc( m_vFormatEtc );
								( (CEnumFormatEtc*)*ppenumFormatEtc )->AddRef();
								return S_OK;
			default:			*ppenumFormatEtc = NULL;
								return E_NOTIMPL;
		}
    }
    
    STDMETHOD(DAdvise)( FORMATETC __RPC_FAR *pformatetc, DWORD advf, IAdviseSink __RPC_FAR *pAdvSink, DWORD __RPC_FAR *pdwConnection )
    {
		return OLE_E_ADVISENOTSUPPORTED;
    }
    
    STDMETHOD(DUnadvise)( DWORD dwConnection )
    {
		return E_NOTIMPL;
    }
    
    STDMETHOD(EnumDAdvise)( IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenumAdvise )
    {
		return OLE_E_ADVISENOTSUPPORTED;
    }
       
	void CopyMedium( STGMEDIUM *pMedDest, STGMEDIUM& MedSrc, FORMATETC& FmtSrc )
	{
		switch( MedSrc.tymed )
		{
			case TYMED_HGLOBAL:		pMedDest->hGlobal = (HGLOBAL)OleDuplicateData( MedSrc.hGlobal, FmtSrc.cfFormat, NULL );
									break;
			case TYMED_GDI:			pMedDest->hBitmap = (HBITMAP)OleDuplicateData( MedSrc.hBitmap, FmtSrc.cfFormat, NULL );
									break;
			case TYMED_MFPICT:		pMedDest->hMetaFilePict = (HMETAFILEPICT)OleDuplicateData( MedSrc.hMetaFilePict, FmtSrc.cfFormat, NULL );
									break;
			case TYMED_ENHMF:		pMedDest->hEnhMetaFile = (HENHMETAFILE)OleDuplicateData( MedSrc.hEnhMetaFile, FmtSrc.cfFormat, NULL );
									break;
			case TYMED_FILE:		pMedDest->lpszFileName = (LPOLESTR)OleDuplicateData( MedSrc.lpszFileName, FmtSrc.cfFormat, NULL );
									break;
			case TYMED_ISTREAM:		pMedDest->pstm = MedSrc.pstm;
									MedSrc.pstm->AddRef();
									break;
			case TYMED_ISTORAGE:	pMedDest->pstg = MedSrc.pstg;
									MedSrc.pstg->AddRef();
									break;
		}
		
		pMedDest->tymed = MedSrc.tymed;
		pMedDest->pUnkForRelease = NULL;
		
		if ( MedSrc.pUnkForRelease != NULL )
		{
			pMedDest->pUnkForRelease = MedSrc.pUnkForRelease;
			MedSrc.pUnkForRelease->AddRef();
		}	
	}
	
	virtual BOOL OnRenderData( FORMATETC& FormatEtc, STGMEDIUM *pStgMedium, BOOL bDropComplete )
	{
		return FALSE;
	}
};

class CDropTarget : public IDropTarget
{
public:	
	CDropTarget( HWND hTargetWnd )
	{
		m_hTargetWnd = hTargetWnd;
		m_nRefCount = 0;
		m_bAllowDrop = FALSE;
		m_pDropTargetHelper = NULL;
		ZeroMemory( &m_FormatEtc, sizeof( FORMATETC ) );
		ZeroMemory( &m_StgMedium, sizeof( STGMEDIUM ) );
		
		if ( FAILED( CoCreateInstance( CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER, IID_IDropTargetHelper, (LPVOID*)&m_pDropTargetHelper ) ) )
			m_pDropTargetHelper = NULL;
	}
	
	virtual ~CDropTarget()
	{
		if ( m_pDropTargetHelper != NULL )
		{
			m_pDropTargetHelper->Release();
			m_pDropTargetHelper = NULL;
		}
	}

protected:
	HWND m_hTargetWnd;	
	int m_nRefCount;
	struct IDropTargetHelper *m_pDropTargetHelper;
	vector < FORMATETC > m_vFormatEtc;
	BOOL m_bAllowDrop;
	FORMATETC m_FormatEtc;
	STGMEDIUM m_StgMedium;
	
public:
	// IUnknown members
	STDMETHOD(QueryInterface)( REFIID refiid, void FAR* FAR* ppvObject )
	{
		*ppvObject = ( refiid == IID_IUnknown || refiid == IID_IDropTarget ) ? this : NULL;
		
		if ( *ppvObject != NULL )
			( (LPUNKNOWN)*ppvObject )->AddRef();
		
		return *ppvObject == NULL ? E_NOINTERFACE : S_OK;
	}
	
	STDMETHOD_(ULONG, AddRef)( void )
	{
		return ++m_nRefCount;
	}
	
	STDMETHOD_(ULONG, Release)( void )
	{
		int nRefCount = --m_nRefCount;
		if ( nRefCount == 0 )
			delete this;
		return nRefCount;
	}

	STDMETHOD(DragEnter)( IDataObject __RPC_FAR *pDataObject, DWORD dwKeyState, POINTL pt, DWORD __RPC_FAR *pdwEffect )
	{
		if ( pDataObject == NULL )
			return E_INVALIDARG;

		if ( m_pDropTargetHelper != NULL )
			m_pDropTargetHelper->DragEnter( m_hTargetWnd, pDataObject, (LPPOINT)&pt, *pdwEffect );
		
		ZeroMemory( &m_FormatEtc, sizeof( FORMATETC ) );
		if ( m_StgMedium.tymed != TYMED_NULL )
			ReleaseStgMedium( &m_StgMedium );
		ZeroMemory( &m_StgMedium, sizeof( STGMEDIUM ) );
		
		for ( int nFormatEtc = 0; nFormatEtc < (int)m_vFormatEtc.size(); nFormatEtc++ )
		{
			STGMEDIUM StgMedium;
			m_bAllowDrop = ( pDataObject->GetData( &m_vFormatEtc[ nFormatEtc ], &StgMedium ) == S_OK );
		
			if ( m_bAllowDrop )
			{
				// store drag data for later use in DragOver
				m_FormatEtc = m_vFormatEtc[ nFormatEtc ];
				m_StgMedium = StgMedium;
				
				// get client cursor position
				CWindow hWnd( m_hTargetWnd );
				CPoint point( pt.x, pt.y );
				hWnd.ScreenToClient( &point );
					
				*pdwEffect = OnDragEnter( m_FormatEtc, m_StgMedium, dwKeyState, point );
				
				break;
			}
		}
		
		QueryDrop( dwKeyState, pdwEffect );
		
		return S_OK;	
	}
	
	STDMETHOD(DragOver)( DWORD dwKeyState, POINTL pt, DWORD __RPC_FAR *pdwEffect )
	{
		if ( m_pDropTargetHelper )
			m_pDropTargetHelper->DragOver( (LPPOINT)&pt, *pdwEffect );
		
		if ( m_bAllowDrop && m_FormatEtc.cfFormat != CF_NULL && m_StgMedium.tymed != TYMED_NULL )
		{
			// get client cursor position
			CWindow hWnd( m_hTargetWnd );
			CPoint point( pt.x, pt.y );
			hWnd.ScreenToClient( &point );
				
			*pdwEffect = OnDragOver( m_FormatEtc, m_StgMedium, dwKeyState, point );
		}
		
		QueryDrop( dwKeyState, pdwEffect );
		
		return S_OK;
	}
	
	STDMETHOD(DragLeave)( void )
	{
		if ( m_pDropTargetHelper )
			m_pDropTargetHelper->DragLeave();
		
		OnDragLeave();

		m_bAllowDrop = FALSE;
		
		ZeroMemory( &m_FormatEtc, sizeof( FORMATETC ) );
		if ( m_StgMedium.tymed != TYMED_NULL )
			ReleaseStgMedium( &m_StgMedium );
		ZeroMemory( &m_StgMedium, sizeof( STGMEDIUM ) );
		
		return S_OK;
	}
	
	STDMETHOD(Drop)( IDataObject __RPC_FAR *pDataObject, DWORD dwKeyState, POINTL pt, DWORD __RPC_FAR *pdwEffect )
    {
		if ( pDataObject == NULL )
			return E_INVALIDARG;	

		if ( m_pDropTargetHelper )
			m_pDropTargetHelper->Drop( pDataObject, (LPPOINT)&pt, *pdwEffect );

		if ( m_bAllowDrop && m_FormatEtc.cfFormat != CF_NULL && QueryDrop( dwKeyState, pdwEffect ) )
		{
			STGMEDIUM StgMedium;
			if ( pDataObject->GetData( &m_FormatEtc, &StgMedium ) == S_OK )
			{
				// get client cursor position
				CWindow hWnd( m_hTargetWnd );
				CPoint point( pt.x, pt.y );
				hWnd.ScreenToClient( &point );
				
				if ( !OnDrop( m_FormatEtc, StgMedium, *pdwEffect, point ) )
					*pdwEffect = DROPEFFECT_NONE;
				
				ReleaseStgMedium( &StgMedium );
			}
		}
		
		m_bAllowDrop = FALSE;
		
		ZeroMemory( &m_FormatEtc, sizeof( FORMATETC ) );
		if ( m_StgMedium.tymed != TYMED_NULL )
			ReleaseStgMedium( &m_StgMedium );
		ZeroMemory( &m_StgMedium, sizeof( STGMEDIUM ) );
		
		return S_OK;
    }    
    
    void AddSupportedFormat( FORMATETC& FormatEtc )
	{
		m_vFormatEtc.push_back( FormatEtc );
	}
	
	void AddSupportedFormat( CLIPFORMAT cfFormat )
	{
		FORMATETC FormatEtc;
		ZeroMemory( &FormatEtc, sizeof( FORMATETC ) );
		
		FormatEtc.cfFormat = cfFormat;
		FormatEtc.dwAspect = DVASPECT_CONTENT;
		FormatEtc.lindex = -1;
		FormatEtc.tymed = TYMED_HGLOBAL;
		
		AddSupportedFormat( FormatEtc );
	}
	
	BOOL QueryDrop( DWORD dwKeyState, LPDWORD pdwEffect )
	{
		DWORD dwEffects = *pdwEffect; 

		if ( !m_bAllowDrop )
		{
			*pdwEffect = DROPEFFECT_NONE;
			return FALSE;
		}
		
		*pdwEffect = ( dwKeyState & MK_CONTROL ) ? ( ( dwKeyState & MK_SHIFT ) ? DROPEFFECT_LINK : DROPEFFECT_COPY ) : ( ( dwKeyState & MK_SHIFT ) ? DROPEFFECT_MOVE : 0 );
		if ( *pdwEffect == 0 ) 
		{
			if ( dwEffects & DROPEFFECT_COPY )
				*pdwEffect = DROPEFFECT_COPY;
			else if ( dwEffects & DROPEFFECT_MOVE )
				*pdwEffect = DROPEFFECT_MOVE; 
			else if (dwEffects & DROPEFFECT_LINK )
				*pdwEffect = DROPEFFECT_LINK; 
			else 
				*pdwEffect = DROPEFFECT_NONE;
		} 
		else if ( !( *pdwEffect & dwEffects ) )
			*pdwEffect = DROPEFFECT_NONE;

		return ( *pdwEffect != DROPEFFECT_NONE );
	}

	virtual DWORD OnDragEnter( FORMATETC& FormatEtc, STGMEDIUM& StgMedium, DWORD dwKeyState, CPoint point )
	{
		return FALSE;
	}
	
	virtual DWORD OnDragOver( FORMATETC& FormatEtc, STGMEDIUM& StgMedium, DWORD dwKeyState, CPoint point )
	{
		return FALSE;
	}
	
	virtual BOOL OnDrop( FORMATETC& FormatEtc, STGMEDIUM& StgMedium, DWORD dwEffect, CPoint point )
	{
		return FALSE;
	}
	
	virtual void OnDragLeave()
	{
	}
};

template < class T >
class CDropTargetT : public CDropTarget
{
public:
	CDropTargetT( HWND hTargetWnd ) : CDropTarget( hTargetWnd )
	{
		m_pDelegate = NULL;
	}

protected:
	T *m_pDelegate;

public:	
	BOOL Register( T *pDelegate )
	{
		m_pDelegate = pDelegate;
		return TRUE;
	}
	
	virtual DWORD OnDragEnter( FORMATETC& FormatEtc, STGMEDIUM& StgMedium, DWORD dwKeyState, CPoint point )
	{
		return m_pDelegate == NULL ? DROPEFFECT_NONE : m_pDelegate->OnDragEnter( FormatEtc, StgMedium, dwKeyState, point );
	}
	
	virtual DWORD OnDragOver( FORMATETC& FormatEtc, STGMEDIUM& StgMedium, DWORD dwKeyState, CPoint point )
	{
		return m_pDelegate == NULL ? DROPEFFECT_NONE : m_pDelegate->OnDragOver( FormatEtc, StgMedium, dwKeyState, point );
	}
	
	virtual BOOL OnDrop( FORMATETC& FormatEtc, STGMEDIUM& StgMedium, DWORD dwEffect, CPoint point )
	{
		return m_pDelegate == NULL ? FALSE : m_pDelegate->OnDrop( FormatEtc, StgMedium, dwEffect, point );
	}
	
	virtual void OnDragLeave()
	{
		if ( m_pDelegate != NULL )
			m_pDelegate->OnDragLeave();
	}
};

template < class T >
class CDataObjectT : public CDataObject
{
public:
	CDataObjectT( CDropSource *pDropSource ) : CDataObject( pDropSource )
	{
		m_pDelegate = FALSE;
	}

protected:
	T *m_pDelegate;

public:	
	BOOL Register( T *pDelegate )
	{
		m_pDelegate = pDelegate;
		return TRUE;
	}
	
	virtual BOOL OnRenderData( FORMATETC& FormatEtc, STGMEDIUM *pStgMedium, BOOL bDropComplete )
	{
		return m_pDelegate == NULL ? FALSE : m_pDelegate->OnRenderData( FormatEtc, pStgMedium, bDropComplete );
	}
};

template < class T >
class CDragDrop
{
public:
	CDragDrop()
	{
		m_pDropSource = NULL;
		m_pDataObject = NULL;
		m_pDropTarget = NULL;
		m_hTargetWnd = NULL;
	}
		
	virtual ~CDragDrop()
	{
		if ( m_pDropSource != NULL )
			m_pDropSource->Release();
		if ( m_pDataObject != NULL )
			m_pDataObject->Release();
	}

protected:
	CDropSource *m_pDropSource;
	CDataObjectT< T > *m_pDataObject;
	CDropTargetT< T > *m_pDropTarget;
	HWND m_hTargetWnd;

public:
	BOOL Register( T *pDelegate, BOOL bDropSource = TRUE )
	{
		m_hTargetWnd = pDelegate->m_hWnd;
		
		// instantiate new drop target object
		m_pDropTarget = new CDropTargetT< T >( m_hTargetWnd );
		m_pDropTarget->Register( pDelegate );
		
		// register drop target
		if ( FAILED( RegisterDragDrop( m_hTargetWnd, m_pDropTarget ) ) )
		{
			m_pDropTarget = NULL;
			return FALSE;
		}
		
		// is this a drop target only?
		if ( !bDropSource )
			return TRUE;
		
		// instantiate new drop source object
		m_pDropSource = new CDropSource;
		m_pDropSource->AddRef();
		
		m_pDataObject = new CDataObjectT< T >( m_pDropSource );
		m_pDataObject->AddRef();
		
		// register drop source delegate for data render
		return m_pDataObject->Register( pDelegate );
	}
	
	BOOL Revoke()
	{
		m_pDropTarget = NULL;
		return ( RevokeDragDrop( m_hTargetWnd ) == S_OK );
	}
	
	BOOL AddTargetFormat( CLIPFORMAT cfFormat )
	{
		if ( m_pDropTarget == NULL )
			return FALSE;
		m_pDropTarget->AddSupportedFormat( cfFormat );
		return TRUE;
	}	
	
	BOOL AddSourceFormat( CLIPFORMAT cfFormat )
	{
		if ( m_pDataObject == NULL )
			return FALSE;
			
		FORMATETC FormatEtc;
		ZeroMemory( &FormatEtc, sizeof( FORMATETC ) );
		
		FormatEtc.cfFormat = cfFormat;
		FormatEtc.dwAspect = DVASPECT_CONTENT;
		FormatEtc.lindex = -1;
		FormatEtc.tymed = TYMED_HGLOBAL;
		
		STGMEDIUM StgMedium;
		ZeroMemory( &StgMedium, sizeof( STGMEDIUM ) );
		
		return SUCCEEDED( m_pDataObject->SetData( &FormatEtc, &StgMedium, TRUE ) );
	}
	
	BOOL SetClipboard( FORMATETC& FormatEtc, STGMEDIUM& StgMedium )
	{
		if ( m_pDataObject == NULL )
			return DROPEFFECT_NONE;
		
		if ( FAILED( m_pDataObject->SetData( &FormatEtc, &StgMedium, TRUE ) ) )
			return DROPEFFECT_NONE;
		
		return ( OleSetClipboard( m_pDataObject ) == S_OK );
	}
	
	BOOL FlushClipboard()
	{
		return ( OleFlushClipboard() == S_OK );
	}
	
	DWORD DoDragDrop( SHDRAGIMAGE *pDragImage = NULL, DWORD dwValidEffects = DROPEFFECT_COPY | DROPEFFECT_MOVE | DROPEFFECT_LINK )
	{
		if ( m_pDataObject == NULL )
			return DROPEFFECT_NONE;
			
		IDragSourceHelper *pDragSourceHelper = NULL;
		
		// instantiate drag source helper object
		if ( pDragImage != NULL )
		{
			if ( FAILED( CoCreateInstance( CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER, IID_IDragSourceHelper, (LPVOID*)&pDragSourceHelper ) ) )
				pDragSourceHelper = NULL;
			
			if ( pDragSourceHelper != NULL )
				pDragSourceHelper->InitializeFromBitmap( pDragImage, m_pDataObject );
		}
		
		DWORD dwEffects = DROPEFFECT_NONE;
		dwEffects = ::DoDragDrop( m_pDataObject, m_pDropSource, dwValidEffects, &dwEffects ) == DRAGDROP_S_DROP ? DROPEFFECT_NONE : dwEffects;
		
		// destroy drag source helper object
		if ( pDragSourceHelper != NULL )
			pDragSourceHelper->Release();
		
		return dwEffects;
	}	
};


/** Drop Arrows **/
class CDropArrows : public CWindowImpl< CDropArrows >
{
public:
	CDropArrows()
	{
		m_bVertical = FALSE;
		m_nSpanLength = 0;
	}
	
	~CDropArrows()
	{
	}
	
	DECLARE_WND_CLASS( _T( "DropArrows" ) )

protected:
	CBrush m_bshArrowBrush;
	CRgn m_rgnArrowRegion;
	BOOL m_bVertical;
	int m_nSpanLength;
	
public:
	BOOL Create( HWND hWndParent, int nSpanLength, BOOL bVertical )
	{
		if ( !m_bshArrowBrush.CreateSolidBrush( RGB( 255, 0, 0 ) ) )
			return FALSE;
			
		m_bVertical = bVertical;
		m_nSpanLength = nSpanLength + 20;

		if ( CWindowImpl< CDropArrows >::Create( hWndParent, CRect( 0, 0, m_bVertical ? 12 : m_nSpanLength, m_bVertical ? m_nSpanLength : 12 ), NULL, WS_POPUP | WS_DISABLED, WS_EX_TOOLWINDOW ) == NULL )
			return FALSE;
		
		POINT ptArrow[ 7 ];

		ptArrow[0].x = bVertical ? 8 : 0;	ptArrow[0].y = bVertical ? 0 : 9;
		ptArrow[1].x = bVertical ? 8 : 4;	ptArrow[1].y = bVertical ? 4 : 9;
		ptArrow[2].x = bVertical ? 11 : 4;	ptArrow[2].y = bVertical ? 4 : 12;
		ptArrow[3].x = bVertical ? 6 : 10;	ptArrow[3].y = bVertical ? 9 : 6;
		ptArrow[4].x = bVertical ? 1 : 4;	ptArrow[4].y = bVertical ? 4 : 0;
		ptArrow[5].x = bVertical ? 4 : 4;	ptArrow[5].y = bVertical ? 4 : 4;
		ptArrow[6].x = bVertical ? 4 : 0;	ptArrow[6].y = bVertical ? 0 : 4;

		CRgn rgnFirstArrow;
		if ( !rgnFirstArrow.CreatePolygonRgn( ptArrow, 7, ALTERNATE ) )
			return FALSE;

		ptArrow[0].x = bVertical ? 4 : m_nSpanLength;		ptArrow[0].y = bVertical ? m_nSpanLength : 4;
		ptArrow[1].x = bVertical ? 4 : m_nSpanLength - 4;	ptArrow[1].y = bVertical ? m_nSpanLength - 4 : 4;
		ptArrow[2].x = bVertical ? 0 : m_nSpanLength - 4;	ptArrow[2].y = bVertical ? m_nSpanLength - 4 : 0;
		ptArrow[3].x = bVertical ? 6 : m_nSpanLength - 10;	ptArrow[3].y = bVertical ? m_nSpanLength - 10 : 6;
		ptArrow[4].x = bVertical ? 12 : m_nSpanLength - 4;	ptArrow[4].y = bVertical ? m_nSpanLength - 4 : 12;
		ptArrow[5].x = bVertical ? 8 : m_nSpanLength - 4;	ptArrow[5].y = bVertical ? m_nSpanLength - 4 : 9;
		ptArrow[6].x = bVertical ? 8 : m_nSpanLength;		ptArrow[6].y = bVertical ? m_nSpanLength : 9;

		CRgn rgnSecondArrow;
		if ( !rgnSecondArrow.CreatePolygonRgn( ptArrow, 7, ALTERNATE ) )
			return FALSE;

		if ( !m_rgnArrowRegion.CreateRectRgn( 0, 0, bVertical ? 12 : nSpanLength, bVertical ? nSpanLength : 12 ) )
			return FALSE;
		
		m_rgnArrowRegion.CombineRgn( rgnFirstArrow, rgnSecondArrow, RGN_OR );
		
		SetWindowRgn( m_rgnArrowRegion, FALSE );

		return TRUE;
	}
	
	BOOL Show( CPoint point )
	{
		return IsWindow() ? SetWindowPos( NULL, m_bVertical ? point.x - 7 : point.x - ( m_nSpanLength / 2 ), m_bVertical ? point.y - ( m_nSpanLength / 2 ) : point.y - 5, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE ) : FALSE;
	}
	
	BOOL Hide()
	{
		return IsWindow() ? ShowWindow( SW_HIDE ) : FALSE;
	}
	
	BEGIN_MSG_MAP_EX(CDropArrows)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
	END_MSG_MAP_EX()
	
	void OnDestroy()
	{
		m_bshArrowBrush.DeleteObject();
		m_rgnArrowRegion.DeleteObject();
	}
	
	BOOL OnEraseBkgnd( HDC dc ) 
	{
		CDCHandle dcErase( dc );
		dcErase.FillRect( CRect( 0, 0, m_bVertical ? 12 : m_nSpanLength, m_bVertical ? m_nSpanLength : 12 ), m_bshArrowBrush );
		return TRUE;
	}
};


/** Title Tips support **/

class CTitleTip : public CWindowImpl< CTitleTip >
{
public:
	CTitleTip()
	{
		m_hWndParent = NULL;
		m_bShowThemed = TRUE;
	}
	
	~CTitleTip()
	{
	}
	
	DECLARE_WND_CLASS_EX( _T( "TitleTip" ), CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_SAVEBITS, COLOR_WINDOW )

protected:
	HWND m_hWndParent;
	BOOL m_bShowThemed;
	CString m_strToolTip;
	
	COLORREF m_rgbBackground;
	COLORREF m_rgbTextColour;
	COLORREF m_rgbBorderOuter;
	COLORREF m_rgbBorderInner;
	COLORREF m_rgbBackgroundTop;
	COLORREF m_rgbBackgroundBottom;
	
	CFont m_fntTitleFont;
	CToolTipCtrl m_ttToolTip;
	
public:
	BOOL Create( HWND hWndParent, BOOL bShowThemed )
	{
		m_hWndParent = hWndParent;
		m_bShowThemed = bShowThemed;
		
		m_rgbBackground = GetSysColor( COLOR_INFOBK );
		m_rgbTextColour = ( m_bShowThemed && CTheme::IsThemingSupported() ) ? GetSysColor( COLOR_WINDOWTEXT ) : GetSysColor( COLOR_INFOTEXT );		
		m_rgbBorderOuter = RGB( 220, 220, 220 );
		m_rgbBorderInner = RGB( 245, 245, 245 );
		m_rgbBackgroundTop = RGB( 250, 250, 250 );
		m_rgbBackgroundBottom = RGB( 235, 235, 235 );
		
		if ( CWindowImpl< CTitleTip >::Create( hWndParent, NULL, NULL, WS_POPUP, WS_EX_TOOLWINDOW | WS_EX_TOPMOST ) == NULL )
			return FALSE;
		
		// create the tooltip
		if ( !m_ttToolTip.Create( m_hWnd ) )
			return FALSE;
		m_ttToolTip.SetMaxTipWidth( SHRT_MAX );
		
		// get system message font
		CLogFont logFont;
		logFont.SetMessageBoxFont();
		if ( !m_fntTitleFont.IsNull() )
			m_fntTitleFont.DeleteObject();
		return ( m_fntTitleFont.CreateFontIndirect( &logFont ) != NULL );
	}
	
	BOOL Show( CRect& rcRect, LPCTSTR lpszItemText, LPCTSTR lpszToolTip )
	{
		CString strItemText = lpszItemText;
		
		if ( !IsWindow() || strItemText.IsEmpty() )
			return FALSE;
		
		m_strToolTip = lpszToolTip;
		SetWindowText( strItemText );
		
		CClientDC dcClient( m_hWnd );
		
		HFONT hOldFont = dcClient.SelectFont( m_fntTitleFont );
			
		CRect rcTextExtent( rcRect );
				
		// calculate item text extent...
		dcClient.DrawText( strItemText, strItemText.GetLength(), rcTextExtent, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER | DT_CALCRECT );
		
		dcClient.SelectFont( hOldFont );
		
		// do not show titletip if entire text is visible
		if ( rcTextExtent.Width() <= rcRect.Width() - 1 )
			return FALSE;
		
		if ( m_strToolTip.IsEmpty() )
			m_ttToolTip.Activate( FALSE );
		else
		{
			m_ttToolTip.Activate( TRUE );
			m_ttToolTip.AddTool( m_hWnd, (LPCTSTR)m_strToolTip.Left( SHRT_MAX ) );
		}
		
		// show titletip at new location
		if ( !SetWindowPos( NULL, rcRect.left - 4, rcRect.top, rcTextExtent.Width() + 11, rcRect.Height(), SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOCOPYBITS ) )
			return FALSE;
		
		SetCapture();
		
		return TRUE;
	}
	
	BOOL Hide()
	{
		if ( GetCapture() == m_hWnd )
			ReleaseCapture();
		return IsWindow() ? ShowWindow( SW_HIDE ) : FALSE;
	}
	
	BEGIN_MSG_MAP_EX(CTitleTip)
		MSG_WM_DESTROY(OnDestroy)
		MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST,WM_MOUSELAST,OnMouseRange)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MSG_WM_PAINT(OnPaint)
	END_MSG_MAP_EX()
	
	void OnDestroy()
	{
		if ( m_ttToolTip.IsWindow() )
			m_ttToolTip.DestroyWindow();
		m_ttToolTip.m_hWnd = NULL;
	}
	
	LRESULT OnMouseRange( UINT nMessage, WPARAM wParam, LPARAM lParam )
	{
		SetMsgHandled( FALSE );
		
		if ( m_ttToolTip.IsWindow() )
		{
			MSG msgRelay = { m_hWnd, nMessage, wParam, lParam };
			m_ttToolTip.RelayEvent( &msgRelay );
		}
		
		CPoint ptMouse( GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) );
		ClientToScreen( &ptMouse );
		
		if ( nMessage == WM_MOUSEMOVE )
		{
			CRect rcWindow;
			GetWindowRect( rcWindow );		
			if ( !rcWindow.PtInRect( ptMouse ) )
				Hide();
			return 0;
		}		
										
		CWindow wndParent( m_hWndParent );
		LRESULT nHitTest = wndParent.SendMessage( WM_NCHITTEST, 0, MAKELPARAM( ptMouse.x, ptMouse.y ) );
		
		// forward notifcation through to parent
		if ( nHitTest == HTCLIENT )
		{
			wndParent.ScreenToClient( &ptMouse );
			wndParent.PostMessage( nMessage, wParam, MAKELPARAM( ptMouse.x, ptMouse.y ) );
		}
		else
		{
			switch ( nMessage )
			{
				case WM_LBUTTONDOWN:	wndParent.PostMessage( WM_NCLBUTTONDOWN, nHitTest, MAKELPARAM( ptMouse.x, ptMouse.y ) );
										break;
				case WM_LBUTTONUP:		wndParent.PostMessage( WM_NCLBUTTONUP, nHitTest, MAKELPARAM( ptMouse.x, ptMouse.y ) );
										break;
				case WM_LBUTTONDBLCLK:	wndParent.PostMessage( WM_NCLBUTTONDBLCLK, nHitTest, MAKELPARAM( ptMouse.x, ptMouse.y ) );
										break;
				case WM_RBUTTONDOWN:	wndParent.PostMessage( WM_NCRBUTTONDOWN, nHitTest, MAKELPARAM( ptMouse.x, ptMouse.y ) );
										break;
				case WM_RBUTTONUP:		wndParent.PostMessage( WM_NCRBUTTONUP, nHitTest, MAKELPARAM( ptMouse.x, ptMouse.y ) );
										break;
				case WM_RBUTTONDBLCLK:	wndParent.PostMessage( WM_NCRBUTTONDBLCLK, nHitTest, MAKELPARAM( ptMouse.x, ptMouse.y ) );
										break;
				case WM_MBUTTONDOWN:	wndParent.PostMessage( WM_NCMBUTTONDOWN, nHitTest, MAKELPARAM( ptMouse.x, ptMouse.y ) );
										break;
				case WM_MBUTTONUP:		wndParent.PostMessage( WM_NCMBUTTONUP, nHitTest, MAKELPARAM( ptMouse.x, ptMouse.y ) );
										break;
				case WM_MBUTTONDBLCLK:	wndParent.PostMessage( WM_NCMBUTTONDBLCLK, nHitTest, MAKELPARAM( ptMouse.x, ptMouse.y ) );
										break;
			}
		}

		return 0;
	}
	
	BOOL OnEraseBkgnd( HDC dc ) 
	{
		return TRUE;
	}
	
	void OnPaint( HDC )
	{
		CPaintDC dcPaint( m_hWnd );
		
		int nContextState = dcPaint.SaveDC();
		
		CRect rcClient;
		GetClientRect( rcClient );
		
		CRect rcTitleTip( rcClient );
					
		if ( m_bShowThemed && CTheme::IsThemingSupported() )
		{
			CPen penBorder;
			penBorder.CreatePen( PS_SOLID, 1, m_rgbBorderOuter );
			
			dcPaint.SelectPen( penBorder );
			dcPaint.SelectStockBrush( HOLLOW_BRUSH );
			
			dcPaint.RoundRect( rcTitleTip, CPoint( 5, 5 ) );
			rcTitleTip.DeflateRect( 1, 1 );
			
			CPen penInnerBorder;
			penInnerBorder.CreatePen( PS_SOLID, 1, m_rgbBorderInner );
			dcPaint.SelectPen( penInnerBorder );
			
			dcPaint.RoundRect( rcTitleTip, CPoint( 2, 2 ) );
			rcTitleTip.DeflateRect( 1, 1 );
			
			GRADIENT_RECT grdRect = { 0, 1 };
			TRIVERTEX triVertext[ 2 ] = {
											rcTitleTip.left,
											rcTitleTip.top,
											GetRValue( m_rgbBackgroundTop ) << 8,
											GetGValue( m_rgbBackgroundTop ) << 8,
											GetBValue( m_rgbBackgroundTop ) << 8,
											0x0000,			
											rcTitleTip.right,
											rcTitleTip.bottom,
											GetRValue( m_rgbBackgroundBottom ) << 8,
											GetGValue( m_rgbBackgroundBottom ) << 8,
											GetBValue( m_rgbBackgroundBottom ) << 8,
											0x0000
										};
			
			dcPaint.GradientFill( triVertext, 2, &grdRect, 1, GRADIENT_FILL_RECT_V );
		}
		else
		{
			dcPaint.SetBkColor( m_rgbBackground );
			dcPaint.ExtTextOut( rcTitleTip.left, rcTitleTip.top, ETO_OPAQUE, rcTitleTip, _T( "" ), 0, NULL );
			
			CBrush bshTitleFrame;
			bshTitleFrame.CreateSolidBrush( m_rgbTextColour );
			dcPaint.FrameRect( rcTitleTip, bshTitleFrame );
		}
		
		int nTextLength = GetWindowTextLength() + 1;
		CString strItemText;
		GetWindowText( strItemText.GetBuffer( nTextLength ), nTextLength );
		strItemText.ReleaseBuffer();
		
		dcPaint.SelectFont( m_fntTitleFont );
		dcPaint.SetTextColor( m_rgbTextColour );
		dcPaint.SetBkMode( TRANSPARENT );
		
		CRect rcItemText( rcClient );
		rcItemText.OffsetRect( 4, 0 );
		
		dcPaint.DrawText( strItemText, strItemText.GetLength(), rcItemText, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER );
	
		dcPaint.RestoreDC( nContextState );
	}
};

/** List Edit Support **/
class CListEdit : public CWindowImpl< CListEdit, CEdit >
{
public:
	CListEdit()
	{
		m_nItem = NULL_ITEM;
		m_nSubItem = NULL_SUBITEM;
		m_nFlags = ITEM_FLAGS_NONE;
		m_nExitChar = 0;
	}
	
	~CListEdit()
	{
	}

protected:
	int m_nItem;
	int m_nSubItem;
	UINT m_nFlags;
	TCHAR m_nExitChar;
	CFont m_fntEditFont;
	
public:
	BOOL Create( HWND hWndParent, int nItem, int nSubItem, CRect& rcRect, UINT nFlags, LPCTSTR lpszItemText )
	{
		m_nItem = nItem;
		m_nSubItem = nSubItem;
		m_nFlags = nFlags;
		m_nExitChar = 0;
		
		// destroy old edit control...
		if ( IsWindow() )
			DestroyWindow();
		
		DWORD dwStyle = WS_CHILD | ES_AUTOHSCROLL;
		
		// right-justify numbers
		if ( nFlags & ( ITEM_FLAGS_EDIT_NUMBER | ITEM_FLAGS_EDIT_FLOAT ) )
			dwStyle |= ES_RIGHT;
		
		if ( nFlags & ITEM_FLAGS_EDIT_UPPER )
			dwStyle |= ES_UPPERCASE;
		
		// create edit control
		if ( CWindowImpl< CListEdit, CEdit >::Create( hWndParent, CRect( rcRect.left + 2, rcRect.top + 3, rcRect.right - 3, rcRect.bottom - 2 ), NULL, dwStyle ) == NULL )
			return FALSE;
		
		// get system message font
		CLogFont logFont;
		logFont.SetMessageBoxFont();
		if ( !m_fntEditFont.IsNull() )
			m_fntEditFont.DeleteObject();
		if ( m_fntEditFont.CreateFontIndirect( &logFont ) == NULL )
			return FALSE;

		SetFont( m_fntEditFont );
		SetMargins( ITEM_EDIT_MARGIN, ITEM_EDIT_MARGIN );		
		SetWindowText( lpszItemText );
		
		// show edit control
		ShowWindow( SW_SHOW );
		
		SetSelAll();
		SetFocus();
		
		return TRUE;
	}
	
	BOOL IsValid( TCHAR nChar )
	{
		// validate number and float input
		if ( !( m_nFlags & ( ITEM_FLAGS_EDIT_NUMBER | ITEM_FLAGS_EDIT_FLOAT ) ) || nChar == VK_BACK )
			return TRUE;
		
		CString strValue;
		int nValueLength = GetWindowTextLength() + 1;
		GetWindowText( strValue.GetBuffer( nValueLength ), nValueLength );
		strValue.ReleaseBuffer();
		
		// get selected positions
		int nStartChar;
		int nEndChar;
		GetSel( nStartChar, nEndChar );
		
		// are we changing the sign?
		if ( ( m_nFlags & ITEM_FLAGS_EDIT_NEGATIVE ) && nChar == _T( '-' ) )
		{
			BOOL bNegative = FALSE;
			if ( m_nFlags & ITEM_FLAGS_EDIT_FLOAT )
			{
				double dblValue = _tstof( strValue );
				bNegative = ( dblValue < 0 );
				strValue.Format( _T( "%lf" ), -dblValue );
			}
			else
			{
				long lValue = _ttol( strValue );
				bNegative = ( lValue < 0 );
				strValue.Format( _T( "%ld" ), -lValue );
			}
			
			SetWindowText( strValue );
			
			// restore select position
			SetSel( bNegative ? nStartChar - 1 : nStartChar + 1, bNegative ? nEndChar - 1 : nEndChar + 1 );
			return FALSE;
		}
		
		// construct new value string using entered character
		CString strNewValue = strValue.Left( nStartChar ) + nChar + strValue.Right( strValue.GetLength() - nEndChar );
		
		int nGreaterThan = 0;
		int nLessThan = 0;
		int nEquals = 0;
		int nDecimalPoint = 0;
		
		int nNegativeIndex = -1;
		int nGreaterIndex = -1;
		int nLessIndex = -1;
		int nEqualIndex = -1;
		int nDecimalIndex = -1;
		int nDigitIndex = -1;
		
		for ( int nCharIndex = 0; nCharIndex < strNewValue.GetLength(); nCharIndex++ )
		{
			TCHAR nCharValue = strNewValue[ nCharIndex ];
			switch ( nCharValue )
			{
				case _T( '-' ):	nNegativeIndex = nCharIndex;
								break;
				case _T( '>' ):	if ( !( m_nFlags & ITEM_FLAGS_EDIT_OPERATOR ) )
									return FALSE;
								nGreaterIndex = nCharIndex;
								nGreaterThan++;
								break;
				case _T( '<' ):	if ( !( m_nFlags & ITEM_FLAGS_EDIT_OPERATOR ) )
									return FALSE;
								nLessIndex = nCharIndex;
								nLessThan++;
								break;
				case _T( '=' ):	if ( !( m_nFlags & ITEM_FLAGS_EDIT_OPERATOR ) )
									return FALSE;
								nEqualIndex = nCharIndex;
								nEquals++;
								break;
				case _T( '.' ):	if ( !( m_nFlags & ITEM_FLAGS_EDIT_FLOAT ) )
									return FALSE;
								nDecimalIndex = nCharIndex;
								nDecimalPoint++;
								break;
				default:		if ( !_istdigit( nCharValue ) )
									return FALSE;
								if ( nDigitIndex < 0 )
									nDigitIndex = nCharIndex;
								break;
			}

			// invalid if text contains more than one '>', '<', '=' or '.'
			if ( nGreaterThan > 1 || nLessThan > 1 || nEquals > 1 || nDecimalPoint > 1 )
				return FALSE;
		}

		// invalid if text contains '=>' or '=<'
		if ( nGreaterIndex != -1 && nEqualIndex != -1 && nGreaterIndex > nEqualIndex )
			return FALSE;
		if ( nLessIndex != -1 && nEqualIndex != -1 && nLessIndex > nEqualIndex )
			return FALSE;

		// invalid if digits exist before operator
		if ( nDigitIndex != -1 && nGreaterIndex != -1 && nGreaterIndex > nDigitIndex )
			return FALSE;
		if ( nDigitIndex != -1 && nLessIndex != -1 && nLessIndex > nDigitIndex )
			return FALSE;
		if ( nDigitIndex != -1 && nEqualIndex != -1 && nEqualIndex > nDigitIndex )
			return FALSE;
		if ( nDigitIndex != -1 && nNegativeIndex != -1 && nNegativeIndex > nDigitIndex )
			return FALSE;
		
		return TRUE;
	}
	
	BEGIN_MSG_MAP_EX(CListEdit)
		MSG_WM_KILLFOCUS(OnKillFocus)
		MSG_WM_GETDLGCODE(OnGetDlgCode)
		MSG_WM_CHAR(OnChar)
	END_MSG_MAP_EX()
	
	void OnKillFocus( HWND hNewWnd )
	{
		CWindow wndParent( GetParent() );
		if ( wndParent.IsWindow() )
		{
			CString strValue;
			int nValueLength = GetWindowTextLength() + 1;
			GetWindowText( strValue.GetBuffer( nValueLength ), nValueLength );
			strValue.ReleaseBuffer();
			
			CListNotify listNotify;
			listNotify.m_hdrNotify.hwndFrom = m_hWnd;
			listNotify.m_hdrNotify.idFrom = GetDlgCtrlID();
			listNotify.m_hdrNotify.code = LCN_ENDEDIT;
			listNotify.m_nItem = m_nItem;
			listNotify.m_nSubItem = m_nSubItem;
			listNotify.m_nExitChar = m_nExitChar;
			listNotify.m_lpszItemText = strValue;
			listNotify.m_lpItemDate = NULL;

			// forward notification to parent
			FORWARD_WM_NOTIFY( wndParent, listNotify.m_hdrNotify.idFrom, &listNotify.m_hdrNotify, ::SendMessage );
		}
		
		ShowWindow( SW_HIDE );
	}
	
	UINT OnGetDlgCode( LPMSG lpMessage )
	{
		return DLGC_WANTALLKEYS;
	}
	
	void OnChar( TCHAR nChar, UINT nRepCnt, UINT nFlags )
	{
		switch ( nChar )
		{
			case VK_TAB:
			case VK_RETURN:
			case VK_ESCAPE:	{
								m_nExitChar = nChar;
								CWindow wndParent( GetParent() );
								if ( wndParent.IsWindow() )
									wndParent.SetFocus();
							}
							break;
			default:		SetMsgHandled( !IsValid( nChar ) );
							break;
		}
	}
};

/** List Combo support **/
class CListCombo : public CWindowImpl< CListCombo, CComboBox >
{
public:
	CListCombo() : m_wndEditCtrl( this, 1 )
	{
		m_nItem = NULL_ITEM;
		m_nSubItem = NULL_SUBITEM;
		m_nFlags = ITEM_FLAGS_NONE;
		m_nExitChar = 0;
		m_bMouseOver = FALSE;
		m_bActivate = FALSE;
		m_bShowThemed = TRUE;
	}
	
	~CListCombo()
	{
	}

protected:
	int m_nItem;
	int m_nSubItem;
	UINT m_nFlags;
	TCHAR m_nExitChar;
	BOOL m_bShowThemed;
	BOOL m_bMouseOver;
	BOOL m_bActivate;
	BOOL m_bSwappedButtons;
	CRect m_rcStatic;
	CRect m_rcButton;
	COLORREF m_rgbStaticBackground;
	COLORREF m_rgbStaticText;
	
	CTheme m_thmCombo;
	CFont m_fntComboFont;
	CContainedWindowT< CEdit > m_wndEditCtrl;
	
public:
	BOOL Create( HWND hWndParent, int nItem, int nSubItem, CRect& rcRect, UINT nFlags, LPCTSTR lpszItemText, BOOL bShowThemed, CListArray < CString >& aComboList )
	{
		m_nItem = nItem;
		m_nSubItem = nSubItem;
		m_nFlags = nFlags;
		m_nExitChar = 0;
		m_bActivate = FALSE;
		m_bShowThemed = bShowThemed;
		
		m_bSwappedButtons = GetSystemMetrics( SM_SWAPBUTTON );
		m_rgbStaticBackground = GetSysColor( COLOR_HIGHLIGHT );
		m_rgbStaticText = GetSysColor( COLOR_HIGHLIGHTTEXT );
		
		// destroy old combo control...
		if ( IsWindow() )
			DestroyWindow();
			
		DWORD dwStyle = WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL | CBS_AUTOHSCROLL | CBS_SORT;
		
		if ( nFlags & ITEM_FLAGS_COMBO_EDIT )
			dwStyle |= CBS_DROPDOWN;
		else
			dwStyle |= CBS_DROPDOWNLIST;
			
		if ( nFlags & ITEM_FLAGS_EDIT_UPPER )
			dwStyle |= CBS_UPPERCASE;
		
		// create combo control
		if ( CWindowImpl< CListCombo, CComboBox >::Create( hWndParent, CRect( ( ( dwStyle & CBS_DROPDOWNLIST ) == CBS_DROPDOWNLIST ) ? rcRect.left + 3 : rcRect.left + 1, rcRect.top, rcRect.right, rcRect.bottom + ( 6 * rcRect.Height() ) ), NULL, dwStyle ) == NULL )
			return FALSE;
		
		// open combo theme
		if ( m_thmCombo.IsThemeNull() )
			m_thmCombo.OpenThemeData( m_hWnd, L"COMBOBOX" );
		
		// get system message font
		CLogFont logFont;
		logFont.SetMessageBoxFont();
		if ( !m_fntComboFont.IsNull() )
			m_fntComboFont.DeleteObject();
		if ( m_fntComboFont.CreateFontIndirect( &logFont ) == NULL )
			return FALSE;
		SetFont( m_fntComboFont, FALSE );
		
		// subclass edit control to capture keyboard input
		HWND hEditControl = GetWindow( GW_CHILD );
		if ( hEditControl != NULL )
			m_wndEditCtrl.SubclassWindow( hEditControl );
				
		for ( int nComboItem = 0; nComboItem < aComboList.GetSize(); nComboItem++ )
			AddString( aComboList[ nComboItem ] );
		
		if ( ( dwStyle & CBS_DROPDOWNLIST ) == CBS_DROPDOWNLIST )
		{
			int nIndex = FindStringExact( -1, lpszItemText );
			if ( nIndex != CB_ERR )
				SetCurSel( nIndex );
		}
		else
		{
			SetWindowText( lpszItemText );
			SetEditSel( 0, -1 );
		}
		
		// set static edit height
		SetItemHeight( -1, rcRect.Height() - 6 );
		
		COMBOBOXINFO infoComboBox = { sizeof( COMBOBOXINFO ) };
		if ( !::GetComboBoxInfo( m_hWnd, &infoComboBox ) )
			return FALSE;
		
		// store combobox details for painting		
		m_rcStatic = infoComboBox.rcItem;
		m_rcButton = infoComboBox.rcButton;
		m_rcButton.DeflateRect( 0, 1 );
		m_rcButton.OffsetRect( -2, 0 );
		
		// show combo control
		ShowWindow( SW_SHOW );

		SetFocus();
		
		// force redraw now
		RedrawWindow();
		
		return TRUE;
	}
	
	BOOL IsValid( TCHAR nChar )
	{
		// validate number and float input
		if ( !( m_nFlags & ( ITEM_FLAGS_EDIT_NUMBER | ITEM_FLAGS_EDIT_FLOAT ) ) || nChar == VK_BACK )
			return TRUE;
		
		CString strValue;
		int nValueLength = GetWindowTextLength() + 1;
		GetWindowText( strValue.GetBuffer( nValueLength ), nValueLength );
		strValue.ReleaseBuffer();
		
		// get selected positions
		DWORD dwSelection = GetEditSel();		
		int nStartChar = LOWORD( dwSelection );
		int nEndChar = HIWORD( dwSelection );
		
		// are we changing the sign?
		if ( ( m_nFlags & ITEM_FLAGS_EDIT_NEGATIVE ) && nChar == _T( '-' ) )
		{
			BOOL bNegative = FALSE;
			if ( m_nFlags & ITEM_FLAGS_EDIT_FLOAT )
			{
				double dblValue = _tstof( strValue );
				bNegative = ( dblValue < 0 );
				strValue.Format( _T( "%lf" ), -dblValue );
			}
			else
			{
				long lValue = _ttol( strValue );
				bNegative = ( lValue < 0 );
				strValue.Format( _T( "%ld" ), -lValue );
			}
			
			SetWindowText( strValue );
			
			// restore select position
			SetEditSel( bNegative ? nStartChar - 1 : nStartChar + 1, bNegative ? nEndChar - 1 : nEndChar + 1 );
			return FALSE;
		}
		
		// construct new value string using entered character
		CString strNewValue = strValue.Left( nStartChar ) + nChar + strValue.Right( strValue.GetLength() - nEndChar );
		
		int nGreaterThan = 0;
		int nLessThan = 0;
		int nEquals = 0;
		int nDecimalPoint = 0;
		
		int nNegativeIndex = -1;
		int nGreaterIndex = -1;
		int nLessIndex = -1;
		int nEqualIndex = -1;
		int nDecimalIndex = -1;
		int nDigitIndex = -1;
		
		for ( int nCharIndex = 0; nCharIndex < strNewValue.GetLength(); nCharIndex++ )
		{
			TCHAR nCharValue = strNewValue[ nCharIndex ];
			switch ( nCharValue )
			{
				case _T( '-' ):	nNegativeIndex = nCharIndex;
								break;
				case _T( '>' ):	if ( !( m_nFlags & ITEM_FLAGS_EDIT_OPERATOR ) )
									return FALSE;
								nGreaterIndex = nCharIndex;
								nGreaterThan++;
								break;
				case _T( '<' ):	if ( !( m_nFlags & ITEM_FLAGS_EDIT_OPERATOR ) )
									return FALSE;
								nLessIndex = nCharIndex;
								nLessThan++;
								break;
				case _T( '=' ):	if ( !( m_nFlags & ITEM_FLAGS_EDIT_OPERATOR ) )
									return FALSE;
								nEqualIndex = nCharIndex;
								nEquals++;
								break;
				case _T( '.' ):	if ( !( m_nFlags & ITEM_FLAGS_EDIT_FLOAT ) )
									return FALSE;
								nDecimalIndex = nCharIndex;
								nDecimalPoint++;
								break;
				default:		if ( !_istdigit( nCharValue ) )
									return FALSE;
								if ( nDigitIndex < 0 )
									nDigitIndex = nCharIndex;
								break;
			}

			// invalid if text contains more than one '>', '<', '=' or '.'
			if ( nGreaterThan > 1 || nLessThan > 1 || nEquals > 1 || nDecimalPoint > 1 )
				return FALSE;
		}

		// invalid if text contains '=>' or '=<'
		if ( nGreaterIndex != -1 && nEqualIndex != -1 && nGreaterIndex > nEqualIndex )
			return FALSE;
		if ( nLessIndex != -1 && nEqualIndex != -1 && nLessIndex > nEqualIndex )
			return FALSE;

		// invalid if digits exist before operator
		if ( nDigitIndex != -1 && nGreaterIndex != -1 && nGreaterIndex > nDigitIndex )
			return FALSE;
		if ( nDigitIndex != -1 && nLessIndex != -1 && nLessIndex > nDigitIndex )
			return FALSE;
		if ( nDigitIndex != -1 && nEqualIndex != -1 && nEqualIndex > nDigitIndex )
			return FALSE;
		if ( nDigitIndex != -1 && nNegativeIndex != -1 && nNegativeIndex > nDigitIndex )
			return FALSE;
		
		return TRUE;
	}
	
	BEGIN_MSG_MAP_EX(CListCombo)
		MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST,WM_MOUSELAST,OnMouseRange)
		MSG_WM_MOUSELEAVE(OnMouseLeave)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MSG_WM_PAINT(OnPaint)
		REFLECTED_COMMAND_CODE_HANDLER_EX(CBN_KILLFOCUS, OnKillFocus)
		MSG_WM_GETDLGCODE(OnGetDlgCode)
		MSG_WM_CHAR(OnChar)
		DEFAULT_REFLECTION_HANDLER()
	ALT_MSG_MAP(1)
		MSG_WM_GETDLGCODE(OnGetDlgCode)
		MSG_WM_CHAR(OnChar)
	END_MSG_MAP_EX()
	
	LRESULT OnMouseRange( UINT nMessage, WPARAM wParam, LPARAM lParam )
	{
		SetMsgHandled( FALSE );
		
		if ( nMessage == WM_MOUSEMOVE && !m_bMouseOver )
		{
			m_bMouseOver = TRUE;
			
			TRACKMOUSEEVENT trkMouse;
			trkMouse.cbSize = sizeof( TRACKMOUSEEVENT );
			trkMouse.dwFlags = TME_LEAVE;
			trkMouse.hwndTrack = m_hWnd;
			
			// notify when the mouse leaves button
			_TrackMouseEvent( &trkMouse );
		}
		
		// do not show button as pressed when first created
		m_bActivate = TRUE;
		
		InvalidateRect( m_rcButton );
		
		return 0;
	}
	
	void OnMouseLeave()
	{
		m_bMouseOver = FALSE;
		InvalidateRect( m_rcButton );
	}
	
	BOOL OnEraseBkgnd( CDCHandle dcErase ) 
	{
		return TRUE;
	}
	
	void OnPaint( HDC )
	{
		CPaintDC dcPaint( m_hWnd );
		
		CRect rcClient;
		GetClientRect( rcClient );
		
		CMemoryDC dcMemory( dcPaint.m_hDC, rcClient );
		
		CRect rcClip;
		if ( dcPaint.GetClipBox( rcClip ) == ERROR )
			return;
			
		int nContextState = dcMemory.SaveDC();		
		
		// do not repaint background if drawing button only
		if ( !rcClip.EqualRect( m_rcButton ) )
		{
			CWindow wndParent( GetParent() );
			if ( wndParent.IsWindow() )
			{
				// draw background from parent
				CPoint ptOrigin( 0 );
				MapWindowPoints( wndParent, &ptOrigin, 1 );
				dcMemory.OffsetWindowOrg( ptOrigin.x, ptOrigin.y, &ptOrigin );
				wndParent.SendMessage( WM_PAINT, (WPARAM)dcMemory.m_hDC );
				dcMemory.SetWindowOrg( ptOrigin );
			}
		}
		
		DWORD dwPoint = GetMessagePos();
		CPoint ptMouse( GET_X_LPARAM( dwPoint ), GET_Y_LPARAM( dwPoint ) );
		ScreenToClient( &ptMouse );
		
		DWORD dwStyle = GetStyle();	
		BOOL bHotButton = m_bActivate && ( ( ( dwStyle & CBS_DROPDOWNLIST ) == CBS_DROPDOWNLIST ) ? rcClient.PtInRect( ptMouse ) : m_rcButton.PtInRect( ptMouse ) );
		BOOL bPressed = bHotButton && ( GetAsyncKeyState( m_bSwappedButtons ? VK_RBUTTON : VK_LBUTTON ) < 0 );
		
		if ( ( dwStyle & CBS_DROPDOWNLIST ) == CBS_DROPDOWNLIST && !rcClip.EqualRect( m_rcButton ) )
		{
			dcMemory.SetBkColor( m_rgbStaticBackground );
			dcMemory.ExtTextOut( m_rcStatic.left, m_rcStatic.top, ETO_OPAQUE, m_rcStatic, _T( "" ), 0, NULL );
			
			// draw static text
			int nIndex = GetCurSel();
			if ( nIndex != CB_ERR )
			{
				CString strText;
				GetLBText( nIndex, strText );
			
				if ( !strText.IsEmpty() )
				{
					CRect rcText( m_rcStatic );
					rcText.OffsetRect( 1, 1 );					
					dcMemory.SelectFont( m_fntComboFont );
					dcMemory.SetTextColor( m_rgbStaticText );
					dcMemory.SetBkMode( TRANSPARENT );
					dcMemory.DrawText( strText, strText.GetLength(), rcText, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER );
				}
			}
		}
		
		// draw drop down button
		if ( m_bShowThemed && !m_thmCombo.IsThemeNull() )
			m_thmCombo.DrawThemeBackground( dcMemory, CP_DROPDOWNBUTTON, bPressed ? CBXS_PRESSED : ( bHotButton ? CBXS_HOT : CBXS_NORMAL ), m_rcButton );
		else
			dcMemory.DrawFrameControl( m_rcButton, DFC_SCROLL, DFCS_SCROLLDOWN | ( bPressed ? DFCS_FLAT | DFCS_PUSHED : 0 ) );

		dcMemory.RestoreDC( nContextState );
	}
	
	void OnKillFocus( UINT uCode, int nCtrlID, HWND hwndCtrl )
	{
		SetMsgHandled( FALSE );
		
		CWindow wndParent( GetParent() );
		if ( wndParent.IsWindow() )
		{
			CString strValue;
			
			if ( ( GetStyle() & CBS_DROPDOWNLIST ) == CBS_DROPDOWNLIST )
			{
				int nIndex = GetCurSel();
				if ( nIndex != CB_ERR )
					GetLBText( nIndex, strValue );
			}
			else
			{
				int nValueLength = GetWindowTextLength() + 1;
				GetWindowText( strValue.GetBuffer( nValueLength ), nValueLength );
				strValue.ReleaseBuffer();
			}
			
			CListNotify listNotify;
			listNotify.m_hdrNotify.hwndFrom = m_hWnd;
			listNotify.m_hdrNotify.idFrom = GetDlgCtrlID();
			listNotify.m_hdrNotify.code = LCN_ENDEDIT;
			listNotify.m_nItem = m_nItem;
			listNotify.m_nSubItem = m_nSubItem;
			listNotify.m_nExitChar = m_nExitChar;
			listNotify.m_lpszItemText = strValue;
			listNotify.m_lpItemDate = NULL;

			// forward notification to parent
			FORWARD_WM_NOTIFY( wndParent, listNotify.m_hdrNotify.idFrom, &listNotify.m_hdrNotify, ::SendMessage );
		}
		
		ShowWindow( SW_HIDE );
	}
	
	UINT OnGetDlgCode( LPMSG lpMessage )
	{
		return DLGC_WANTALLKEYS;
	}
	
	void OnChar( TCHAR nChar, UINT nRepCnt, UINT nFlags )
	{
		switch ( nChar )
		{
			case VK_TAB:
			case VK_RETURN:
			case VK_ESCAPE:	{
								m_nExitChar = nChar;
								CWindow wndParent( GetParent() );
								if ( wndParent.IsWindow() )
									wndParent.SetFocus();
							}
							break;
			default:		SetMsgHandled( !IsValid( nChar ) );
							break;
		}
	}
};

/** List Date Support **/

#define DATE_STRING					32

class CListDate : public CWindowImpl< CListDate, CDateTimePickerCtrl >
{
public:
	CListDate()
	{
		m_nItem = NULL_ITEM;
		m_nSubItem = NULL_SUBITEM;
		m_nFlags = ITEM_FLAGS_NONE;
		m_nExitChar = 0;
	}
	
	~CListDate()
	{
	}

protected:
	int m_nItem;
	int m_nSubItem;
	UINT m_nFlags;
	TCHAR m_nExitChar;
	CFont m_fntDateFont;
	
public:
	BOOL Create( HWND hWndParent, int nItem, int nSubItem, CRect& rcRect, UINT nFlags, SYSTEMTIME& stItemDate )
	{
		m_nItem = nItem;
		m_nSubItem = nSubItem;
		m_nFlags = nFlags;
		m_nExitChar = 0;
		
		// destroy old date control...
		if ( IsWindow() )
			DestroyWindow();
			
		DWORD dwStyle = WS_CHILD | WS_CLIPCHILDREN;
		
		if ( nFlags & ITEM_FLAGS_DATETIME_NONE )
			dwStyle |= DTS_SHOWNONE;
		
		if ( nFlags & ITEM_FLAGS_TIME_ONLY )
			dwStyle |= DTS_UPDOWN;
		
		// create date-time control
		if ( CWindowImpl< CListDate, CDateTimePickerCtrl >::Create( hWndParent, CRect( rcRect.left + 3, rcRect.top + 2, rcRect.right - 3, rcRect.bottom - 2 ), NULL, dwStyle ) == NULL )
			return FALSE;
		
		// remove border
		ModifyStyleEx( WS_EX_CLIENTEDGE, 0, SWP_FRAMECHANGED );
		
		// get system message font
		CLogFont logFont;
		logFont.SetMessageBoxFont();
		if ( !m_fntDateFont.IsNull() )
			m_fntDateFont.DeleteObject();
		if ( m_fntDateFont.CreateFontIndirect( &logFont ) == NULL )
			return FALSE;
		SetMonthCalFont( m_fntDateFont );
		SetFont( m_fntDateFont );
		
		TCHAR szDateFormat[ DATE_STRING ];
		GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_SSHORTDATE, szDateFormat, DATE_STRING );
		
		TCHAR szTimeFormat[ DATE_STRING ];
		GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_STIMEFORMAT, szTimeFormat, DATE_STRING );
		
		if ( nFlags & ITEM_FLAGS_DATE_ONLY )
			SetFormat( szDateFormat );
		else if ( nFlags & ITEM_FLAGS_TIME_ONLY )
			SetFormat( szTimeFormat );
		else
			SetFormat( CString( szDateFormat ) + _T( " " ) + CString( szTimeFormat ) );
		
		// get current date if setting time-only
		if ( nFlags & ITEM_FLAGS_TIME_ONLY )
		{
			SYSTEMTIME stCurrentDate;
			if ( GetSystemTime( &stCurrentDate ) == GDT_VALID )
			{
				stItemDate.wYear = stCurrentDate.wYear;
				stItemDate.wMonth = stCurrentDate.wMonth;
				stItemDate.wDay = stCurrentDate.wDay;
			}
		}
		
		SetSystemTime( ( !( nFlags & ITEM_FLAGS_TIME_ONLY ) && stItemDate.wYear == 0 ) ? GDT_NONE : GDT_VALID, &stItemDate );
		
		// show date-time control
		ShowWindow( SW_SHOW );
		
		SetFocus();
		
		return TRUE;
	}
		
	BEGIN_MSG_MAP_EX(CListDate)
		MSG_WM_KILLFOCUS(OnKillFocus)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(DTN_CLOSEUP, OnCloseUp)
		MSG_WM_GETDLGCODE(OnGetDlgCode)
		MSG_WM_CHAR(OnChar)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP_EX()
	
	void OnKillFocus( HWND hNewWnd )
	{
		// have we dropped down the calendar control?
		if ( hNewWnd != NULL && GetMonthCal() == hNewWnd )
			return;
		
		// have we selected a new date from the calendar control?
		if ( GetFocus() == m_hWnd )
			return;
		
		// hide calendar control in case it's not closed by losing focus
		if ( GetMonthCal().IsWindow() )
			GetMonthCal().ShowWindow( SW_HIDE );
		
		CWindow wndParent( GetParent() );
		if ( wndParent.IsWindow() )
		{
			SYSTEMTIME stItemDate;
			BOOL bValidDate = ( GetSystemTime( &stItemDate ) == GDT_VALID );
			if ( !bValidDate )
				ZeroMemory( &stItemDate, sizeof( SYSTEMTIME ) );
			
			if ( m_nFlags & ITEM_FLAGS_DATE_ONLY )
			{
				stItemDate.wHour = 0;
				stItemDate.wMinute = 0;
				stItemDate.wSecond = 0;
				stItemDate.wMilliseconds = 0;
			}
			
			if ( m_nFlags & ITEM_FLAGS_TIME_ONLY )
			{
				stItemDate.wYear = 0;
				stItemDate.wMonth = 0;
				stItemDate.wDay = 0;
				stItemDate.wDayOfWeek = 0;				
			}
				
			CListNotify listNotify;
			listNotify.m_hdrNotify.hwndFrom = m_hWnd;
			listNotify.m_hdrNotify.idFrom = GetDlgCtrlID();
			listNotify.m_hdrNotify.code = LCN_ENDEDIT;
			listNotify.m_nItem = m_nItem;
			listNotify.m_nSubItem = m_nSubItem;
			listNotify.m_nExitChar = m_nExitChar;
			listNotify.m_lpszItemText = bValidDate ? _T( "1" ) : _T( "0" );
			listNotify.m_lpItemDate = &stItemDate;

			// forward notification to parent
			FORWARD_WM_NOTIFY( wndParent, listNotify.m_hdrNotify.idFrom, &listNotify.m_hdrNotify, ::SendMessage );
		}
		
		ShowWindow( SW_HIDE );
	}
	
	LRESULT OnCloseUp( LPNMHDR lpNMHDR )
	{
		SetMsgHandled( FALSE );
		SetFocus();
		return TRUE;
	}
	
	UINT OnGetDlgCode( LPMSG lpMessage )
	{
		return DLGC_WANTALLKEYS;
	}
	
	void OnChar( TCHAR nChar, UINT nRepCnt, UINT nFlags )
	{
		switch ( nChar )
		{
			case VK_TAB:
			case VK_RETURN:
			case VK_ESCAPE:	{
								m_nExitChar = nChar;
								CWindow wndParent( GetParent() );
								if ( wndParent.IsWindow() )
									wndParent.SetFocus();
							}
							break;
			default:		SetMsgHandled( FALSE );
							break;
		}
	}
};



struct CListColumn
{
	CString m_strText;
	int m_nWidth;
	BOOL m_bFixed;
	UINT m_nFormat;
	UINT m_nFlags;
	int m_nImage;
	int m_nIndex;	
	CListArray < CString > m_aComboList;
};

template < class T >
class CListImpl : //public CListViewCtrlT< CWindowImpl< CListImpl< T > > >, //
				  public CWindowImpl< CListImpl< T > >,
				  public CDoubleBufferImpl< CListImpl< T > >
{
	
public:
	CListImpl()
	{
		m_bShowHeader = TRUE;
		m_bShowThemed = TRUE;
		m_bSortAscending = TRUE;
		m_bButtonDown = FALSE;
		m_bMouseOver = FALSE;
		m_bColumnSizing = FALSE;
		m_bBeginSelect = FALSE;
		m_bSingleSelect = FALSE;
		m_bFocusSubItem = FALSE;
		m_bGroupSelect = FALSE;
		m_bEnableHorizScroll = FALSE;
		m_bEnableVertScroll = FALSE;
		m_bShowHorizScroll = TRUE;
		m_bShowVertScroll = TRUE;
		m_bShowSort = TRUE;
		m_bResizeTimer = FALSE;
		m_bDragDrop = FALSE;
		m_bSmoothScroll = TRUE;
		m_bEditItem = FALSE;
		m_bScrolling = FALSE;
		m_bScrollDown = FALSE;
		m_bTileBackground = FALSE;
		m_nMouseWheelScroll = 3;
		m_nTotalWidth = 0;
		m_nHeaderHeight = 0;
		m_nItemHeight = 0;
		m_nFirstSelected = NULL_ITEM;
		m_nFocusItem = NULL_ITEM;
		m_nFocusSubItem = NULL_SUBITEM;
		m_nHotItem = NULL_ITEM;
		m_nHotSubItem = NULL_SUBITEM;
		m_nTitleTipItem = NULL_ITEM;
		m_nTitleTipSubItem = NULL_SUBITEM;
		m_nSortColumn = NULL_COLUMN;
		m_nHighlightColumn = NULL_COLUMN;
		m_nDragColumn = NULL_COLUMN;
		m_nHotColumn = NULL_COLUMN;
		m_nHotDivider = NULL_COLUMN;
		m_nColumnSizing = NULL_COLUMN;
		m_nScrollOffset = 0;
		m_nScrollDelta = 0;
		m_nScrollUnit = 0;
		m_nStartScrollPos = 0;
		m_nStartSize = 0;
		m_nStartPos = 0;
		m_ptDownPoint = 0;
		m_ptSelectPoint = 0;
		m_rcGroupSelect = 0;
		m_dwSearchTick = 0;
		m_dwScrollTick = 0;
		m_strSearchString = _T( "" );
	}
	
	~CListImpl()
	{
	}

protected:
	BOOL m_bShowHeader;
	BOOL m_bShowThemed;
	BOOL m_bShowSort;
	BOOL m_bSortAscending;
	BOOL m_bButtonDown;
	BOOL m_bMouseOver;
	BOOL m_bColumnSizing;
	BOOL m_bBeginSelect;
	BOOL m_bSingleSelect;
	BOOL m_bFocusSubItem;
	BOOL m_bGroupSelect;
	BOOL m_bShowHorizScroll;
	BOOL m_bShowVertScroll;
	BOOL m_bEnableHorizScroll;
	BOOL m_bEnableVertScroll;
	BOOL m_bResizeTimer;
	BOOL m_bDragDrop;
	BOOL m_bSmoothScroll;
	BOOL m_bEditItem;
	BOOL m_bScrolling;
	BOOL m_bScrollDown;
	BOOL m_bTileBackground;
	CPoint m_ptDownPoint;
	CPoint m_ptSelectPoint;
	CRect m_rcGroupSelect;
	int m_nItemHeight;
	int m_nHeaderHeight;
	int m_nFirstSelected;
	int m_nFocusItem;
	int m_nFocusSubItem;
	int m_nHotItem;
	int m_nHotSubItem;
	int m_nTitleTipItem;
	int m_nTitleTipSubItem;
	int m_nMouseWheelScroll;
	int m_nTotalWidth;
	int m_nSortColumn;
	int m_nDragColumn;
	int m_nHighlightColumn;
	int m_nHotColumn;
	int m_nHotDivider;
	int m_nColumnSizing;
	int m_nScrollOffset;
	int m_nScrollDelta;
	int m_nScrollUnit;
	int m_nStartScrollPos;
	int m_nStartSize;
	int m_nStartPos;
	DWORD m_dwSearchTick;
	DWORD m_dwScrollTick;
	CString m_strSearchString;
	CBitmap m_bmpScrollList;
	CBitmap m_bmpBackground;
	
	UINT m_nHeaderClipboardFormat;
	
	COLORREF m_rgbBackground;
	COLORREF m_rgbHeaderBackground;
	COLORREF m_rgbHeaderBorder;
	COLORREF m_rgbHeaderShadow;
	COLORREF m_rgbHeaderText;
	COLORREF m_rgbHeaderHighlight;
	COLORREF m_rgbSelectedItem;
	COLORREF m_rgbSelectedText;
	COLORREF m_rgbItemText;
	COLORREF m_rgbSelectOuter;
	COLORREF m_rgbSelectInner;
	COLORREF m_rgbSelectTop;
	COLORREF m_rgbSelectBottom;
	COLORREF m_rgbNoFocusTop;
	COLORREF m_rgbNoFocusBottom;
	COLORREF m_rgbNoFocusOuter;
	COLORREF m_rgbNoFocusInner;
	COLORREF m_rgbFocusTop;
	COLORREF m_rgbFocusBottom;
	COLORREF m_rgbProgressTop;
	COLORREF m_rgbProgressBottom;
	COLORREF m_rgbItemFocus;
	COLORREF m_rgbHyperLink;
	
	CTheme m_thmHeader;
	CTheme m_thmProgress;
	CCursor m_curDivider;
	CCursor m_curHyperLink;
	CFont m_fntListFont;
	CFont m_fntUnderlineFont;
	CImageList m_ilListItems;
	CImageList m_ilItemImages;
	CDragDrop < CListImpl > m_oleDragDrop;
	CToolTipCtrl m_ttToolTip;
	CDropArrows m_wndDropArrows;
	CTitleTip m_wndTitleTip;
	CListEdit m_wndItemEdit;
	CListCombo m_wndItemCombo;
	CListDate m_wndItemDate;
	
	CListArray < CListColumn > m_aColumns;
	set < int > m_setSelectedItems;

public:
	BOOL SubclassWindow( HWND hWnd )
	{
		T* pT = static_cast<T*>(this);
		return CWindowImpl< CListImpl >::SubclassWindow( hWnd ) ? pT->Initialise() : FALSE;
	}
	
	void RegisterClass()
	{
		T* pT = static_cast<T*>(this);
		pT->GetWndClassInfo().m_wc.lpfnWndProc = m_pfnSuperWindowProc;
		pT->GetWndClassInfo().Register( &m_pfnSuperWindowProc );
	}
	
	BOOL Initialise()
	{
		// load list images
		if ( !m_ilListItems.CreateFromImage( IDB_LISTITEMS, 16, 0, RGB( 255, 0, 255 ), IMAGE_BITMAP, LR_CREATEDIBSECTION ) )
			return FALSE;
		
		if ( m_curDivider.LoadCursor( IDC_DIVIDER ) == NULL )
			return FALSE;
		if ( m_curHyperLink.LoadCursor( IDC_HYPERLINK ) == NULL )
			return FALSE;
		
		// load interface settings
		if ( !LoadSettings() )
			return FALSE;
			
		// give control a static border
		ModifyStyle( WS_BORDER, WS_CLIPCHILDREN );
		ModifyStyleEx( WS_EX_CLIENTEDGE, WS_EX_STATICEDGE, SWP_FRAMECHANGED );		
		
		// register drag drop
		m_oleDragDrop.Register( this );
		m_oleDragDrop.AddTargetFormat( m_nHeaderClipboardFormat );
		m_oleDragDrop.AddSourceFormat( m_nHeaderClipboardFormat );
		
		// create the tooltip
		if ( !m_ttToolTip.Create( m_hWnd ) )
			return FALSE;
		m_ttToolTip.SetMaxTipWidth( SHRT_MAX );
		
		return TRUE;
	}
	
	BOOL LoadSettings()
	{
		m_rgbBackground = GetSysColor( COLOR_WINDOW );
		m_rgbHeaderBackground = GetSysColor( COLOR_BTNFACE );
		m_rgbHeaderBorder = GetSysColor( COLOR_3DHIGHLIGHT );
		m_rgbHeaderShadow = GetSysColor( COLOR_3DSHADOW );
		m_rgbHeaderText = GetSysColor( COLOR_WINDOWTEXT );
		m_rgbHeaderHighlight = RGB( 130, 140, 180 );
		m_rgbSelectedItem = GetSysColor( COLOR_HIGHLIGHT );
		m_rgbSelectedText = GetSysColor( COLOR_HIGHLIGHTTEXT );
		m_rgbItemText = GetSysColor( COLOR_WINDOWTEXT );
		m_rgbSelectOuter = RGB( 170, 200, 245 );
		m_rgbSelectInner = RGB( 230, 250, 250 );
		m_rgbSelectTop = RGB( 210, 240, 250 );
		m_rgbSelectBottom = RGB( 185, 215, 250 );
		m_rgbNoFocusTop = RGB( 250, 250, 250 );
		m_rgbNoFocusBottom = RGB( 235, 235, 235 );
		m_rgbNoFocusOuter = RGB( 220, 220, 220 );
		m_rgbNoFocusInner = RGB( 245, 245, 245 );
		m_rgbFocusTop = RGB( 235, 245, 245 );
		m_rgbFocusBottom = RGB( 225, 235, 245 );
		m_rgbProgressTop = RGB( 170, 240, 170 );
		m_rgbProgressBottom = RGB( 45, 210, 50 );
		m_rgbItemFocus = RGB( 180, 190, 210 );
		m_rgbHyperLink = RGB( 0, 0, 200 );
		
		m_nHeaderClipboardFormat = RegisterClipboardFormat( _T( "HEADERCLIPBOARDFORMAT" ) );
		
		// get number of lines to scroll
		SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &m_nMouseWheelScroll, 0 );
		
		// get system message font
		CLogFont logFont;
		logFont.SetMessageBoxFont();
		if ( !m_fntListFont.IsNull() )
			m_fntListFont.DeleteObject();
		if ( m_fntListFont.CreateFontIndirect( &logFont ) == NULL )
			return FALSE;
		
		// get system underline font
		logFont.lfUnderline = BYTE(TRUE);
		if ( !m_fntUnderlineFont.IsNull() )
			m_fntUnderlineFont.DeleteObject();
		if ( m_fntUnderlineFont.CreateFontIndirect( &logFont ) == NULL )
			return FALSE;
		
		CClientDC dcClient( m_hWnd );
		
		HFONT hOldFont = dcClient.SelectFont( m_fntListFont );
		
		CSize sizeExtent;
		if ( !dcClient.GetTextExtent( _T( "Height" ), -1, &sizeExtent ) )
			return FALSE;
		
		dcClient.SelectFont( hOldFont );
		
		// has system font changed
		if ( m_nItemHeight != sizeExtent.cy + ITEM_HEIGHT_MARGIN )
		{
			m_nItemHeight = sizeExtent.cy + ITEM_HEIGHT_MARGIN;
			m_nHeaderHeight = m_nItemHeight;
			
			// create drop arrows window
			if ( m_wndDropArrows.IsWindow() )
				m_wndDropArrows.DestroyWindow();			
			if ( !m_wndDropArrows.Create( m_hWnd, m_nHeaderHeight, TRUE ) )
				return FALSE;
		}
		
		// open header theme
		if ( !m_thmHeader.IsThemeNull() )
			m_thmHeader.CloseThemeData();
		m_thmHeader.OpenThemeData( m_hWnd, L"HEADER" );
		
		// open progress theme
		if ( !m_thmProgress.IsThemeNull() )
			m_thmProgress.CloseThemeData();
		m_thmProgress.OpenThemeData( m_hWnd, L"PROGRESS" );
		
		// create titletip window
		if ( m_wndTitleTip.IsWindow() )
			m_wndTitleTip.DestroyWindow();			
		if ( !m_wndTitleTip.Create( m_hWnd, m_bShowThemed ) )
			return FALSE;
		
		return TRUE;
	}
	
	void ShowThemed( BOOL bShowThemed = TRUE )
	{
		m_bShowThemed = bShowThemed;
		LoadSettings();
		Invalidate();
	}
	
	void ShowHeader( BOOL bShowHeader = TRUE )
	{
		m_bShowHeader = bShowHeader;		
		ResetScrollBars();
		Invalidate();
	}

	void ShowHeaderSort( BOOL bShowSort = TRUE )
	{
		m_bShowSort = bShowSort;
		Invalidate();
	}

	void SetSingleSelect( BOOL bSingleSelect = TRUE )
	{
		m_bSingleSelect = bSingleSelect;
		Invalidate();
	}
	
	void SetFocusSubItem( BOOL bFocusSubItem = TRUE )
	{
		m_bFocusSubItem = bFocusSubItem;
		Invalidate();
	}
	
	void SetDragDrop( BOOL bDragDrop = TRUE )
	{
		m_bDragDrop = bDragDrop;
	}
	
	void SetSmoothScroll( BOOL bSmoothScroll = TRUE )
	{
		m_bSmoothScroll = bSmoothScroll;
	}
	
	void SetBackgroundImage( HBITMAP hBackgroundImage, BOOL bTileImage = FALSE )
	{
		m_bmpBackground = hBackgroundImage;
		m_bTileBackground = bTileImage;
	}
	
	void SetImageList( CImageList& ilItemImages )
	{
		m_ilItemImages = ilItemImages;
	}
	
	UINT ValidateFlags( UINT nFlags )
	{
		if ( nFlags & ITEM_FLAGS_CENTRE )
			nFlags &= ~( ITEM_FLAGS_LEFT | ITEM_FLAGS_RIGHT );
		if ( nFlags & ITEM_FLAGS_RIGHT )
			nFlags &= ~ITEM_FLAGS_LEFT;
		if ( nFlags & ITEM_FLAGS_DATE_ONLY )
			nFlags &= ~ITEM_FLAGS_TIME_ONLY;
		if ( nFlags & ( ITEM_FLAGS_EDIT_NUMBER | ITEM_FLAGS_EDIT_FLOAT ) )
			nFlags &= ~ITEM_FLAGS_EDIT_UPPER;
		if ( !( nFlags & ( ITEM_FLAGS_EDIT_NUMBER | ITEM_FLAGS_EDIT_FLOAT ) ) )
			nFlags &= ~( ITEM_FLAGS_EDIT_NEGATIVE | ITEM_FLAGS_EDIT_OPERATOR );
		if ( nFlags & ITEM_FLAGS_COMBO_EDIT )
			nFlags &= ~( ITEM_FLAGS_DATE_ONLY | ITEM_FLAGS_TIME_ONLY | ITEM_FLAGS_DATETIME_NONE );
		return nFlags;
	}
	
	void AddColumn( CListColumn& listColumn )
	{
		// minimum column width
		if ( listColumn.m_strText.IsEmpty() && listColumn.m_nImage != ITEM_IMAGE_NONE )
		{
			CSize sizeIcon;
			m_ilListItems.GetIconSize( sizeIcon );
			listColumn.m_nWidth = sizeIcon.cx + 5;
			listColumn.m_nFlags |= ITEM_FLAGS_CENTRE;
		}
		
		// correct incompatible flag mask values
		listColumn.m_nFlags = ValidateFlags( listColumn.m_nFlags );
		
		// initial data index
		listColumn.m_nIndex = GetColumnCount();
		
		m_aColumns.Add( listColumn );
		
		ResetScrollBars();
		Invalidate();
	}
	
	void AddColumn( LPCTSTR lpszText, int nWidth = 0, int nImage = ITEM_IMAGE_NONE, BOOL bFixed = FALSE, UINT nFormat = ITEM_FORMAT_NONE, UINT nFlags = ITEM_FLAGS_NONE )
	{
		CListColumn listColumn;
		listColumn.m_strText = lpszText;
		listColumn.m_nWidth = nWidth;
		listColumn.m_bFixed = bFixed;
		listColumn.m_nFormat = nFormat;
		listColumn.m_nFlags = nFlags;
		listColumn.m_nImage = nImage;
		AddColumn( listColumn );
	}
	
	int GetColumnCount()
	{
		return m_aColumns.GetSize();
	}
	
	BOOL GetColumn( int nColumn, CListColumn& listColumn )
	{
		if ( nColumn < 0 || nColumn >= GetColumnCount() ) 
			return FALSE;
		listColumn = m_aColumns[ nColumn ];
		return TRUE;
	}
		
	int GetTotalWidth( BOOL bRecalc = FALSE )
	{
		if ( bRecalc )
		{
			m_nTotalWidth = 0;
			for ( int nColumn = 0; nColumn < GetColumnCount(); nColumn++ )
				m_nTotalWidth += GetColumnWidth( nColumn );
		}		
		return m_nTotalWidth - 1;
	}
	
	int GetTotalHeight()
	{
		T* pT = static_cast<T*>(this);
		return max( ( pT->GetItemCount() * m_nItemHeight ) + ( m_bShowHeader ? m_nHeaderHeight : 0 ), 1 );
	}
	
	BOOL SetColumnWidth( int nColumn, int nWidth )
	{
		if ( nColumn < 0 || nColumn >= GetColumnCount() ) 
			return FALSE;
		
		// set new column size if not fixed
		if ( !m_aColumns[ nColumn ].m_bFixed )
		{
			m_aColumns[ nColumn ].m_nWidth = nWidth;
			
			ResetScrollBars();
			Invalidate();
		}
		
		return TRUE;
	}

	int GetColumnWidth( int nColumn )
	{
		CListColumn listColumn;
		return GetColumn( nColumn, listColumn ) ? listColumn.m_nWidth : 0;
	}
	
	int GetColumnIndex( int nColumn )
	{
		CListColumn listColumn;
		return GetColumn( nColumn, listColumn ) ? listColumn.m_nIndex : 0;
	}
	
	int IndexToOrder( int nIndex )
	{
		for ( int nColumn = 0; nColumn < GetColumnCount(); nColumn++ )
		{
			if ( GetColumnIndex( nColumn ) == nIndex )
				return nColumn;
		}
		return -1;
	}
	
	BOOL SetColumnFormat( int nColumn, UINT nFormat, UINT nFlags = ITEM_FLAGS_NONE )
	{
		if ( nColumn < 0 || nColumn >= GetColumnCount() ) 
			return FALSE;
		m_aColumns[ nColumn ].m_nFormat = nFormat;
		m_aColumns[ nColumn ].m_nFlags = ValidateFlags( nFlags );
		return TRUE;
	}
	
	BOOL SetColumnFormat( int nColumn, UINT nFormat, UINT nFlags, CListArray < CString >& aComboList )
	{
		if ( nColumn < 0 || nColumn >= GetColumnCount() ) 
			return FALSE;
		m_aColumns[ nColumn ].m_nFormat = nFormat;
		m_aColumns[ nColumn ].m_nFlags = ValidateFlags( nFlags );
		m_aColumns[ nColumn ].m_aComboList = aComboList;
		return TRUE;
	}
	
	UINT GetColumnFormat( int nColumn )
	{
		CListColumn listColumn;
		return GetColumn( nColumn, listColumn ) ? listColumn.m_nFormat : ITEM_FORMAT_NONE;
	}
	
	UINT GetColumnFlags( int nColumn )
	{
		CListColumn listColumn;
		return GetColumn( nColumn, listColumn ) ? listColumn.m_nFlags : ITEM_FLAGS_NONE;
	}
	
	BOOL GetColumnComboList( int nColumn, CListArray < CString >& aComboList )
	{
		CListColumn listColumn;
		if ( !GetColumn( nColumn, listColumn ) )
			return FALSE;
		aComboList = listColumn.m_aComboList;
		return !aComboList.IsEmpty();		
	}
	
	BOOL GetColumnRect( int nColumn, CRect& rcColumn )
	{
		if ( nColumn < 0 || nColumn >= GetColumnCount() ) 
			return FALSE;
			
		GetClientRect( rcColumn );
		rcColumn.bottom = m_nHeaderHeight;
		
		for ( int nColumnOrder = 0; nColumnOrder < GetColumnCount(); nColumnOrder++ )
		{
			int nWidth = GetColumnWidth( nColumnOrder );
			
			if ( nColumn == nColumnOrder )
			{
				rcColumn.right = rcColumn.left + nWidth;
				break;
			}
			
			rcColumn.left += nWidth;
		}
		
		// offset column by scroll position
		rcColumn.OffsetRect( -GetScrollPos( SB_HORZ ), 0 );
		
		return TRUE;
	}
	
	BOOL AddItem()
	{
		ResetScrollBars();
		return Invalidate();
	}
	
	BOOL DeleteItem( int nItem )
	{
		m_setSelectedItems.erase( nItem );		
		ResetScrollBars();
		return Invalidate();
	}
	
	BOOL DeleteAllItems()
	{
		m_setSelectedItems.clear();		
		ResetScrollBars();
		return Invalidate();
	}
	
	int GetItemCount()
	{
		ATLASSERT( FALSE ); // must be implemented in a derived class
		return 0;
	}
	
	CString GetItemText( int nItem, int nSubItem )
	{
		ATLASSERT( FALSE ); // must be implemented in a derived class
		return _T( "" );
	}
	
	BOOL GetItemDate( int nItem, int nSubItem, SYSTEMTIME& stItemDate )
	{
		T* pT = static_cast<T*>(this);
		
		ZeroMemory( &stItemDate, sizeof( SYSTEMTIME ) );
		
		CString strItemText = pT->GetItemText( nItem, nSubItem );
		if ( strItemText.IsEmpty() )
			return FALSE;
		
		// get date-time from item text: yyyymmddhhmmss
		stItemDate.wYear = _ttoi( strItemText.Left( 4 ) );
		stItemDate.wMonth = _ttoi( strItemText.Mid( 4, 2 ) );
		stItemDate.wDay = _ttoi( strItemText.Mid( 6, 2 ) );
		stItemDate.wHour = _ttoi( strItemText.Mid( 8, 2 ) );
		stItemDate.wMinute = _ttoi( strItemText.Mid( 10, 2 ) );
		stItemDate.wSecond = _ttoi( strItemText.Mid( 12, 2 ) );
		stItemDate.wMilliseconds = 0;
		
		return TRUE;
	}
	
	int GetItemImage( int nItem, int nSubItem )
	{
		return ITEM_IMAGE_NONE; // may be implemented in a derived class
	}
	
	UINT GetItemFormat( int nItem, int nSubItem )
	{
		return GetColumnFormat( IndexToOrder( nSubItem ) ); // may be implemented in a derived class
	}
	
	UINT GetItemFlags( int nItem, int nSubItem )
	{
		return GetColumnFlags( IndexToOrder( nSubItem ) ); // may be implemented in a derived class
	}
	
	BOOL GetItemComboList( int nItem, int nSubItem, CListArray < CString >& aComboList )
	{
		return GetColumnComboList( IndexToOrder( nSubItem ), aComboList ); // may be implemented in a derived class
	}
	
	HFONT GetItemFont( int nItem, int nSubItem )
	{
		return m_fntListFont; // may be implemented in a derived class
	}
	
	BOOL GetItemColours( int nItem, int nSubItem, COLORREF& rgbBackground, COLORREF& rgbText )
	{
		rgbBackground = m_rgbBackground;
		rgbText = m_rgbItemText;
		return TRUE;
	}
	
	CString GetItemToolTip( int nItem, int nSubItem )
	{
		return _T( "" ); // may be implemented in a derived class
	}
	
	BOOL SetItemText( int nItem, int nSubItem, LPCTSTR lpszText )
	{
		ATLASSERT( FALSE ); // must be implemented in a derived class
		return FALSE;
	}
	
	BOOL SetItemComboIndex( int nItem, int nSubItem, int nIndex )
	{
		ATLASSERT( FALSE ); // must be implemented in a derived class
		return FALSE;
	}
	
	BOOL SetItemDate( int nItem, int nSubItem, SYSTEMTIME& stItemDate )
	{
		T* pT = static_cast<T*>(this);
		
		// set date-time in format (yyyymmddhhmmss)
		CString strFormatDate;
		strFormatDate.Format( _T( "%04d%02d%02d%02d%02d%02d" ), stItemDate.wYear, stItemDate.wMonth, stItemDate.wDay, stItemDate.wHour, stItemDate.wMinute, stItemDate.wSecond );
		
		return pT->SetItemText( nItem, nSubItem, strFormatDate );
	}
	
	BOOL SetItemCheck( int nItem, int nSubItem, int nCheckValue )
	{
		T* pT = static_cast<T*>(this);
		
		switch ( pT->GetItemFormat( nItem, nSubItem ) )
		{
			case ITEM_FORMAT_CHECKBOX:			return pT->SetItemText( nItem, nSubItem, nCheckValue > 0 ? _T( "1" ) : _T( "0" ) );
			case ITEM_FORMAT_CHECKBOX_3STATE:	if ( nCheckValue < 0 )
													return pT->SetItemText( nItem, nSubItem, _T( "-1" ) );
												if ( nCheckValue > 0 )
													return pT->SetItemText( nItem, nSubItem, _T( "1" ) );
												return pT->SetItemText( nItem, nSubItem, _T( "0" ) );
		}
		
		return FALSE;
	}
		
	BOOL SetItemImage( int nItem, int nSubItem, int nImage )
	{
		ATLASSERT( FALSE ); // must be implemented in a derived class
		return FALSE;
	}
	
	BOOL SetItemFormat( int nItem, int nSubItem, UINT nFormat, UINT nFlags = ITEM_FLAGS_NONE )
	{
		ATLASSERT( FALSE ); // must be implemented in a derived class
		return FALSE;
	}
	
	BOOL SetItemFormat( int nItem, int nSubItem, UINT nFormat, UINT nFlags, CListArray < CString >& aComboList )
	{
		ATLASSERT( FALSE ); // must be implemented in a derived class
		return FALSE;
	}
	
	BOOL SetItemFont( int nItem, int nSubItem, HFONT hFont )
	{
		ATLASSERT( FALSE ); // must be implemented in a derived class
		return FALSE;
	}
	
	BOOL SetItemColours( int nItem, int nSubItem, COLORREF rgbBackground, COLORREF rgbText )
	{
		ATLASSERT( FALSE ); // must be implemented in a derived class
		return FALSE;
	}
		
	void ReverseItems()
	{
		ATLASSERT( FALSE ); // must be implemented in a derived class
	}
	
	void SortItems( int nColumn, BOOL bAscending )
	{
		ATLASSERT( FALSE ); // must be implemented in a derived class
	}
	
	BOOL GetItemRect( int nItem, int nSubItem, CRect& rcItem )
	{
		T* pT = static_cast<T*>(this);
		
		int nTopItem = GetTopItem();		
		if ( nItem < nTopItem || nItem >= pT->GetItemCount() || nItem >= nTopItem + GetCountPerPage() )
			return FALSE;
		
		CRect rcClient;
		GetClientRect( rcClient );
		
		// calculate item rect based on scroll position
		rcItem = rcClient;
		rcItem.top = ( m_bShowHeader ? m_nHeaderHeight : 0 ) + ( ( nItem - nTopItem ) * m_nItemHeight );
		rcItem.bottom = rcItem.top + m_nItemHeight;
		rcItem.right = min( rcClient.right, GetTotalWidth() );
		
		if ( nSubItem != NULL_SUBITEM )
		{
			CRect rcColumn;
			if ( !GetColumnRect( nSubItem, rcColumn ) )
				return FALSE;

			rcItem.left = rcColumn.left;
			rcItem.right = rcColumn.right;
		}
		
		return TRUE;
	}
	
	BOOL GetItemRect( int nItem, CRect& rcItem )
	{
		return GetItemRect( nItem, NULL_SUBITEM, rcItem );
	}
	
	BOOL InvalidateItem( int nItem, int nSubItem = NULL_SUBITEM )
	{
		CRect rcItem;
		return GetItemRect( nItem, nSubItem, rcItem ) ? InvalidateRect( rcItem ) : FALSE;
	}

	BOOL InvalidateHeader()
	{
		if ( !m_bShowHeader )
			return TRUE;
		CRect rcHeader;
		if ( !GetClientRect( rcHeader ) )
			return FALSE;
		rcHeader.bottom = m_nHeaderHeight;
		return InvalidateRect( rcHeader );	
	}
	
	int GetTopItem()
	{
		return (int)( GetScrollPos( SB_VERT ) / m_nItemHeight );
	}
	
	int GetCountPerPage( BOOL bPartial = TRUE )
	{
		CRect rcClient;
		GetClientRect( rcClient );
		rcClient.top = ( m_bShowHeader ? m_nHeaderHeight : 0 );
		
		// calculate number of items per control height (include partial item)
		div_t divHeight = div( rcClient.Height(), m_nItemHeight );
			
		// round up to nearest item count
		return max( bPartial && divHeight.rem > 0 ? divHeight.quot + 1 : divHeight.quot, 1 );
	}
	
	BOOL IsItemVisible( int nItem, int nSubItem = NULL_SUBITEM, BOOL bPartial = TRUE )
	{
		T* pT = static_cast<T*>(this);
		
		int nTopItem = GetTopItem();
		if ( nItem < nTopItem || nItem >= pT->GetItemCount() )
			return FALSE;
		
		// check whether item is visible
		if ( nItem < nTopItem || nItem >= nTopItem + GetCountPerPage( bPartial ) )
			return FALSE;
		
		// check whether subitem is visible
		if ( m_bFocusSubItem && nSubItem != NULL_SUBITEM )
		{
			CRect rcColumn;
			if ( !GetColumnRect( nSubItem, rcColumn ) )
				return FALSE;
			
			CRect rcClient;
			GetClientRect( rcClient );
			
			if ( rcColumn.left < rcClient.left || rcColumn.right > rcClient.right )
				return FALSE;
		}
		
		return TRUE;
	}
	
	BOOL EnsureItemVisible( int nItem, int nSubItem = NULL_SUBITEM )
	{
		if ( IsItemVisible( nItem, nSubItem, FALSE ) )
			return TRUE;
		
		HideTitleTip();
		
		CRect rcClient;
		GetClientRect( rcClient );
		rcClient.top = ( m_bShowHeader ? m_nHeaderHeight : 0 );
		
		CRect rcItem;
		rcItem.top = ( m_bShowHeader ? m_nHeaderHeight : 0 ) + ( ( nItem - GetTopItem() ) * m_nItemHeight );
		rcItem.bottom = rcItem.top + m_nItemHeight;
		
		if ( rcItem.top < rcClient.top || rcItem.bottom > rcClient.bottom )
		{
			int nScrollItem = NULL_ITEM;
			
			// scroll list up/down to include item
			if ( rcItem.top < rcClient.top || rcItem.Height() > rcClient.Height() )
				nScrollItem = nItem;
			else if ( rcItem.bottom > rcClient.bottom )
				nScrollItem = nItem - ( GetCountPerPage( FALSE ) - 1 );
			
			if ( nScrollItem != NULL_ITEM )
				SetScrollPos( SB_VERT, nScrollItem * m_nItemHeight );
		}
		
		if ( m_bFocusSubItem && nSubItem != NULL_SUBITEM )
		{
			CRect rcColumn;
			if ( !GetColumnRect( nSubItem, rcColumn ) )
				return FALSE;
			
			CRect rcClient;
			GetClientRect( rcClient );

			int nScrollPos = 0;

			// scroll list left/right to include subitem
			if ( rcColumn.Width() > rcClient.Width() || rcColumn.left < 0 )
				nScrollPos = rcColumn.left;
			else if ( rcColumn.right > rcClient.right )
				nScrollPos = rcColumn.right - rcClient.right;

			if ( nScrollPos != 0 )
				SetScrollPos( SB_HORZ, GetScrollPos( SB_HORZ ) + nScrollPos );
		}
		
		return Invalidate();
	}
	
	void ShowScrollBar( int nScrollBar, BOOL bShow = TRUE )
	{
		switch ( nScrollBar )
		{
			case SB_HORZ:	m_bShowHorizScroll = bShow;
							break;
			case SB_VERT:	m_bShowVertScroll = bShow;
							break;
			case SB_BOTH:	m_bShowHorizScroll = bShow;
							m_bShowVertScroll = bShow;
							break;
		}
		
		ResetScrollBars();
		Invalidate();
	}
	
	void ResetScrollBars( int nScrollBar = SB_BOTH, int nScrollPos = -1, BOOL bRecalc = TRUE )
	{
		T* pT = static_cast<T*>(this);
		
		CRect rcClient;
		GetClientRect( rcClient );
		
		SCROLLINFO infoScroll;
		infoScroll.cbSize = sizeof( SCROLLINFO );
		infoScroll.fMask = nScrollPos < 0 ? SIF_PAGE | SIF_RANGE : SIF_PAGE | SIF_RANGE | SIF_POS;
		infoScroll.nPos = nScrollPos;
		infoScroll.nMin = 0;
		
		if ( ( nScrollBar == SB_BOTH || nScrollBar == SB_VERT ) && m_bShowVertScroll )
		{
			infoScroll.nMax = ( pT->GetItemCount() * m_nItemHeight ) + ( m_bShowHeader ? m_nHeaderHeight : 0 );
			infoScroll.nPage = rcClient.Height() - ( m_bShowHeader ? m_nHeaderHeight : 0 );
			
			// are we within client range?
			if ( (UINT)infoScroll.nMax <= infoScroll.nPage + ( m_bShowHeader ? m_nHeaderHeight : 0 ) )
				infoScroll.nMax = 0;
				    
			// set vertical scroll bar
			m_bEnableVertScroll = SetScrollInfo( SB_VERT, &infoScroll, TRUE ) ? ( infoScroll.nMax > 0 ) : FALSE;
		}
		
		if ( ( nScrollBar == SB_BOTH || nScrollBar == SB_HORZ ) && m_bShowHorizScroll )
		{
			infoScroll.nMax = GetTotalWidth( bRecalc );
			infoScroll.nPage = rcClient.Width();
			
			// are we within client range?
			if ( infoScroll.nPage >= (UINT)infoScroll.nMax )
				infoScroll.nMax = 0;
				
			// set horizontal scroll bar
			m_bEnableHorizScroll = SetScrollInfo( SB_HORZ, &infoScroll, TRUE ) ? ( infoScroll.nMax > (int)infoScroll.nPage ) : FALSE;
		}
	}

	BOOL IsScrollBarVisible( int nScrollBar )
	{
		switch ( nScrollBar )
		{
			case SB_HORZ:	return m_bEnableHorizScroll;
			case SB_VERT:	return m_bEnableVertScroll;
			case SB_BOTH:	return ( m_bEnableHorizScroll && m_bEnableVertScroll );
			default:		return FALSE;
		}
	}
	
	BOOL ResetSelected()
	{
		m_setSelectedItems.clear();
		m_nFocusItem = NULL_ITEM;
		m_nFocusSubItem = NULL_SUBITEM;
		m_nFirstSelected = NULL_ITEM;
		return Invalidate();
	}
	
	BOOL SelectItem( int nItem, int nSubItem = NULL_SUBITEM, UINT nFlags = 0 )
	{
		T* pT = static_cast<T*>(this);
		
		if ( nItem < 0 || nItem >= pT->GetItemCount() )
			return FALSE;
		
		BOOL bSelectItem = TRUE;
		BOOL bSelectRange = !m_bSingleSelect && ( nFlags & MK_SHIFT );
		BOOL bNewSelect = !( bSelectRange || ( nFlags & MK_CONTROL ) );
		BOOL bEnsureVisible = FALSE;

		// are we starting a new select sequence?
		if ( bNewSelect || bSelectRange )
		{
			// are we simply reselecting the same item?
			if ( m_setSelectedItems.size() == 1 && *m_setSelectedItems.begin() == nItem )
			{
				bSelectItem = FALSE;
				m_nFirstSelected = nItem;
				m_nFocusItem = nItem;
				m_nFocusSubItem = nSubItem;
			}
			else
				m_setSelectedItems.clear();
		}
		else // we adding to or removing from select sequence
		{
			if ( m_bSingleSelect )
				m_setSelectedItems.clear();
			
			set < int >::iterator posSelectedItem = m_setSelectedItems.find( nItem );
			
			// is this item already selected?
			if ( posSelectedItem != m_setSelectedItems.end() )
			{
				bSelectItem = FALSE;
				m_setSelectedItems.erase( posSelectedItem );				
				m_nFirstSelected = nItem;
				m_nFocusItem = nItem;
				m_nFocusSubItem = m_setSelectedItems.size() > 1 ? NULL_SUBITEM : nSubItem;
			}
		}
		
		// are we adding this item to the select sequence?
		if ( bSelectItem )
		{
			bEnsureVisible = TRUE;
			
			if ( bSelectRange )
			{
				if ( m_nFirstSelected == NULL_ITEM )
					m_nFirstSelected = nItem;
					
				for ( int nSelectedItem = min( m_nFirstSelected, nItem ); nSelectedItem <= max( m_nFirstSelected, nItem ); nSelectedItem++ )
					m_setSelectedItems.insert( nSelectedItem );
			}
			else
			{
				m_nFirstSelected = nItem;
				m_setSelectedItems.insert( nItem );
			}
			
			m_nFocusItem = nItem;
			m_nFocusSubItem = m_setSelectedItems.size() > 1 ? NULL_SUBITEM : nSubItem;
			
			// notify parent of selected item
			NotifyParent( m_nFocusItem, m_nFocusSubItem, LCN_SELECTED );
		}
		
		// start visible timer (scrolls list to partially hidden item)
		if ( !IsItemVisible( nItem, m_setSelectedItems.size() > 1 ? NULL_SUBITEM : nSubItem, FALSE ) )
			SetTimer( ITEM_VISIBLE_TIMER, ITEM_VISIBLE_PERIOD );
		else if ( m_nFocusItem != NULL_ITEM && m_nFocusSubItem != NULL_SUBITEM )
			EditItem( m_nFocusItem, m_nFocusSubItem );

		return Invalidate();
	}
	
	BOOL IsSelected( int nItem )
	{
		set < int >::iterator posSelectedItem = m_setSelectedItems.find( nItem );
		return ( posSelectedItem != m_setSelectedItems.end() );
	}
	
	BOOL GetSelectedItems( CListArray < int >& aSelectedItems )
	{
		aSelectedItems.RemoveAll();
		for ( set < int >::iterator posSelectedItem = m_setSelectedItems.begin(); posSelectedItem != m_setSelectedItems.end(); ++posSelectedItem )
			aSelectedItems.Add( *posSelectedItem );
		return !aSelectedItems.IsEmpty();
	}
	
	BOOL SetFocusItem( int nItem, int nSubItem = NULL_SUBITEM )
	{
		m_nFocusItem = nItem;
		m_nFocusSubItem = nSubItem;		
		return EnsureItemVisible( m_nFocusItem, m_nFocusSubItem );
	}

	BOOL GetFocusItem( int& nItem, int& nSubItem )
	{
		nItem = IsSelected( m_nFocusItem ) ? m_nFocusItem : ( m_setSelectedItems.empty() ? NULL_ITEM : *m_setSelectedItems.begin() );
		nSubItem = !m_bFocusSubItem || nItem == NULL_ITEM ? NULL_SUBITEM : m_nFocusSubItem;
		return ( nItem != NULL_ITEM );
	}
	
	int GetFocusItem()
	{
		return IsSelected( m_nFocusItem ) ? m_nFocusItem : ( m_setSelectedItems.empty() ? NULL_ITEM : *m_setSelectedItems.begin() );
	}
	
	BOOL HitTestHeader( CPoint point, int& nColumn, UINT& nFlags )
	{
		// reset hittest flags
		nFlags = HITTEST_FLAG_NONE;
		
		if ( !m_bShowHeader )
			return FALSE;
		
		CRect rcClient;
		if ( !GetClientRect( rcClient ) )
			return FALSE;
		
		// are we over the header?
		if ( point.y < rcClient.top || point.y > m_nHeaderHeight )
			return FALSE;
		
		int nDividerPos = 0;
		int nColumnCount = GetColumnCount();
	
		// get hit-test subitem
		for ( nColumn = 0; nColumn < nColumnCount; nColumn++ )
		{
			int nColumnWidth = GetColumnWidth( nColumn );
			nDividerPos += nColumnWidth;

			// offset divider position with current scroll position
			int nRelativePos = nDividerPos - GetScrollPos( SB_HORZ );

			// are we over the divider zone?
			if ( point.x >= nRelativePos - DRAG_HEADER_OFFSET - 1 && point.x <= nRelativePos + DRAG_HEADER_OFFSET )
			{
				nFlags |= HITTEST_FLAG_HEADER_DIVIDER;
				
				// are we to the left of the divider (or over last column divider)?
				if ( ( point.x >= nRelativePos - DRAG_HEADER_OFFSET - 1 && point.x < nRelativePos ) || nColumn + 1 >= nColumnCount - 1 )
				{
					nFlags |= HITTEST_FLAG_HEADER_LEFT;
					return TRUE;
				}

				// find last zero-length column after this column
				for ( int nNextColumn = nColumn + 1; nNextColumn < nColumnCount; nNextColumn++ )
				{
					if ( GetColumnWidth( nNextColumn ) > 0 )
						break;
					nColumn = nNextColumn;
				}
				
				nFlags |= HITTEST_FLAG_HEADER_RIGHT;

				return TRUE;
			}

			// are we over a column?
			if ( point.x > nRelativePos - nColumnWidth && point.x < nRelativePos )
				return TRUE;
		}	
		
		return FALSE;
	}
	
	BOOL HitTest( CPoint point, int& nItem, int& nSubItem )
	{
		T* pT = static_cast<T*>(this);
		
		// are we over the header?
		if ( point.y < ( m_bShowHeader ? m_nHeaderHeight : 0 ) )
			return FALSE;
		
		// calculate hit test item
		nItem = GetTopItem() + (int)( ( point.y - ( m_bShowHeader ? m_nHeaderHeight : 0 ) ) / m_nItemHeight );
		
		if ( nItem < 0 || nItem >= pT->GetItemCount() )
			return FALSE;
		
		int nTotalWidth = 0;
		int nColumnCount = GetColumnCount();
	
		// get hit-test subitem
		for ( nSubItem = 0; nSubItem < nColumnCount; nSubItem++ )
		{
			int nColumnWidth = GetColumnWidth( nSubItem );
			nTotalWidth += nColumnWidth;

			// offset position with current scroll position
			int nRelativePos = nTotalWidth - GetScrollPos( SB_HORZ );

			// are we over a subitem?
			if ( point.x > nRelativePos - nColumnWidth && point.x < nRelativePos )
				return TRUE;
		}
		
		return FALSE;
	}
	
	BOOL AutoSizeColumn( int nColumn )
	{
		T* pT = static_cast<T*>(this);
		
		CListColumn listColumn;
		if ( !GetColumn( nColumn, listColumn ) || listColumn.m_bFixed )
			return FALSE;
			
		CClientDC dcClient( m_hWnd );
		HFONT hOldFont = dcClient.SelectFont( m_fntListFont );
		
		// set to column text width if zero-length
		CSize sizeExtent;
		if ( !dcClient.GetTextExtent( listColumn.m_strText, -1, &sizeExtent ) )
			return FALSE;
		
		int nMaxWidth = sizeExtent.cx + ITEM_WIDTH_MARGIN;
		
		CSize sizeIcon = 0;
		if ( !m_ilItemImages.IsNull() )
			m_ilItemImages.GetIconSize( sizeIcon );
		
		// calculate maximum column width required
		for ( int nItem = 0; nItem < pT->GetItemCount(); nItem++ )
		{
			if ( !dcClient.GetTextExtent( pT->GetItemText( nItem, listColumn.m_nIndex ), -1, &sizeExtent ) )
				return FALSE;
			
			if ( !m_ilItemImages.IsNull() && pT->GetItemImage( nItem, listColumn.m_nIndex ) != ITEM_IMAGE_NONE )
				sizeExtent.cx += sizeIcon.cx;
			
			nMaxWidth = max( nMaxWidth, (int)sizeExtent.cx + ITEM_WIDTH_MARGIN );
		}
		
		dcClient.SelectFont( hOldFont );
		
		return SetColumnWidth( nColumn, nMaxWidth );	
	}
	
	void ResizeColumn( BOOL bColumnScroll = FALSE )
	{
		HideTitleTip();
		
		int nCurrentPos = GET_X_LPARAM( GetMessagePos() );
		
		CRect rcClient;
		GetClientRect( rcClient );
		int nScrollLimit = GetTotalWidth() - rcClient.Width();
		
		if ( bColumnScroll )
		{
			// have we finished scrolling list to accommodate new column size?
			if ( !m_bColumnSizing || !m_bEnableHorizScroll || nCurrentPos - m_nStartScrollPos > 0 )
			{
				KillTimer( RESIZE_COLUMN_TIMER );
				
				// reset resize start point
				m_nStartPos = nCurrentPos;
				m_bResizeTimer = FALSE;
			}
			else if ( nCurrentPos < m_nStartPos && GetScrollPos( SB_HORZ ) >= nScrollLimit )
			{
				// reset start column size
				m_nStartSize = max( GetColumnWidth( m_nColumnSizing ) + ( nCurrentPos - m_nStartScrollPos ), 0 );
				
				// resize column
				SetColumnWidth( m_nColumnSizing, m_nStartSize );
			}
		}
		else
		{
			int nColumnSize = max( m_nStartSize + ( nCurrentPos - m_nStartPos ), 0 );
			
			// are we scrolled fully to the right and wanting to reduce the size of a column?
			if ( m_bEnableHorizScroll && GetScrollPos( SB_HORZ ) >= nScrollLimit && nColumnSize < GetColumnWidth( m_nColumnSizing ) )
			{
				if ( !m_bResizeTimer )
				{
					// only start the scroll timer once
					m_bResizeTimer = TRUE;

					// set new start scroll position
					m_nStartScrollPos = nCurrentPos;

					// start column resize / scroll timer
					SetTimer( RESIZE_COLUMN_TIMER, RESIZE_COLUMN_PERIOD );
				}
			}
			else
			{
				// resizing is done in scroll timer (if started)
				if ( !m_bResizeTimer )
					SetColumnWidth( m_nColumnSizing, nColumnSize );
			}
		}
	}
	
	void DragColumn()
	{
		HideTitleTip();
		
		CRect rcColumn;
		if ( !GetColumnRect( m_nHighlightColumn, rcColumn ) )
			return;
		
		CRect rcHeaderItem( rcColumn );
		rcHeaderItem.MoveToXY( 0, 0 );
		
		CListColumn listColumn;
		if ( !GetColumn( m_nHighlightColumn, listColumn ) )
			return;
		
		// store drag column
		m_nDragColumn = m_nHighlightColumn;
		
		CClientDC dcClient( m_hWnd );
		
		CDC dcHeader;
		dcHeader.CreateCompatibleDC( dcClient );
		
		int nContextState = dcHeader.SaveDC();
		
		// create drag header bitmap
		CBitmapHandle bmpHeader;
		bmpHeader.CreateCompatibleBitmap( dcClient, rcHeaderItem.Width(), rcHeaderItem.Height() );
		dcHeader.SelectBitmap( bmpHeader );
		
		if ( m_bShowThemed && !m_thmHeader.IsThemeNull() )
			m_thmHeader.DrawThemeBackground( dcHeader, HP_HEADERITEM, HIS_PRESSED, rcHeaderItem, NULL );
		else
		{
			dcHeader.SetBkColor( m_rgbHeaderBackground );
			dcHeader.ExtTextOut( rcHeaderItem.left, rcHeaderItem.top, ETO_OPAQUE, rcHeaderItem, _T( "" ), 0, NULL );
			dcHeader.Draw3dRect( rcHeaderItem, m_rgbHeaderBorder, m_rgbHeaderShadow );
		}
		
		CRect rcHeaderText( rcHeaderItem );
		rcHeaderText.left += m_nHighlightColumn == 0 ? 4 : 3;
		rcHeaderText.OffsetRect( 0, 1 );
		
		// margin header text
		rcHeaderText.DeflateRect( 4, 0, 5, 0 );
		
		// has this header item an associated image?
		if ( listColumn.m_nImage != ITEM_IMAGE_NONE )
		{
			CSize sizeIcon;
			m_ilListItems.GetIconSize( sizeIcon );
			
			CRect rcHeaderImage;
			rcHeaderImage.left = listColumn.m_strText.IsEmpty() ? ( ( rcHeaderText.left + rcHeaderText.right ) / 2 ) - ( sizeIcon.cx / 2 ) - ( ( !m_bShowThemed || m_thmHeader.IsThemeNull() ) ? 0 : 1 ) : rcHeaderText.left;
			rcHeaderImage.right = min( rcHeaderImage.left + sizeIcon.cx, rcHeaderItem.right );
			rcHeaderImage.top = ( ( rcHeaderItem.top + rcHeaderItem.bottom ) / 2 ) - ( sizeIcon.cy / 2 );
			rcHeaderImage.bottom = min( rcHeaderImage.top + sizeIcon.cy, rcHeaderItem.bottom );
				
			m_ilListItems.DrawEx( listColumn.m_nImage, dcHeader, rcHeaderImage, CLR_DEFAULT, CLR_DEFAULT, ILD_TRANSPARENT );

			// offset header text (for image)
			rcHeaderText.left += sizeIcon.cx + 4;
		}
		
		dcHeader.SelectFont( m_fntListFont );
		dcHeader.SetTextColor( m_rgbHeaderText );
		dcHeader.SetBkMode( TRANSPARENT );

		UINT nFormat = DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER | DT_END_ELLIPSIS;

		if ( listColumn.m_nFlags & ITEM_FLAGS_CENTRE )
			nFormat |= DT_CENTER;
		else if ( listColumn.m_nFlags & ITEM_FLAGS_RIGHT )
			nFormat |= DT_RIGHT;
		else
			nFormat |= DT_LEFT;
			
		// draw header text
		if ( !listColumn.m_strText.IsEmpty() )
			dcHeader.DrawText( listColumn.m_strText, listColumn.m_strText.GetLength(), rcHeaderText, nFormat );

		dcHeader.RestoreDC( nContextState );
		
		SHDRAGIMAGE shDragImage;
		ZeroMemory( &shDragImage, sizeof( SHDRAGIMAGE ) );
		
		shDragImage.sizeDragImage.cx = rcHeaderItem.Width();
		shDragImage.sizeDragImage.cy = rcHeaderItem.Height();
		shDragImage.ptOffset.x = rcColumn.Width() / 2;
		shDragImage.ptOffset.y = rcColumn.Height() / 2;
		shDragImage.hbmpDragImage = bmpHeader;
		shDragImage.crColorKey = m_rgbBackground;
		
		// start header drag operation
		m_oleDragDrop.DoDragDrop( &shDragImage, DROPEFFECT_MOVE );
		
		// hide drop arrows after moving column
		m_wndDropArrows.Hide();
		
		if ( m_bButtonDown )
		{
			ReleaseCapture();
			m_bButtonDown = FALSE;
			m_bBeginSelect = FALSE;
			m_ptDownPoint = 0;
			m_ptSelectPoint = 0;
		}
		
		// finish moving a column
		if ( m_nHighlightColumn != NULL_COLUMN )
		{
			m_nHighlightColumn = NULL_COLUMN;
			InvalidateHeader();
		}
		
		m_nDragColumn = NULL_COLUMN;
	}
	
	BOOL DropColumn( CPoint point )
	{
		if ( !m_bShowHeader )
			return FALSE;
			
		m_nHotDivider = NULL_COLUMN;
		m_nHotColumn = NULL_COLUMN;
		UINT nHeaderFlags = HITTEST_FLAG_NONE;
		
		// are we over the header?
		if ( HitTestHeader( point, m_nHotColumn, nHeaderFlags ) )
		{
			CRect rcColumn;
			if ( !GetColumnRect( m_nHotColumn, rcColumn ) )
				return FALSE;
			m_nHotDivider = point.x < ( ( rcColumn.left + rcColumn.right ) / 2 ) ? m_nHotColumn : m_nHotColumn + 1;
			
			if ( m_nHotDivider == m_nDragColumn || m_nHotDivider == m_nDragColumn + 1 )
				m_nHotDivider = NULL_COLUMN;
		}
		
		if ( m_nHotDivider != NULL_COLUMN )
		{
			CRect rcHeader;
			GetClientRect( rcHeader );
			rcHeader.bottom = m_nHeaderHeight;
		
			CPoint ptDivider( 0, rcHeader.Height() / 2 );
			
			CRect rcColumn;
			int nColumnCount = GetColumnCount();
			
			// set closest divider position
			if ( GetColumnRect( m_nHotDivider < nColumnCount ? m_nHotDivider : nColumnCount - 1, rcColumn ) )
				ptDivider.x = m_nHotDivider < nColumnCount ? rcColumn.left : rcColumn.right;
			
			ClientToScreen( &ptDivider );
			
			// track drop window
			m_wndDropArrows.Show( ptDivider );
			return TRUE;
		}
		
		m_wndDropArrows.Hide();
		
		return FALSE;
	}
	
	BOOL SortColumn( int nColumn )
	{
		T* pT = static_cast<T*>(this);
		
		if ( !m_bShowHeader || !m_bShowSort )
			return FALSE;
			
		BOOL bReverseSort = FALSE;
		int nSortIndex = GetColumnIndex( nColumn );
		
		CWaitCursor curWait;
		
		if ( nSortIndex != m_nSortColumn )
		{
			// sort by new column
			m_bSortAscending = TRUE;
			m_nSortColumn = nSortIndex;
			pT->SortItems( m_nSortColumn, m_bSortAscending );
		}
		else
		{
			// toggle sort order if sorting same column
			m_bSortAscending = !m_bSortAscending;
			pT->ReverseItems();
		}
			
		return ResetSelected();
	}
	
	BOOL GetSortColumn( int& nColumn, BOOL& bAscending )
	{
		if ( !m_bShowHeader || !m_bShowSort || m_nSortColumn == NULL_COLUMN )
			return FALSE;
		nColumn = m_nSortColumn;
		bAscending = m_bSortAscending;
		return TRUE;
	}
	
	BOOL DragItem()
	{
		ATLASSERT( FALSE ); // must be implemented in a derived class
		return FALSE;
	}
	
	BOOL GroupSelect( CPoint point )
	{
		HideTitleTip();
		
		int nHorzScroll = GetScrollPos( SB_HORZ );
		int nVertScroll = GetScrollPos( SB_VERT );
		
		m_rcGroupSelect.left = min( m_ptSelectPoint.x, point.x + nHorzScroll );
		m_rcGroupSelect.right = max( m_ptSelectPoint.x, point.x + nHorzScroll );
		m_rcGroupSelect.top = min( m_ptSelectPoint.y, point.y + nVertScroll );
		m_rcGroupSelect.bottom = max( m_ptSelectPoint.y, point.y + nVertScroll );
		
		if ( m_rcGroupSelect.IsRectEmpty() )
			return FALSE;
		
		// select items in group
		AutoSelect( point );
		
		// start auto scroll timer
		SetTimer( ITEM_AUTOSCROLL_TIMER, ITEM_SCROLL_PERIOD );
		
		DWORD dwCurrentTick = GetTickCount();
		
		// timer messages are a low priority, therefore we need to simulate the timer when moving the mouse
		if ( ( dwCurrentTick - m_dwScrollTick ) > ITEM_SCROLL_PERIOD - 10 )
		{
			if ( AutoScroll( point ) )
				m_dwScrollTick = dwCurrentTick;
		}
		
		// redraw list immediately
		return RedrawWindow();
	}
	
	void AutoSelect( CPoint point )
	{
		m_setSelectedItems.clear();
			
		if ( m_rcGroupSelect.left < GetTotalWidth() )
		{
			int nHorzScroll = GetScrollPos( SB_HORZ );
			int nVertScroll = GetScrollPos( SB_VERT );
			
			CRect rcGroupSelect( m_rcGroupSelect );
			rcGroupSelect.OffsetRect( -nHorzScroll, -nVertScroll );
		
			int nTopItem = GetTopItem();
			int nLastItem = nTopItem + ( ( rcGroupSelect.bottom - ( m_bShowHeader ? m_nHeaderHeight : 0 ) ) / m_nItemHeight );
			nTopItem += ( ( rcGroupSelect.top - ( m_bShowHeader ? m_nHeaderHeight : 0 ) ) / m_nItemHeight ) - ( ( rcGroupSelect.top < 0 ) ? 1 : 0 );
			
			for ( int nItem = nTopItem; nItem <= nLastItem; nItem++ )
			{
				if ( m_setSelectedItems.empty() )
					m_nFirstSelected = nItem;
				m_setSelectedItems.insert( nItem );
				
				m_nFocusItem = nItem;
				m_nFocusSubItem = NULL_SUBITEM;
			}
		}

		if ( m_setSelectedItems.empty() )
			m_nFirstSelected = NULL_ITEM;
	}
	
	BOOL AutoScroll( CPoint point )
	{
		CRect rcClient;
		GetClientRect( rcClient );
		rcClient.top = ( m_bShowHeader ? m_nHeaderHeight : 0 );
		
		int nHorzScroll = GetScrollPos( SB_HORZ );
		int nVertScroll = GetScrollPos( SB_VERT );

		BOOL bAutoScroll = FALSE;
		
		if ( point.y < rcClient.top )
		{
			SendMessage( WM_VSCROLL, MAKEWPARAM( SB_LINEUP, 0 ) );
			int nAutoScroll = GetScrollPos( SB_VERT );
			if ( nVertScroll != nAutoScroll )
			{
				m_rcGroupSelect.top = rcClient.top + nAutoScroll - 1;
				m_rcGroupSelect.bottom = max( m_ptSelectPoint.y, point.y + nAutoScroll - 1 );
				bAutoScroll = TRUE;
			}
		}
		if ( point.y > rcClient.bottom )
		{
			SendMessage( WM_VSCROLL, MAKEWPARAM( SB_LINEDOWN, 0 ) );
			int nAutoScroll = GetScrollPos( SB_VERT );
			if ( nVertScroll != nAutoScroll )
			{
				m_rcGroupSelect.top = min( m_ptSelectPoint.y, point.y + nAutoScroll + 1 );
				m_rcGroupSelect.bottom = rcClient.bottom + nAutoScroll + 1;
				bAutoScroll = TRUE;
			}
		}
		if ( point.x < rcClient.left )
		{
			SendMessage( WM_HSCROLL, MAKEWPARAM( SB_LINELEFT, 0 ) );
			int nAutoScroll = GetScrollPos( SB_HORZ );
			if ( nHorzScroll != nAutoScroll )
			{
				m_rcGroupSelect.left = rcClient.left + nAutoScroll - 1;
				m_rcGroupSelect.right = max( m_ptSelectPoint.x, point.x + nAutoScroll - 1 );
				bAutoScroll = TRUE;
			}
		}
		if ( point.x > rcClient.right )
		{
			SendMessage( WM_HSCROLL, MAKEWPARAM( SB_LINERIGHT, 0 ) );
			int nAutoScroll = GetScrollPos( SB_HORZ );
			if ( nHorzScroll != nAutoScroll )
			{
				m_rcGroupSelect.left = min( m_ptSelectPoint.x, point.x + nAutoScroll + 1 );
				m_rcGroupSelect.right = rcClient.right + nAutoScroll + 1;
				bAutoScroll = TRUE;
			}
		}
		
		// was scrolling performed?
		return bAutoScroll;
	}
	
	BOOL BeginScroll( int nBeginItem, int nEndItem )
	{
		T* pT = static_cast<T*>(this);
		
		// any scroll required?
		if ( nBeginItem == nEndItem )
			return FALSE;
		
		// calculate scroll offset
		m_nScrollOffset = abs( nEndItem - nBeginItem ) * m_nItemHeight;
		m_nScrollUnit = min( max( m_nScrollOffset / m_nItemHeight, ITEM_SCROLL_UNIT_MIN ), ITEM_SCROLL_UNIT_MAX );
		m_nScrollDelta = ( m_nScrollOffset - m_nScrollUnit ) / m_nScrollUnit;
		m_bScrollDown = ( nBeginItem < nEndItem );
		
		CClientDC dcClient( m_hWnd );
		
		CDC dcScrollList;
		dcScrollList.CreateCompatibleDC( dcClient );
		
		int nContextState = dcScrollList.SaveDC();
		
		CRect rcScrollList;
		GetClientRect( rcScrollList );
		rcScrollList.bottom = ( GetCountPerPage() + abs( nEndItem - nBeginItem ) ) * m_nItemHeight;
		
		if ( !m_bmpScrollList.IsNull() )
			m_bmpScrollList.DeleteObject();
		m_bmpScrollList.CreateCompatibleBitmap( dcClient, rcScrollList.Width(), rcScrollList.Height() ); 
		dcScrollList.SelectBitmap( m_bmpScrollList );
		
		pT->DrawBkgnd( dcScrollList.m_hDC );
		
		CRect rcItem;
		rcItem.left = -GetScrollPos( SB_HORZ );
		rcItem.right = GetTotalWidth();
		rcItem.top = 0;
		rcItem.bottom = rcItem.top;
		
		// draw all visible items into bitmap
		for ( int nItem = min( nBeginItem, nEndItem ); nItem < pT->GetItemCount(); rcItem.top = rcItem.bottom, nItem++ )
		{
			rcItem.bottom = rcItem.top + m_nItemHeight;
			
			if ( rcItem.top > rcScrollList.bottom )
				break;
			
			// may be implemented in a derived class
			pT->DrawItem( dcScrollList.m_hDC, nItem, rcItem );
		}
		
		dcScrollList.RestoreDC( nContextState );
		
		ScrollList();
		
		// start scrolling timer
		SetTimer( ITEM_SCROLL_TIMER, ITEM_SCROLL_PERIOD );
		
		return TRUE;
	}
	
	BOOL EndScroll()
	{
		KillTimer( ITEM_SCROLL_TIMER );
		if ( !m_bmpScrollList.IsNull() )
			m_bmpScrollList.DeleteObject();
		return Invalidate();
	}
	
	BOOL ScrollList()
	{
		if ( m_nScrollOffset <= m_nScrollUnit )
			m_nScrollOffset--;
		else
		{
			m_nScrollOffset -= m_nScrollDelta;
			if ( m_nScrollOffset < m_nScrollDelta )
				m_nScrollOffset = m_nScrollUnit;
		}
		
		if ( m_bmpScrollList.IsNull() || m_nScrollOffset < 0 )
			return FALSE;
		
		CClientDC dcClient( m_hWnd );
	
		CDC dcScrollList;
		dcScrollList.CreateCompatibleDC( dcClient );
		
		CRect rcClient;
		GetClientRect( rcClient );
		rcClient.top = ( m_bShowHeader ? m_nHeaderHeight : 0 );
		
		HBITMAP hOldBitmap = dcScrollList.SelectBitmap( m_bmpScrollList );
		
		CSize sizScrollBitmap;
		m_bmpScrollList.GetSize( sizScrollBitmap );
		
		// draw scrolled list
		dcClient.BitBlt( 0, rcClient.top, rcClient.Width(), rcClient.Height(), dcScrollList, 0, m_bScrollDown ? ( sizScrollBitmap.cy - ( GetCountPerPage() * m_nItemHeight ) - m_nScrollOffset ) : m_nScrollOffset, SRCCOPY );

		dcScrollList.SelectBitmap( hOldBitmap );
		
		return TRUE;
	}
	
	BOOL EditItem( int nItem, int nSubItem = NULL_SUBITEM )
	{
		T* pT = static_cast<T*>(this);
		
		if ( !EnsureItemVisible( nItem, nSubItem ) )
			return FALSE;
		
		if ( GetFocus() != m_hWnd )
			return FALSE;
		
		CRect rcSubItem;
		if ( !GetItemRect( nItem, nSubItem, rcSubItem ) )
			return FALSE;
		
		int nIndex = GetColumnIndex( nSubItem );
		if ( pT->GetItemFlags( nItem, nIndex ) & ITEM_FLAGS_READ_ONLY )
			return TRUE;
		
		switch ( pT->GetItemFormat( nItem, nIndex ) )
		{
			case ITEM_FORMAT_EDIT:				m_bEditItem = TRUE;
												if ( !RedrawWindow() )
													return FALSE;
												if ( !m_wndItemEdit.Create( m_hWnd, nItem, nSubItem, rcSubItem, pT->GetItemFlags( nItem, nIndex ), pT->GetItemText( nItem, nIndex ) ) )
													return FALSE;
												break;
			case ITEM_FORMAT_DATETIME:			{
													m_bEditItem = TRUE;
													if ( !RedrawWindow() )
														return FALSE;
													SYSTEMTIME stItemDate;
													GetItemDate( nItem, nIndex, stItemDate );
													if ( !m_wndItemDate.Create( m_hWnd, nItem, nSubItem, rcSubItem, pT->GetItemFlags( nItem, nIndex ), stItemDate ) )
														return FALSE;
												}
												break;
			case ITEM_FORMAT_COMBO:				{
													m_bEditItem = TRUE;
													if ( !RedrawWindow() )
														return FALSE;
													CListArray < CString > aComboList;
													if ( !pT->GetItemComboList( nItem, nIndex, aComboList ) )
														return FALSE;
													if ( !m_wndItemCombo.Create( m_hWnd, nItem, nSubItem, rcSubItem, pT->GetItemFlags( nItem, nIndex ), pT->GetItemText( nItem, nIndex ), m_bShowThemed, aComboList ) )
														return FALSE;
												}
												break;
		}
		return TRUE;
	}
	
	CString FormatDate( SYSTEMTIME& stFormatDate )
	{
		if ( stFormatDate.wYear == 0 )
			return _T( "" );
		
		// format date to local format
		TCHAR szDateFormat[ DATE_STRING ];
		return GetDateFormat( LOCALE_USER_DEFAULT, DATE_SHORTDATE, &stFormatDate, NULL, szDateFormat, DATE_STRING ) == 0 ? _T( "" ) : szDateFormat;
	}
	
	CString FormatTime( SYSTEMTIME& stFormatDate )
	{
		SYSTEMTIME stFormatTime = stFormatDate;
		stFormatTime.wYear = 0;
		stFormatTime.wMonth = 0;
		stFormatTime.wDay = 0;
		
		// format time to local format
		TCHAR szTimeFormat[ DATE_STRING ];
		return GetTimeFormat( LOCALE_USER_DEFAULT, 0, &stFormatTime, NULL, szTimeFormat, DATE_STRING ) == 0 ? _T( "" ) : szTimeFormat;
	}
	
	void NotifyParent( int nItem, int nSubItem, int nMessage )
	{
		T* pT = static_cast<T*>(this);
		
		CListNotify listNotify;
		listNotify.m_hdrNotify.hwndFrom = pT->m_hWnd;
		listNotify.m_hdrNotify.idFrom = pT->GetDlgCtrlID();
		listNotify.m_hdrNotify.code = nMessage;
		listNotify.m_nItem = nItem;
		listNotify.m_nSubItem = GetColumnIndex( nSubItem );
		listNotify.m_nExitChar = 0;
		listNotify.m_lpszItemText = NULL;
		listNotify.m_lpItemDate = NULL;

		// forward notification to parent
		FORWARD_WM_NOTIFY( pT->GetParent(), listNotify.m_hdrNotify.idFrom, &listNotify.m_hdrNotify, ::SendMessage );
	}
	
	BOOL ShowTitleTip( CPoint point, int nItem, int nSubItem )
	{
		T* pT = static_cast<T*>(this);
		
		// do not show titletip if editing
		if ( m_bEditItem )
			return FALSE;
		
		// is titletip already shown for this item?
		if ( nItem == m_nTitleTipItem && nSubItem == m_nTitleTipSubItem )
			return FALSE;
		
		CRect rcSubItem;
		if ( !GetItemRect( nItem, nSubItem, rcSubItem ) )
		{
			HideTitleTip();
			return FALSE;
		}
		
		int nIndex = GetColumnIndex( nSubItem );
		CRect rcItemText( rcSubItem );
				
		// margin item text
		rcItemText.left += nSubItem == 0 ? 4 : 3;
		rcItemText.DeflateRect( 4, 0 );
		
		// offset item text (for image)
		if ( !m_ilItemImages.IsNull() && pT->GetItemImage( nItem, nIndex ) != ITEM_IMAGE_NONE )
		{
			CSize sizeIcon;
			m_ilItemImages.GetIconSize( sizeIcon );
			rcItemText.left += sizeIcon.cx + 4;
		}
				
		// is current cursor position over item text (not over an icon)?
		if ( !rcItemText.PtInRect( point ) )
			return FALSE;
		
		CString strItemText;
		
		switch ( pT->GetItemFormat( nItem, nIndex ) )
		{
			case ITEM_FORMAT_CHECKBOX:
			case ITEM_FORMAT_CHECKBOX_3STATE:	
			case ITEM_FORMAT_PROGRESS:			break; // no titletip for checkboxes or progress
			case ITEM_FORMAT_DATETIME:			{
													SYSTEMTIME stItemDate;
													if ( !GetItemDate( nItem, nIndex, stItemDate ) )
														break;
													
													UINT nItemFlags = pT->GetItemFlags( nItem, nIndex );
													if ( nItemFlags & ITEM_FLAGS_DATE_ONLY )
														strItemText = FormatDate( stItemDate );
													else if ( nItemFlags & ITEM_FLAGS_TIME_ONLY )
														strItemText = FormatTime( stItemDate );
													else
														strItemText = FormatDate( stItemDate ) + _T( " " ) + FormatTime( stItemDate );
												}
												break;
			default:							strItemText = pT->GetItemText( nItem, nIndex );
												break;
		}

		if ( strItemText.IsEmpty() )
		{
			HideTitleTip();
			return FALSE;
		}
		
		ClientToScreen( rcItemText );
		if ( !m_wndTitleTip.Show( rcItemText, strItemText, pT->GetItemToolTip( nItem, nIndex ) ) )
		{
			HideTitleTip();
			return FALSE;
		}
		
		m_nTitleTipItem = nItem;
		m_nTitleTipSubItem = nSubItem;
						
		return TRUE;
	}
	
	BOOL HideTitleTip( BOOL bResetItem = TRUE )
	{
		if ( bResetItem )
		{
			m_nTitleTipItem = NULL_ITEM;
			m_nTitleTipSubItem = NULL_SUBITEM;
		}
		return m_wndTitleTip.Hide();
	}
	
	BEGIN_MSG_MAP_EX(CListImpl)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_SETFOCUS(OnSetFocus)
		MSG_WM_KILLFOCUS(OnKillFocus)
		MSG_WM_GETDLGCODE(OnGetDlgCode)
		MSG_WM_SIZE(OnSize)
		MSG_WM_HSCROLL(OnHScroll)
		MSG_WM_VSCROLL(OnVScroll)
		MSG_WM_CANCELMODE(OnCancelMode)
		MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST,WM_MOUSELAST,OnMouseRange)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
		MSG_WM_RBUTTONDOWN(OnRButtonDown)
		MSG_WM_RBUTTONUP(OnRButtonUp)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_MOUSELEAVE(OnMouseLeave)
		MSG_WM_MOUSEWHEEL(OnMouseWheel)
		//MSG_WM_TIMER(OnTimer)
		MSG_WM_KEYDOWN(OnKeyDown)
		MSG_WM_SYSKEYDOWN(OnSysKeyDown)
		MSG_WM_SETTINGCHANGE(OnSettingsChange)
		MSG_WM_SYSCOLORCHANGE(OnSettingsChange)
		MSG_WM_FONTCHANGE(OnSettingsChange)
		MSG_WM_THEMECHANGED(OnSettingsChange)
		NOTIFY_CODE_HANDLER_EX(LCN_ENDEDIT,OnEndEdit)
		MESSAGE_HANDLER(WM_CTLCOLORLISTBOX, OnCtlColorListBox)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, OnCtlColorListBox)
		CHAIN_MSG_MAP(CDoubleBufferImpl< CListImpl >)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP_EX()
	
	int OnCreate( LPCREATESTRUCT lpCreateStruct )
	{
		T* pT = static_cast<T*>(this);
		return pT->Initialise() ? 0 : -1;
	}
	
	void OnDestroy()
	{
		m_oleDragDrop.Revoke();
		
		if ( m_wndDropArrows.IsWindow() )
			m_wndDropArrows.DestroyWindow();
		
		if ( m_wndTitleTip.IsWindow() )
			m_wndTitleTip.DestroyWindow();
		
		if ( m_wndItemEdit.IsWindow() )
			m_wndItemEdit.DestroyWindow();
		
		if ( m_wndItemCombo.IsWindow() )
			m_wndItemCombo.DestroyWindow();
		
		if ( m_wndItemDate.IsWindow() )
			m_wndItemDate.DestroyWindow();
		
		if ( m_ttToolTip.IsWindow() )
			m_ttToolTip.DestroyWindow();
	}
	
	void OnSetFocus( HWND hOldWnd )
	{
		Invalidate();
	}
	
	void OnKillFocus( HWND hNewWnd )
	{
		Invalidate();
	}
	
	UINT OnGetDlgCode( LPMSG lpMessage )
	{
		return DLGC_WANTARROWS | DLGC_WANTTAB | DLGC_WANTCHARS;
	}
	
	void OnSize( UINT nType, CSize size )
	{
		// stop any pending scroll
		EndScroll();
		
		// end any pending edit
		if ( m_bEditItem )
			SetFocus();
			
		ResetScrollBars( SB_BOTH, -1, FALSE );
		Invalidate();
	}
	
	void OnHScroll( int nSBCode, short nPos, HWND hScrollBar )
	{
		// stop any pending scroll
		EndScroll();
		
		// end any pending edit
		if ( m_bEditItem )
			SetFocus();
		
		HideTitleTip();
			
		CRect rcClient;
		GetClientRect( rcClient );
		
		int nScrollPos = GetScrollPos( SB_HORZ );

		switch ( nSBCode )
		{
			case SB_LEFT:			nScrollPos = 0;
									break;
			case SB_LINELEFT:		nScrollPos = max( nScrollPos - ITEM_SCROLL_OFFSET, 0 );
									break;
			case SB_PAGELEFT:		nScrollPos = max( nScrollPos - rcClient.Width(), 0 );
									break;
			case SB_RIGHT:			nScrollPos = rcClient.Width();
									break;
			case SB_LINERIGHT:		nScrollPos = min( nScrollPos + ITEM_SCROLL_OFFSET, GetTotalWidth() );
									break;
			case SB_PAGERIGHT:		nScrollPos = min( nScrollPos + rcClient.Width(), GetTotalWidth() );
									break;
			case SB_THUMBPOSITION:
			case SB_THUMBTRACK:		{
										SCROLLINFO infoScroll;
										ZeroMemory( &infoScroll, sizeof( SCROLLINFO ) );
										infoScroll.cbSize = sizeof( SCROLLINFO );
										infoScroll.fMask = SIF_TRACKPOS;
										
										// get 32-bit scroll position
										if ( !GetScrollInfo( SB_HORZ, &infoScroll ) )
											return;
										
										// has scroll position changed?
										if ( infoScroll.nTrackPos == nScrollPos )
											return;
										
										nScrollPos = infoScroll.nTrackPos;
									}
									break;
			default:				return;
		}

		ResetScrollBars( SB_HORZ, nScrollPos, FALSE );	
		Invalidate();
	}

	void OnVScroll( int nSBCode, short nPos, HWND hScrollBar )
	{
		T* pT = static_cast<T*>(this);
		
		// end any pending edit
		if ( m_bEditItem )
			SetFocus();
		
		HideTitleTip();

		CRect rcClient;
		GetClientRect( rcClient );
		rcClient.top = ( m_bShowHeader ? m_nHeaderHeight : 0 );
		
		int nScrollPos = GetScrollPos( SB_VERT );
		BOOL bScrollList = m_bSmoothScroll;
		
		switch ( nSBCode )
		{
			case SB_TOP:			nScrollPos = 0;
									bScrollList = FALSE;
									break;
			case SB_LINEUP:			nScrollPos = max( nScrollPos - m_nItemHeight, 0 );
									break;
			case SB_PAGEUP:			nScrollPos = max( nScrollPos - rcClient.Height(), 0 );
									break;
			case SB_BOTTOM:			nScrollPos = pT->GetItemCount() * m_nItemHeight;
									bScrollList = FALSE;
									break;
			case SB_LINEDOWN:		nScrollPos += m_nItemHeight;
									break;
			case SB_PAGEDOWN:		nScrollPos += rcClient.Height();
									break;
			case SB_THUMBTRACK:
			case SB_THUMBPOSITION:	{
										SCROLLINFO infoScroll;
										ZeroMemory( &infoScroll, sizeof( SCROLLINFO ) );
										infoScroll.cbSize = sizeof( SCROLLINFO );
										infoScroll.fMask = SIF_TRACKPOS;
										
										// get 32-bit scroll position
										if ( !GetScrollInfo( SB_VERT, &infoScroll ) )
											return;
										
										// has scroll position changed?
										if ( infoScroll.nTrackPos == nScrollPos )
											return;
										
										nScrollPos = infoScroll.nTrackPos;
										bScrollList = FALSE;
									}
									break;
			case SB_ENDSCROLL:		m_bScrolling = FALSE;
			default:				return;
		}
		
		// store original top item before scrolling
		int nTopItem = GetTopItem();
		ResetScrollBars( SB_VERT, nScrollPos, FALSE );
		
		if ( bScrollList && !m_bScrolling )
			m_bScrolling = BeginScroll( nTopItem, GetTopItem() );
		else
			EndScroll();
	}
	
	void OnCancelMode() 
	{
		if ( m_bButtonDown )
			ReleaseCapture();
	
		HideTitleTip();
		m_wndDropArrows.Hide();
		m_nDragColumn = NULL_COLUMN;
		m_nHighlightColumn = NULL_COLUMN;
	}
	
	LRESULT OnMouseRange( UINT nMessage, WPARAM wParam, LPARAM lParam )
	{
		if ( m_ttToolTip.IsWindow() )
		{
			MSG msgRelay = { m_hWnd, nMessage, wParam, lParam };
			m_ttToolTip.RelayEvent( &msgRelay );
		}
		SetMsgHandled( FALSE );
		return 0;
	}
	
	void OnLButtonDown( UINT nFlags, CPoint point )
	{
		T* pT = static_cast<T*>(this);
		
		HideTitleTip( FALSE );
		
		m_bButtonDown = TRUE;
		m_ptDownPoint = point;
		m_ptSelectPoint = CPoint( point.x + GetScrollPos( SB_HORZ ), point.y + GetScrollPos( SB_VERT ) );

		// stop any pending scroll
		EndScroll();
		
		SetFocus();
		
		// capture all mouse input
		SetCapture();
		
		int nColumn = NULL_COLUMN;
		UINT nHeaderFlags = HITTEST_FLAG_NONE;
		
		// are we over the header?
		if ( HitTestHeader( point, nColumn, nHeaderFlags ) )
		{
			CListColumn listColumn;
			if ( !GetColumn( nColumn, listColumn ) )
				return;
				
			if ( !listColumn.m_bFixed && ( nHeaderFlags & HITTEST_FLAG_HEADER_DIVIDER ) )
			{
				SetCursor( m_curDivider );
			
				// begin column resizing
				m_bColumnSizing = TRUE;
				m_nColumnSizing = nColumn;
				m_nStartSize = listColumn.m_nWidth;
				m_nStartPos = GET_X_LPARAM( GetMessagePos() );
			}
			else
			{
				m_nHighlightColumn = nColumn;
				InvalidateHeader();
			}
			
			return;			
		}
		
		int nItem = NULL_ITEM;
		int nSubItem = NULL_SUBITEM;
		
		if ( !HitTest( point, nItem, nSubItem ) )
		{
			m_nFirstSelected = NULL_ITEM;
			m_bBeginSelect = TRUE;		
		}
		else
		{
			// do not begin group select from first columns
			if ( !( nFlags & MK_SHIFT ) && !( nFlags & MK_CONTROL ) && nSubItem != 0 )
			{
				m_bBeginSelect = TRUE;
				m_nFirstSelected = nItem;
			}
			
			// only select item if not already selected
			if ( ( nFlags & MK_SHIFT ) || ( nFlags & MK_CONTROL ) || !IsSelected( nItem ) || m_setSelectedItems.size() <= 1 )
				SelectItem( nItem, nSubItem, nFlags );
			
			int nIndex = GetColumnIndex( nSubItem );
			if ( !( pT->GetItemFlags( nItem, nIndex ) & ITEM_FLAGS_READ_ONLY ) )
			{
				switch ( pT->GetItemFormat( nItem, nIndex ) )
				{
					case ITEM_FORMAT_CHECKBOX:			m_bBeginSelect = FALSE;
														pT->SetItemText( nItem, nIndex, _ttoi( pT->GetItemText( nItem, nIndex ) ) > 0 ? _T( "0" ) : _T( "1" ) );
														NotifyParent( nItem, nSubItem, LCN_MODIFIED );
														InvalidateItem( nItem );
														break;
					case ITEM_FORMAT_CHECKBOX_3STATE:	{
															m_bBeginSelect = FALSE;
															
															int nCheckImage = _ttoi( pT->GetItemText( nItem, nIndex ) );
															if ( nCheckImage < 0 )
																pT->SetItemText( nItem, nIndex, _T( "0" ) );
															else if ( nCheckImage > 0 )
																pT->SetItemText( nItem, nIndex, _T( "-1" ) );
															else
																pT->SetItemText( nItem, nIndex, _T( "1" ) );
															
															NotifyParent( nItem, nSubItem, LCN_MODIFIED );
															InvalidateItem( nItem );
														}
														break;
					case ITEM_FORMAT_HYPERLINK:			m_bBeginSelect = FALSE;
														SetCursor( m_curHyperLink );
														NotifyParent( nItem, nSubItem, LCN_HYPERLINK );
														break;
				}
			}
		}
	}
	
	void OnLButtonUp( UINT nFlags, CPoint point )
	{
		if ( m_bButtonDown )
			ReleaseCapture();
		
		// finish resizing or selecting a column
		if ( m_bColumnSizing || m_nHighlightColumn != NULL_COLUMN )
		{
			// are we changing the sort order?
			if ( !m_bColumnSizing && m_nHighlightColumn != NULL_COLUMN )
				SortColumn( m_nHighlightColumn );
			
			m_bColumnSizing = FALSE;
			m_nColumnSizing = NULL_COLUMN;
			m_nHighlightColumn = NULL_COLUMN;
			m_nStartSize = 0;
			m_nStartPos = 0;

			InvalidateHeader();
		}
		
		m_bBeginSelect = FALSE;
		m_bButtonDown = FALSE;
		m_ptDownPoint = 0;
		m_ptSelectPoint = 0;
		
		// have we finished a group select?
		if ( m_bGroupSelect )
		{
			m_bGroupSelect = FALSE;
			Invalidate();
		}
		else
		{
			int nItem = NULL_ITEM;
			int nSubItem = NULL_SUBITEM;
			
			// de-select item if current item is selected
			if ( HitTest( point, nItem, nSubItem ) && IsSelected( nItem ) && m_setSelectedItems.size() > 1 && !( nFlags & MK_SHIFT ) && !( nFlags & MK_CONTROL ) )
				SelectItem( nItem, nSubItem, nFlags );
				
			// notify parent of left-click item
			NotifyParent( nItem, nSubItem, LCN_LEFTCLICK );
		}
	}
	
	void OnLButtonDblClk( UINT nFlags, CPoint point ) 
	{
		HideTitleTip( FALSE );
		
		// handle double-clicks (for drawing)
		SendMessage( WM_LBUTTONDOWN, 0, MAKELPARAM( point.x, point.y ) );

		int nColumn = NULL_COLUMN;
		UINT nHeaderFlags = HITTEST_FLAG_NONE;
		
		// resize column if double-click on a divider
		if ( HitTestHeader( point, nColumn, nHeaderFlags ) && ( nHeaderFlags & HITTEST_FLAG_HEADER_DIVIDER ) )
			AutoSizeColumn( nColumn );
		
		int nItem = NULL_ITEM;
		int nSubItem = NULL_SUBITEM;
		
		HitTest( point, nItem, nSubItem );
		
		// notify parent of double-clicked item
		NotifyParent( nItem, nSubItem, LCN_DBLCLICK );
	}
	
	void OnRButtonDown( UINT nFlags, CPoint point ) 
	{
		// stop any pending scroll
		EndScroll();
		
		SetFocus();
		
		HideTitleTip( FALSE );
		
		int nItem = NULL_ITEM;
		int nSubItem = NULL_SUBITEM;
		
		// only select item if not already selected (de-select in OnLButtonUp)
		if ( HitTest( point, nItem, nSubItem ) && !IsSelected( nItem ) )
			SelectItem( nItem, nSubItem, nFlags );
	}

	void OnRButtonUp( UINT nFlags, CPoint point )
	{
		int nItem = NULL_ITEM;
		int nSubItem = NULL_SUBITEM;
		
		if ( !HitTest( point, nItem, nSubItem ) )
			ResetSelected();
		
		// notify parent of right-click item
		NotifyParent( nItem, nSubItem, LCN_RIGHTCLICK );
	}
	
	void OnMouseMove( UINT nFlags, CPoint point )
	{
		T* pT = static_cast<T*>(this);
		
		if ( !( nFlags & MK_LBUTTON ) )
		{
			if ( m_bButtonDown )
				ReleaseCapture();
			
			m_bButtonDown = FALSE;
		}
		
		if ( !m_bMouseOver )
		{
			m_bMouseOver = TRUE;
			
			TRACKMOUSEEVENT trkMouse;
			trkMouse.cbSize = sizeof( TRACKMOUSEEVENT );
			trkMouse.dwFlags = TME_LEAVE;
			trkMouse.hwndTrack = m_hWnd;
			
			// notify when the mouse leaves button
			_TrackMouseEvent( &trkMouse );
		}
		
		if ( m_bButtonDown )
		{
			// are we resizing a column?
			if ( m_bColumnSizing )
			{
				ResizeColumn();
				return;
			}
			
			// are we beginning to drag a column? 
			if ( m_nHighlightColumn != NULL_COLUMN && ( point.x < m_ptDownPoint.x - DRAG_HEADER_OFFSET || point.x > m_ptDownPoint.x + DRAG_HEADER_OFFSET || point.y < m_ptDownPoint.y - DRAG_HEADER_OFFSET || point.y > m_ptDownPoint.y + DRAG_HEADER_OFFSET ) )
			{
				DragColumn();
				return;
			}
			
			// are we beginning a group select or dragging an item?
			if ( point.x < m_ptDownPoint.x - DRAG_ITEM_OFFSET || point.x > m_ptDownPoint.x + DRAG_ITEM_OFFSET || point.y < m_ptDownPoint.y - DRAG_ITEM_OFFSET || point.y > m_ptDownPoint.y + DRAG_ITEM_OFFSET )
			{
				if ( m_bBeginSelect || !m_bDragDrop )
					m_bGroupSelect = ( !m_bSingleSelect && !m_bEditItem );
				else
				{
					int nItem = NULL_ITEM;
					int nSubItem = NULL_SUBITEM;
					
					if ( HitTest( point, nItem, nSubItem ) )
					{
						// select the drag item (if not already selected)
						if ( !IsSelected( nItem ) )
							SelectItem( nItem, nSubItem, nFlags );
						
						// begin drag item operation
						pT->DragItem();
					}
				}
			}							
			
			if ( m_bGroupSelect )
			{
				GroupSelect( point );
				return;
			}
		}
		else
		{
			int nColumn = NULL_COLUMN;
			UINT nHeaderFlags = HITTEST_FLAG_NONE;
			
			// are we over the header?
			BOOL bHitTestHeader = HitTestHeader( point, nColumn, nHeaderFlags );
			
			if ( m_bShowThemed && !m_thmHeader.IsThemeNull() )
			{
				if ( bHitTestHeader && m_nHotColumn != nColumn )
				{
					m_nHotColumn = nColumn;
					InvalidateHeader();
				}
				else if ( !bHitTestHeader && m_nHotColumn != NULL_COLUMN )
				{
					m_nHotColumn = NULL_COLUMN;
					InvalidateHeader();
				}
			}
			
			if ( bHitTestHeader )
			{
				HideTitleTip();
				CListColumn listColumn;
				if ( GetColumn( nColumn, listColumn ) && !listColumn.m_bFixed && ( nHeaderFlags & HITTEST_FLAG_HEADER_DIVIDER ) )
					SetCursor( m_curDivider );
				return;
			}
			
			int nItem = NULL_ITEM;
			int nSubItem = NULL_SUBITEM;
			
			if ( !HitTest( point, nItem, nSubItem ) )
			{
				if ( m_nHotItem != NULL_ITEM && m_nHotSubItem != NULL_SUBITEM )
				{
					// redraw old hot item
					int nIndex = GetColumnIndex( m_nHotSubItem );
					if ( pT->GetItemFormat( m_nHotItem, nIndex ) == ITEM_FORMAT_HYPERLINK && !( pT->GetItemFlags( m_nHotItem, nIndex ) & ITEM_FLAGS_READ_ONLY ) )
						InvalidateItem( m_nHotItem, m_nHotSubItem );
				}
				
				m_ttToolTip.Activate( FALSE );
				m_ttToolTip.DelTool( m_hWnd, TOOLTIP_TOOL_ID );
					
				m_nHotItem = NULL_ITEM;
				m_nHotSubItem = NULL_SUBITEM;
				HideTitleTip();
			}
			else
			{
				// has the hot item changed?
				if ( nItem != m_nHotItem || nSubItem != m_nHotSubItem )
				{
					// redraw old hot item
					int nIndex = GetColumnIndex( m_nHotSubItem );
					if ( pT->GetItemFormat( m_nHotItem, nIndex ) == ITEM_FORMAT_HYPERLINK && !( pT->GetItemFlags( m_nHotItem, nIndex ) & ITEM_FLAGS_READ_ONLY ) )
						InvalidateItem( m_nHotItem, m_nHotSubItem );
					
					m_nHotItem = nItem;
					m_nHotSubItem = nSubItem;
				}
				
				int nIndex = GetColumnIndex( m_nHotSubItem );
				UINT nItemFormat = pT->GetItemFormat( m_nHotItem, nIndex );
				UINT nItemFlags = pT->GetItemFlags( m_nHotItem, nIndex );
				
				// draw new hot hyperlink item
				if ( nItemFormat == ITEM_FORMAT_HYPERLINK && !( nItemFlags & ITEM_FLAGS_READ_ONLY ) )
				{
					InvalidateItem( m_nHotItem, m_nHotSubItem );
					SetCursor( m_curHyperLink );
				}
				
				// get tooltip for this item
				CString strToolTip = pT->GetItemToolTip( m_nHotItem, nIndex );
				
				CRect rcSubItem;
				if ( !strToolTip.IsEmpty() && GetItemRect( m_nHotItem, rcSubItem ) )
				{
					m_ttToolTip.Activate( TRUE );
					m_ttToolTip.AddTool( m_hWnd, (LPCTSTR)strToolTip.Left( SHRT_MAX ), rcSubItem, TOOLTIP_TOOL_ID );
				}
				else
				{
					m_ttToolTip.Activate( FALSE );
					m_ttToolTip.DelTool( m_hWnd, TOOLTIP_TOOL_ID );
				}
				
				// show titletips for this item
				ShowTitleTip( point, m_nHotItem, m_nHotSubItem );
			}
		}
	}
	
	void OnMouseLeave()
	{
		m_bMouseOver = FALSE;
		
		if ( m_nHotColumn != NULL_COLUMN )
		{
			m_nHotColumn = NULL_COLUMN;
			InvalidateHeader();
		}
		
		if ( m_nHotItem != NULL_ITEM || m_nHotSubItem != NULL_SUBITEM )
		{
			m_nHotItem = NULL_ITEM;
			m_nHotSubItem = NULL_SUBITEM;
			Invalidate();
		}
	}
	
	BOOL OnMouseWheel( UINT nFlags, short nDelta, CPoint point )
	{
		HideTitleTip();
		
		// end any pending edit
		if ( m_bEditItem )
			SetFocus();
		
		int nRowsScrolled = m_nMouseWheelScroll * nDelta / 120;
		int nScrollPos = GetScrollPos( SB_VERT );
		
		if ( nRowsScrolled > 0 )
			nScrollPos = max( nScrollPos - ( nRowsScrolled * m_nItemHeight ), 0 );
		else
			nScrollPos += ( -nRowsScrolled * m_nItemHeight );
		
		ResetScrollBars( SB_VERT, nScrollPos, FALSE );
		Invalidate();
		
		return TRUE;
	}
	
	void OnTimer( UINT nIDEvent, TIMERPROC timer )
	{
		switch ( nIDEvent )
		{
			case RESIZE_COLUMN_TIMER:	ResizeColumn( TRUE );
										break;
			case ITEM_VISIBLE_TIMER:	{
											KillTimer( ITEM_VISIBLE_TIMER );
											
											int nFocusItem = NULL_ITEM;
											int nFocusSubItem = NULL_SUBITEM;
											
											// get current focus item
											if ( !GetFocusItem( nFocusItem, nFocusSubItem ) )
												break;
											
											// make sure current focus item is visible before editing
											if ( !EditItem( nFocusItem, nFocusSubItem ) )
												break;
										}
										break;
			case ITEM_AUTOSCROLL_TIMER:	if ( !m_bGroupSelect )
											KillTimer( ITEM_AUTOSCROLL_TIMER );
										else
										{
											DWORD dwPoint = GetMessagePos();
											CPoint ptMouse( GET_X_LPARAM( dwPoint ), GET_Y_LPARAM( dwPoint ) );
											ScreenToClient( &ptMouse );
		
											// automatically scroll when group selecting
											AutoScroll( ptMouse );
											AutoSelect( ptMouse );
										}
										break;
			case ITEM_SCROLL_TIMER:		if ( !ScrollList() )
											EndScroll();
										break;
		}
	}
	
	void OnKeyDown( TCHAR nChar, UINT nRepCnt, UINT nFlags )
	{
		T* pT = static_cast<T*>(this);
		
		// stop any pending scroll
		EndScroll();
		
		BOOL bCtrlKey = ( ( GetKeyState( VK_CONTROL ) & 0x8000 ) != 0 );
		BOOL bShiftKey = ( ( GetKeyState( VK_SHIFT ) & 0x8000 ) != 0 );
		
		CRect rcClient;
		GetClientRect( rcClient );
		
		int nFocusItem = NULL_ITEM;
		int nFocusSubItem = NULL_SUBITEM;
		GetFocusItem( nFocusItem, nFocusSubItem );
		
		switch ( nChar )
		{
			case VK_DOWN:	SetFocusItem( min( nFocusItem + 1, pT->GetItemCount() - 1 ), nFocusSubItem );
							break;
			case VK_UP:		SetFocusItem( max( nFocusItem - 1, 0 ), nFocusSubItem );
							break;
			case VK_NEXT:	SetFocusItem( min( nFocusItem + GetCountPerPage( FALSE ) - 1, pT->GetItemCount() - 1 ), nFocusSubItem );
							break;
			case VK_PRIOR:	SetFocusItem( max( nFocusItem - GetCountPerPage( FALSE ) + 1, 0 ), nFocusSubItem );
							break;
			case VK_HOME:	SetFocusItem( 0, nFocusSubItem );
							break;
			case VK_END:	SetFocusItem( pT->GetItemCount() - 1, nFocusSubItem );
							break;
			case VK_LEFT:	if ( m_bFocusSubItem )
								SetFocusItem( nFocusItem, max( nFocusSubItem - 1, 0 ) );
							else
								SetScrollPos( SB_HORZ, max( GetScrollPos( SB_HORZ ) - ( bCtrlKey ? ITEM_SCROLL_OFFSET * 10 : ITEM_SCROLL_OFFSET ), 0 ) );
							break;
			case VK_RIGHT:	if ( m_bFocusSubItem )
								SetFocusItem( nFocusItem, min( nFocusSubItem + 1, GetColumnCount() - 1 ) );
							else
								SetScrollPos( SB_HORZ, min( GetScrollPos( SB_HORZ ) + ( bCtrlKey ? ITEM_SCROLL_OFFSET * 10 : ITEM_SCROLL_OFFSET ), rcClient.Width() ) );
							break;
			case VK_TAB:	if ( !bCtrlKey && m_bFocusSubItem )
								SetFocusItem( nFocusItem, bShiftKey ? max( nFocusSubItem - 1, 0 ) : min( nFocusSubItem + 1, GetColumnCount() - 1 ) );
							break;
			default:		if ( nChar == VK_SPACE )
							{
								int nIndex = GetColumnIndex( nFocusSubItem );
								if ( !( pT->GetItemFlags( nFocusItem, nIndex ) & ITEM_FLAGS_READ_ONLY ) )
								{
									switch ( pT->GetItemFormat( nFocusItem, nIndex ) )
									{
										case ITEM_FORMAT_CHECKBOX:			pT->SetItemText( nFocusItem, nIndex, _ttoi( pT->GetItemText( nFocusItem, nIndex ) ) > 0 ? _T( "0" ) : _T( "1" ) );
																			NotifyParent( nFocusItem, nFocusSubItem, LCN_MODIFIED );
																			InvalidateItem( nFocusItem );
																			return;
										case ITEM_FORMAT_CHECKBOX_3STATE:	{
																				int nCheckImage = _ttoi( pT->GetItemText( nFocusItem, nIndex ) );
																				if ( nCheckImage < 0 )
																					pT->SetItemText( nFocusItem, nIndex, _T( "0" ) );
																				else if ( nCheckImage > 0 )
																					pT->SetItemText( nFocusItem, nIndex, _T( "-1" ) );
																				else
																					pT->SetItemText( nFocusItem, nIndex, _T( "1" ) );
																				
																				NotifyParent( nFocusItem, nFocusSubItem, LCN_MODIFIED );
																				InvalidateItem( nFocusItem );
																			}
																			return;
									}
								}
							}
							
							if ( bCtrlKey && nChar == _T( 'A' ) && !m_bSingleSelect )
							{
								m_setSelectedItems.clear();
								for ( int nItem = 0; nItem < pT->GetItemCount(); nItem++ )
									m_setSelectedItems.insert( nItem );
								Invalidate();
								return;
							}
							
							if ( !bCtrlKey && iswprint( nChar ) && iswupper( nChar ) )
							{
								int nSortIndex = GetColumnIndex( m_nSortColumn );
								int nStartItem = nFocusItem + 1;
								DWORD dwCurrentTick = GetTickCount();
								
								CString strStart;
								strStart += nChar;
								
								// has there been another keypress since last search period?
								if ( ( dwCurrentTick - m_dwSearchTick ) < SEARCH_PERIOD )
								{
									if ( m_strSearchString.Left( 1 ) != strStart )
										m_strSearchString += nChar;
									
									CString strFocusText = pT->GetItemText( nFocusItem, nSortIndex );
									
									// are we continuing to type characters under current focus item?
									if ( m_strSearchString.GetLength() > 1 && m_strSearchString.CompareNoCase( strFocusText.Left( m_strSearchString.GetLength() ) ) == 0 )
									{
										m_dwSearchTick = GetTickCount();
										return;
									}
								}
								else
								{
									if ( m_strSearchString.Left( 1 ) != strStart )
										nStartItem = 0;
									m_strSearchString = strStart;
								}
								
								m_dwSearchTick = GetTickCount();
								
								// scan for next search string
								for ( int nFirst = nStartItem; nFirst < pT->GetItemCount(); nFirst++ )
								{
									CString strItemText = pT->GetItemText( nFirst, nSortIndex );
									
									if ( m_strSearchString.CompareNoCase( strItemText.Left( m_strSearchString.GetLength() ) ) == 0 )
									{
										SelectItem( nFirst, nFocusSubItem, TRUE );
										EnsureItemVisible( nFirst, nFocusSubItem );
										return;
									}
								}
								
								// re-scan from top if not found search string
								for ( int nSecond = 0; nSecond < pT->GetItemCount(); nSecond++ )
								{
									CString strItemText = pT->GetItemText( nSecond, nSortIndex );
									
									if ( m_strSearchString.CompareNoCase( strItemText.Left( m_strSearchString.GetLength() ) ) == 0 )
									{
										SelectItem( nSecond, nFocusSubItem, TRUE );
										EnsureItemVisible( nSecond, nFocusSubItem );
										return;
									}
								}
							}
							return;
		}

		if ( !bCtrlKey )
			SelectItem( m_nFocusItem, m_nFocusSubItem, bShiftKey ? MK_SHIFT : 0 );
	}
	
	void OnSysKeyDown( TCHAR nChar, UINT nRepCnt, UINT nFlags )
	{
		HideTitleTip( FALSE );
		SetMsgHandled( FALSE );		
	}
	
	void OnSettingsChange( UINT nFlags, LPCTSTR lpszSection )
	{
		OnSettingsChange();
	}
	
	void OnSettingsChange()
	{
		LoadSettings();		
		ResetScrollBars();
		Invalidate();
	}
	
	LRESULT OnCtlColorListBox( UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
	{
		return DefWindowProc( nMsg, wParam, lParam );
	}
	
	LRESULT OnEndEdit( LPNMHDR lpNMHDR )
	{
		T* pT = static_cast<T*>(this);
		CListNotify *pListNotify = reinterpret_cast<CListNotify *>( lpNMHDR );
		
		m_bEditItem = FALSE;
		int nIndex = GetColumnIndex( pListNotify->m_nSubItem );
		
		switch ( pListNotify->m_nExitChar )
		{
			case VK_ESCAPE:	break; // do nothing
			case VK_DELETE:	pT->SetItemText( pListNotify->m_nItem, nIndex, _T( "" ) );
							NotifyParent( pListNotify->m_nItem, pListNotify->m_nSubItem, LCN_MODIFIED );
							break;
			default:		if ( pListNotify->m_lpItemDate == NULL )
								pT->SetItemText( pListNotify->m_nItem, nIndex, pListNotify->m_lpszItemText );
							else
							{
								if ( _ttoi( pListNotify->m_lpszItemText ) == 0 )
									pT->SetItemText( pListNotify->m_nItem, nIndex, _T( "" ) );
								else
									pT->SetItemDate( pListNotify->m_nItem, nIndex, *pListNotify->m_lpItemDate );
							}
							if ( pListNotify->m_nExitChar == VK_TAB )
								PostMessage( WM_KEYDOWN, (WPARAM)VK_TAB );
							NotifyParent( pListNotify->m_nItem, pListNotify->m_nSubItem, LCN_MODIFIED );
							break;
		}
		
		InvalidateItem( pListNotify->m_nItem );
		
		return 0;	
	}
	
	DWORD OnDragEnter( FORMATETC& FormatEtc, STGMEDIUM& StgMedium, DWORD dwKeyState, CPoint point )
	{
		DWORD dwEffect = DROPEFFECT_NONE;
		
		if ( FormatEtc.cfFormat == m_nHeaderClipboardFormat )
		{
			LPBYTE lpDragHeader = (LPBYTE)GlobalLock( StgMedium.hGlobal );
			if ( lpDragHeader == NULL )
				return DROPEFFECT_NONE;
			
			// dragged column must originate from this control
			if ( *( (HWND*)lpDragHeader ) == m_hWnd )
				dwEffect = DropColumn( point ) ? DROPEFFECT_MOVE : DROPEFFECT_NONE;
			
			GlobalUnlock( StgMedium.hGlobal );
		}
		
		return dwEffect;
	}
	
	DWORD OnDragOver( FORMATETC& FormatEtc, STGMEDIUM& StgMedium, DWORD dwKeyState, CPoint point )
	{
		DWORD dwEffect = DROPEFFECT_NONE;
		
		if ( FormatEtc.cfFormat == m_nHeaderClipboardFormat )
		{
			LPBYTE lpDragHeader = (LPBYTE)GlobalLock( StgMedium.hGlobal );
			if ( lpDragHeader == NULL )
				return DROPEFFECT_NONE;
			
			// dragged column must originate from this control
			if ( *( (HWND*)lpDragHeader ) == m_hWnd )
				dwEffect = DropColumn( point ) ? DROPEFFECT_MOVE : DROPEFFECT_NONE;
			
			GlobalUnlock( StgMedium.hGlobal );
		}
		
		return dwEffect;
	}

	BOOL RemoveColumn(int i)
	{
		m_aColumns.RemoveAt( i );
		Invalidate();
		return TRUE;
	}

	BOOL RemoveAllColumns()
	{
		m_aColumns.RemoveAll();
		Invalidate();
		return TRUE;
	}
	
	BOOL OnDrop( FORMATETC& FormatEtc, STGMEDIUM& StgMedium, DWORD dwEffect, CPoint point )
	{
		if ( FormatEtc.cfFormat == m_nHeaderClipboardFormat )
		{
			if ( m_nDragColumn != NULL_COLUMN && m_nHotDivider != NULL_COLUMN )
			{
				CListColumn listColumn;
				if ( !GetColumn( m_nDragColumn, listColumn ) )
					return FALSE;
			
				// move column to new position
				m_aColumns.RemoveAt( m_nDragColumn );
				m_aColumns.InsertAt( ( m_nDragColumn < m_nHotColumn ? ( m_nHotDivider == 0 ? 0 : m_nHotDivider - 1 ) : m_nHotDivider ), listColumn );
				Invalidate();
			}

			return TRUE;
		}			
		
		// not supported
		return FALSE;
	}
	
	void OnDragLeave()
	{
	}
	
	BOOL OnRenderData( FORMATETC& FormatEtc, STGMEDIUM *pStgMedium, BOOL bDropComplete )
	{
		if ( FormatEtc.cfFormat == m_nHeaderClipboardFormat )
		{
			pStgMedium->tymed = TYMED_HGLOBAL;
			pStgMedium->hGlobal = GlobalAlloc( GMEM_MOVEABLE, sizeof( HWND ) );
			if ( pStgMedium->hGlobal == NULL )
				return FALSE;
			
			LPBYTE lpDragHeader = (LPBYTE)GlobalLock( pStgMedium->hGlobal );
			if ( lpDragHeader == NULL )
				return FALSE;
			
			// store this window handle
			*( (HWND*)lpDragHeader ) = m_hWnd;
			
			GlobalUnlock( pStgMedium->hGlobal );
		
			return TRUE;
		}
			
		return FALSE;
	}
	
	void DoPaint( CDCHandle dcPaint )
	{
		T* pT = static_cast<T*>(this);
		
		int nContextState = dcPaint.SaveDC();
		
		pT->DrawBkgnd( dcPaint );
		pT->DrawList( dcPaint );
		pT->DrawSelect( dcPaint );
		pT->DrawHeader( dcPaint );
		
		dcPaint.RestoreDC( nContextState );
	}
	
	void DrawBkgnd( CDCHandle dcPaint )
	{
		CRect rcClip;
		if ( dcPaint.GetClipBox( rcClip ) == ERROR )
			return;
		
		dcPaint.SetBkColor( m_rgbBackground );
		dcPaint.ExtTextOut( rcClip.left, rcClip.top, ETO_OPAQUE, rcClip, _T( "" ), 0, NULL );
		
		//CRect rcClient;
		CRect rcClient;
		
		GetClientRect( rcClient );
		rcClient.top = ( m_bShowHeader ? m_nHeaderHeight : 0 );
		
		if ( !m_bmpBackground.IsNull() && rcClip.bottom > rcClient.top )
		{
			CSize sizBackground;
			m_bmpBackground.GetSize( sizBackground );
		
			CDC dcBackgroundImage;
			dcBackgroundImage.CreateCompatibleDC( dcPaint );
		
			HBITMAP hOldBitmap = dcBackgroundImage.SelectBitmap( m_bmpBackground );
			
			if ( m_bTileBackground )
			{
				// calculate tile image maximum rows and columns
				div_t divRows = div( (int)rcClient.Height(), (int)sizBackground.cy );
				int nTileRows = divRows.rem > 0 ? divRows.quot + 1 : divRows.quot;
				div_t divColumns = div( (int)rcClient.Width(), (int)sizBackground.cx );
				int nTileColumns = divColumns.rem > 0 ? divColumns.quot + 1 : divColumns.quot;
				
				// draw tiled background image
				for ( int nRow = 0; nRow <= nTileRows; nRow++ )
				{
					for ( int nColumn = 0; nColumn <= nTileColumns; nColumn++ )
						dcPaint.BitBlt( nColumn * sizBackground.cx, nRow * sizBackground.cy, sizBackground.cx, sizBackground.cy, dcBackgroundImage, 0, 0, SRCCOPY );
				}
			}
			else
			{
				CRect rcCentreImage( rcClient );
				
				// horizontally centre image if smaller than the client width
				if ( sizBackground.cx < rcClient.Width() )
				{
					rcCentreImage.left = ( rcClient.Width() / 2 ) - (int)( sizBackground.cx / 2 );
					rcCentreImage.right = rcCentreImage.left + sizBackground.cx;
				}
				
				// vertically centre image if smaller than the client height
				if ( sizBackground.cy + 16 < rcClient.Height() )
				{
					rcCentreImage.top = ( rcClient.Height() / 2 ) - (int)( ( sizBackground.cy + 16 ) / 2 );
					rcCentreImage.bottom = rcCentreImage.top + sizBackground.cy;
				}
				
				// draw centred background image
				dcPaint.BitBlt( rcCentreImage.left, rcCentreImage.top, rcCentreImage.Width(), rcCentreImage.Height(), dcBackgroundImage, 0, 0, SRCCOPY );
			}

			dcBackgroundImage.SelectBitmap( hOldBitmap );
		}		
	}
	
	void DrawHeader( CDCHandle dcPaint )
	{
		if ( !m_bShowHeader )
			return;
		
		CRect rcClip;
		if ( dcPaint.GetClipBox( rcClip ) == ERROR )
			return;
		
		CRect rcHeader;
		GetClientRect( rcHeader );
		rcHeader.bottom = m_nHeaderHeight;
		
		if ( rcClip.top > rcHeader.bottom )
			return;
		
		if ( !m_bShowThemed || m_thmHeader.IsThemeNull() )
		{
			dcPaint.SetBkColor( m_rgbHeaderBackground );
			dcPaint.ExtTextOut( rcHeader.left, rcHeader.top, ETO_OPAQUE, rcHeader, _T( "" ), 0, NULL );
		}
			
		CPen penHighlight;
		penHighlight.CreatePen( PS_SOLID, 1, m_rgbHeaderBorder );
		CPen penShadow;
		penShadow.CreatePen( PS_SOLID, 1, m_rgbHeaderShadow );
		
		CRect rcHeaderItem( rcHeader );
		rcHeaderItem.OffsetRect( -GetScrollPos( SB_HORZ ), 0 );
		
		int nHeaderWidth = 0;
		
		for ( int nColumn = 0, nColumnCount = GetColumnCount(); nColumn < nColumnCount; rcHeaderItem.left = rcHeaderItem.right, nColumn++ )
		{
			CListColumn listColumn;
			if ( !GetColumn( nColumn, listColumn ) )
				break;
			
			rcHeaderItem.right = rcHeaderItem.left + listColumn.m_nWidth;
			nHeaderWidth += rcHeaderItem.Width();
			
			if ( rcHeaderItem.right < rcClip.left )
				continue;
			if ( rcHeaderItem.left > rcClip.right )
				break;
			
			// draw header and divider
			if ( m_bShowThemed && !m_thmHeader.IsThemeNull() )
				m_thmHeader.DrawThemeBackground( dcPaint, HP_HEADERITEM, nColumn == m_nHighlightColumn ? HIS_PRESSED : ( nColumn == m_nHotColumn ? HIS_HOT : HIS_NORMAL ), rcHeaderItem, NULL );
			else
			{
				if ( nColumn == m_nHighlightColumn )
				{
					dcPaint.SetBkColor( m_rgbHeaderHighlight );
					dcPaint.ExtTextOut( rcHeaderItem.left, rcHeaderItem.top, ETO_OPAQUE, rcHeaderItem, _T( "" ), 0, NULL );
				}
				
				dcPaint.SelectPen( penShadow );
				dcPaint.MoveTo( rcHeaderItem.right - 1, rcHeaderItem.top + 4 );
				dcPaint.LineTo( rcHeaderItem.right - 1, m_nHeaderHeight - 4 );

				dcPaint.SelectPen( penHighlight );
				dcPaint.MoveTo( rcHeaderItem.right, rcHeaderItem.top + 4 );
				dcPaint.LineTo( rcHeaderItem.right, m_nHeaderHeight - 4 );
			}
			
			CRect rcHeaderText( rcHeaderItem );
			rcHeaderText.left += nColumn == 0 ? 4 : 3;
			rcHeaderText.OffsetRect( 0, 1 );
			
			BOOL bShowArrow = m_bShowSort && ( rcHeaderItem.Width() > 15 );
			
			// offset text bounding rectangle to account for sorting arrow
			if ( bShowArrow && !listColumn.m_bFixed && listColumn.m_nIndex == m_nSortColumn )
				rcHeaderText.right -= 15;
			
			// margin header text
			rcHeaderText.DeflateRect( 4, 0, 5, 0 );
			
			// has this header item an associated image?
			if ( listColumn.m_nImage != ITEM_IMAGE_NONE )
			{
				CSize sizeIcon;
				m_ilListItems.GetIconSize( sizeIcon );
				
				CRect rcHeaderImage;
				rcHeaderImage.left = listColumn.m_strText.IsEmpty() ? ( ( rcHeaderText.left + rcHeaderText.right ) / 2 ) - ( sizeIcon.cx / 2 ) - ( ( !m_bShowThemed || m_thmHeader.IsThemeNull() ) ? 0 : 1 ) : rcHeaderText.left;
				rcHeaderImage.right = min( rcHeaderImage.left + sizeIcon.cx, rcHeaderItem.right - 2 );
				rcHeaderImage.top = ( ( rcHeaderItem.top + rcHeaderItem.bottom ) / 2 ) - ( sizeIcon.cy / 2 );
				rcHeaderImage.bottom = min( rcHeaderImage.top + sizeIcon.cy, rcHeaderItem.bottom );
				
				m_ilListItems.DrawEx( listColumn.m_nImage, dcPaint, rcHeaderImage, CLR_DEFAULT, CLR_DEFAULT, ILD_TRANSPARENT );

				// offset header text (for image)
				rcHeaderText.left += sizeIcon.cx + 4;
			}
			
			dcPaint.SelectFont( m_fntListFont );
			dcPaint.SetTextColor( m_rgbHeaderText );
			dcPaint.SetBkMode( TRANSPARENT );

			UINT nFormat = DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER | DT_END_ELLIPSIS;

			if ( listColumn.m_nFlags & ITEM_FLAGS_CENTRE )
				nFormat |= DT_CENTER;
			else if ( listColumn.m_nFlags & ITEM_FLAGS_RIGHT )
				nFormat |= DT_RIGHT;
			else
				nFormat |= DT_LEFT;
			
			// draw header text
			if ( !rcHeaderText.IsRectEmpty() && !listColumn.m_strText.IsEmpty() )
				dcPaint.DrawText( listColumn.m_strText, listColumn.m_strText.GetLength(), rcHeaderText, nFormat );

			// draw sorting arrow
			if ( bShowArrow && !listColumn.m_bFixed && listColumn.m_nIndex == m_nSortColumn )
			{
				CSize sizeIcon;
				m_ilListItems.GetIconSize( sizeIcon );
				
				CRect rcSortArrow;
				rcSortArrow.left = rcHeaderText.right + 4;
				rcSortArrow.right = min( rcSortArrow.left + sizeIcon.cx, rcHeaderItem.right );
				rcSortArrow.top = rcHeaderItem.Height() / 2 - 3;
				rcSortArrow.bottom = min( rcSortArrow.top + sizeIcon.cy, rcHeaderItem.bottom );
					
				m_ilListItems.DrawEx( m_bSortAscending ? ITEM_IMAGE_UP : ITEM_IMAGE_DOWN, dcPaint, rcSortArrow, CLR_DEFAULT, CLR_DEFAULT, ILD_TRANSPARENT );
			}
		}
		
		// draw remaining blank header
		if ( m_bShowThemed && !m_thmHeader.IsThemeNull() && nHeaderWidth < rcHeader.Width() )
			m_thmHeader.DrawThemeBackground( dcPaint, HP_HEADERITEM, HIS_NORMAL, CRect( nHeaderWidth, rcHeader.top, rcHeader.right + 2, rcHeader.bottom ), NULL );

		// draw a frame around all header columns
		if ( ( !m_bShowThemed || m_thmHeader.IsThemeNull() ) && nHeaderWidth > 0 )
			dcPaint.Draw3dRect( CRect( rcHeader.left, rcHeader.top, rcHeader.right + 2, rcHeader.bottom ), m_rgbHeaderBorder, m_rgbHeaderShadow );
	}
	
	void DrawRoundRect( CDCHandle dcPaint, CRect& rcRect, COLORREF rgbOuter, COLORREF rgbInner )
	{
		CRect rcRoundRect( rcRect );
					
		CPen penBorder;
		penBorder.CreatePen( PS_SOLID, 1, rgbOuter );
		CBrush bshInterior;
		bshInterior.CreateSolidBrush( m_rgbBackground );
		
		dcPaint.SelectPen( penBorder );
		dcPaint.SelectBrush( bshInterior );
		
		dcPaint.RoundRect( rcRoundRect, CPoint( 5, 5 ) );
		rcRoundRect.DeflateRect( 1, 1 );
		
		CPen penInnerBorder;
		penInnerBorder.CreatePen( PS_SOLID, 1, rgbInner );
		dcPaint.SelectPen( penInnerBorder );
		
		dcPaint.RoundRect( rcRoundRect, CPoint( 2, 2 ) );
	}
	
	void DrawGradient( CDCHandle dcPaint, CRect& rcRect, COLORREF rgbTop, COLORREF rgbBottom )
	{
		GRADIENT_RECT grdRect = { 0, 1 };
		TRIVERTEX triVertext[ 2 ] = {
										rcRect.left,
										rcRect.top,
										GetRValue( rgbTop ) << 8,
										GetGValue( rgbTop ) << 8,
										GetBValue( rgbTop ) << 8,
										0x0000,			
										rcRect.right,
										rcRect.bottom,
										GetRValue( rgbBottom ) << 8,
										GetGValue( rgbBottom ) << 8,
										GetBValue( rgbBottom ) << 8,
										0x0000
									};
		
		dcPaint.GradientFill( triVertext, 2, &grdRect, 1, GRADIENT_FILL_RECT_V );
	}
	
	void DrawList( CDCHandle dcPaint )
	{
		T* pT = static_cast<T*>(this);
		
		CRect rcClip;
		if ( dcPaint.GetClipBox( rcClip ) == ERROR )
			return;
		
		CRect rcItem;
		rcItem.left = -GetScrollPos( SB_HORZ );
		rcItem.right = GetTotalWidth();
		rcItem.top = ( m_bShowHeader ? m_nHeaderHeight : 0 );
		rcItem.bottom = rcItem.top;
		
		// draw all visible items
		for ( int nItem = GetTopItem(); nItem < pT->GetItemCount(); rcItem.top = rcItem.bottom, nItem++ )
		{
			rcItem.bottom = rcItem.top + m_nItemHeight;
			
			if ( rcItem.bottom < rcClip.top || rcItem.right < rcClip.left )
				continue;			
			if ( rcItem.top > rcClip.bottom || rcItem.left > rcClip.right )
				break;
			
			// may be implemented in a derived class
			pT->DrawItem( dcPaint, nItem, rcItem );
		}
	}
	
	void DrawItem( CDCHandle dcPaint, int nItem, CRect& rcItem )
	{
		T* pT = static_cast<T*>(this);
		
		CRect rcClip;
		if ( dcPaint.GetClipBox( rcClip ) == ERROR )
			return;
			
		int nFocusItem = NULL_ITEM;
		int nFocusSubItem = NULL_SUBITEM;
		GetFocusItem( nFocusItem, nFocusSubItem );
		
		BOOL bSelectedItem = IsSelected( nItem );
		BOOL bControlFocus = ( GetFocus() == m_hWnd || m_bEditItem );
		
		// draw selected background
		if ( bSelectedItem )
		{
			if ( m_bShowThemed && !m_thmHeader.IsThemeNull() )
			{
				// draw select border
				DrawRoundRect( dcPaint, rcItem, bControlFocus ? m_rgbSelectOuter : m_rgbNoFocusOuter, bControlFocus ? m_rgbSelectInner : m_rgbNoFocusInner );
				
				CRect rcSelect( rcItem );
				rcSelect.DeflateRect( 2, 2 );
				
				// fill selected area
				DrawGradient( dcPaint, rcSelect, bControlFocus ? m_rgbSelectTop : m_rgbNoFocusTop, bControlFocus ? m_rgbSelectBottom : m_rgbNoFocusBottom );
			}
			else
			{
				dcPaint.SetBkColor( m_rgbSelectedItem );
				dcPaint.ExtTextOut( rcItem.left, rcItem.top, ETO_OPAQUE, rcItem, _T( "" ), 0, NULL );
			}
		}
		
		CRect rcSubItem( rcItem );
		rcSubItem.right = rcSubItem.left;
		
		for ( int nSubItem = 0, nColumnCount = GetColumnCount(); nSubItem < nColumnCount; rcSubItem.left = rcSubItem.right + 1, nSubItem++ )
		{
			CListColumn listColumn;
			if ( !GetColumn( nSubItem, listColumn ) )
				break;
			
			rcSubItem.right = rcSubItem.left + listColumn.m_nWidth - 1;
			
			if ( rcSubItem.right < rcClip.left || rcSubItem.Width() == 0 )
				continue;
			if ( rcSubItem.left > rcClip.right )
				break;
			
			CString strItemText = pT->GetItemText( nItem, listColumn.m_nIndex );
			int nItemImage = pT->GetItemImage( nItem, listColumn.m_nIndex );
			UINT nItemFormat = pT->GetItemFormat( nItem, listColumn.m_nIndex );
			UINT nItemFlags = pT->GetItemFlags( nItem, listColumn.m_nIndex );
			
			// custom draw subitem format
			if ( nItemFormat == ITEM_FORMAT_CUSTOM )
			{
				pT->DrawCustomItem( dcPaint, nItem, nSubItem, rcSubItem );
				return;
			}
			
			BOOL bFocusSubItem = ( m_bFocusSubItem && nFocusItem == nItem && nFocusSubItem == nSubItem );
			
			COLORREF rgbBackground = m_rgbBackground;
			COLORREF rgbText = m_rgbItemText;
			
			if ( bFocusSubItem )
			{
				if ( m_bShowThemed && !m_thmHeader.IsThemeNull() )
				{
					// only draw subitem focus if control has focus
					if ( bControlFocus )
					{
						// draw select border
						DrawRoundRect( dcPaint, rcSubItem, bControlFocus ? m_rgbSelectOuter : m_rgbNoFocusOuter, bControlFocus ? m_rgbSelectInner : m_rgbNoFocusInner );
				
						CRect rcSelect( rcSubItem );
						rcSelect.DeflateRect( 2, 2 );
				
						// fill selected area
						if ( !m_bEditItem )
							DrawGradient( dcPaint, rcSelect, m_rgbFocusTop, m_rgbFocusBottom );
						else
						{
							dcPaint.SetBkColor( m_rgbBackground );
							dcPaint.ExtTextOut( rcSelect.left, rcSelect.top, ETO_OPAQUE, rcSelect, _T( "" ), 0, NULL );
						}
					}
				}
				else
				{
					dcPaint.SetBkColor( m_bEditItem ? m_rgbBackground : m_rgbItemFocus );
					dcPaint.ExtTextOut( rcSubItem.left, rcSubItem.top, ETO_OPAQUE, rcSubItem, _T( "" ), 0, NULL );
					
					if ( m_bEditItem )
					{
						CBrush bshSelectFrame;
						bshSelectFrame.CreateSolidBrush( m_rgbItemFocus );
						dcPaint.FrameRect( rcSubItem, bshSelectFrame );
					}
				}
			}
			else if ( pT->GetItemColours( nItem, nSubItem, rgbBackground, rgbText ) && rgbBackground != m_rgbBackground )
			{
				CPen penBorder;
				penBorder.CreatePen( PS_SOLID, 1, rgbBackground );
				CBrush bshInterior;
				bshInterior.CreateSolidBrush( rgbBackground );
				
				dcPaint.SelectPen( penBorder );
				dcPaint.SelectBrush( bshInterior );
				
				dcPaint.RoundRect( rcSubItem, CPoint( 3, 3 ) );
			}
			
			CRect rcItemText( rcSubItem );
			
			// margin item text
			rcItemText.left += nSubItem == 0 ? 4 : 3;
			rcItemText.DeflateRect( 4, 0 );
			
			// draw subitem image if supplied
			if ( !m_ilItemImages.IsNull() && nItemImage != ITEM_IMAGE_NONE && ( !m_bEditItem || ( m_bEditItem && !bFocusSubItem ) ) )
			{
				CSize sizeIcon;
				m_ilItemImages.GetIconSize( sizeIcon );
				
				CRect rcItemImage;
				rcItemImage.left = strItemText.IsEmpty() ? ( ( rcItemText.left + rcItemText.right ) / 2 ) - ( sizeIcon.cx / 2 ) - ( ( !m_bShowThemed || m_thmHeader.IsThemeNull() ) ? 0 : 1 ) : rcItemText.left;
				rcItemImage.right = min( rcItemImage.left + sizeIcon.cx, rcSubItem.right );
				rcItemImage.top = ( ( rcSubItem.top + rcSubItem.bottom ) / 2 ) - ( sizeIcon.cy / 2 );
				rcItemImage.bottom = min( rcItemImage.top + sizeIcon.cy, rcSubItem.bottom );
				
				m_ilItemImages.DrawEx( nItemImage, dcPaint, rcItemImage, CLR_DEFAULT, CLR_DEFAULT, ILD_TRANSPARENT );
					
				// offset item text (for image)
				rcItemText.left += sizeIcon.cx + 4;
			}
			
			if ( rcItemText.IsRectEmpty() )
				continue;
			
			dcPaint.SelectFont( pT->GetItemFont( nItem, nSubItem ) );
			dcPaint.SetTextColor( ( ( !m_bShowThemed || m_thmHeader.IsThemeNull() ) && bSelectedItem && !bFocusSubItem ) ? m_rgbSelectedText : rgbText );
			dcPaint.SetBkMode( TRANSPARENT );

			UINT nFormat = DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER | DT_END_ELLIPSIS;

			if ( nItemFlags & ITEM_FLAGS_CENTRE )
				nFormat |= DT_CENTER;
			else if ( nItemFlags & ITEM_FLAGS_RIGHT )
				nFormat |= DT_RIGHT;
			else
				nFormat |= DT_LEFT;
		
			switch ( nItemFormat )
			{
				case ITEM_FORMAT_DATETIME:			if ( !strItemText.IsEmpty() )
													{
														SYSTEMTIME stItemDate;
														if ( !GetItemDate( nItem, listColumn.m_nIndex, stItemDate ) )
															break;
														
														CString strItemDate;
														if ( nItemFlags & ITEM_FLAGS_DATE_ONLY )
															strItemDate = FormatDate( stItemDate );
														else if ( nItemFlags & ITEM_FLAGS_TIME_ONLY )
															strItemDate = FormatTime( stItemDate );
														else
															strItemDate = FormatDate( stItemDate ) + _T( " " ) + FormatTime( stItemDate );															
														dcPaint.DrawText( strItemDate, strItemDate.GetLength(), rcItemText, nFormat );
													}
													break;
				case ITEM_FORMAT_CHECKBOX:			
				case ITEM_FORMAT_CHECKBOX_3STATE:	{
														CSize sizeIcon;
														m_ilListItems.GetIconSize( sizeIcon );
				
														CRect rcCheckBox;
														rcCheckBox.left = ( ( rcItemText.left + rcItemText.right ) / 2 ) - ( sizeIcon.cx / 2 ) - 1;
														rcCheckBox.right = min( rcCheckBox.left + sizeIcon.cx, rcSubItem.right );
														rcCheckBox.top = ( ( rcSubItem.top + rcSubItem.bottom ) / 2 ) - ( sizeIcon.cy / 2 );
														rcCheckBox.bottom = min( rcCheckBox.top + sizeIcon.cy, rcSubItem.bottom );
														
														int nCheckValue = _ttoi( strItemText );
																
														if ( nItemFormat == ITEM_FORMAT_CHECKBOX )
															m_ilListItems.DrawEx( nCheckValue > 0 ? ITEM_IMAGE_CHECK_ON : ITEM_IMAGE_CHECK_OFF, dcPaint, rcCheckBox, CLR_DEFAULT, CLR_DEFAULT, ILD_TRANSPARENT );
														else
														{
															int nCheckImage = ITEM_IMAGE_3STATE_UNDEF;
															if ( nCheckValue < 0 )
																nCheckImage = ITEM_IMAGE_3STATE_OFF;
															else if ( nCheckValue > 0 )
																nCheckImage = ITEM_IMAGE_3STATE_ON;
															m_ilListItems.DrawEx( nCheckImage, dcPaint, rcCheckBox, CLR_DEFAULT, CLR_DEFAULT, ILD_TRANSPARENT );
														}
													}														
													break;
				case ITEM_FORMAT_PROGRESS:			if ( m_bShowThemed && !m_thmProgress.IsThemeNull() && !( nItemFlags & ITEM_FLAGS_PROGRESS_SOLID ) )
													{
														// draw progress frame
														CRect rcProgress( rcSubItem );
														rcProgress.DeflateRect( 3, 2 );
														m_thmProgress.DrawThemeBackground( dcPaint, PP_BAR, 0, rcProgress, NULL );
														
														// draw progress bar															
														rcProgress.DeflateRect( 3, 3, 2, 3 );
														rcProgress.right = rcProgress.left + (int)( (double)rcProgress.Width() * ( ( max( min( _tstof( strItemText ), 100 ), 0 ) ) / 100.0 ) );
														m_thmProgress.DrawThemeBackground( dcPaint, PP_CHUNK, 0, rcProgress, NULL );
													}
													else
													{
														CRect rcProgress( rcSubItem );
														rcProgress.DeflateRect( 3, 2 );
														
														// draw progress border
														DrawRoundRect( dcPaint, rcProgress, m_rgbHeaderShadow, m_rgbHeaderBackground );
														
														// fill progress bar area
														rcProgress.DeflateRect( 3, 3 );
														rcProgress.right = rcProgress.left + (int)( (double)rcProgress.Width() * ( ( max( min( _tstof( strItemText ), 100 ), 0 ) ) / 100.0 ) );
														DrawGradient( dcPaint, rcProgress, m_rgbProgressTop, m_rgbProgressBottom );
													}					
													break;
				case ITEM_FORMAT_HYPERLINK:			if ( nItem == m_nHotItem && nSubItem == m_nHotSubItem && !( nItemFlags & ITEM_FLAGS_READ_ONLY ) )
													{
														dcPaint.SelectFont( m_fntUnderlineFont );
														dcPaint.SetTextColor( m_rgbHyperLink );
													}
				default:							// draw item text
													if ( !strItemText.IsEmpty() )
														dcPaint.DrawText( strItemText, strItemText.GetLength(), rcItemText, nFormat );
													break;
			}
		}
	}
	
	void DrawSelect( CDCHandle dcPaint )
	{
		if ( !m_bGroupSelect )
			return;
		
		int nHorzScroll = GetScrollPos( SB_HORZ );
		int nVertScroll = GetScrollPos( SB_VERT );
		
		CRect rcGroupSelect( m_rcGroupSelect );
		rcGroupSelect.OffsetRect( -nHorzScroll, -nVertScroll );
		
		CRect rcClient;
		GetClientRect( rcClient );
		rcClient.top = ( m_bShowHeader ? m_nHeaderHeight : 0 );
		
		// limit box to list client area if scrolled to limits
		if ( nHorzScroll > ( GetTotalWidth() - rcClient.Width() ) )
			rcGroupSelect.right = min( rcClient.right, rcGroupSelect.right );
		if ( nHorzScroll == 0 )
			rcGroupSelect.left = max( rcClient.left, rcGroupSelect.left );
		if ( nVertScroll > ( GetTotalHeight() - rcClient.Height() ) )
			rcGroupSelect.bottom = min( rcClient.bottom, rcGroupSelect.bottom );
		if ( nVertScroll == 0 )
			rcGroupSelect.top = max( rcClient.top, rcGroupSelect.top );
		
		// limit bitmap to client area
		CRect rcSelectArea( rcGroupSelect );
		rcSelectArea.IntersectRect( rcSelectArea, rcClient );
		
		CDC dcBackground;
		dcBackground.CreateCompatibleDC( dcPaint );
		
		int nBackgroundContext = dcBackground.SaveDC();
		
		CBitmap bmpBackground;
		bmpBackground.CreateCompatibleBitmap( dcPaint, rcSelectArea.Width(), rcSelectArea.Height() ); 
		dcBackground.SelectBitmap( bmpBackground );
		
		// take a copy of existing backgroud
		dcBackground.BitBlt( 0, 0, rcSelectArea.Width(), rcSelectArea.Height(), dcPaint, rcSelectArea.left, rcSelectArea.top, SRCCOPY );
		
		CDC dcGroupSelect;
		dcGroupSelect.CreateCompatibleDC( dcPaint );
		
		int nGroupSelectContext = dcGroupSelect.SaveDC();
		
		CBitmap bmpGroupSelect;
		bmpGroupSelect.CreateCompatibleBitmap( dcPaint, rcSelectArea.Width(), rcSelectArea.Height() ); 
		dcGroupSelect.SelectBitmap( bmpGroupSelect );
		
		// draw group select box
		dcGroupSelect.SetBkColor( ( !m_bShowThemed || m_thmHeader.IsThemeNull() ) ? m_rgbItemFocus : m_rgbSelectedItem );
		dcGroupSelect.ExtTextOut( 0, 0, ETO_OPAQUE, CRect( CPoint( 0 ), rcSelectArea.Size() ), _T( "" ), 0, NULL );
		
		BLENDFUNCTION blendFunction;
		blendFunction.BlendOp = AC_SRC_OVER;
		blendFunction.BlendFlags = 0;
		blendFunction.SourceConstantAlpha = 180;
		blendFunction.AlphaFormat = 0;
		
		// blend existing background with selection box
		dcGroupSelect.AlphaBlend( 0, 0, rcSelectArea.Width(), rcSelectArea.Height(), dcBackground, 0, 0, rcSelectArea.Width(), rcSelectArea.Height(), blendFunction ); 
		
		// draw blended selection box
		dcPaint.BitBlt( rcSelectArea.left, rcSelectArea.top, rcSelectArea.Width(), rcSelectArea.Height(), dcGroupSelect, 0, 0, SRCCOPY );
		
		// draw selection box frame
		CBrush bshSelectFrame;
		bshSelectFrame.CreateSolidBrush( ( !m_bShowThemed || m_thmHeader.IsThemeNull() ) ? m_rgbItemText : m_rgbSelectedItem );
		dcPaint.FrameRect( rcGroupSelect, bshSelectFrame );
		
		dcBackground.RestoreDC( nBackgroundContext );
		dcGroupSelect.RestoreDC( nGroupSelectContext );
	}
	
	void DrawCustomItem( CDCHandle dcPaint, int nItem, int nSubItem, CRect& rcSubItem )
	{
		ATLASSERT( FALSE ); // must be implemented in a derived class
	}
};

struct CSubItem
{
	CString m_strText;
	int m_nImage;
	UINT m_nFormat;
	UINT m_nFlags;
	CListArray < CString > m_aComboList;
	HFONT m_hFont;
	COLORREF m_rgbBackground;
	COLORREF m_rgbText;
};

template < class TData = DWORD >
struct CListItem
{
	CListArray < CSubItem > m_aSubItems;
	CString m_strToolTip;
	TData m_tData;	
};

template < class TData >
class CListCtrlData : public CListImpl< CListCtrlData< TData > >
{
public:
	DECLARE_WND_CLASS( _T( "ListCtrl" ) )

protected:
	CListArray < CListItem< TData > > m_aItems;
	
public:
	int AddItem( CListItem< TData >& listItem )
	{
		if ( !m_aItems.Add( listItem ) )
			return -1;
		return CListImpl< CListCtrlData >::AddItem() ? GetItemCount() - 1 : -1;
	}
	
	int AddItem( LPCTSTR lpszText, int nImage = ITEM_IMAGE_NONE, UINT nFormat = ITEM_FORMAT_NONE, UINT nFlags = ITEM_FLAGS_NONE )
	{
		CSubItem listSubItem;
		listSubItem.m_nImage = ITEM_IMAGE_NONE;
		listSubItem.m_nFormat = nFormat;
		listSubItem.m_nFlags = ValidateFlags( nFlags );
		listSubItem.m_hFont = NULL;
		listSubItem.m_rgbBackground = m_rgbBackground;
		listSubItem.m_rgbText = m_rgbItemText;
		
		CListItem< TData > listItem;
		for ( int nSubItem = 0; nSubItem < GetColumnCount(); nSubItem++ )
			listItem.m_aSubItems.Add( listSubItem );
			
		// set item details for first subitem
		listItem.m_aSubItems[ 0 ].m_strText = lpszText;
		listItem.m_aSubItems[ 0 ].m_nImage = nImage;
		
		return AddItem( listItem );
	}
	
	BOOL DeleteItem( int nItem )
	{
		if ( nItem < 0 || nItem >= GetItemCount() ) 
			return FALSE;
		return m_aItems.RemoveAt( nItem ) ? CListImpl< CListCtrlData >::DeleteItem( nItem ) : FALSE;
	}
	
	BOOL DeleteAllItems()
	{
		m_aItems.RemoveAll();
		return CListImpl< CListCtrlData >::DeleteAllItems();
	}
	
	int GetItemCount()
	{
		return m_aItems.GetSize();
	}
	
	BOOL GetItem( int nItem, CListItem< TData >& listItem )
	{
		if ( nItem < 0 || nItem >= GetItemCount() ) 
			return FALSE;
		for( int i = m_aItems[ nItem ].m_aSubItems.GetSize(); i <  GetColumnCount(); i ++ )
		{
			CSubItem subItem;
			subItem.m_nImage = ITEM_IMAGE_NONE;
			subItem.m_nFormat = GetColumnFormat( i );
			subItem.m_nFlags = GetColumnFlags( i );
			subItem.m_hFont = NULL;
			subItem.m_rgbBackground = m_rgbBackground;
			subItem.m_rgbText = m_rgbItemText;
			m_aItems[ nItem ].m_aSubItems.Add( subItem );
		}
		listItem = m_aItems[ nItem ];
		
		return TRUE;
	}
	
	BOOL GetSubItem( int nItem, int nSubItem, CSubItem& listSubItem )
	{
		CListItem< TData > listItem;
		if ( !GetItem( nItem, listItem ) )
			return FALSE;
		if ( nSubItem < 0 || nSubItem >= (int)listItem.m_aSubItems.GetSize() )
			return FALSE;
		listSubItem = listItem.m_aSubItems[ nSubItem ];
		return TRUE;
	}
	
	CString GetItemText( int nItem, int nSubItem )
	{
		CSubItem listSubItem;
		return GetSubItem( nItem, nSubItem, listSubItem ) ? listSubItem.m_strText : _T( "" );
	}
	
	int GetItemImage( int nItem, int nSubItem )
	{
		CSubItem listSubItem;
		return GetSubItem( nItem, nSubItem, listSubItem ) ? listSubItem.m_nImage : ITEM_IMAGE_NONE;
	}
	
	UINT GetItemFormat( int nItem, int nSubItem )
	{
		CSubItem listSubItem;
		if ( !GetSubItem( nItem, nSubItem, listSubItem ) )
			return FALSE;
		return listSubItem.m_nFormat == ITEM_FORMAT_NONE ? GetColumnFormat( IndexToOrder( nSubItem ) ) : listSubItem.m_nFormat;
	}
	
	UINT GetItemFlags( int nItem, int nSubItem )
	{
		CSubItem listSubItem;
		if ( !GetSubItem( nItem, nSubItem, listSubItem ) )
			return FALSE;
		return listSubItem.m_nFlags == ITEM_FLAGS_NONE ? GetColumnFlags( IndexToOrder( nSubItem ) ) : listSubItem.m_nFlags;
	}
	
	BOOL GetItemComboList( int nItem, int nSubItem, CListArray < CString >& aComboList )
	{
		CSubItem listSubItem;
		if ( !GetSubItem( nItem, nSubItem, listSubItem ) )
			return FALSE;
		aComboList = listSubItem.m_aComboList;
		return aComboList.IsEmpty() ? GetColumnComboList( IndexToOrder( nSubItem ), aComboList ) : !aComboList.IsEmpty();
	}
	
	HFONT GetItemFont( int nItem, int nSubItem )
	{
		CSubItem listSubItem;
		if ( !GetSubItem( nItem, nSubItem, listSubItem ) )
			return FALSE;
		return listSubItem.m_hFont == NULL ? CListImpl< CListCtrlData >::GetItemFont( nItem, nSubItem ) : listSubItem.m_hFont;
	}
	
	BOOL GetItemColours( int nItem, int nSubItem, COLORREF& rgbBackground, COLORREF& rgbText )
	{
		CSubItem listSubItem;
		if ( !GetSubItem( nItem, nSubItem, listSubItem ) )
			return FALSE;
		rgbBackground = listSubItem.m_rgbBackground;
		rgbText = listSubItem.m_rgbText;
		return TRUE;
	}
	
	CString GetItemToolTip( int nItem, int nSubItem )
	{
		CListItem< TData > listItem;
		return GetItem( nItem, listItem ) ? listItem.m_strToolTip : _T( "" );
	}
	
	BOOL GetItemData( int nItem, TData& tData )
	{
		CListItem< TData > listItem;
		if ( !GetItem( nItem, listItem ) )
			return FALSE;
		tData = listItem.m_tData;
		return TRUE;
	}
	
	BOOL SetItemText( int nItem, int nSubItem, LPCTSTR lpszText )
	{
		if ( nItem < 0 || nItem >= GetItemCount() ) 
			return FALSE;
		if ( nSubItem < 0 || nSubItem >= (int)m_aItems[ nItem ].m_aSubItems.GetSize() )
			return FALSE;
		m_aItems[ nItem ].m_aSubItems[ nSubItem ].m_strText = lpszText;
		return TRUE;
	}
	
	BOOL SetItemComboIndex( int nItem, int nSubItem, int nIndex )
	{
		CListArray < CString > aComboList;
		if ( !GetItemComboList( nItem, nSubItem, aComboList ) )
			return FALSE;
		return SetItemText( nItem, nSubItem, nIndex < 0 || nIndex >= aComboList.GetSize() ? _T( "" ) : aComboList[ nIndex ] );
	}
	
	BOOL SetItemImage( int nItem, int nSubItem, int nImage )
	{
		if ( nItem < 0 || nItem >= GetItemCount() ) 
			return FALSE;
		if ( nSubItem < 0 || nSubItem >= (int)m_aItems[ nItem ].m_aSubItems.GetSize() )
			return FALSE;
		m_aItems[ nItem ].m_aSubItems[ nSubItem ].m_nImage = nImage;
		return TRUE;
	}
	
	BOOL SetItemFormat( int nItem, int nSubItem, UINT nFormat, UINT nFlags = ITEM_FLAGS_NONE )
	{
		if ( nItem < 0 || nItem >= GetItemCount() ) 
			return FALSE;
		if ( nSubItem < 0 || nSubItem >= (int)m_aItems[ nItem ].m_aSubItems.GetSize() )
			return FALSE;
		m_aItems[ nItem ].m_aSubItems[ nSubItem ].m_nFormat = nFormat;
		m_aItems[ nItem ].m_aSubItems[ nSubItem ].m_nFlags = nFlags;
		return TRUE;
	}

	BOOL RemoveColumn(int i)
	{
		m_aColumns.RemoveAt( i );
		Invalidate();
		return TRUE;
	}

	BOOL RemoveAllColumns()
	{
		m_aColumns.RemoveAll();
		Invalidate();
		return TRUE;
	}
	
	BOOL SetItemFormat( int nItem, int nSubItem, UINT nFormat, UINT nFlags, CListArray < CString >& aComboList )
	{
		if ( nItem < 0 || nItem >= GetItemCount() ) 
			return FALSE;
		if ( nSubItem < 0 || nSubItem >= (int)m_aItems[ nItem ].m_aSubItems.GetSize() )
			return FALSE;
		m_aItems[ nItem ].m_aSubItems[ nSubItem ].m_nFormat = nFormat;
		m_aItems[ nItem ].m_aSubItems[ nSubItem ].m_nFlags = nFlags;
		m_aItems[ nItem ].m_aSubItems[ nSubItem ].m_aComboList = aComboList;
		return TRUE;
	}
	
	BOOL SetItemFont( int nItem, int nSubItem, HFONT hFont )
	{
		if ( nItem < 0 || nItem >= GetItemCount() ) 
			return FALSE;
		if ( nSubItem < 0 || nSubItem >= (int)m_aItems[ nItem ].m_aSubItems.GetSize() )
			return FALSE;
		m_aItems[ nItem ].m_aSubItems[ nSubItem ].m_hFont = hFont;
		return TRUE;
	}
	
	BOOL SetItemColours( int nItem, int nSubItem, COLORREF rgbBackground, COLORREF rgbText )
	{
		if ( nItem < 0 || nItem >= GetItemCount() )
			return FALSE;
		if ( nSubItem < 0 || nSubItem >= (int)m_aItems[ nItem ].m_aSubItems.GetSize() )
			return FALSE;
		m_aItems[ nItem ].m_aSubItems[ nSubItem ].m_rgbBackground = rgbBackground;
		m_aItems[ nItem ].m_aSubItems[ nSubItem ].m_rgbText = rgbText;
		return TRUE;
	}
	
	void ReverseItems()
	{
		m_aItems.Reverse();
	}
	
	class CompareItem
	{
	public:
		CompareItem( int nColumn ) : m_nColumn( nColumn ) {}
		inline bool operator() ( const CListItem< TData >& listItem1, const CListItem< TData >& listItem2 )
		{
			return ( listItem1.m_aSubItems[ m_nColumn ].m_strText.Compare( listItem2.m_aSubItems[ m_nColumn ].m_strText ) < 0 );
		}
		
	protected:
		int m_nColumn;
	};
	
	void SortItems( int nColumn, BOOL bAscending )
	{
		m_aItems.Sort( CompareItem( nColumn ) );
	}
	
	BOOL SetItemToolTip( int nItem, LPCTSTR lpszToolTip )
	{
		if ( nItem < 0 || nItem >= GetItemCount() ) 
			return FALSE;
		m_aItems[ nItem ].m_strToolTip = lpszToolTip;
		return TRUE;
	}
	
	BOOL SetItemData( int nItem, TData& tData )
	{
		if ( nItem < 0 || nItem >= GetItemCount() ) 
			return FALSE;
		m_aItems[ nItem ].m_tData = tData;
		return TRUE;
	}
};

typedef CListCtrlData< DWORD > CListCtrl;
