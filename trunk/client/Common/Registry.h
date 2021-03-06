/*
 Copyright (c) 2004-2008 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.6 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#ifdef  __cplusplus
extern "C" {
#endif

BOOL ReadLocalMachineRegistryDword (char *subKey, char *name, DWORD *value);
BOOL ReadLocalMachineRegistryMultiString (char *subKey, char *name, char *value, DWORD *size);
int ReadRegistryInt (char *subKey, char *name, int defaultValue);
char *ReadRegistryString (char *subKey, char *name, char *defaultValue, char *str, int maxLen);
DWORD ReadRegistryBytes (char *path, char *name, char *value, int maxLen);
void WriteRegistryInt (char *subKey, char *name, int value);
BOOL WriteLocalMachineRegistryDword (char *subKey, char *name, DWORD value);
BOOL WriteLocalMachineRegistryMultiString (char *subKey, char *name, char *multiString, DWORD size);
void WriteRegistryString (char *subKey, char *name, char *str);
BOOL WriteRegistryBytes (char *path, char *name, char *str, DWORD size);
void DeleteRegistryValue (char *subKey, char *name);

#ifdef  __cplusplus
}
#endif
