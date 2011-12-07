#ifndef __GINGKO_REMOTE_H__
#define __GINGKO_REMOTE_H__

BOOL Initialization();

BOOL ObtainSeDebugPrivilege();

ULONG Inject(HANDLE    hProcess,       // Remote process handle
               DWORD     ProcessFlags,   // ProcessFlags returned by GetProcessInfo()
               LPCWSTR   szDllPath,      // Path of Dll to load
               DWORD     dwTimeout,      // Timeout value
               HINSTANCE *hRemoteDll);    // Return handle of loaded Dll

ULONG Eject(HANDLE     hProcess,       // Remote process handle
              DWORD      ProcessFlags,   // ProcessFlags returned by GetProcessInfo()
              LPCTSTR     szDllPath,      // Path of Dll to unload
              HINSTANCE  hRemoteDll,     // Dll handle
              DWORD      dwTimeout);      // Timeout value

BOOL IsProcessWow32( HANDLE hProcess );

DWORD GetProcessInfo(HANDLE  hProcess);

#endif
