//
// gcc -c -DBUILD_DLL source.c
// gcc -shared -o source.dll -W1,--out-implib,libkernel32.a source.o
//

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

VOID     GetProcessInfo( VOID );
VOID WriteToActivityLog( CHAR *szFilePath, CHAR *szDomainName, CHAR *szUserName, CHAR *szCommandLine, DWORD *dwProcessID );

BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpvReserved )
{
	BOOL bSuccess = TRUE;

	switch ( dwReason )
	{
		case DLL_PROCESS_ATTACH:
			GetProcessInfo();

			break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_THREAD_DETACH:
			break;

		case DLL_PROCESS_DETACH:
			break;
	}

	return bSuccess;
}

VOID GetProcessInfo( VOID )
{
	CHAR     szFilePath[ 1024 ];
	CHAR   szDomainName[ 128 ];
	CHAR     szUserName[ 128 ];
	DWORD      dwResult;
	CHAR  szCommandLine[ 1024 ];
	DWORD   dwProcessID;

	strcpy( szFilePath,   "" );
	strcpy( szDomainName, "" );
	strcpy( szUserName,   "" );

	dwResult = GetModuleFileName( NULL, szFilePath, 1024 );

	if ( dwResult > 0 && strcmp( szFilePath, "" ) != 0 )
	{
		strupr( szFilePath );

		dwResult = GetEnvironmentVariable( "USERDOMAIN", szDomainName, 128 );

		strupr( szDomainName );

		if ( dwResult > 0 && strcmp( szDomainName, "" ) != 0 )
		{
			dwResult = GetEnvironmentVariable( "USERNAME", szUserName, 128 );

			strupr( szUserName );

			if ( dwResult > 0 && strcmp( szUserName, "" ) != 0 )
			{
				strcpy( szCommandLine, GetCommandLine() );

				dwProcessID = GetCurrentProcessId();

				WriteToActivityLog( szFilePath, szDomainName, szUserName, szCommandLine, &dwProcessID );
			}
		}
	}
}

VOID WriteToActivityLog( CHAR szFilePath[], CHAR szDomainName[], CHAR szUserName[], CHAR szCommandLine[], DWORD *dwProcessID )
{
	FILE *pOutputFile;
	CHAR    szLogFile[ 1024 ];
	CHAR       szDate[ 64 ];
	CHAR       szTime[ 64 ];

	strcpy( szLogFile, "C:\\WinTrack-Activity.log" );

	pOutputFile = fopen( szLogFile, "r" );

	if ( pOutputFile != NULL )
	{
		fclose( pOutputFile );
	}
	else
	{
		pOutputFile = fopen( szLogFile, "w" );

		if ( pOutputFile != NULL )
		{
			fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
			fprintf( pOutputFile, "\n" );
			fprintf( pOutputFile, "Date\tTime\tFile Path\tCommand Line\tProcess ID\tDomain Name\tUsername\n" );

			fclose( pOutputFile );
		}
	}

	pOutputFile = fopen( szLogFile, "a+" );

	if ( pOutputFile != NULL )
	{
		_strdate( szDate );
		_strtime( szTime );

		fprintf( pOutputFile, "%s %s\t%s\t%s\t%d\t%s\\%s\n", szDate, szTime, szFilePath, szCommandLine, *dwProcessID, szDomainName, szUserName );

		fclose( pOutputFile );
	}
}

// Written by Reed Arvin | reed@canaudit.com
