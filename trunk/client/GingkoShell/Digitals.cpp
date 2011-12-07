#include "stdafx.h"
#include "Digitals.h"

const CDigitalShareForm  *_g_pDigitalSharedDialog = NULL; 

TCHAR*       m_SharedOptionsArray[5][2] = { 
				{_T("PL"),_T("Public Limit")}, 
				{_T("PF"),_T("Public Full")},
				{_T("PR"),_T("Private")},
				{_T("RQ"),_T("Request")},
				{_T("NN"),_T("Owner Only")}
			};

TCHAR*       m_SecurityOptionsArray[5][2] = { 
				{_T("OWN"),_T("Owner Only")}, 
				{_T("HAE"),_T("Holder able to assign permission")},
				{_T("HAA"),_T("Holder able to assign holder")},
				{_T("SAE"),_T("Admin able to assign permission")},
				{_T("SAA"),_T("Admin able to assign holder")}
			};
 

static BOOL IsValidChar(TCHAR ch)
{
	if( (ch>=97) && (ch<=122) ) //26个小写字母
		return TRUE;
	if( (ch>=65) && (ch<=90) ) //26个大写字母
		return TRUE;
	if((ch>=48) && (ch<=57)) //0~9
		return TRUE;
	if( ch==95 || ch==45 || ch==46 || ch==64 ) //_-.@
		return TRUE;
	return FALSE;
}

//EMAIL有效性验证
BOOL IsValidEmail(CString& strEmail)
{
	if(strEmail.GetLength()<5) //a@b.c
		return FALSE;
	TCHAR ch = strEmail[0];
	//1. 首字符必须用字母
	if( ((ch>=97) && (ch<=122)) || ((ch>=65) && (ch<=90)) )
	{
		int atCount =0;
		int atPos = 0;
		int dotCount = 0;	
		for(int i=1;i<strEmail.GetLength();i++) //0已经判断过了，从1开始
		{
			ch = strEmail[i];
			if(IsValidChar(ch))
			{
				if( ch == 64 ) //"@"
				{
					atCount ++;
					atPos = i;
				}
				else if( ( atCount > 0 ) && ( ch == 46 ) )//@符号后的"."号
				dotCount ++;
			}
			else
				return FALSE;
		}
			//6. 结尾不得是字符“@”或者“.”
		if( ch == 46 )
			return FALSE;
		//2. 必须包含一个并且只有一个符号“@”
		//3. @后必须包含至少一个至多三个符号“.”
		if( (atCount!=1) || (dotCount<1) || (dotCount>3) )
			return FALSE;
		//5. 不允许出现“@.”或者.@
		if( strEmail.Find(_T("@."))>0 || strEmail.Find(_T(".@"))>0 )
			return FALSE;
		return TRUE;
	}
	return FALSE;
}
