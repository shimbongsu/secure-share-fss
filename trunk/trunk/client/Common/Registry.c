/*
 Copyright (c) 2004-2008 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.6 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#include "Tcdefs.h"
#include "Registry.h"

BOOL ReadLocalMachineRegistryDword (char *subKey, char *name, DWORD *value)
{
	HKEY hkey = 0;
	DWORD size = sizeof (*value);
	DWORD type;

	if (RegOpenKeyExA (HKEY_LOCAL_MACHINE, subKey, 0, KEY_READ, &hkey) != ERROR_SUCCESS)
		return FALSE;

	if (RegQueryValueExA (hkey, name, NULL, &type, (BYTE *) value, &size) != ERROR_SUCCESS)
	{
		RegCloseKey (hkey);
		return FALSE;
	}

	RegCloseKey (hkey);
	return type == REG_DWORD;
}

BOOL ReadLocalMachineRegistryMultiString (char *subKey, char *name, char *value, DWORD *size)
{
	HKEY hkey = 0;
	DWORD type;

	if (RegOpenKeyExA (HKEY_LOCAL_MACHINE, subKey, 0, KEY_READ, &hkey) != ERROR_SUCCESS)
		return FALSE;

	if (RegQueryValueExA (hkey, name, NULL, &type, (BYTE *) value, size) != ERROR_SUCCESS)
	{
		RegCloseKey (hkey);
		return FALSE;
	}

	RegCloseKey (hkey);
	return type == REG_MULTI_SZ;
}

int ReadRegistryInt (char *subKey, char *name, int defaultValue)
{
	HKEY hkey = 0;
	DWORD value, size = sizeof (DWORD);

	if (RegOpenKeyExA (HKEY_CURRENT_USER, subKey,
		0, KEY_READ, &hkey) != ERROR_SUCCESS)
		return defaultValue;

	if (RegQueryValueExA (hkey, name, 0,	0, (LPBYTE) &value, &size) != ERROR_SUCCESS)
		value = defaultValue;

	RegCloseKey (hkey);
	return value;
}

char *ReadRegistryString (char *subKey, char *name, char *defaultValue, char *str, int maxLen)
{
	HKEY hkey = 0;
	char value[MAX_PATH*4];
	DWORD size = sizeof (value);

	strncpy_s (str, maxLen-1, defaultValue, maxLen-1);

	ZeroMemory (value, sizeof value);
	if (RegOpenKeyExA (HKEY_CURRENT_USER, subKey,
		0, KEY_READ, &hkey) == ERROR_SUCCESS)
		if (RegQueryValueExA (hkey, name, 0,	0, (LPBYTE) &value,	&size) == ERROR_SUCCESS)
			strncpy_s (str,maxLen-1, value, maxLen-1);

	RegCloseKey (hkey);
	return str;
}

DWORD ReadRegistryBytes (char *path, char *name, char *value, int maxLen)
{
	HKEY hkey = 0;
	DWORD size = maxLen;
	BOOL success = FALSE;

	if (RegOpenKeyExA (HKEY_CURRENT_USER, path, 0, KEY_READ, &hkey) != ERROR_SUCCESS)
		return 0;

	success = (RegQueryValueExA (hkey, name, 0,	0, (LPBYTE) value,	&size) == ERROR_SUCCESS);
	RegCloseKey (hkey);

	return success ? size : 0;
}

void WriteRegistryInt (char *subKey, char *name, int value)
{
	HKEY hkey = 0;
	DWORD disp;

	if (RegCreateKeyExA (HKEY_CURRENT_USER, subKey,
		0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkey, &disp) != ERROR_SUCCESS)
		return;

	RegSetValueExA (hkey, name, 0, REG_DWORD, (BYTE *) &value, sizeof value);
	RegCloseKey (hkey);
}

BOOL WriteLocalMachineRegistryDword (char *subKey, char *name, DWORD value)
{
	HKEY hkey = 0;
	DWORD disp;
	LONG status;

	if ((status = RegCreateKeyExA (HKEY_LOCAL_MACHINE, subKey,
		0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkey, &disp)) != ERROR_SUCCESS)
	{
		SetLastError (status);
		return FALSE;
	}

	if ((status = RegSetValueExA (hkey, name, 0, REG_DWORD, (BYTE *) &value, sizeof value)) != ERROR_SUCCESS)
	{
		RegCloseKey (hkey);
		SetLastError (status);
		return FALSE;
	}

	RegCloseKey (hkey);
	return TRUE;
}

BOOL WriteLocalMachineRegistryMultiString (char *subKey, char *name, char *multiString, DWORD size)
{
	HKEY hkey = 0;
	DWORD disp;
	LONG status;

	if ((status = RegCreateKeyExA (HKEY_LOCAL_MACHINE, subKey,
		0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkey, &disp)) != ERROR_SUCCESS)
	{
		SetLastError (status);
		return FALSE;
	}

	if ((status = RegSetValueExA (hkey, name, 0, REG_MULTI_SZ, (BYTE *) multiString, size)) != ERROR_SUCCESS)
	{
		RegCloseKey (hkey);
		SetLastError (status);
		return FALSE;
	}

	RegCloseKey (hkey);
	return TRUE;
}

void WriteRegistryString (char *subKey, char *name, char *str)
{
	HKEY hkey = 0;
	DWORD disp;

	if (RegCreateKeyExA (HKEY_CURRENT_USER, subKey,
		0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkey, &disp) != ERROR_SUCCESS)
		return;

	RegSetValueExA (hkey, name, 0, REG_SZ, (BYTE *) str, strlen (str) + 1);
	RegCloseKey (hkey);
}

BOOL WriteRegistryBytes (char *path, char *name, char *str, DWORD size)
{
	HKEY hkey = 0;
	DWORD disp;
	BOOL res;

	if (RegCreateKeyExA (HKEY_CURRENT_USER, path,
		0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkey, &disp) != ERROR_SUCCESS)
		return FALSE;

	res = RegSetValueExA (hkey, name, 0, REG_BINARY, (BYTE *) str, size);
	RegCloseKey (hkey);
	return res == ERROR_SUCCESS;
}

void DeleteRegistryValue (char *subKey, char *name)
{
	HKEY hkey = 0;

	if (RegOpenKeyExA (HKEY_CURRENT_USER, subKey, 0, KEY_WRITE, &hkey) != ERROR_SUCCESS)
		return;

	RegDeleteValueA (hkey, name);
	RegCloseKey (hkey);
}