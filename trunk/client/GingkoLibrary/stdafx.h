// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <atlbase.h>
#include <atlstr.h>
#include <atltypes.h>


// TODO: reference additional headers your program requires here

extern FILE* hLogFile;
extern HANDLE hLogMutex;
#ifdef _DEBUG
#define LOG_START()	hLogMutex = CreateMutex( NULL, FALSE, NULL );

#define LOG( ... )  WaitForSingleObject( hLogMutex, INFINITE );					\
					fopen_s( &hLogFile, "C:\\GingkoLogs\\gingko_library.log", "a+" );	\
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
