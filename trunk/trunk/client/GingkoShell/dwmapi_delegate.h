#ifndef __DWMAPI_DYNAMICAL_H__
#define __DWMAPI_DYNAMICAL_H__
#include <uxtheme.h>
#include <dwmapi.h>
BOOL FreeDWMTheme();
BOOL InitDWMTheme();

BOOL IsCompositionEnabled();

HRESULT _DwmIsCompositionEnabled(BOOL *pfEnabled);
HRESULT _DwmExtendFrameIntoClientArea(HWND hWnd,const MARGINS *pMarInset);
HRESULT _DwmDefWindowProc(HWND hwnd, UINT msg,WPARAM wParam, LPARAM lParam, LRESULT *plResult );
HRESULT _DwmEnableBlurBehindWindow(HWND hWnd,const DWM_BLURBEHIND *pBlurBehind);
HRESULT _DwmEnableComposition( UINT uCompositionAction );
HRESULT _DwmEnableMMCSS(BOOL fEnableMMCSS);
HRESULT _DwmGetColorizationColor( DWORD *pcrColorization, BOOL *pfOpaqueBlend);
HRESULT _DwmGetCompositionTimingInfo( HWND hwnd, DWM_TIMING_INFO *pTimingInfo);
HRESULT _DwmGetWindowAttribute(HWND hwnd,DWORD dwAttribute,PVOID pvAttribute,DWORD cbAttribute);
HRESULT _DwmModifyPreviousDxFrameDuration(HWND hwnd,INT cRefreshes,BOOL fRelative);
HRESULT _DwmQueryThumbnailSourceSize(HTHUMBNAIL hThumbnail,PSIZE pSize);
HRESULT _DwmRegisterThumbnail(HWND hwndDestination,HWND *hwndSource,PHTHUMBNAIL phThumbnailId);
HRESULT _DwmSetDxFrameDuration(HWND hwnd, INT cRefreshes);
HRESULT _DwmSetPresentParameters(HWND hwnd,DWM_PRESENT_PARAMETERS *pPresentParams);
HRESULT _DwmSetWindowAttribute(HWND hwnd,DWORD dwAttribute,LPCVOID pvAttribute,DWORD cbAttribute);
HRESULT _DwmUnregisterThumbnail(HTHUMBNAIL hThumbnailId);
HRESULT _DwmUpdateThumbnailProperties(HTHUMBNAIL hThumbnailId, const DWM_THUMBNAIL_PROPERTIES *ptnProperties);

///UxTheme.dll
HRESULT _EndBufferedPaint(HPAINTBUFFER hBufferedPaint, BOOL fUpdateTarget);
HPAINTBUFFER _BeginBufferedPaint(HDC hdcTarget, const RECT *prcTarget, BP_BUFFERFORMAT dwFormat, BP_PAINTPARAMS *pPaintParams, HDC *phdc);
HRESULT _BufferedPaintSetAlpha(HPAINTBUFFER hBufferedPaint,const RECT *prc,BYTE alpha);
HRESULT _BufferedPaintClear(HPAINTBUFFER hBufferedPaint,const RECT *prc);
HRESULT _BufferedPaintUnInit(VOID);
HRESULT _BufferedPaintInit(VOID);
HRESULT _GetThemeBitmap(HTHEME hTheme,int iPartId,int iStateId,int iPropId,ULONG dwFlags,HBITMAP *phBitmap);
HRESULT _DrawThemeTextEx(HTHEME hTheme, HDC hdc, int iPartId,int iStateId,LPCWSTR pszText,int iCharCount,DWORD dwFlags,LPRECT pRect,const DTTOPTS *pOptions);

//#define BeginBufferedAnimation			_BeginBufferedAnimation        
//#define BeginBufferedPaint              _BeginBufferedPaint            
//#define BeginPanningFeedback            _BeginPanningFeedback          
//#define BufferedPaintClear              _BufferedPaintClear            
//#define BufferedPaintInit               _BufferedPaintInit             
//#define BufferedPaintRenderAnimation    _BufferedPaintRenderAnimation  
//#define BufferedPaintSetAlpha           _BufferedPaintSetAlpha         
//#define BufferedPaintStopAllAnimations  _BufferedPaintStopAllAnimations
//#define BufferedPaintUnInit             _BufferedPaintUnInit           
//#define DrawThemeParentBackgroundEx     _DrawThemeParentBackgroundEx   
//#define DrawThemeTextEx                 _DrawThemeTextEx               
//#define EndBufferedAnimation            _EndBufferedAnimation          
//#define EndBufferedPaint                _EndBufferedPaint              
//#define EndPanningFeedback              _EndPanningFeedback            
//#define GetBufferedPaintBits            _GetBufferedPaintBits          
//#define GetBufferedPaintDC              _GetBufferedPaintDC            
//#define GetBufferedPaintTargetDC        _GetBufferedPaintTargetDC      
//#define GetBufferedPaintTargetRect      _GetBufferedPaintTargetRect    
//#define GetThemeBitmap                  _GetThemeBitmap                
//#define GetThemeStream                  _GetThemeStream                
//#define GetThemeTransitionDuration      _GetThemeTransitionDuration    
//#define IsCompositionActive             _IsCompositionActive           
//#define SetWindowThemeAttribute         _SetWindowThemeAttribute

#define EndBufferedPaint		_EndBufferedPaint
#define BeginBufferedPaint		_BeginBufferedPaint
#define BufferedPaintSetAlpha	_BufferedPaintSetAlpha
#define BufferedPaintClear		_BufferedPaintClear
#define	BufferedPaintUnInit		_BufferedPaintUnInit
#define	BufferedPaintInit		_BufferedPaintInit
#define GetThemeBitmap			_GetThemeBitmap
#define DrawThemeTextEx			_DrawThemeTextEx

#endif 