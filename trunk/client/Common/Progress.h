/*
 Legal Notice: Some portions of the source code contained in this file were
 derived from the source code of Encryption for the Masses 2.02a, which is
 Copyright (c) 1998-2000 Paul Le Roux and which is governed by the 'License
 Agreement for Encryption for the Masses'. Modifications and additions to
 the original source code (contained in this file) and all other portions of
 this file are Copyright (c) 2003-2008 TrueCrypt Foundation and are governed
 by the TrueCrypt License 2.6 the full text of which is contained in the
 file License.txt included in TrueCrypt binary and source code distribution
 packages. */

#ifdef __cplusplus
extern "C" {
#endif

void InitProgressBar (__int64 totalSecs, __int64 bytesDone, BOOL bReverse, BOOL bIOThroughput, BOOL bDisplayStatus, BOOL bShowPercent);
BOOL UpdateProgressBar ( __int64 nSecNo );
BOOL UpdateProgressBarProc (__int64 nSecNo);

#ifdef __cplusplus
}
#endif
