#include "stdafx.h"
#include "dwmapi_delegate.h"

typedef HRESULT (WINAPI *FunDwmExtendFrameIntoClientArea)(HWND,const MARGINS *);
typedef HRESULT (WINAPI *FunDwmIsCompositionEnabled)(BOOL *);
typedef HRESULT (WINAPI *FunDwmDefWindowProc)(HWND hwnd, UINT msg,WPARAM wParam, LPARAM lParam, LRESULT *plResult );
typedef HRESULT (WINAPI *FunDwmEnableBlurBehindWindow)(HWND hWnd,const DWM_BLURBEHIND *pBlurBehind);
typedef HRESULT (WINAPI *FunDwmEnableComposition)( UINT uCompositionAction );
typedef HRESULT (WINAPI *FunDwmEnableMMCSS)(BOOL fEnableMMCSS);
typedef HRESULT (WINAPI *FunDwmGetColorizationColor)( DWORD *pcrColorization, BOOL *pfOpaqueBlend);
typedef HRESULT (WINAPI *FunDwmGetCompositionTimingInfo)( HWND hwnd, DWM_TIMING_INFO *pTimingInfo);
typedef HRESULT (WINAPI *FunDwmGetWindowAttribute)(HWND hwnd,DWORD dwAttribute,PVOID pvAttribute,DWORD cbAttribute);
typedef HRESULT (WINAPI *FunDwmModifyPreviousDxFrameDuration)(HWND hwnd,INT cRefreshes,BOOL fRelative);
typedef HRESULT (WINAPI *FunDwmQueryThumbnailSourceSize)(HTHUMBNAIL hThumbnail,PSIZE pSize);
typedef HRESULT (WINAPI *FunDwmRegisterThumbnail)(HWND hwndDestination,HWND *hwndSource,PHTHUMBNAIL phThumbnailId);
typedef HRESULT (WINAPI *FunDwmSetDxFrameDuration)(HWND hwnd, INT cRefreshes);
typedef HRESULT (WINAPI *FunDwmSetPresentParameters)(HWND hwnd,DWM_PRESENT_PARAMETERS *pPresentParams);
typedef HRESULT (WINAPI *FunDwmSetWindowAttribute)(HWND hwnd,DWORD dwAttribute,LPCVOID pvAttribute,DWORD cbAttribute);
typedef HRESULT (WINAPI *FunDwmUnregisterThumbnail)(HTHUMBNAIL hThumbnailId);
typedef HRESULT (WINAPI *FunDwmUpdateThumbnailProperties)(HTHUMBNAIL hThumbnailId, const DWM_THUMBNAIL_PROPERTIES *ptnProperties);

typedef HRESULT (WINAPI *FunEndBufferedPaint)(HPAINTBUFFER hBufferedPaint, BOOL fUpdateTarget);
typedef HPAINTBUFFER (WINAPI *FunBeginBufferedPaint)(HDC hdcTarget, const RECT *prcTarget, BP_BUFFERFORMAT dwFormat, BP_PAINTPARAMS *pPaintParams, HDC *phdc);
typedef HRESULT (WINAPI *FunBufferedPaintSetAlpha)(HPAINTBUFFER hBufferedPaint,const RECT *prc,BYTE alpha);
typedef HRESULT (WINAPI *FunBufferedPaintClear)(HPAINTBUFFER hBufferedPaint,const RECT *prc);
typedef HRESULT (WINAPI *FunBufferedPaintUnInit)();
typedef HRESULT (WINAPI *FunBufferedPaintInit)();
typedef HRESULT (WINAPI *FunGetThemeBitmap)(HTHEME hTheme,int iPartId,int iStateId,int iPropId,ULONG dwFlags,HBITMAP *phBitmap);
typedef HRESULT (WINAPI *FunDrawThemeTextEx)(HTHEME hTheme,HDC hdc,int iPartId,int iStateId,LPCWSTR pszText,int iCharCount, DWORD dwFlags,LPRECT pRect,const DTTOPTS *pOptions);


static HMODULE g_hDWMModule				= NULL;
static HMODULE g_hUxThemeModule			= NULL;

static FunDwmExtendFrameIntoClientArea		 __DwmExtendFrameIntoClientArea	   = NULL;
static FunDwmIsCompositionEnabled			 __DwmIsCompositionEnabled		   = NULL;
static FunDwmDefWindowProc					 __DwmDefWindowProc                = NULL;
static FunDwmEnableBlurBehindWindow          __DwmEnableBlurBehindWindow       = NULL;
static FunDwmEnableComposition               __DwmEnableComposition            = NULL;
static FunDwmEnableMMCSS                     __DwmEnableMMCSS                  = NULL;
static FunDwmGetColorizationColor            __DwmGetColorizationColor         = NULL;
static FunDwmGetCompositionTimingInfo        __DwmGetCompositionTimingInfo     = NULL;
static FunDwmGetWindowAttribute              __DwmGetWindowAttribute           = NULL;
static FunDwmModifyPreviousDxFrameDuration   __DwmModifyPreviousDxFrameDuration= NULL;
static FunDwmQueryThumbnailSourceSize        __DwmQueryThumbnailSourceSize     = NULL;
static FunDwmRegisterThumbnail               __DwmRegisterThumbnail            = NULL;
static FunDwmSetDxFrameDuration              __DwmSetDxFrameDuration           = NULL;
static FunDwmSetPresentParameters            __DwmSetPresentParameters         = NULL;
static FunDwmSetWindowAttribute              __DwmSetWindowAttribute           = NULL;
static FunDwmUnregisterThumbnail             __DwmUnregisterThumbnail          = NULL;
static FunDwmUpdateThumbnailProperties       __DwmUpdateThumbnailProperties    = NULL;

static FunEndBufferedPaint					 __EndBufferedPaint				   = NULL;
static FunBeginBufferedPaint				 __BeginBufferedPaint			   = NULL;
static FunBufferedPaintSetAlpha				 __BufferedPaintSetAlpha		   = NULL;
static FunBufferedPaintClear				 __BufferedPaintClear			   = NULL;
static FunBufferedPaintUnInit				 __BufferedPaintUnInit			   = NULL;
static FunBufferedPaintInit					 __BufferedPaintInit			   = NULL;
static FunGetThemeBitmap					 __GetThemeBitmap				   = NULL;
static FunDrawThemeTextEx					 __DrawThemeTextEx				   = NULL;

BOOL InitDWMTheme()
{
	HMODULE hModule = LoadLibrary( _T( "dwmapi.dll" ) );
	if( hModule )
	{
		__DwmExtendFrameIntoClientArea	  = (FunDwmExtendFrameIntoClientArea)::GetProcAddress( hModule, "DwmExtendFrameIntoClientArea" );
		__DwmIsCompositionEnabled		  = (FunDwmIsCompositionEnabled)::GetProcAddress( hModule, "DwmIsCompositionEnabled" );
		__DwmDefWindowProc                = (FunDwmDefWindowProc)::GetProcAddress( hModule, "DwmDefWindowProc" );
		__DwmEnableBlurBehindWindow       = (FunDwmEnableBlurBehindWindow)::GetProcAddress( hModule, "DwmEnableBlurBehindWindow" );
		__DwmEnableComposition            = (FunDwmEnableComposition)::GetProcAddress( hModule, "DwmEnableComposition" );
		__DwmEnableMMCSS                  = (FunDwmEnableMMCSS)::GetProcAddress( hModule, "DwmEnableMMCSS" );
		__DwmGetColorizationColor         = (FunDwmGetColorizationColor)::GetProcAddress( hModule, "DwmGetColorizationColor" );
		__DwmGetCompositionTimingInfo     = (FunDwmGetCompositionTimingInfo)::GetProcAddress( hModule, "DwmGetCompositionTimingInfo" );
		__DwmGetWindowAttribute           = (FunDwmGetWindowAttribute)::GetProcAddress( hModule, "DwmGetWindowAttribute" );
		__DwmModifyPreviousDxFrameDuration= (FunDwmModifyPreviousDxFrameDuration)::GetProcAddress( hModule, "DwmModifyPreviousDxFrameDuration" );
		__DwmQueryThumbnailSourceSize     = (FunDwmQueryThumbnailSourceSize)::GetProcAddress( hModule, "DwmQueryThumbnailSourceSize" );
		__DwmRegisterThumbnail            = (FunDwmRegisterThumbnail)::GetProcAddress( hModule, "DwmRegisterThumbnail" );
		__DwmSetDxFrameDuration           = (FunDwmSetDxFrameDuration)::GetProcAddress( hModule, "DwmSetDxFrameDuration" );
		__DwmSetPresentParameters         = (FunDwmSetPresentParameters)::GetProcAddress( hModule, "DwmSetPresentParameters" );
		__DwmSetWindowAttribute           = (FunDwmSetWindowAttribute)::GetProcAddress( hModule, "DwmSetWindowAttribute" );
		__DwmUnregisterThumbnail          = (FunDwmUnregisterThumbnail)::GetProcAddress( hModule, "DwmUnregisterThumbnail" );
		__DwmUpdateThumbnailProperties    = (FunDwmUpdateThumbnailProperties)::GetProcAddress( hModule, "DwmUpdateThumbnailProperties" );
		g_hDWMModule = hModule;

		HMODULE hThemeModule = LoadLibrary( _T("UxTheme.dll") );
		if( hThemeModule )
		{
			g_hUxThemeModule = hThemeModule;
			__EndBufferedPaint				= (FunEndBufferedPaint)::GetProcAddress( hThemeModule, "EndBufferedPaint" );
			__BeginBufferedPaint			= (FunBeginBufferedPaint)::GetProcAddress( hThemeModule, "BeginBufferedPaint" );
			__BufferedPaintSetAlpha			= (FunBufferedPaintSetAlpha)::GetProcAddress( hThemeModule, "BufferedPaintSetAlpha" );
			__BufferedPaintClear			= (FunBufferedPaintClear)::GetProcAddress( hThemeModule, "BufferedPaintClear" );
			__BufferedPaintUnInit			= (FunBufferedPaintUnInit)::GetProcAddress( hThemeModule, "BufferedPaintUnInit" );
			__BufferedPaintInit				= (FunBufferedPaintInit)::GetProcAddress( hThemeModule, "BufferedPaintInit" );
			__GetThemeBitmap				= (FunGetThemeBitmap)::GetProcAddress( hThemeModule, "GetThemeBitmap" );
			__DrawThemeTextEx				= (FunDrawThemeTextEx)::GetProcAddress( hThemeModule, "DrawThemeTextEx" );
		}
		return TRUE;
	}
	return FALSE;
}

BOOL FreeDWMTheme()
{
	__DwmExtendFrameIntoClientArea	  = NULL;
	__DwmIsCompositionEnabled		  = NULL;
	__DwmDefWindowProc                = NULL;
	__DwmEnableBlurBehindWindow       = NULL;
	__DwmEnableComposition            = NULL;
	__DwmEnableMMCSS                  = NULL;
	__DwmGetColorizationColor         = NULL;
	__DwmGetCompositionTimingInfo     = NULL;
	__DwmGetWindowAttribute           = NULL;
	__DwmModifyPreviousDxFrameDuration= NULL;
	__DwmQueryThumbnailSourceSize     = NULL;
	__DwmRegisterThumbnail            = NULL;
	__DwmSetDxFrameDuration           = NULL;
	__DwmSetPresentParameters         = NULL;
	__DwmSetWindowAttribute           = NULL;
	__DwmUnregisterThumbnail          = NULL;
	__DwmUpdateThumbnailProperties    = NULL;
	__EndBufferedPaint				  = NULL;
	__BeginBufferedPaint			  = NULL;
	__BufferedPaintSetAlpha		 	  = NULL;
	__BufferedPaintClear			  = NULL;
	__BufferedPaintUnInit			  = NULL;
	__BufferedPaintInit				  = NULL;
	__GetThemeBitmap				  = NULL;
	__DrawThemeTextEx				  = NULL;

	if( g_hDWMModule )
	{
		FreeLibrary( g_hDWMModule );
		g_hDWMModule = NULL;
	}

	if( g_hUxThemeModule )
	{
		FreeLibrary( g_hUxThemeModule );
		g_hUxThemeModule = NULL;
	}
	return TRUE;
}

HRESULT _DwmIsCompositionEnabled( BOOL * bl )
{
	if( __DwmIsCompositionEnabled )
	{
		return __DwmIsCompositionEnabled( bl );
	}
	return S_FALSE;
}


HRESULT _DwmExtendFrameIntoClientArea(HWND hWnd,const MARGINS *pMarInset)
{
	if( __DwmExtendFrameIntoClientArea )
	{
		return __DwmExtendFrameIntoClientArea( hWnd, pMarInset );
	}
	return S_FALSE;
}

HRESULT _DwmDefWindowProc(HWND hwnd, UINT msg,WPARAM wParam, LPARAM lParam, LRESULT *plResult )
{
	if( __DwmDefWindowProc )
	{
		return __DwmDefWindowProc( hwnd, msg, wParam, lParam, plResult );
	}
	return S_FALSE;
}

HRESULT _DwmEnableBlurBehindWindow(HWND hWnd,const DWM_BLURBEHIND *pBlurBehind)
{
	if( __DwmEnableBlurBehindWindow )
	{
		return __DwmEnableBlurBehindWindow( hWnd, pBlurBehind );
	}
	return S_FALSE;
}

HRESULT _DwmEnableComposition( UINT uCompositionAction )
{
	if( __DwmEnableComposition )
	{
		return __DwmEnableComposition( uCompositionAction );
	}
	return S_FALSE;
}

HRESULT _DwmEnableMMCSS(BOOL fEnableMMCSS)
{
	if( __DwmEnableMMCSS )
	{
		return __DwmEnableMMCSS( fEnableMMCSS );
	}
	return S_FALSE;
}

HRESULT _DwmGetColorizationColor( DWORD *pcrColorization, BOOL *pfOpaqueBlend)
{
	if( __DwmGetColorizationColor )
	{
		return __DwmGetColorizationColor( pcrColorization, pfOpaqueBlend );
	}
	return S_FALSE;
}

HRESULT _DwmGetCompositionTimingInfo( HWND hwnd, DWM_TIMING_INFO *pTimingInfo)
{
	if( __DwmGetCompositionTimingInfo )
	{
		return __DwmGetCompositionTimingInfo( hwnd, pTimingInfo );
	}
	return S_FALSE;
}

HRESULT _DwmGetWindowAttribute(HWND hwnd,DWORD dwAttribute,PVOID pvAttribute,DWORD cbAttribute)
{
	if( __DwmGetWindowAttribute )
	{
		return __DwmGetWindowAttribute( hwnd, dwAttribute, pvAttribute, cbAttribute );
	}
	return S_FALSE;
}

HRESULT _DwmModifyPreviousDxFrameDuration(HWND hwnd,INT cRefreshes,BOOL fRelative)
{
	if( __DwmModifyPreviousDxFrameDuration )
	{
		return __DwmModifyPreviousDxFrameDuration( hwnd, cRefreshes, fRelative );
	}
	return S_FALSE;
}

HRESULT _DwmQueryThumbnailSourceSize(HTHUMBNAIL hThumbnail,PSIZE pSize)
{
	if( __DwmQueryThumbnailSourceSize )
	{
		return __DwmQueryThumbnailSourceSize( hThumbnail, pSize );
	}
	return S_FALSE;
}

HRESULT _DwmRegisterThumbnail(HWND hwndDestination,HWND *hwndSource,PHTHUMBNAIL phThumbnailId)
{
	if( __DwmRegisterThumbnail )
	{
		return __DwmRegisterThumbnail( hwndDestination, hwndSource, phThumbnailId );
	}
	return S_FALSE;
}

HRESULT _DwmSetDxFrameDuration(HWND hwnd, INT cRefreshes)
{
	if( __DwmSetDxFrameDuration )
	{
		return __DwmSetDxFrameDuration( hwnd, cRefreshes );
	}
	return S_FALSE;
}

HRESULT _DwmSetPresentParameters(HWND hwnd,DWM_PRESENT_PARAMETERS *pPresentParams)
{
	if( __DwmSetPresentParameters )
	{
		return __DwmSetPresentParameters( hwnd, pPresentParams );
	}
	return S_FALSE;
}

HRESULT _DwmSetWindowAttribute(HWND hwnd,DWORD dwAttribute,LPCVOID pvAttribute,DWORD cbAttribute)
{
	if( __DwmSetWindowAttribute )
	{
		return __DwmSetWindowAttribute( hwnd, dwAttribute, pvAttribute, cbAttribute );
	}
	return S_FALSE;
}

HRESULT _DwmUnregisterThumbnail(HTHUMBNAIL hThumbnailId)
{
	if( __DwmUnregisterThumbnail )
	{
		return __DwmUnregisterThumbnail( hThumbnailId );
	}
	return S_FALSE;
}

HRESULT _DwmUpdateThumbnailProperties(HTHUMBNAIL hThumbnailId, const DWM_THUMBNAIL_PROPERTIES *ptnProperties)
{
	if( __DwmUpdateThumbnailProperties )
	{
		return __DwmUpdateThumbnailProperties( hThumbnailId, ptnProperties );
	}
	return S_FALSE;
}

/****
 * Helper function implementation
 ****/
BOOL IsCompositionEnabled()
{
	BOOL bl = FALSE; 
	_DwmIsCompositionEnabled( &bl );
	return bl;
}


///UxTheme.dll
HRESULT _EndBufferedPaint(HPAINTBUFFER hBufferedPaint, BOOL fUpdateTarget)
{
	if( __EndBufferedPaint )
	{
		return __EndBufferedPaint( hBufferedPaint, fUpdateTarget );
	}
	return S_FALSE;
}

HPAINTBUFFER _BeginBufferedPaint(HDC hdcTarget, const RECT *prcTarget, BP_BUFFERFORMAT dwFormat, BP_PAINTPARAMS *pPaintParams, HDC *phdc)
{
	if( __BeginBufferedPaint )
	{
		return __BeginBufferedPaint( hdcTarget, prcTarget, dwFormat, pPaintParams, phdc);
	}

	if( phdc )
	{
		*phdc = NULL;
	}

	return NULL;
}

HRESULT _BufferedPaintSetAlpha(HPAINTBUFFER hBufferedPaint,const RECT *prc,BYTE alpha)
{
	if( __BufferedPaintSetAlpha )
	{
		return __BufferedPaintSetAlpha( hBufferedPaint, prc, alpha );
	}
	return S_FALSE;
}

HRESULT _BufferedPaintClear(HPAINTBUFFER hBufferedPaint,const RECT *prc)
{
	if( __BufferedPaintClear )
	{
		return __BufferedPaintClear( hBufferedPaint, prc );
	}
	return S_FALSE;
}

HRESULT _BufferedPaintUnInit()
{
	if( __BufferedPaintUnInit )
	{
		return __BufferedPaintUnInit();
	}
	return S_FALSE;
}

HRESULT _BufferedPaintInit()
{
	if( __BufferedPaintInit )
	{
		return __BufferedPaintInit();
	}
	return S_FALSE;
}

HRESULT _GetThemeBitmap(HTHEME hTheme,
    int iPartId,
    int iStateId,
    int iPropId,
    ULONG dwFlags,
    HBITMAP *phBitmap
)
{
	if( __GetThemeBitmap )
	{
		return __GetThemeBitmap( hTheme, iPartId, iStateId, iPropId, dwFlags, phBitmap );
	}
	return S_FALSE;
}

HRESULT _DrawThemeTextEx(          HTHEME hTheme,
    HDC hdc,
    int iPartId,
    int iStateId,
    LPCWSTR pszText,
    int iCharCount,
    DWORD dwFlags,
    LPRECT pRect,
    const DTTOPTS *pOptions
)
{
	if( __DrawThemeTextEx )
	{
		return __DrawThemeTextEx(hTheme, hdc, iPartId, iStateId, pszText, iCharCount, dwFlags, pRect, pOptions );
	}else
	{
		return DrawThemeText( hTheme, hdc, iPartId,iStateId,pszText,iCharCount, dwFlags, pOptions != NULL ? pOptions->dwFlags : 0, pRect );
	}
}
