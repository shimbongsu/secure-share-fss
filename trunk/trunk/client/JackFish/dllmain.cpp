// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "JackFish.h"

extern BYTE *pBridgeBuffer; // Here are going to be stored all the bridges
FILE* hLogFile = NULL;
HANDLE hLogMutex = NULL;

LPTOP_LEVEL_EXCEPTION_FILTER __gTopExceptionHandler;

LONG WINAPI JackUnhandledExceptionFilter( struct _EXCEPTION_POINTERS* ExceptionInfo );

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		{
			pBridgeBuffer = (BYTE *) VirtualAlloc(NULL, MAX_HOOKS * (JUMP_WORST * 3),
				MEM_COMMIT, PAGE_EXECUTE_READWRITE);
			LOG_START()
			BOOL bJack = JackHookBoost();
			if( bJack )
			{
				__gTopExceptionHandler = SetUnhandledExceptionFilter( JackUnhandledExceptionFilter );
			}
			return bJack;
		}
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		{
			__try{
				JackUnHook();
				LOG_SHUTDOWN();
				NotifyProcessExit();
			}__finally{
				if( pBridgeBuffer != NULL )
				{
					VirtualFree( pBridgeBuffer, 0, MEM_RELEASE);
				}

				if( __gTopExceptionHandler != NULL )
				{
					SetUnhandledExceptionFilter( __gTopExceptionHandler );
				}
			}
		}
		break;
	}

	return TRUE;
}


LONG WINAPI JackUnhandledExceptionFilter(	struct _EXCEPTION_POINTERS* ExceptionInfo )
{
	if( ExceptionInfo != NULL )
	{
		if( ExceptionInfo->ExceptionRecord != NULL )
		{
			if( ExceptionInfo->ExceptionRecord->ExceptionFlags & EXCEPTION_NONCONTINUABLE )
			{
				return EXCEPTION_CONTINUE_EXECUTION;
			}
		}

		if( __gTopExceptionHandler != NULL )
		{
			return __gTopExceptionHandler( ExceptionInfo );
		}
	}
	return EXCEPTION_CONTINUE_SEARCH;
}