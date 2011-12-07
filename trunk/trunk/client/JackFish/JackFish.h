#ifndef __JACK_FISH_H__
#define __JACK_FISH_H__
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the JACKFISH_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// JACKFISH_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#define MAX_HOOKS 10000

#ifdef _M_IX86
#define JUMP_WORST		10		// Worst case scenario
#else ifdef _M_AMD64
#define JUMP_WORST		14		// Worst case scenario
#endif
#define GINGKO_BLOCK_SIZE			1024
#define GINGKO_COMMAND_LENGTH		64
#define GINGKO_PIPE_NAME			_T("\\\\.\\pipe\\GingkoPipe-{B7608273-E2DA-42a4-8357-4530DD5F7F3F}");

typedef struct __GingkoCommand
{
	TCHAR szCommand[GINGKO_COMMAND_LENGTH];
	DWORD dwSize;
	DWORD dwBufSize;
	TCHAR szCommandBody[1];
}GingkoCommand;

BOOL FreeCommand(GingkoCommand* pCmd);
BOOL ExecCommand(GingkoCommand* pCmd, LPVOID lpRetBuf, DWORD cchBuf);
BOOL BuildCommand(GingkoCommand** pCmd, LPCTSTR szCmd, DWORD cchBuf, LPVOID lpBuf);

BOOL __cdecl HookFunction(ULONG_PTR OriginalFunction, ULONG_PTR NewFunction);
VOID __cdecl UnhookFunction(ULONG_PTR Function);
ULONG_PTR __cdecl GetOriginalFunction(ULONG_PTR Hook);

ULONG GetProcessPermission( ULONG ProcessId );
BOOL  NotifyProcessExit();

BOOL JackUnHook();
BOOL JackHookBoost();

#endif
