/*
 Copyright (c) 2005 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.6 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DATA_POOL_CAPACITY 1000000

typedef struct
{
	char	*Key;
	int		IntKey;
	void	*Value;
} DictionaryEntry;

int AddDictionaryEntry (char *key, int intKey, void *value);
void *GetDictionaryValue (const char *key);
void *GetDictionaryValueByInt (int intKey);
void *AddPoolData (void *data, size_t dataSize);
void ClearDictionaryPool ();

#ifdef __cplusplus
}
#endif

#endif
