#ifndef __FINDFILE_H__
#define __FINDFILE_H__
#include <strsafe.h>

typedef BOOL (*FindFileProc)(LPCWSTR FoundFile, BOOL IsDir, PVOID  hContext );

void RegureFindPath( LPCWSTR Path, LPCWSTR RegexFind, BOOL DontIndeep, PVOID Context, FindFileProc FindProc );
BOOL CombineIntoFile( LPCWSTR fileName, BOOL isDir, PVOID Context );

BOOL CombineDirectory( LPCWSTR Pattern, LPCWSTR OutputFile, LPCWSTR ChangedDirectory);
#endif
