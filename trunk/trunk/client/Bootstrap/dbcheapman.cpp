#include "stdafx.h"
#include "bootstrap.h"
#include "cheapman.h"
#include <sqlite3.h>
#include "Crypto.h"


sqlite3 *g_gingkodb = NULL; //Database connect
sqlite3_stmt *pQueryPrimaryKeyStmt = NULL; //The primary key query statement
sqlite3_stmt *pQueryProcessStmt = NULL;    //The process query statement
sqlite3_stmt *pQueryPrimaryKeyByFilenameStmt = NULL; //The primary key query statement by filename
sqlite3_stmt *pInsertNewPrivateKey = NULL; //The primary key query statement by filename

static unsigned char hex_chars[] = "0123456789ABCDEF";

static int HexCode( unsigned char ch )
{
	if( ch >= '0' && ch <= '9' )
	{
		return ch - '0';
	}
	if( ch >= 'A' && ch <= 'F' )
	{
		return ch - 'A' + 10; 
	}
	if( ch >= 'a' && ch <= 'a' )
	{
		return ch - 'a' + 10; 
	}
	return -1;
}

void HexToBytes( const unsigned char *p, unsigned char* pbuf, int maxPlen )
{
	for( int i = 0; i < maxPlen; i += 2 )
	{
		pbuf[i/2] = (HexCode(p[i]) * 16 + HexCode( p[i+1] )) & 0xFF; 
	}
}

int DatabaseInstall()
{
	if( SQLITE_OK == OpenDatabase(TRUE) )
	{
		sqlite3_exec( g_gingkodb, "CREATE TABLE processes(process_name varchar(100), disable_decrypt int);", NULL, NULL, NULL );
		sqlite3_exec( g_gingkodb, "CREATE TABLE private_certs(filename varchar(260),file_id varchar(32),company_id varchar(12),permission int,private_key varchar(1024) );", NULL, NULL, NULL );
		sqlite3_exec( g_gingkodb, "CREATE TABLE unencryptionfiles(filename varchar(260), disable_decrypt int);", NULL, NULL, NULL );
	}
	CloseDatabase();
	return 0;
}

static int queryPriverKeyCallback(void *context, int argc, char **argv, char **azColName){
  int i;
  for(i=0; i<argc; i++){
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}

int OpenDatabase(BOOL Create)
{
	int ret = sqlite3_open( "gingkodb", &g_gingkodb );
	if( ret != SQLITE_OK )
	{
		const char *err = sqlite3_errmsg( g_gingkodb );
		printf( "Can not open the database gingkodb. The error message is %s.\n", err );
		sqlite3_close( g_gingkodb );
	}

	if( !Create )
	{
		ret = sqlite3_prepare_v2( g_gingkodb, "SELECT company_id, file_id, filename,private_key, permission FROM private_certs WHERE company_id=$companyId AND file_id=$fileId", -1, &pQueryPrimaryKeyStmt, NULL );
		if( ret != SQLITE_OK )
		{
			const char *err = sqlite3_errmsg( g_gingkodb );
			printf( "Error when prepare statement: %s.\n", err );
		}
		ret = sqlite3_prepare_v2( g_gingkodb, "SELECT company_id, file_id, filename,private_key, permission FROM private_certs WHERE filename=$fileName", -1, &pQueryPrimaryKeyByFilenameStmt, NULL );
		if( ret != SQLITE_OK )
		{
			const char *err = sqlite3_errmsg( g_gingkodb );
			printf( "Error when prepare statement: %s.\n", err );
		}
		ret = sqlite3_prepare_v2( g_gingkodb, "SELECT process_name, disable_decrypt FROM processes WHERE process_name=$processName", -1, &pQueryProcessStmt, NULL );
		if( ret != SQLITE_OK )
		{
			const char *err = sqlite3_errmsg( g_gingkodb );
			printf( "Error when prepare statement: %s.\n", err );
		}
		ret = sqlite3_prepare_v2( g_gingkodb, "INSERT INTO private_certs(company_id, file_id, filename, permission, private_key) VALUES($companyId, $fileId, $fileName, $permission, $privateKey);", -1, &pInsertNewPrivateKey, NULL );
		if( ret != SQLITE_OK )
		{
			const char *err = sqlite3_errmsg( g_gingkodb );
			printf( "Error when prepare statement: %s.\n", err );
		}
	}
	return ret;
}

int CloseDatabase()
{
	int ret = SQLITE_OK;

	if( pQueryPrimaryKeyStmt )
	{
		sqlite3_finalize(	pQueryPrimaryKeyStmt );
		pQueryPrimaryKeyStmt = NULL; //The primary key query statement
	}
	if( pQueryProcessStmt )
	{
		sqlite3_finalize(	pQueryProcessStmt );
		pQueryProcessStmt = NULL; //The primary key query statement
	}
	if( pQueryPrimaryKeyByFilenameStmt )
	{
		sqlite3_finalize(	pQueryPrimaryKeyByFilenameStmt );
		pQueryPrimaryKeyByFilenameStmt = NULL; //The primary key query statement
	}
	if( pInsertNewPrivateKey )
	{
		sqlite3_finalize(	pInsertNewPrivateKey );
		pInsertNewPrivateKey = NULL; //The primary key query statement
	}

	if( g_gingkodb != NULL )
	{
		ret = sqlite3_close( g_gingkodb );
		g_gingkodb = NULL;
	}
	return ret;
}


LPSTR CompanyId(PrivateKeyRequest* pHeader)
{
	char arrFileId[14];
	memset( arrFileId, 0, sizeof( arrFileId ) );
	_snprintf( arrFileId, sizeof(arrFileId), 
		"%02x%02x%02x%02x%02x%02x",
		pHeader->Company[0], pHeader->Company[1],pHeader->Company[2],pHeader->Company[3],pHeader->Company[4],pHeader->Company[5]
	);
	return _strdup( arrFileId );
}



LPSTR FileId(PrivateKeyRequest* pHeader)
{
	char arrFileId[34];
	memset( arrFileId, 0, sizeof( arrFileId ) );
	_snprintf( arrFileId, sizeof(arrFileId), 
		"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
		pHeader->Md5[0], pHeader->Md5[1],pHeader->Md5[2],pHeader->Md5[3],
		pHeader->Md5[4], pHeader->Md5[5],pHeader->Md5[6],pHeader->Md5[7],
		pHeader->Md5[8], pHeader->Md5[9],pHeader->Md5[10],pHeader->Md5[11],
		pHeader->Md5[12], pHeader->Md5[13],pHeader->Md5[14],pHeader->Md5[15]
	);
	return _strdup( arrFileId );
}

LPSTR Filename( PrivateKeyRequest *request )
{
	LPSTR retfile = NULL;
	char* filenamebuf = (char*)malloc( request->FilenameLength + 2 );
	if( filenamebuf != NULL )
	{
		memset( filenamebuf, 0, request->FilenameLength + 2 );
		if( 0 == WideCharToMultiByte( CP_ACP, 0, request->Filename, request->FilenameLength, filenamebuf, request->FilenameLength / 2, NULL, NULL ) )
		{
			int len = WideCharToMultiByte( CP_ACP, 0, request->Filename, request->FilenameLength, NULL, 0, NULL, NULL );
			free( filenamebuf );
			filenamebuf = (char*)malloc( len + 1 );
			if( filenamebuf )
			{
				memset( filenamebuf, 0, len + 1 );
				WideCharToMultiByte( CP_ACP, 0, request->Filename, request->FilenameLength, filenamebuf, len, NULL, NULL );
			}
		}
	}
	if( filenamebuf )
	{
		retfile = _strdup( filenamebuf );
		free( filenamebuf );
		return retfile;
	}
	return NULL;
}

void FreeText( void* pText )
{
	if( pText )
	{
		free( pText );
	}
}

int EncodeBuffer( const unsigned char* pbuf, unsigned char* pOutputBuf, int maxLength )
{
	for( int i = 0; i < maxLength; i ++ )
	{
		pOutputBuf[2 * i]		= hex_chars[ pbuf[i] / 16 ];
		pOutputBuf[2 * i + 1]	= hex_chars[ pbuf[i] % 16 ];
	}
	return 0;
}

int GetHexValue( char p ){
	for( int i = 0; i < 16; i ++ )
	{
		if( p == hex_chars[i] )
			return i;
	}
	return -1;
}

int DecodeBuffer( const unsigned char* pbuf, unsigned char* pOutputBuf, int maxLength )
{
	HexToBytes( pbuf, pOutputBuf, maxLength );
	return 0;
}

void CopyRequestToResponse( PrivateKeyRequest *request, PrivateKeyResponse *response )
{
	response->FileObject = (LONG_PTR) request->FileObject;
	response->ProcessId = request->dwProcessId;
	response->RequestKeyEvent = (LONG_PTR) request->RequestKeyEvent;
	response->SizeOfBlock = request->SizeOfBlock;
	response->SizeOfFile = request->SizeOfFile;
	response->SizeOfHeader = request->SizeOfHeader;
	
	memcpy( response->Company,		request->Company,		COMPANY_LENGTH );
	memcpy( response->Identifier,	request->Identifier,	GINGKO_IDENTIFIER_LENGTH );
	memcpy( response->Version,		request->Version,		GINGKO_VERSION_LENGTH );
	memcpy( response->Md5,			request->Md5,			MD5_DIGEST_LENGTH );
	memcpy( response->Method,		request->Method,		GINGKO_METHOD_LENGTH );
	response->SizeOfResponse = sizeof( PrivateKeyResponse );
}

int QueryPrimaryKey( PrivateKeyRequest *request, PrivateKeyResponse *response )
{
	if( g_gingkodb != NULL )
	{
		sqlite3_reset( pQueryPrimaryKeyStmt );
		sqlite3_clear_bindings( pQueryPrimaryKeyStmt );
		sqlite3_bind_text( pQueryPrimaryKeyStmt, 1, CompanyId( request ), -1, FreeText ); //"company id"
		sqlite3_bind_text( pQueryPrimaryKeyStmt, 2, FileId( request ) , -1, FreeText ); //"md5"
		while(  SQLITE_ROW == sqlite3_step( pQueryPrimaryKeyStmt ) )
		{
			const unsigned char* pkeys = sqlite3_column_text( pQueryPrimaryKeyStmt, 3 );
			KEY_INFO key;
			DecodeBuffer( pkeys, response->CryptoKey, strlen((const char*)pkeys) );
			memcpy( &key, response->CryptoKey, sizeof(KEY_INFO) );
			response->Permission = 0x80000000 | sqlite3_column_int( pQueryPrimaryKeyStmt, 4 );
			wprintf(L"Found the primary key with this request.\n");
			return SQLITE_OK;
		}
	}

	return SQLITE_ERROR;
}


int RenewPrimaryKey( PrivateKeyRequest *request, PrivateKeyResponse *response )
{

	if( g_gingkodb != NULL )
	{
		int ret = 0;
		sqlite3_reset( pQueryPrimaryKeyByFilenameStmt );
		sqlite3_clear_bindings( pQueryPrimaryKeyByFilenameStmt );
		sqlite3_bind_text( pQueryPrimaryKeyByFilenameStmt, 1, Filename( request ), -1, FreeText ); //"company id"
		//sqlite3_bind_text( pQueryPrimaryKeyByFilenameStmt, 2, FileId( request ) , 0, FreeText ); //"md5"
		ret = sqlite3_step( pQueryPrimaryKeyByFilenameStmt );
		if(  SQLITE_ROW ==  ret )
		{
			const unsigned char* pcompany = sqlite3_column_text( pQueryPrimaryKeyByFilenameStmt, 0 );
			if( memcmp( pcompany, "000000000000", 12 ) == 0 )
			{
				const unsigned char* pkeys	=	sqlite3_column_text( pQueryPrimaryKeyByFilenameStmt, 3 );
				response->KeySize			=	DecodeBuffer( pkeys, response->CryptoKey, strlen((const char*)pkeys) );
				response->Permission		=	0x80000000 | sqlite3_column_int( pQueryPrimaryKeyByFilenameStmt, 4 );
				return SQLITE_OK;
			}
		}

		if( TRUE )
		{
			unsigned char Header[512] = {0};
			//unsigned char mastKey[512] = {0};
			PCRYPTO_INFO retInfo = NULL;
			int nStatus = 0;
			//nStatus = CreatePrimaryKeyInMemory( (char *)Header, AES, XTS, SHA512, &retInfo );
			wprintf(L"After create the ValumeHeaderInMemory. The Status is %d and retInfo is %p.\n", nStatus, retInfo );
			if( retInfo != NULL )
			{
				char *pEncodedBuffer = NULL;
				request->Version[0]=1;
				request->Version[1]=0;
				request->SizeOfHeader = 0;
				memcpy( request->Identifier, "GKTF", 4 );
				memcpy( request->Method, "AES#1", 5 );
				GUID guid;
				if( S_OK == CoCreateGuid(&guid) )
				{
					memcpy( request->Md5, &guid, sizeof(GUID) );
				}

				pEncodedBuffer = (char*) malloc( 1025 * sizeof(char) );

				EncodeBuffer( Header, (unsigned char*)pEncodedBuffer, sizeof(Header) );
				
				memcpy( response->CryptoKey, Header, sizeof(Header) );

				sqlite3_bind_text( pInsertNewPrivateKey, 1, CompanyId( request ), -1, FreeText );
				sqlite3_bind_text( pInsertNewPrivateKey, 2, FileId( request ), -1, FreeText );
				sqlite3_bind_text( pInsertNewPrivateKey, 3, Filename( request ), -1, FreeText );
				sqlite3_bind_int(  pInsertNewPrivateKey, 4, 0x0001 );
				sqlite3_bind_text( pInsertNewPrivateKey, 5, pEncodedBuffer, -1,  SQLITE_TRANSIENT );
				sqlite3_step( pInsertNewPrivateKey );
				sqlite3_reset( pInsertNewPrivateKey );
				sqlite3_clear_bindings( pInsertNewPrivateKey );
				free( pEncodedBuffer );
				//CommitPrimaryKey( request, Header, sizeof(Header) );
				free( retInfo );
			}
		}
		
	}

	return SQLITE_ERROR;
}
