#ifndef __DLL_GINGKO_SHELL_EXT_MAIN_H__
#define __DLL_GINGKO_SHELL_EXT_MAIN_H__
#include <winerror.h>

//class CGkoShellExtModule : public CAtlDllModuleT< CGkoShellExtModule >
//{
//public :
//	DECLARE_LIBID(LIBID_GkoShellExtLib)
//	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_GKOSHELLEXT, "{15392719-4F64-41D7-82FE-A719A788AB98}")
//public:
//	HINSTANCE		m_Instance;
//
//	CGkoShellExtModule()
//	{
//		m_Instance = NULL;
//	}
//};

//extern class CComModule _AtlModule; // _AtlModule;

BOOL InitGingkoLibrary();
BOOL FreeGingkoLibrary();

#endif

