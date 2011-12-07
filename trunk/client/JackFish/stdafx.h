// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>


#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#include <atlbase.h>
#include <atlstr.h>

extern FILE* hLogFile;
extern HANDLE hLogMutex;
#ifdef _DEBUG
#define LOG_START()	hLogMutex = CreateMutex( NULL, FALSE, NULL );
#define LOG( ... )  WaitForSingleObject( hLogMutex, INFINITE );					\
					fopen_s( &hLogFile, "C:\\GingkoLogs\\gingko_jackfish.log", "a+" );	\
					if(hLogFile){												\
						fwprintf( hLogFile, __VA_ARGS__ );						\
						fclose( hLogFile );										\
					}															\
					hLogFile = NULL;											\
					ReleaseMutex( hLogMutex );									\
					
#define LOG_SHUTDOWN() CloseHandle( hLogMutex );								\

#else

#define LOG_START()	
#define LOG( ... )  
#define LOG_SHUTDOWN() 

#endif

// TODO: reference additional headers your program requires here
