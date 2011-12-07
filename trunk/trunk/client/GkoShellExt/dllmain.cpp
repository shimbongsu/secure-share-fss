// dllmain.cpp : Implementation of DllMain.

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "GkoShellExt_i.h"
#include "CGingkoContextMenuExt.h"

//CGkoShellExtModule _AtlModule;

CComModule _AtlModule;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_CGingkoContextMenuExt, CCGingkoContextMenuExt)
END_OBJECT_MAP()

HBITMAP  __g_hBitmap = NULL;
// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
		InitGingkoLibrary();
		
		__g_hBitmap = LoadBitmap( hInstance, 
			MAKEINTRESOURCE(IDB_GKOSHELLEXT) );
		
        _AtlModule.Init(ObjectMap, hInstance, &LIBID_GkoShellExtLib);
        
		DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
	{
		FreeGingkoLibrary();
		if( __g_hBitmap )
		{
			DeleteObject( __g_hBitmap );
		}
        _AtlModule.Term();
	}
    return TRUE;    // ok
}
