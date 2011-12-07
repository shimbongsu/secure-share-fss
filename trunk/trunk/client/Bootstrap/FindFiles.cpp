#include "stdafx.h"
#include <winerror.h>
#include "FindFiles.h"

void RegureFindPath( LPCWSTR Path, LPCWSTR RegexFind, BOOL DontIndeep, PVOID Context, FindFileProc FindProc )
{
	WIN32_FIND_DATA wfd;
	WCHAR FileSearch[MAX_PATH];
	WCHAR FileSearchPath[MAX_PATH];
	HANDLE hFindFile = INVALID_HANDLE_VALUE;
	
	StringCchPrintf( FileSearch, sizeof(FileSearch), L"%s\\%s", Path, RegexFind );

	hFindFile = FindFirstFile( FileSearch, &wfd );

	if( hFindFile == INVALID_HANDLE_VALUE )
	{
		if( GetLastError() != ERROR_NO_MORE_FILES && GetLastError() != ERROR_FILE_NOT_FOUND )
		{
			wprintf(L"Can not open to find %s at %s. Error code is %d.\n", RegexFind, Path, GetLastError() );
			return;
		}
	}


	if( hFindFile != INVALID_HANDLE_VALUE )
	{

		do{
			if( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				if( wcscmp(L".", wfd.cFileName ) != 0 && wcscmp( L"..", wfd.cFileName ) != 0 )
				{
					if( !DontIndeep )
					{
						StringCchPrintf( FileSearchPath, sizeof(FileSearchPath), L"%s\\%s", Path, wfd.cFileName );
						RegureFindPath( FileSearchPath, RegexFind, DontIndeep, Context, FindProc);
					}
					if( FindProc != NULL )
					{
						StringCchPrintf( FileSearchPath, sizeof(FileSearchPath), L"%s\\%s", Path, wfd.cFileName );
						if( FindProc( FileSearchPath, TRUE, Context ) == FALSE ){
							break;
						}
					}
				}
			}else{
				if( FindProc != NULL )
				{
					StringCchPrintf( FileSearchPath, sizeof(FileSearchPath), L"%s\\%s", Path, wfd.cFileName );				
					if( FindProc( FileSearchPath, FALSE, Context ) == FALSE ){
						break;
					}
				}
			}
		}while( FindNextFile( hFindFile, &wfd ) );

		if( GetLastError() != ERROR_NO_MORE_FILES && GetLastError() != 0 )
		{
			wprintf(L"Exit finding files with error %d.\n", GetLastError() );
			SetLastError(0);
		}

		FindClose(hFindFile);
	}

	StringCchPrintf( FileSearch, sizeof(FileSearch), L"%s\\*", Path, RegexFind );

	hFindFile = FindFirstFile( FileSearch, &wfd );

	if( hFindFile == INVALID_HANDLE_VALUE )
	{
		wprintf(L"Can not open to find %s at %s.\n", RegexFind, Path );
		return;
	}

	do{
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{
			if( wcscmp(L".", wfd.cFileName ) != 0 && wcscmp( L"..", wfd.cFileName ) != 0 )
			{
				if( !DontIndeep )
				{
					StringCchPrintf( FileSearchPath, sizeof(FileSearchPath), L"%s\\%s", Path, wfd.cFileName );
					RegureFindPath( FileSearchPath, RegexFind, DontIndeep, Context, FindProc);
				}
				if( FindProc != NULL )
				{
					StringCchPrintf( FileSearchPath, sizeof(FileSearchPath), L"%s\\%s", Path, wfd.cFileName );
					if( FindProc( FileSearchPath, TRUE, Context ) == FALSE ){
						break;
					}
				}
			}
		}
	}while( FindNextFile( hFindFile, &wfd ) );

	if( GetLastError() != ERROR_NO_MORE_FILES && GetLastError() != 0 )
	{
		wprintf(L"Exit finding files with error %d.\n", GetLastError() );
		SetLastError(0);
	}

	FindClose(hFindFile);	
}

BOOL CombineIntoFile( LPCWSTR fileName, BOOL isDir, PVOID Context )
{
	if( !isDir )
	{
		HANDLE hOutputFile = (HANDLE) Context;
		char Buffer[4096];
		DWORD dwSize = 0;

		if( hOutputFile == INVALID_HANDLE_VALUE )
		{
			return FALSE;
		}
		
		sprintf_s( Buffer, sizeof(Buffer), "***%S***\n",
						 fileName );
		WriteFile( hOutputFile, Buffer, strlen(Buffer), &dwSize, NULL );

		HANDLE hFile = CreateFile( fileName, 
									GENERIC_READ, 
									FILE_SHARE_READ, 
									NULL, 
									OPEN_ALWAYS, 
									FILE_ATTRIBUTE_NORMAL, NULL );
		if( hFile != INVALID_HANDLE_VALUE )
		{
			dwSize = 0;
			do{
				BOOL bRet = ReadFile( hFile, Buffer, sizeof(Buffer), &dwSize, NULL );
				if( bRet == FALSE ){
					break;
				}
				WriteFile( hOutputFile, Buffer, dwSize, &dwSize, NULL );
			}while( dwSize > 0 );
			CloseHandle( hFile );
		}
	}
	return TRUE;
}

BOOL CombineDirectory( LPCWSTR Pattern, LPCWSTR OutputFile, LPCWSTR ChangedDirectory)
{
	HANDLE hOutputFile = CreateFile( OutputFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if( hOutputFile == INVALID_HANDLE_VALUE )
	{
		return FALSE;
	}

	RegureFindPath( ChangedDirectory, Pattern, FALSE, hOutputFile, CombineIntoFile);

	CloseHandle( hOutputFile );
	return TRUE;
}