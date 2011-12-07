#include "stdafx.h"
#include <winspool.h>
#include "JackFish.h"


typedef HANDLE (WINAPI *FuncSetClipboardData)(UINT, HANDLE);

typedef BOOL (WINAPI *FuncOpenPrinterA)(LPSTR pPrinterName, 
										 LPHANDLE phPrinter,
										 LPPRINTER_DEFAULTSA pDefault);

typedef BOOL (WINAPI *FuncOpenPrinterW)(LPWSTR pPrinterName, 
										 LPHANDLE phPrinter,
										 LPPRINTER_DEFAULTSW pDefault);

typedef BOOL (WINAPI *FuncOpenPrinter2A)(LPCSTR pPrinterName,
										  LPHANDLE phPrinter,
										  PPRINTER_DEFAULTSA pDefault,
										  PPRINTER_OPTIONSA pOptions);

typedef BOOL (WINAPI *FuncOpenPrinter2W)(LPCWSTR pPrinterName,         
										  LPHANDLE phPrinter,
										  PPRINTER_DEFAULTSW pDefault,
										  PPRINTER_OPTIONSW pOptions);

typedef DWORD (WINAPI *FuncStartDocPrinterA)(
										  HANDLE hPrinter,  // handle to printer object
										  DWORD Level,      // information level
										  LPBYTE pDocInfo );  // information buffer

typedef DWORD (WINAPI *FuncStartDocPrinterW)(
										  HANDLE hPrinter,  // handle to printer object
										  DWORD Level,      // information level
										  LPBYTE pDocInfo );  // information buffer

typedef BOOL (WINAPI *FuncEmptyClipboard)(void);

FuncStartDocPrinterA pStartDocPrinterA = NULL;
FuncStartDocPrinterW pStartDocPrinterW = NULL;
FuncEmptyClipboard   __EmptyClipboard = NULL;
FuncSetClipboardData pSetClipboardData = NULL;
FuncOpenPrinter2W pOpenPrinter2W = NULL;
FuncOpenPrinter2A pOpenPrinter2A = NULL;
FuncOpenPrinterW pOpenPrinterW = NULL;
FuncOpenPrinterA pOpenPrinterA = NULL;

HMODULE hUser32 = NULL;
HMODULE hPrintspool = NULL;

DWORD WINAPI JackStartDocPrinterA(
  HANDLE hPrinter,  // handle to printer object
  DWORD Level,      // information level
  LPBYTE pDocInfo   // information buffer
)
{


	ULONG Perms = GetProcessPermission( GetCurrentProcessId() );
	if( (Perms & 0x80000F00) != 0x80000F00 )
	{
		return 0L;
	}

	if( pStartDocPrinterA == NULL )
	{
		return 0L;
	}

	return pStartDocPrinterA( hPrinter, Level, pDocInfo );
}

DWORD WINAPI JackStartDocPrinterW(
  HANDLE hPrinter,  // handle to printer object
  DWORD Level,      // information level
  LPBYTE pDocInfo   // information buffer
)
{


	ULONG Perms = GetProcessPermission( GetCurrentProcessId() );

	if( (Perms & 0x80000F00) != 0x80000F00 )
	{
		return 0L;
	}

	if( pStartDocPrinterW == NULL )
	{
		return 0L;
	}

	return pStartDocPrinterW( hPrinter, Level, pDocInfo );
}

HANDLE WINAPI JackSetClipboardData(
  UINT uFormat, 
  HANDLE hMem
)
{
	LOG( L"Calling JackSetClipboardData function.\n" );
	if( pSetClipboardData == NULL )
	{
		LOG( L"Calling pSetClipboardData function is NULL.\n" );
		return NULL;
	}
	
	HANDLE hData = NULL;
	
	if( __EmptyClipboard != NULL )
	{
		hData = pSetClipboardData( uFormat, hMem );
	}

	LOG( L"Calling pSetClipboardData function return: %08x.\n", hData );
	ULONG Perms = GetProcessPermission( GetCurrentProcessId() );
	LOG( L"Calling GetProcessPermission function return: %08x.\n", Perms );
	if( (Perms & 0x8FF00000) != 0x8FF00000 )
	{
		LOG( L"Calling Permission Is Not allowed, clear clipboard by calling EmptyClipboard ");
		if( __EmptyClipboard != NULL )
		{
			__EmptyClipboard();
		}
		//return NULL;  ///If the Permission has not owner and holder permission, the process's Copy/Paste function will be disabled.
	}
	return hData;
}




BOOL JackHookBoost()
{
	hUser32 = LoadLibrary(_T("User32.dll"));

	if( hUser32 == NULL )
	{
		return FALSE;
	}

	hPrintspool = LoadLibrary(_T("WINSPOOL.DRV"));
	if( hPrintspool == NULL )
	{
		FreeLibrary( hUser32 );
		return FALSE;
	}

	HookFunction((ULONG_PTR) GetProcAddress( hUser32,
		"SetClipboardData"), 
		(ULONG_PTR) &JackSetClipboardData);

	HookFunction((ULONG_PTR) GetProcAddress(hPrintspool,
		"StartDocPrinterA"), 
		(ULONG_PTR) &JackStartDocPrinterA);

	HookFunction((ULONG_PTR) GetProcAddress(hPrintspool,
		"StartDocPrinterW"), 
		(ULONG_PTR) &JackStartDocPrinterW);

	__EmptyClipboard = (FuncEmptyClipboard) GetProcAddress(hUser32, "EmptyClipboard");

	if( pStartDocPrinterA == NULL )
	{
		pStartDocPrinterA = (FuncStartDocPrinterA) GetOriginalFunction((ULONG_PTR) JackStartDocPrinterA);
	}
	if( pStartDocPrinterW == NULL )
	{
		pStartDocPrinterW = (FuncStartDocPrinterW) GetOriginalFunction((ULONG_PTR) JackStartDocPrinterW);
	}

	if( pSetClipboardData == NULL )
	{
		pSetClipboardData = (FuncSetClipboardData) GetOriginalFunction((ULONG_PTR) JackSetClipboardData);
	}
	return TRUE;
}

BOOL JackUnHook()
{
	UnhookFunction((ULONG_PTR) GetProcAddress( hUser32, "SetClipboardData"));
	UnhookFunction((ULONG_PTR) GetProcAddress(hPrintspool, "StartDocPrinterA")) ;
	UnhookFunction((ULONG_PTR) GetProcAddress(hPrintspool, "StartDocPrinterW")) ;
	FreeLibrary( hUser32 );
	FreeLibrary( hPrintspool );
	return TRUE;
}


