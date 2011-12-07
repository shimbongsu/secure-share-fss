#include "stdafx.h"
#include "GingkoInjection.h"

LDRSHUTDOWNTHREAD           LdrShutdownThread = NULL;
RTLNTSTATUSTODOSERROR       RtlNtStatusToDosError = NULL;
RTLCREATEUSERTHREAD         RtlCreateUserThread = NULL;
NTALLOCATEVIRTUALMEMORY     NtAllocateVirtualMemory = NULL;
NTFREEVIRTUALMEMORY         NtFreeVirtualMemory = NULL;
NTOPENTHREAD                NtOpenThread = NULL;
NTQUERYINFORMATIONPROCESS   NtQueryInformationProcess = NULL;
NTQUERYINFORMATIONTHREAD    NtQueryInformationThread = NULL;
NTQUERYSYSTEMINFORMATION    NtQuerySystemInformation = NULL;
NTQUEUEAPCTHREAD            NtQueueApcThread = NULL;
NTTERMINATETHREAD           NtTerminateThread = NULL;
LPFN_ISWOW64PROCESS			_IsWow64Process = NULL;

static DWORD WINAPI RemoteInjectDll( RemoteInjectData* pData )
{

	pData->hRemoteDll = pData->LoadLibrary( pData->szDllPath );

    if ( pData->hRemoteDll == NULL )
        pData->Result = -1;
    else
        pData->Result = 0; // 0 = OK

    return pData->Result;

}

/*******************************************************
 * Return a TID (thread id) for the specified process. *
 * (use the Toolhelp functions)                        *
 *******************************************************/
DWORD GetProcessThreadToolhelp(DWORD dwPID)
{
  HANDLE        hSnapshot = INVALID_HANDLE_VALUE;
  THREADENTRY32 te;
  HANDLE        hThread;
  DWORD         TID = 0;
  CONTEXT       c = {CONTEXT_CONTROL | CONTEXT_INTEGER};

  hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, dwPID);
  if (hSnapshot == INVALID_HANDLE_VALUE)
      return 0;

  te.dwSize = sizeof(THREADENTRY32);

  if (!Thread32First(hSnapshot, &te))
  {
      CloseHandle(hSnapshot);
      return 0;
  }

  do
  {
      if (te.th32OwnerProcessID == dwPID)
      {
          TID = te.th32ThreadID;

          if (!(hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, TID)))
              continue;

          CloseHandle(hThread);
          CloseHandle(hSnapshot);
          return TID;
      }
  } while (Thread32Next(hSnapshot, &te));

  CloseHandle(hSnapshot);
  return 0;
}

BOOL IsProcessWow32( HANDLE hProcess )
{
	BOOL IsWow32 = TRUE;
	if( _IsWow64Process != NULL )
	{
		_IsWow64Process( hProcess, &IsWow32 );
	}
	return IsWow32;
}


/////////////////////////////////// GetProcessInfo() ///////////////////////////////////

/*****************************************************************
 * GetProcessInfo()                                              *
 *                                                               *
 * Return info about a running process.                          *
 * The returned DWORD consists of two parts:                     *
 * The HIWORD contains the process subsystem:                    *
 *  0 = IMAGE_SUBSYSTEM_UNKNOWN        (unknown process type)    *
 *  1 = IMAGE_SUBSYSTEM_NATIVE         (native process)          *
 *  2 = IMAGE_SUBSYSTEM_WINDOWS_GUI    (GUI process)             *
 *  3 = IMAGE_SUBSYSTEM_WINDOWS_CUI    (character mode process)  *
 *  5 = IMAGE_SUBSYSTEM_OS2_CUI        (OS/2 character process)  *
 *  7 = IMAGE_SUBSYSTEM_POSIX_CUI      (Posix character process) *
 *  8 = IMAGE_SUBSYSTEM_NATIVE_WINDOWS (Win9x driver)            *
 *  9 = IMAGE_SUBSYSTEM_WINDOWS_CE_GUI (Windows CE process)      *
 * 14 = IMAGE_SUBSYSTEM_XBOX           (XBox system)             *
 * The LOWORD contains one or more flags:                        *
 *  1 = fWIN9X           (Win 9x process)                        *
 *  2 = fWINNT           (Win NT process)                        *
 *  4 = fINVALID         (invalid process)                       *
 *  8 = fDEBUGGED        (process is being debugged)             *
 * 16 = fNOTINITIALIZED  (process didn't finished initialization)*
 * In case of error HIWORD=Error Code and LOWORD=-1              *
 *****************************************************************/
DWORD GetProcessInfo(HANDLE  hProcess)
{
    WORD                        ProcessFlags = 0;	// Initialize to zero

    NTSTATUS                    Status;
    DWORD_PTR                   DebugPort;
    PROCESS_BASIC_INFORMATION   pbi;
    PPEB_NT                     pPEB;
    PEB_NT                      PEB;

    __try
    {
        /***** Win NT *****/
        // Assume Win NT process
        ProcessFlags |= fWINNT;

        //// Get process handle
        //if (!(hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwPID)))
        //    return MAKELONG(-1, ERROR_OPENPROCESS);

        // Get Debug port status
        Status = NtQueryInformationProcess(hProcess,
                                           ProcessDebugPort,
                                           &DebugPort,
                                           sizeof(DebugPort),
                                           NULL);

        if (!NT_SUCCESS(Status))
            return MAKELONG(-1, ERROR_NTQUERYINFORMATIONPROCESS);

        // Process is being debugged
        if (DebugPort)
            ProcessFlags |= fDEBUGGED;

        // Get PEB base address of process
        Status = NtQueryInformationProcess(hProcess,
                                           ProcessBasicInformation,
                                           &pbi,
                                           sizeof(pbi),
                                           NULL);

        if (!NT_SUCCESS(Status))
            return MAKELONG(-1, ERROR_NTQUERYINFORMATIONPROCESS);

        // Exit status must be 0x103
        if (pbi.ExitStatus != 0x103)
            ProcessFlags |= fINVALID;

        // Read PEB
        // (for local process this is the same as FS:[0x30])
        pPEB = pbi.PebBaseAddress;
        if (pPEB == NULL)
        {
            ProcessFlags |= fINVALID;
            return MAKELONG(ProcessFlags, IMAGE_SUBSYSTEM_NATIVE);
        }
        else
        {
            if (!ReadProcessMemory(hProcess, pPEB, &PEB, sizeof(PEB), NULL))
                return MAKELONG(-1, ERROR_READPROCESSMEMORY);

            // Process is being debugged
            if (PEB.BeingDebugged)
                ProcessFlags |= fDEBUGGED;

            // Process not yet initialized
			
            if (!PEB.LdrData || !PEB.LoaderLock)
                ProcessFlags |= fNOTINITIALIZED;
        }

        //CloseHandle(hProcess);

        // Return Subsystem + Process Flags
        return MAKELONG(ProcessFlags, PEB.ImageSubsystem);
    }
    // Exception ocurred
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return MAKELONG(-1, ERROR_EXCEPTION);
    }
}


/////////////////////////// RemoteExecute() ////////////////////////////

/****************************************
 * InitializeAndPatchStub()             *
 *                                      *
 * Patches remote stub data at runtime. *
 ****************************************/
int InitializeAndPatchStub(HANDLE hProcess, PBYTE pCode, OFFSETS offs, DWORD UserFunc, DWORD Native)
{
    SIZE_T   nBytesWritten = 0;
    BOOL    fFinished = FALSE;

    if (!WriteProcessMemory(hProcess, pCode + offs.PUserFunc, &UserFunc, sizeof(UserFunc), &nBytesWritten) ||
        nBytesWritten != sizeof(UserFunc))
        return -1;
    if (!WriteProcessMemory(hProcess, pCode + offs.PLdrShutdownThread, &LdrShutdownThread,
        sizeof(LdrShutdownThread), &nBytesWritten) || nBytesWritten != sizeof(LdrShutdownThread))
        return -1;
    if (!WriteProcessMemory(hProcess, pCode + offs.PNtFreeVirtualMemory, &NtFreeVirtualMemory,
        sizeof(NtFreeVirtualMemory), &nBytesWritten) || nBytesWritten != sizeof(NtFreeVirtualMemory))
        return -1;
    if (!WriteProcessMemory(hProcess, pCode + offs.PNtTerminateThread, &NtTerminateThread,
        sizeof(NtTerminateThread), &nBytesWritten) || nBytesWritten != sizeof(NtTerminateThread))
        return -1;
    if (!WriteProcessMemory(hProcess, pCode + offs.PNative, &Native, sizeof(Native), &nBytesWritten) ||
        nBytesWritten != sizeof(Native))
        return -1;
    if (!WriteProcessMemory(hProcess, pCode + offs.PFinished, &fFinished, sizeof(fFinished), &nBytesWritten) ||
        nBytesWritten != sizeof(fFinished))
        return -1;
    return 0;
}

/****************************************************
 * RemoteExecute()                                  *
 *                                                  *
 * Execute code in the context of a remote process. *
 * Return zero if everything went ok or error code. *
 ****************************************************/
ULONG RemoteExecute(HANDLE hRemoteProcess,                      // Remote process handle
                  DWORD  ProcessFlags,                  // ProcessFlags returned by GetProcessInfo()
                  LPTHREAD_START_ROUTINE Function,      // Remote thread function
                  PVOID  pData,                         // User data passed to remote thread
                  DWORD  Size,                          // Size of user data block (0=treat pData as DWORD)
                  DWORD  dwTimeout,                     // Timeout value
				  BOOL   blIgnoreInit,                     // Ignore the INIT status
                  ULONG_PTR *ExitCode)                      // Return exit code from remote code
{
    PBYTE       pStubCode = NULL;
    PBYTE       pRemoteCode = NULL;
    PBYTE       pRemoteData = NULL;
    PVOID       pParams = NULL;
    SIZE_T       nBytesWritten = 0, nBytesRead = 0;
    HANDLE      hThread = NULL;
    DWORD       dwThreadId;
    ULONG_PTR   dwExitCode = -1;
    int         ErrorCode = 0;
    DWORD       dwCreationFlags = 0; // dwCreationFlags parameter for CreateRemoteThread()
    NTSTATUS    Status;
    BOOL        fNative;
    BOOL        fFinished;
    DWORD       dwTmpTimeout = 100;	// 100 ms
    //OFFSETS     StubOffs;

    __try
    {
        __try
        {
            // Initialize ExitCode to -1
            if (ExitCode)
                *ExitCode = -1;

            // ProcessFlags = 0 ?
            if (!ProcessFlags)
                ProcessFlags = GetProcessInfo(hRemoteProcess);

            // Invalid Process flags
            if (ProcessFlags & fINVALID)
            {
                ErrorCode = ERROR_INVALIDPROCESS;
                __leave;
            }

			// Get ASM code offsets
            // GetOffsets(&StubOffs);

            // Check if function code is safe to be relocated
            //if (IsCodeSafe((PBYTE)Function, &FunctionSize) != 0)
            //{
            //    ErrorCode = ERROR_ISCODESAFE;
            //    __leave;
            //}

			SetLastError( 0 );

			//LOG( _T("Current ERROR CODE: %d.\n"), GetLastError() );
            // Allocate memory for function in remote process

    //        if (!(pRemoteCode = (PBYTE)VirtualAllocEx(hProcess, NULL, FunctionSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE )))
    //        {
				//DWORD dwError = GetLastError();
				//LOG( _T("VirtualAllocEx ERROR CODE: %d.\n"), dwError );
    //            ErrorCode = ERROR_VIRTUALALLOCEX;
    //            __leave;
    //        }

            // Copy function code to remote process
    //        if (!WriteProcessMemory(hProcess, pRemoteCode, Function, FunctionSize, &nBytesWritten) ||
    //            nBytesWritten != FunctionSize)
    //        {
				//LOG( _T("WriteProcessMemory ERROR CODE: %d.\n"), GetLastError() );
    //            ErrorCode = ERROR_WRITEPROCESSMEMORY;
    //            __leave;
    //        }

            // Data block specified ?
            if (pData && Size)
            {
                // Allocate memory for data block in remote process
                if (!(pRemoteData = (PBYTE)VirtualAllocEx(hRemoteProcess, 0, Size, MEM_COMMIT, PAGE_READWRITE)))
                {
					LOG( _T("VirtualAllocEx ERROR CODE: %d.\n"), GetLastError() );
                    ErrorCode = ERROR_VIRTUALALLOCEX;
                    __leave;
                }

                // Copy data block to remote process
                if (!WriteProcessMemory(hRemoteProcess, pRemoteData, pData, Size, &nBytesWritten) || nBytesWritten != Size)
                {
					LOG( _T("WriteProcessMemory ERROR CODE: %d.\n"), GetLastError() );
                    ErrorCode = ERROR_WRITEPROCESSMEMORY;
                    __leave;
                }

                pParams = pRemoteData;
            }
            // Pass value directly to CreateThread()
            else
                pParams = pData;

			pStubCode = (PBYTE)Function;
            // Size of stub code
    //        FunctionSize = StubOffs.StubSize;

    //        // Allocate memory for stub code in remote process
    //        if (!(pStubCode = (PBYTE)VirtualAllocEx(hProcess, 0, FunctionSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE)))
    //        {
				//LOG( _T("VirtualAllocEx ERROR CODE: %d.\n"), GetLastError() );
    //            ErrorCode = ERROR_VIRTUALALLOCEX;
    //            __leave;
    //        }

    //        // Copy stub code to remote process
    //        if (!WriteProcessMemory(hProcess, pStubCode, (LPVOID)StubOffs.StubStart,
    //             FunctionSize, &nBytesWritten) || nBytesWritten != FunctionSize)
    //        {
				//LOG( _T("WriteProcessMemory ERROR CODE: %d.\n"), GetLastError() );
    //            ErrorCode = ERROR_WRITEPROCESSMEMORY;
    //            __leave;
    //        }

			// NT native process requires a different stub exit code
            fNative = ((HIWORD(ProcessFlags) == IMAGE_SUBSYSTEM_NATIVE) && (ProcessFlags & fWINNT));

            // Patch Stub data
            //if (InitializeAndPatchStub(hProcess, pStubCode, StubOffs, (DWORD)pRemoteCode, fNative) != 0)
            //{
            //    ErrorCode = ERROR_PATCH;
            //    __leave;
            //}

            // Process not initialized
            if ( (ProcessFlags & fNOTINITIALIZED) && (!blIgnoreInit) )
            {
				// WinNT
                if ( ProcessFlags & fWINNT )
                {
                    if ( !(dwThreadId = GetProcessThreadToolhelp( GetProcessId( hRemoteProcess ) ) ) )
                    {
                        ProcessFlags |= ~fNOTINITIALIZED;
                        goto Initialized;					// Try initialized
                    }

                    if (!(hThread = OpenThread(THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME, FALSE, dwThreadId)))
                    {
                        ProcessFlags |= ~fNOTINITIALIZED;
                        goto Initialized;					// Try initialized
                    }

                    Status = NtQueueApcThread(hThread,                      // hThread
                                              (PKNORMAL_ROUTINE)pStubCode,  // APC Routine
                                              pParams,                      // Argument 1
                                              NULL,                         // Argument 2
                                              NULL);                        // Argument 3

                    if (!NT_SUCCESS(Status))
                    {
                        ProcessFlags |= ~fNOTINITIALIZED;
                        goto Initialized;					// Try initialized
                    }

					DWORD dwError = 0;

					DWORD dwRet = ResumeThread( hThread );

					if( dwRet == (DWORD)-1 )
					{
						dwError = GetLastError();
						dwError = 0;
					}

                    // Wait for remote code to finish
                    dwTmpTimeout = min(dwTmpTimeout, dwTimeout);
                    for (fFinished = FALSE; !fFinished && dwTimeout != 0; dwTimeout -= min(dwTmpTimeout, dwTimeout))
                    {
                        WaitForSingleObject(GetCurrentThread(), dwTmpTimeout);
       //                 if (!ReadProcessMemory(hProcess, pStubCode + StubOffs.PFinished, &
       //                                        fFinished, sizeof(fFinished), &nBytesRead) || nBytesRead != sizeof(fFinished) )
       //                 {
       //                     ErrorCode = ERROR_READPROCESSMEMORY;
							//dwError = GetLastError();
							//dwError = 0;
       //                     __leave;
       //                 }
                    }

                    // Timeout ocurred
                    if (dwTimeout == 0 && !fFinished)
                        ErrorCode = ERROR_WAITTIMEOUT;

                    // Doesn't make sense to GetExitCodeThread() on a "hijacked" thread !
                    dwExitCode = 0;
                }/*Win NT*/
            }/*Not initialized*/

Initialized:
            // Initialized process
            if ( (!(ProcessFlags & fNOTINITIALIZED)) || (blIgnoreInit) )
            {
                // NT native
                if (fNative)
                {
                    Status = RtlCreateUserThread(hRemoteProcess,      // hProcess
                                                 NULL,          // &SecurityDescriptor
                                                 FALSE,         // CreateSuspended
                                                 0,             // StackZeroBits
                                                 NULL,          // StackReserved
                                                 NULL,          // StackCommit
                                                 pStubCode,     // StartAddress
                                                 pParams,       // StartParameter
                                                 &hThread,      // &hThread
                                                 NULL);         // &ClientId

                    if (!NT_SUCCESS(Status))
                    {
                        SetLastError(RtlNtStatusToDosError(Status));
                        ErrorCode = ERROR_RTLCREATETHREAD;
                        __leave;
                    }
                }
                // Win32 process
                else
                {
                    // Create remote thread
                    hThread = CreateRemoteThread(hRemoteProcess,
                                                  NULL,
                                                  0,
                                                  (LPTHREAD_START_ROUTINE) pStubCode,
                                                  pParams,
                                                  dwCreationFlags,
                                                  &dwThreadId);
                }

                // Error in creating thread
                if (!hThread)
                {
                    ErrorCode = ERROR_CREATETHREAD;
                    __leave;
                }

                // Wait for thread to terminate
                if (WaitForSingleObject(hThread, dwTimeout) != WAIT_OBJECT_0)
                {
                    ErrorCode = ERROR_WAITTIMEOUT;
                    __leave;
                }

                // Get thread exit code
                GetExitCodeThread(hThread, (LPDWORD)&dwExitCode);
            }/*Initialized*/

            // Data block specified ?
			// We don't need this code to read.
            //if (pData && Size)
            //{
            //    // Read back remote data block
            //    if (!ReadProcessMemory(hRemoteProcess, pRemoteData, pData, Size, &nBytesRead) || nBytesRead != Size)
            //    {
            //        ErrorCode = ERROR_READPROCESSMEMORY;
            //        __leave;
            //    }
            //}
        }
        // Cleanup
        __finally
        {
            if (pStubCode)
                VirtualFreeEx(hRemoteProcess, pStubCode, 0, MEM_RELEASE);
            if (pRemoteCode)
                VirtualFreeEx(hRemoteProcess, pRemoteCode, 0, MEM_RELEASE);
            if (pRemoteData)
                VirtualFreeEx(hRemoteProcess, pRemoteData, 0, MEM_RELEASE);
            if (hThread)
                CloseHandle(hThread);
        }
    }
    // Exception ocurred
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return GetExceptionCode() | LOCAL_EXCEPTION;
    }

    // Return remote stub exit code
    if (ExitCode)
        *ExitCode = dwExitCode;

    // Return RemoteExecute() error code or remote code exception code
    if ((ErrorCode == 0) && (dwExitCode & REMOTE_EXCEPTION))
        return (ULONG)dwExitCode;
    else
        return ErrorCode;
}

/**********************************************************
 * Load a Dll into the address space of a remote process. *
 * (Unicode version)                                      *
 **********************************************************/
ULONG Inject(HANDLE    hProcess,       // Remote process handle
               DWORD     ProcessFlags,   // ProcessFlags returned by GetProcessInfo()
               LPCWSTR   szDllPath,      // Path of Dll to load
               DWORD     dwTimeout,      // Timeout value
               HINSTANCE *hRemoteDll)    // Return handle of loaded Dll
{
	ULONG		  rc;
	ULONG		  ErrorCode = 0;
	ULONG_PTR     ExitCode = -1;
	HINSTANCE	  hKernel32 = 0;
	RemoteInjectData  rjd;

    __try
    {
        __try
        {
            // Load Kernel32.dll
			
            if (!(hKernel32 = LoadLibrary( _T("Kernel32.dll") ) ) )
            {
                ErrorCode = ERROR_LOADLIBRARY;
                __leave;
            }

            // Initialize data block passed to RemoteInjectDll()
            rjd.Result = -1;
            rjd.hRemoteDll = NULL;
			//lstrcpyA( rjd.szDllPath, szDllPath );
			lstrcpy( rjd.szDllPath, szDllPath );

            rjd.LoadLibrary = (LOADLIBRARY) GetProcAddress(hKernel32, "LoadLibraryW");

            if ( !rjd.LoadLibrary )
            {
                ErrorCode = ERROR_GETPROCADDRESS;
                __leave;
            }

            // Execute RemoteInjectDll() in remote process
            rc = RemoteExecute(hProcess,
                               ProcessFlags,
							   (LPTHREAD_START_ROUTINE)rjd.LoadLibrary,
							   &rjd.szDllPath,
                               sizeof(rjd.szDllPath),
                               dwTimeout,
							   FALSE,
                               &ExitCode);
        }

        __finally
        {
            if (hKernel32)
                FreeLibrary(hKernel32);
        }
    }
    // Exception ocurred
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return GetExceptionCode() | LOCAL_EXCEPTION;
    }

    // Return handle of loaded dll
    if (hRemoteDll)
        *hRemoteDll = rjd.hRemoteDll;

    // Return error code
    if (ErrorCode == 0 && rc != 0)
        return rc;
    else if (ErrorCode == 0 && ExitCode != 0)
        return ERROR_REMOTE;
    else
        return ErrorCode;

    //return InjectDllA(hProcess, ProcessFlags, DllPath, dwTimeout, hRemoteDll);
}


#pragma check_stack(off)
static DWORD WINAPI RemoteEjectDll(RemoteInjectData* pData)
{
    int i = 0;

    do
    {
        if (pData->szDllPath[0] != '\0')
            pData->hRemoteDll = pData->GetModuleHandle(pData->szDllPath);

        pData->Result = pData->FreeLibrary(pData->hRemoteDll);
        i++;
    } while (pData->Result);

    return (i > 1 ? 0 : -1); // 0 = OK
}


/************************************************************
 * Unload a Dll from the address space of a remote process. *
 * (ANSI version)                                           *
 ************************************************************/
int Eject(HANDLE     hProcess,       // Remote process handle
              DWORD      ProcessFlags,   // ProcessFlags returned by GetProcessInfo()
              LPCTSTR     szDllPath,      // Path of Dll to unload
              HINSTANCE  hRemoteDll,     // Dll handle
              DWORD      dwTimeout)      // Timeout value
{
    int       rc;
    int       ErrorCode = 0;
    ULONG_PTR     ExitCode = -1;
    HINSTANCE hKernel32 = 0;
    RemoteInjectData  rdDll;

    __try
    {
        __try
        {
            // Load Kernel32.dll
            if (!(hKernel32 = LoadLibraryA("Kernel32.dll")))
            {
                ErrorCode = ERROR_LOADLIBRARY;
                __leave;
            }

            // Initialize data block passed to RemoteInjectDll()
            rdDll.Result = -1;
            rdDll.hRemoteDll = hRemoteDll;
            
			if (szDllPath)
				lstrcpy(rdDll.szDllPath, szDllPath);

            rdDll.FreeLibrary = (FREELIBRARY)GetProcAddress(hKernel32, "FreeLibrary");
            rdDll.GetModuleHandle = (GETMODULEHANDLE)GetProcAddress(hKernel32, "GetModuleHandleW");

            if (!rdDll.FreeLibrary || !rdDll.GetModuleHandle)
            {
                ErrorCode = ERROR_GETPROCADDRESS;
                __leave;
            }

            // Execute RemoteEjectDll() in remote process
            rc = RemoteExecute(hProcess,
                               ProcessFlags,
                               (LPTHREAD_START_ROUTINE)RemoteEjectDll,
                               &rdDll,
                               sizeof(rdDll),
                               dwTimeout,
							   TRUE,
                               &ExitCode);
        }

        __finally
        {
            if (hKernel32)
                FreeLibrary(hKernel32);
        }
    }
    // Exception ocurred
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return GetExceptionCode() | LOCAL_EXCEPTION;
    }

    // Return error code
    if (ErrorCode == 0 && rc != 0)
        return rc;
    else if (ErrorCode == 0 && ExitCode != 0)
        return ERROR_REMOTE;
    else
        return ErrorCode;
}


/////////////////////////////////////// Initialization ///////////////////////////////////////////

/**************************************************************
 * Initialize required data depending on the Windows version. *
 **************************************************************/
BOOL Initialization()
{
    OSVERSIONINFO   osvi;
    HMODULE         hNTDLL = NULL, hKernel32 = NULL;
    BOOL            Result = FALSE;
    static BOOL     bInitializing = FALSE;

    // Not reentrant
    if (bInitializing)
        return TRUE;
    bInitializing = TRUE;

    __try
    {
        __try
        {
            // Get Windows version
            osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
            if (!GetVersionEx(&osvi))
                __leave;

            // Save version data in global variables
            /***** Win NT *****/
            if (TRUE)
            {
                if (!(hKernel32 = LoadLibrary(TEXT("Kernel32.dll"))))
                    __leave;

                if (!(hNTDLL = LoadLibrary(TEXT("NTDLL.DLL"))))
                    __leave;

				_IsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress( hKernel32, "IsWow64Process" );

                // Win NT all versions
                if (!(RtlCreateUserThread = (RTLCREATEUSERTHREAD)GetProcAddress(hNTDLL, "RtlCreateUserThread")))
					__leave;
                if (!(NtQueueApcThread = (NTQUEUEAPCTHREAD)GetProcAddress(hNTDLL, "NtQueueApcThread")))
                    __leave;
                if (!(LdrShutdownThread = (LDRSHUTDOWNTHREAD)GetProcAddress(hNTDLL, "LdrShutdownThread")))
                    __leave;
                if (!(NtTerminateThread = (NTTERMINATETHREAD)GetProcAddress(hNTDLL, "NtTerminateThread")))
                    __leave;
                if (!(NtAllocateVirtualMemory = (NTALLOCATEVIRTUALMEMORY)GetProcAddress(hNTDLL, "NtAllocateVirtualMemory")))
                    __leave;
                if (!(NtFreeVirtualMemory = (NTFREEVIRTUALMEMORY)GetProcAddress(hNTDLL, "NtFreeVirtualMemory")))
                    __leave;
                if (!(NtOpenThread = (NTOPENTHREAD)GetProcAddress(hNTDLL, "NtOpenThread")))
                    __leave;
                if (!(RtlNtStatusToDosError = (RTLNTSTATUSTODOSERROR)GetProcAddress(hNTDLL, "RtlNtStatusToDosError")))
                    __leave;
                if (!(NtQuerySystemInformation = (NTQUERYSYSTEMINFORMATION)GetProcAddress(hNTDLL, "NtQuerySystemInformation")))
                    __leave;
                if (!(NtQueryInformationProcess = (NTQUERYINFORMATIONPROCESS)GetProcAddress(hNTDLL, "NtQueryInformationProcess")))
                    __leave;
                if (!(NtQueryInformationThread = (NTQUERYINFORMATIONTHREAD)GetProcAddress(hNTDLL, "NtQueryInformationThread")))
                    __leave;

            }//WinNT

            Result = TRUE;
        }
        __finally
        {
            if (hKernel32)
                FreeLibrary(hKernel32);
            if (hNTDLL)
                FreeLibrary(hNTDLL);

            bInitializing = FALSE;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = FALSE;
    }

    return Result;
}

BOOL ObtainSeDebugPrivilege()
{
	TOKEN_PRIVILEGES TokenPrivileges;
	TOKEN_PRIVILEGES PreviousTokenPrivileges;
	LUID luid = {0, 0};
	HANDLE hToken;
	DWORD dwPreviousTokenPrivilegesSize = sizeof(TOKEN_PRIVILEGES);

	if( ! OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken ) )
	{
		LOG(_T("debug 001"));
		return false;
	}
	
	
	if(!LookupPrivilegeValue(NULL, (SE_DEBUG_NAME), &luid))
	{
		LOG(_T("debug 002"));
		return false;
	}
  
	TokenPrivileges.PrivilegeCount            = 1;
	TokenPrivileges.Privileges[0].Luid.LowPart        = luid.LowPart;
	TokenPrivileges.Privileges[0].Luid.HighPart       = luid.HighPart;
	TokenPrivileges.Privileges[0].Attributes  = SE_PRIVILEGE_ENABLED;

	if(!AdjustTokenPrivileges(hToken, FALSE, &TokenPrivileges, sizeof(TOKEN_PRIVILEGES),
			&PreviousTokenPrivileges, &dwPreviousTokenPrivilegesSize))
	{
		LOG(_T("debug 003"));
		return false;
	}

	PreviousTokenPrivileges.PrivilegeCount             = 1;
	PreviousTokenPrivileges.Privileges[0].Luid.HighPart        = luid.HighPart;
	PreviousTokenPrivileges.Privileges[0].Luid.LowPart         = luid.LowPart;
	PreviousTokenPrivileges.Privileges[0].Attributes  |= SE_PRIVILEGE_ENABLED;

	if(!AdjustTokenPrivileges(hToken, FALSE, &PreviousTokenPrivileges,
			dwPreviousTokenPrivilegesSize, NULL, NULL))
	{
		LOG(_T("debug 004"));
		return false;
	}

	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);

	CloseHandle( hToken );

	//WriteLog("debug ok");

	return true;
}