// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

///#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#define _WTL_NO_UNION_CLASSES
#define _WTL_NO_CSTRING
#define _WTL_NO_WTYPES
#define _WTL_NEW_PAGE_NOTIFY_HANDLERS

#include <atlbase.h>
#include <atlstr.h>
#include <atltypes.h>

#include <atlwin.h>
#include <atlcoll.h>
#include "dwmapi_delegate.h"
#include <atlapp.h>

extern CAppModule _Module;
#include <atlimage.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>
#include <atlctrlx.h>
#include <atlcrack.h>
#include <atlmisc.h>
#include <atlddx.h>
#include <atlcom.h>
#include <atlhost.h>
#include <ATLComTime.h>	// Date and Time classes
#include <atlprint.h>
#include <atlwin.h>
#include <atlimage.h>
#include <atlframe.h>
#include <atldlgs.h>
#include <atlctrls.h>
#include <atlctrlw.h>
#include <atlsplit.h>
#include <atlctrlx.h>
#include <dwmapi.h>
#include <ResizableLib.h>
#include "gingko_def.h"
#include <gdiplus.h>
using namespace Gdiplus;

#include "Extends\Browser.h"

#define END_MSG_MAP_EX	END_MSG_MAP
#include "GingkoLibrary.h"
#include "XmlConfigParse.h"

BOOL GetExecutablePath(CString &szFilepath);
BOOL GetRelatedPath( CString &szFile, CString& szFullPath );
DWORD WINAPI StartThread(LPTHREAD_START_ROUTINE ThreadRoutine, LPVOID pCaller);
void SetSoapConnectionOptions();

#define MESSAGE_WARNING_TITLE  GetGlobalCaption(CAPTION_WARNING_TITLE, _T("Warning"))
#define MESSAGE_ERROR_TITLE  GetGlobalCaption(CAPTION_ERROR_TITLE, _T("Error"))
#define MESSAGE_CONFIRM_TITLE GetGlobalCaption(CAPTION_CONFIRM_TITLE, _T("Confirm"))

#define LOGO_FILENAME		  _T("gingko_logo.png")

extern FILE* hLogFile;
extern HANDLE hLogMutex;
#ifdef _DEBUG
#define LOG_START()	hLogMutex = CreateMutex( NULL, FALSE, NULL );

#define LOG( ... )  WaitForSingleObject( hLogMutex, INFINITE );					\
					fopen_s( &hLogFile, "C:\\GingkoLogs\\gingko_shell.log", "a+" );	\
					if(hLogFile){												\
						fwprintf( hLogFile, __VA_ARGS__ );						\
						fclose( hLogFile );										\
					}															\
					hLogFile = NULL;											\
					ReleaseMutex( hLogMutex );									\
					
#define LOG_SHUTDOWN() CloseHandle( hLogMutex );								\

#else


#define LOG_START()	hLogMutex = CreateMutex( NULL, FALSE, NULL );

#define LOG( ... )  WaitForSingleObject( hLogMutex, INFINITE );					\
					fopen_s( &hLogFile, "C:\\GingkoLogs\\gingko_shell.log", "a+" );	\
					if(hLogFile){												\
						fwprintf( hLogFile, __VA_ARGS__ );						\
						fclose( hLogFile );										\
					}															\
					hLogFile = NULL;											\
					ReleaseMutex( hLogMutex );									\
					
#define LOG_SHUTDOWN() CloseHandle( hLogMutex );								\

//#define LOG_START()	
//#define LOG( ... )  
//#define LOG_SHUTDOWN() 

#endif


#if defined _M_IX86
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
  //#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.VC90.CRT' version='9.0.21022.8' processorArchitecture='x86' publicKeyToken='1fc8b3b9a1e18e3b' language='*'\"")
#elif defined _M_IA64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.VC90.CRT' version='9.0.21022.8' processorArchitecture='ia64' publicKeyToken='1fc8b3b9a1e18e3b' language='*'\"")
#elif defined _M_X64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.VC90.CRT' version='9.0.21022.8' processorArchitecture='amd64' publicKeyToken='1fc8b3b9a1e18e3b' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.VC90.CRT' version='9.0.21022.8' processorArchitecture='*' publicKeyToken='1fc8b3b9a1e18e3b' language='*'\"")
#endif
