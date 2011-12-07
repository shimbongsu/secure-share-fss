#ifndef __GINGKOSERVICEDISPATCHER_H__
#define __GINGKOSERVICEDISPATCHER_H__

#include <stdio.h> 
#include <tchar.h>

#define BUFSIZE 4096

typedef struct _GINGKO_PROGRAM_PASS
{
	TCHAR szPathName[MAX_PATH + 1];
	struct _GINGKO_PROGRAM_PASS* Next;
} GINGKO_PROGRAM_PASS;

void StartGingkoConsole();
BOOL GetProcessExeFile(DWORD dwProcessId, TCHAR* szFileName, INT Length );
int OpenDatabase(BOOL Create);
int CloseDatabase();
BOOL QueryAuthenticApplication( const unsigned char* szMd5, const TCHAR* szOpenFileName );
BOOL AuthenticProcess( DWORD dwProcessId, TCHAR* szOpenFileName );
BOOL VerifyEmbeddedSignature(LPCWSTR pwszSourceFile);
BOOL LoadProgramPassThroughtTable();
BOOL FreeProgramPassThroghtTable();
#endif
