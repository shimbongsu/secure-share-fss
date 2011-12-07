#include "stdafx.h"
#include "GingkoLibrary.h"
#include <atlconv.h>
#include <map>
#include <list>
#include <libxml/parser.h>
#include <libxml/tree.h>

typedef std::map<UINT, const TCHAR*> GlobalLableMap;

typedef struct {
	UINT IDC;
	TCHAR* Caption;
} Lable;

typedef struct{
	UINT				IDD;
	TCHAR*				Name;
	TCHAR*				Caption;
	std::list<Lable*>	Lables;
} FormLables;

static GlobalLableMap _globalLables;

static std::list<FormLables*> FormList;

static GingkoOptions _gingkoOptions= {0};


static xmlChar* WideCharToXmlCharWinAPI( LPCTSTR lpszText, int length );

#define SetXmlNodeContent( ___pXmlNodePtr, ___tchars )				\
	if( ___pXmlNodePtr )													\
	{																\
		if( ___tchars )								\
		{															\
			xmlChar* __content = WideCharToXmlCharWinAPI( ___tchars, (int)_tcslen( ___tchars ) );	\
			if( __content )											\
			{														\
				xmlNodeSetContent( ___pXmlNodePtr, __content );			\
				free( __content );									\
			}														\
		}															\
	}																\


#define SetGingkoOption( __field, __xmlStr )							\
	if( __xmlStr != NULL )												\
	{																	\
		if( _gingkoOptions.__field != NULL ){							\
			free( _gingkoOptions.__field );								\
			_gingkoOptions.__field = NULL;								\
		}																\
		_gingkoOptions.__field = _tcsdup(A2W_CP( (char*) __xmlStr, CP_UTF8 ));	\
	}																	\


TCHAR*  ch64 = _T("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");

GINGKO_METHOD TCHAR* GINGKO_API Base64Encode( LPVOID src,UINT srclen )
{
	int n,buflen,i,j;
	int pading = 0;
	unsigned char *buf;
	TCHAR *dst; 
	buf = (unsigned char*) src;
	
	buflen = n = srclen;
	
	if( n % 3 != 0 )  /* pad with '=' by using a temp buffer */
	{
		pading = 1;
		buflen = n + 3 - n % 3;
		buf	= (unsigned char*) malloc( sizeof(unsigned char) * (buflen + 1) );
		memset( buf, 0, sizeof(unsigned char) * (buflen + 1));
		memcpy( buf, src, n);
		for( i = 0; i < 3 - n % 3; i ++ )
		{
			buf[ n + i ] = _T('=');
		}
	}

	dst = (TCHAR*) malloc( sizeof(TCHAR) * (buflen * 4 / 3 + 1) );
	memset( dst, 0,sizeof(TCHAR) * ( buflen * 4 / 3 + 1 ) );

	for(i = 0, j = 0; i < buflen; i += 3, j += 4)
	{
		dst[j] = (( buf[i] & 0xFC ) >> 2);
		dst[j + 1] = (((buf[i] & 0x03 ) << 4) + (( buf[i+1] & 0xF0) >> 4));
		dst[j + 2] = (((buf[i + 1] & 0x0F ) << 2) + ((buf[i+2] & 0xC0 ) >> 6));
		dst[j + 3] = (buf[i + 2] & 0x3F); 
	}

	for( i = 0; i < buflen * 4 / 3; i ++) /* map 6 bit value to base64 ASCII character */
	{
		dst[ i ] = ch64[ dst[ i ] ];
	}

	if( pading )
	{
		free( buf );
	}
	return dst;
}


GINGKO_METHOD LPVOID GINGKO_API Base64Decode(TCHAR *src)
{
	int n,i,j;
	TCHAR *p;
	unsigned char* dst; 

	n = (int) _tcslen( src );
	for( i=0; i < n; i ++) /* map base64 ASCII character to 6 bit value */
	{
		p = _tcschr( ch64, src[i]);
		
		if( !p )
			break;

		src[ i ] = (TCHAR)(p - ch64);
	}
	
	dst = (unsigned char*) malloc( sizeof(TCHAR) * (n * 3 / 4 + 1) );
	
	memset( dst, 0, sizeof(TCHAR) * (n * 3 / 4 + 1) );
	
	for( i = 0, j = 0; i < n; i += 4, j += 3 )
	{
		dst[ j ] = (unsigned char)( ( src[ i ] << 2 ) + (( src[ i + 1 ] & 0x30 ) >> 4 ) );
		dst[ j + 1 ] = (unsigned char)(( ( src[ i + 1 ] & 0x0F ) << 4 ) + ( ( src[ i + 2 ] & 0x3C ) >> 2 ));
		dst[ j + 2 ] = (unsigned char)(( ( src[ i + 2 ] & 0x03 ) << 6 ) + src[ i + 3 ]);
	}

	return dst;
}

GINGKO_METHOD BOOL   GINGKO_API FreeBase64Buffer(LPVOID lp)
{
	if( lp )
		free( lp );
	return TRUE;
}





static xmlChar* WideCharToXmlCharWinAPI( LPCTSTR lpszText, int length )
{
	//WideCharToMultiByte( CP_UTF8, 0, text, text.GetLength(), 
	xmlChar* out = NULL;

	int size = WideCharToMultiByte( CP_UTF8, 0, lpszText, length, NULL, 0, NULL, NULL );
	if( size > 0 )
	{
		out = (xmlChar*) malloc( size + 1 );
		if( out != NULL )
		{
			memset( out, 0, size + 1 );
			size = WideCharToMultiByte( CP_UTF8, 0, lpszText, length, (char*) out, size, NULL, NULL );
			if( size == 0 )
			{
				free( out );
				out = NULL;
			}
		}
	}
	return out;
}

static const FormLables* findDialogLables( UINT Idd )
{
	std::list<FormLables*>::const_iterator iter = FormList.begin();
	while( iter != FormList.end() )
	{
		FormLables * fl = *iter;
		if( fl->IDD == Idd )
		{
			return fl;
		}
		iter ++;
	}
	return NULL;
}

GINGKO_METHOD BOOL GINGKO_API UpdateDialogUI( HWND hDlg, UINT IDD )
{
	const FormLables* formLables = findDialogLables( IDD );
	if( formLables != NULL )
	{
		std::list<Lable*>::const_iterator it = formLables->Lables.begin();
		while( it != formLables->Lables.end() )
		{
			Lable* lbl = *it;
			if( lbl != NULL )
			{
				HWND hCtrlWnd = GetDlgItem( hDlg, lbl->IDC );
				if( hCtrlWnd != NULL )
				{
					::SetWindowText( hCtrlWnd, lbl->Caption );
				}
			}
			it ++;
		}
		::SetWindowText( hDlg, formLables->Caption );
	}
	return TRUE;
}

GINGKO_METHOD const TCHAR* GINGKO_API GetGlobalCaption( UINT Id, const TCHAR* pszDefault )
{
	GlobalLableMap::iterator iter = _globalLables.find( Id );
	if( iter != _globalLables.end() )
	{
		return (iter)->second;
	}
	return pszDefault;
}

GINGKO_METHOD BOOL GINGKO_API ParseConfigFile( const TCHAR* szFilename )
{
	USES_CONVERSION;
	HANDLE hXmlFile = CreateFile(  szFilename, 
		GENERIC_READ, FILE_SHARE_READ, 
		NULL, 
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL );

	if( hXmlFile != NULL && hXmlFile != INVALID_HANDLE_VALUE )
	{
			//Create File map
		DWORD FileSizeHigh = 0;
		DWORD FileSizeLow = 0;
		FileSizeLow = GetFileSize( hXmlFile, &FileSizeHigh );
		HANDLE hMap = CreateFileMapping( hXmlFile, NULL, PAGE_READONLY, FileSizeHigh, FileSizeLow, NULL );
		if( hMap != NULL && hMap != INVALID_HANDLE_VALUE )
		{
			char* pBuf = (char*) MapViewOfFile( hMap, FILE_MAP_READ, 0, 0, FileSizeLow );
			if( pBuf != NULL )
			{
				xmlDocPtr pDoc = xmlParseMemory( pBuf, FileSizeLow );
				if( pDoc != NULL )
				{
					xmlNodePtr pRoot = xmlDocGetRootElement( pDoc );
					if( xmlStrcmp( pRoot->name, BAD_CAST"GingkoShell") == 0 )
					{
						xmlNodePtr first = xmlFirstElementChild( pRoot );
						while( first != NULL )
						{
							if( xmlStrcmp( first->name, BAD_CAST"Options") == 0 )
							{
								xmlNodePtr optionsChild = xmlFirstElementChild( first );
								while( optionsChild != NULL )
								{
									if( xmlStrcmp( optionsChild->name, BAD_CAST"GingkoServer") == 0 )
									{
										xmlNodePtr serverChild = xmlFirstElementChild( optionsChild );
										while( serverChild != NULL )
										{
											if( xmlStrcmp( serverChild->name, BAD_CAST"MasterServer") == 0 )
											{
												SetGingkoOption(MasterServer, xmlNodeGetContent( serverChild ) ); 
											}
											if( xmlStrcmp( serverChild->name, BAD_CAST"UnitServer") == 0 )
											{
												SetGingkoOption(UnitServer, xmlNodeGetContent( serverChild ) ); 
											}
											serverChild = xmlNextElementSibling( serverChild );
										}
									}
									if( xmlStrcmp( optionsChild->name, BAD_CAST"Proxy") == 0 )
									{
										xmlNodePtr proxyChild = xmlFirstElementChild( optionsChild );
										while( proxyChild  != NULL )
										{
											if( xmlStrcmp( proxyChild->name, BAD_CAST"Connection") == 0 )
											{
												SetGingkoOption( Connection, xmlNodeGetContent( proxyChild  ) ); 
											}else if( xmlStrcmp( proxyChild->name, BAD_CAST"Server") == 0 )
											{
												SetGingkoOption( ProxyServer, xmlNodeGetContent( proxyChild  ) ); 
											}else if( xmlStrcmp( proxyChild->name, BAD_CAST"Port") == 0 )
											{
												SetGingkoOption( ProxyPort, xmlNodeGetContent( proxyChild  ) ); 
											}else if( xmlStrcmp( proxyChild->name, BAD_CAST"Username") == 0 )
											{
												SetGingkoOption( ProxyUser, xmlNodeGetContent( proxyChild  ) ); 
											}else if( xmlStrcmp( proxyChild->name, BAD_CAST"Password") == 0 )
											{
												SetGingkoOption( ProxyPassword, xmlNodeGetContent( proxyChild  ) ); 
											}
											proxyChild  = xmlNextElementSibling( proxyChild );
										}
									}
									if( xmlStrcmp( optionsChild->name, BAD_CAST"Miscellaneous") == 0 )
									{
										xmlNodePtr miscChild = xmlFirstElementChild( optionsChild );
										while( miscChild != NULL )
										{
											if( xmlStrcmp( miscChild->name, BAD_CAST"Timeout") == 0 )
											{
												SetGingkoOption(MicsTimeout, xmlNodeGetContent( miscChild ) ); 
											}
											if( xmlStrcmp( miscChild->name, BAD_CAST"Language") == 0 )
											{
												SetGingkoOption(Language, xmlNodeGetContent( miscChild ) ); 
											}
											miscChild = xmlNextElementSibling( miscChild );
										}
									}

									optionsChild = xmlNextElementSibling( optionsChild );
								}
							}
							first = xmlNextElementSibling( first );
						}

					}
					xmlFreeDoc( pDoc );
				}
				UnmapViewOfFile( pBuf );
			}
			CloseHandle( hMap );
		}
		CloseHandle( hXmlFile );
	}
	return TRUE;
}

GINGKO_METHOD BOOL GINGKO_API  GetGingkoOptions( GingkoOptions** pOpts )
{
	if( pOpts != NULL )
	{
		*pOpts = &_gingkoOptions;
	}
	return TRUE;
}

GINGKO_METHOD BOOL GINGKO_API  SaveConfigFile( const TCHAR* szFilename )
{
	USES_CONVERSION_EX;
	xmlDocPtr pDoc = xmlNewDoc(BAD_CAST "1.0");
	xmlNodePtr pRootNode = xmlNewNode( NULL, BAD_CAST "GingkoShell" );
	xmlDocSetRootElement( pDoc, pRootNode );
	xmlNodePtr pOptionNode = xmlNewChild( pRootNode, NULL, BAD_CAST "Options", NULL );
	xmlNodePtr pServerNode = xmlNewChild( pOptionNode, NULL, BAD_CAST "GingkoServer", NULL );
	xmlNodePtr pMasterNode = xmlNewChild( pServerNode, NULL, BAD_CAST "MasterServer", NULL );
	xmlNodePtr pUnitNode = xmlNewChild( pServerNode, NULL, BAD_CAST "UnitServer", NULL );
	SetXmlNodeContent( pUnitNode, _gingkoOptions.UnitServer );
	SetXmlNodeContent( pMasterNode ,_gingkoOptions.MasterServer );
									
	xmlNodePtr pProxyNode = xmlNewChild( pOptionNode, NULL, BAD_CAST "Proxy", NULL );
	xmlNodePtr pConnectNode = xmlNewChild( pProxyNode, NULL, BAD_CAST "Connection", NULL );
	SetXmlNodeContent( pConnectNode ,_gingkoOptions.Connection );
	
	xmlNodePtr pProxyServerNode = xmlNewChild( pProxyNode, NULL, BAD_CAST "Server", NULL );
	SetXmlNodeContent( pProxyServerNode ,_gingkoOptions.ProxyServer );
	
	xmlNodePtr pPortNode = xmlNewChild( pProxyNode, NULL, BAD_CAST "Port", NULL );
	SetXmlNodeContent( pPortNode ,_gingkoOptions.ProxyPort );

	xmlNodePtr pUsernameNode = xmlNewChild( pProxyNode, NULL, BAD_CAST "Username", NULL );
	SetXmlNodeContent( pUsernameNode ,_gingkoOptions.ProxyUser );

	xmlNodePtr pPassswordNode = xmlNewChild( pProxyNode, NULL, BAD_CAST "Password", NULL );
	SetXmlNodeContent( pPassswordNode ,_gingkoOptions.ProxyPassword );

	xmlNodePtr pMiscNode = xmlNewChild( pOptionNode, NULL, BAD_CAST "Miscellaneous", NULL );
	xmlNodePtr pTimeoutNode = xmlNewChild( pMiscNode, NULL, BAD_CAST "Timeout", NULL );
	SetXmlNodeContent( pTimeoutNode ,_gingkoOptions.MicsTimeout );

	xmlNodePtr pLanguageNode = xmlNewChild( pMiscNode, NULL, BAD_CAST "Language", NULL );
	SetXmlNodeContent( pLanguageNode ,_gingkoOptions.Language );

	//xmlSaveFormatFile(W2A_EX(lpszFile, _tcslen(lpszFile)), pDoc, 1);
	xmlSaveFormatFileEnc( W2A_EX(szFilename, _tcslen(szFilename)), pDoc, "utf-8", 1 ); 
	xmlFreeDoc( pDoc );
	return TRUE;
}


GINGKO_METHOD BOOL GINGKO_API  ParseLanguageFile( const TCHAR* lpszFilePath )
{
	USES_CONVERSION;
	//CString szFilePath;
	//if( GetExecutablePath( szFilePath ) )
	//{
	//	if( _gingkoOptions.Language )
	//	{
	//		szFilePath.AppendFormat( _T("\\Language_%s.xml"), _gingkoOptions.Language  );
	//	}else{
	//		szFilePath.Append( _T("\\Language.xml") );
	//	}
	//}

	HANDLE hXmlFile = CreateFile(  lpszFilePath, 
		GENERIC_READ, FILE_SHARE_READ, 
		NULL, 
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL );

	if( hXmlFile != NULL && hXmlFile != INVALID_HANDLE_VALUE )
	{
			//Create File map
		DWORD FileSizeHigh = 0;
		DWORD FileSizeLow = 0;
		FileSizeLow = GetFileSize( hXmlFile, &FileSizeHigh );
		HANDLE hMap = CreateFileMapping( hXmlFile, NULL, PAGE_READONLY, FileSizeHigh, FileSizeLow, NULL );
		if( hMap != NULL && hMap != INVALID_HANDLE_VALUE )
		{
			char* pBuf = (char*) MapViewOfFile( hMap, FILE_MAP_READ, 0, 0, FileSizeLow );
			if( pBuf != NULL )
			{
				xmlDocPtr pDoc = xmlParseMemory( pBuf, FileSizeLow );
				if( pDoc != NULL )
				{
					xmlNodePtr pRoot = xmlDocGetRootElement( pDoc );
					if( xmlStrcmp( pRoot->name, BAD_CAST"Language") == 0 )
					{
						xmlNodePtr pFormNode = xmlFirstElementChild( pRoot );
						while( pFormNode != NULL )
						{
							if( xmlStrcmp( pFormNode->name, BAD_CAST"Form" ) == 0 )
							{
								FormLables* formLables = new FormLables();
								xmlNodePtr pLablePtr = xmlFirstElementChild( pFormNode );
								xmlAttrPtr attr = pFormNode->properties;
								while( attr != NULL )
								{
									if( xmlStrcmp( attr->name, BAD_CAST"caption") == 0 )
									{
										formLables->Caption = _tcsdup( A2W_CP( (char*)attr->children->content, CP_UTF8 ) );
									}else if( xmlStrcmp( attr->name, BAD_CAST"name") == 0 )
									{
										formLables->Name = _tcsdup( A2W_CP( (char*)attr->children->content, CP_UTF8 ) );
									}else if( xmlStrcmp( attr->name, BAD_CAST"id") == 0 )
									{
										formLables->IDD = (UINT)_ttol( A2W_CP( (char*)attr->children->content, CP_UTF8 ) );
									}
									attr = attr->next;
								}
								
								while( pLablePtr != NULL )
								{
									if( xmlStrcmp( pLablePtr->name, BAD_CAST"lable") == 0 )
									{
										Lable* lable = new Lable();
										xmlAttrPtr lblAttr = pLablePtr->properties;
										while( lblAttr != NULL )
										{
											if( xmlStrcmp( lblAttr->name, BAD_CAST"id") == 0 )
											{
												lable->IDC = (UINT) _ttol( A2W_CP( (char*) lblAttr->children->content, CP_UTF8 ) );
											}
											lblAttr = lblAttr->next;
										}

										lable->Caption = _tcsdup( A2W_CP( (char*) xmlNodeGetContent( pLablePtr ), CP_UTF8 ) );
										
										formLables->Lables.push_back(lable);
									}
									pLablePtr = xmlNextElementSibling( pLablePtr );
								}
								FormList.push_back( formLables );
							}else if( xmlStrcmp( pFormNode->name, BAD_CAST"Global" ) == 0 ){
								xmlNodePtr pLablePtr = xmlFirstElementChild( pFormNode );
								while( pLablePtr != NULL )
								{
									if( xmlStrcmp( pLablePtr->name, BAD_CAST"lable") == 0 )
									{
										UINT Id = 0;
										xmlAttrPtr lblAttr = pLablePtr->properties;
										while( lblAttr != NULL )
										{
											if( xmlStrcmp( lblAttr->name, BAD_CAST"id") == 0 )
											{
												Id = (UINT) _ttol( A2W_CP( (char*) lblAttr->children->content, CP_UTF8 ) );
											}
											lblAttr = lblAttr->next;
										}

										if( Id != 0 )
										{
											_globalLables[Id] = _tcsdup( A2W_CP( (char*) xmlNodeGetContent( pLablePtr ), CP_UTF8 ) );
										}
									}
									pLablePtr = xmlNextElementSibling( pLablePtr );
								}
							}
							pFormNode = xmlNextElementSibling( pFormNode );
						}
					}
					xmlFreeDoc( pDoc );
				}
				UnmapViewOfFile( pBuf );
			}
			CloseHandle( hMap );
		}
		CloseHandle( hXmlFile );
	}
	return FALSE;
}



GINGKO_METHOD BOOL GINGKO_API FreeConfig()
{
	GingkoFree(_gingkoOptions.MasterServer);
	GingkoFree(_gingkoOptions.UnitServer);
	GingkoFree(_gingkoOptions.Connection);
	GingkoFree(_gingkoOptions.ProxyServer);
	GingkoFree(_gingkoOptions.ProxyPort);
	GingkoFree(_gingkoOptions.ProxyUser);
	GingkoFree(_gingkoOptions.ProxyPassword);
	GingkoFree(_gingkoOptions.MicsTimeout);
	GingkoFree(_gingkoOptions.Language);

	std::list<FormLables*>::const_iterator iter = FormList.begin();
	while( iter != FormList.end() )
	{
		FormLables * fl = *iter;
		GingkoFree( fl->Caption );
		GingkoFree( fl->Name );
		std::list<Lable*>::const_iterator it = fl->Lables.begin();
		while( it != fl->Lables.end() )
		{
			Lable *lbl = (*it);
			GingkoFree( lbl->Caption );
			delete lbl;
			it ++;
		}
		fl->Lables.clear();
		delete fl;
		iter ++;
	}

	FormList.clear();

	return TRUE;
}
