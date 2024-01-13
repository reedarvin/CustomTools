//
// gcc source.c -o source.exe -lnetapi32 -ladvapi32 -lmprapi
//

#define _WIN32_WINNT 0x0500
#define WINVER       0x0500

#include <windows.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h>
#include <process.h>
#include <lm.h>
#include <ntsecapi.h>
#include <time.h>
#include <mprapi.h>
#include <sddl.h>
#include <stdlib.h>

#define MAX_THREADS 64

VOID                    Usage( VOID );
VOID              ThreadedSub( VOID *pParameter );
BOOL                  Connect( CHAR *szTarget, BOOL *bMultipleHosts );
VOID WriteLastErrorToErrorLog( CHAR *szTarget, CHAR *szFunction, DWORD *dwError, BOOL *bMultipleHosts );
VOID     GetAccountPolicyInfo( CHAR *szTarget, BOOL *bMultipleHosts );
VOID          WriteToErrorLog( CHAR *szTarget, CHAR *szFunction, CHAR *szErrorMsg, BOOL *bMultipleHosts );
VOID            GetDomainInfo( CHAR *szTarget, BOOL *bMultipleHosts );
VOID        GetLocalGroupInfo( CHAR *szTarget, BOOL *bMultipleHosts );
VOID       GetGlobalGroupInfo( CHAR *szTarget, BOOL *bMultipleHosts );
VOID         GetLoggedOnUsers( CHAR *szTarget, BOOL *bMultipleHosts );
VOID             GetShareInfo( CHAR *szTarget, BOOL *bMultipleHosts );
VOID              GetUserInfo( CHAR *szTarget, BOOL *bMultipleHosts );
VOID            GetRAUserInfo( CHAR *szTarget, BOOL *bMultipleHosts );
BOOL            GetMachineSID( CHAR *szTarget, CHAR *szMachineSID, BOOL *bMultipleHosts );
BOOL    GetAccountNameFromSID( CHAR *szTarget, CHAR *szStringSID, CHAR *szDomainName, CHAR *szAccountName, BOOL *bMultipleHosts );
VOID     GetInstalledPrograms( CHAR *szTarget, BOOL *bMultipleHosts );
VOID       GetAuditPolicyInfo( CHAR *szTarget, BOOL *bMultipleHosts );
VOID            GetServerInfo( CHAR *szTarget, BOOL *bMultipleHosts );
VOID           GetServiceInfo( CHAR *szTarget, BOOL *bMultipleHosts );
VOID        GetUserRightsInfo( CHAR *szTarget, BOOL *bMultipleHosts );
BOOL               Disconnect( CHAR *szTarget, BOOL *bMultipleHosts );

typedef struct _THREAD_ARGS
{
	CHAR                 Target[ 128 ];
	BOOL UseAnonymousConnection;
	BOOL   GetAccountPolicyInfo;
	BOOL          GetDomainInfo;
	BOOL           GetGroupInfo;
	BOOL       GetLoggedOnUsers;
	BOOL           GetShareInfo;
	BOOL            GetUserInfo;
	BOOL          GetRAUserInfo;
	BOOL   GetInstalledPrograms;
	BOOL     GetAuditPolicyInfo;
	BOOL          GetServerInfo;
	BOOL         GetServiceInfo;
	BOOL      GetUserRightsInfo;
	BOOL          MultipleHosts;
} THREAD_ARGS, *PTHREAD_ARGS;

HANDLE hSemaphore;

INT nThreads = 0;

INT main( INT argc, CHAR *argv[] )
{
	BOOL                bContinue;
	BOOL  bUseAnonymousConnection;
	BOOL    bGetAccountPolicyInfo;
	BOOL           bGetDomainInfo;
	BOOL            bGetGroupInfo;
	BOOL        bGetLoggedOnUsers;
	BOOL            bGetShareInfo;
	BOOL             bGetUserInfo;
	BOOL           bGetRAUserInfo;
	BOOL    bGetInstalledPrograms;
	BOOL      bGetAuditPolicyInfo;
	BOOL           bGetServerInfo;
	BOOL          bGetServiceInfo;
	BOOL       bGetUserRightsInfo;
	INT                   nGetOpt;
	DWORD                       i;
	CHAR            szTargetInput[ 128 ];
	FILE              *pInputFile;
	CHAR               szReadLine[ 128 ];
	CHAR                 szTarget[ 128 ];

	PTHREAD_ARGS pThreadArgs;

	hSemaphore = CreateSemaphore( NULL, 1, 1, NULL );

	opterr = 0;

	bContinue = TRUE;

	bUseAnonymousConnection = FALSE;
	bGetAccountPolicyInfo   = FALSE;
	bGetDomainInfo          = FALSE;
	bGetGroupInfo           = FALSE;
	bGetLoggedOnUsers       = FALSE;
	bGetShareInfo           = FALSE;
	bGetUserInfo            = FALSE;
	bGetRAUserInfo          = FALSE;
	bGetInstalledPrograms   = FALSE;
	bGetAuditPolicyInfo     = FALSE;
	bGetServerInfo          = FALSE;
	bGetServiceInfo         = FALSE;
	bGetUserRightsInfo      = FALSE;

	while ( bContinue )
	{
		nGetOpt = getopt( argc, argv, "nadglsurpcijk" );

		if ( nGetOpt == -1 )
		{
			bContinue = FALSE;
		}

		switch ( nGetOpt )
		{
			case 'n':
				bUseAnonymousConnection = TRUE;

				break;

			case 'a':
				bGetAccountPolicyInfo = TRUE;

				break;

			case 'd':
				bGetDomainInfo = TRUE;

				break;

			case 'g':
				bGetGroupInfo = TRUE;

				break;

			case 'l':
				bGetLoggedOnUsers = TRUE;

				break;

			case 's':
				bGetShareInfo = TRUE;

				break;

			case 'u':
				bGetUserInfo = TRUE;

				break;

			case 'r':
				bGetRAUserInfo = TRUE;

				break;

			case 'p':
				bGetInstalledPrograms = TRUE;

				break;

			case 'c':
				bGetAuditPolicyInfo = TRUE;

				break;

			case 'i':
				bGetServerInfo = TRUE;

				break;

			case 'j':
				bGetServiceInfo = TRUE;

				break;

			case 'k':
				bGetUserRightsInfo = TRUE;

				break;
		}
	}

	if ( !bGetAccountPolicyInfo && !bGetDomainInfo && !bGetGroupInfo && !bGetLoggedOnUsers && !bGetShareInfo && !bGetUserInfo && !bGetRAUserInfo && !bGetInstalledPrograms && !bGetAuditPolicyInfo && !bGetServerInfo && !bGetServiceInfo && !bGetUserRightsInfo )
	{
		Usage();

		return 1;
	}

	i = 0;

	strcpy( szTargetInput, "" );

	if ( optind < argc )
	{
		while ( optind < argc )
		{
			if ( i == 0 )
			{
				strcpy( szTargetInput, argv[optind] );
			}

			optind++;

			i++;
		}
	}

	if ( i != 1 )
	{
		Usage();

		return 1;
	}

	printf( "Running AuditorToolkitPE v1.0 with the following arguments:\n" );
	printf( "[+] Target Host(s):           \"%s\"\n", szTargetInput );

	if ( bUseAnonymousConnection )
	{
		printf( "[+] Use Anonymous Connection: \"Yes\"\n" );
	}

	if ( bGetAccountPolicyInfo )
	{
		printf( "[+] Get Account Policy Info:  \"Yes\"\n" );
	}

	if ( bGetDomainInfo )
	{
		printf( "[+] Get Domain Info:          \"Yes\"\n" );
	}

	if ( bGetGroupInfo )
	{
		printf( "[+] Get Group Info:           \"Yes\"\n" );
	}

	if ( bGetLoggedOnUsers )
	{
		printf( "[+] Get Logged On Users:      \"Yes\"\n" );
	}

	if ( bGetShareInfo )
	{
		printf( "[+] Get Share Info:           \"Yes\"\n" );
	}

	if ( bGetUserInfo )
	{
		printf( "[+] Get User Info:            \"Yes\"\n" );
	}

	if ( bGetRAUserInfo )
	{
		printf( "[+] Get RA User Info:         \"Yes\"\n" );
	}

	if ( bGetInstalledPrograms )
	{
		printf( "[+] Get Installed Programs:   \"Yes\"\n" );
	}

	if ( bGetAuditPolicyInfo )
	{
		printf( "[+] Get Audit Policy Info:    \"Yes\"\n" );
	}

	if ( bGetServerInfo )
	{
		printf( "[+] Get Server Info:          \"Yes\"\n" );
	}

	if ( bGetServiceInfo )
	{
		printf( "[+] Get Service Info:         \"Yes\"\n" );
	}

	if ( bGetUserRightsInfo )
	{
		printf( "[+] Get User Rights Info:     \"Yes\"\n" );
	}

	printf( "[+] # of Threads:             \"64\"\n" );
	printf( "\n" );

	fflush( stdout );

	pInputFile = fopen( szTargetInput, "r" );

	if ( pInputFile != NULL )
	{
		while ( fscanf( pInputFile, "%s", szReadLine ) != EOF )
		{
			strcpy( szTarget, szReadLine );

			while ( nThreads >= MAX_THREADS )
			{
				Sleep( 200 );
			}

			pThreadArgs = (PTHREAD_ARGS)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( THREAD_ARGS ) );

			if ( pThreadArgs != NULL )
			{
				strcpy( pThreadArgs->Target, szTarget );

				pThreadArgs->UseAnonymousConnection = bUseAnonymousConnection;
				pThreadArgs->GetAccountPolicyInfo   = bGetAccountPolicyInfo;
				pThreadArgs->GetDomainInfo          = bGetDomainInfo;
				pThreadArgs->GetGroupInfo           = bGetGroupInfo;
				pThreadArgs->GetLoggedOnUsers       = bGetLoggedOnUsers;
				pThreadArgs->GetShareInfo           = bGetShareInfo;
				pThreadArgs->GetUserInfo            = bGetUserInfo;
				pThreadArgs->GetRAUserInfo          = bGetRAUserInfo;
				pThreadArgs->GetInstalledPrograms   = bGetInstalledPrograms;
				pThreadArgs->GetAuditPolicyInfo     = bGetAuditPolicyInfo;
				pThreadArgs->GetServerInfo          = bGetServerInfo;
				pThreadArgs->GetServiceInfo         = bGetServiceInfo;
				pThreadArgs->GetUserRightsInfo      = bGetUserRightsInfo;
				pThreadArgs->MultipleHosts          = TRUE;

				WaitForSingleObject( hSemaphore, INFINITE );

				nThreads++;

				ReleaseSemaphore( hSemaphore, 1, NULL );

				_beginthread( ThreadedSub, 0, (VOID *)pThreadArgs );
			}
		}

		fclose( pInputFile );

		Sleep( 1000 );

		printf( "Waiting for threads to terminate...\n" );

		fflush( stdout );
	}
	else
	{
		strcpy( szTarget, szTargetInput );

		pThreadArgs = (PTHREAD_ARGS)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( THREAD_ARGS ) );

		if ( pThreadArgs != NULL )
		{
			strcpy( pThreadArgs->Target, szTarget );

			pThreadArgs->UseAnonymousConnection = bUseAnonymousConnection;
			pThreadArgs->GetAccountPolicyInfo   = bGetAccountPolicyInfo;
			pThreadArgs->GetDomainInfo          = bGetDomainInfo;
			pThreadArgs->GetGroupInfo           = bGetGroupInfo;
			pThreadArgs->GetLoggedOnUsers       = bGetLoggedOnUsers;
			pThreadArgs->GetShareInfo           = bGetShareInfo;
			pThreadArgs->GetUserInfo            = bGetUserInfo;
			pThreadArgs->GetRAUserInfo          = bGetRAUserInfo;
			pThreadArgs->GetInstalledPrograms   = bGetInstalledPrograms;
			pThreadArgs->GetAuditPolicyInfo     = bGetAuditPolicyInfo;
			pThreadArgs->GetServerInfo          = bGetServerInfo;
			pThreadArgs->GetServiceInfo         = bGetServiceInfo;
			pThreadArgs->GetUserRightsInfo      = bGetUserRightsInfo;
			pThreadArgs->MultipleHosts          = FALSE;

			WaitForSingleObject( hSemaphore, INFINITE );

			nThreads++;

			ReleaseSemaphore( hSemaphore, 1, NULL );

			_beginthread( ThreadedSub, 0, (VOID *)pThreadArgs );
		}
	}

	while ( nThreads > 0 )
	{
		Sleep( 200 );
	}

	CloseHandle( hSemaphore );

	return 0;
}

VOID Usage( VOID )
{
	printf( "AuditorToolkitPE v1.0 | Canaudit, Inc. http://www.canaudit.com/\n" );
	printf( "\n" );
	printf( "Usage: AuditorToolkitPE [-nadglsurpcijk] <target host(s)>\n" );
	printf( "\n" );
	printf( "-n  -- Use Anonymous Connection\n" );
	printf( "-a  -- Get Account Policy Information\n" );
	printf( "-d  -- Get Domain Information\n" );
	printf( "-g  -- Get Group Information\n" );
	printf( "-l  -- Get Logged On Users\n" );
	printf( "-s  -- Get Share Information\n" );
	printf( "-u  -- Get User Information\n" );
	printf( "-r  -- Get User Information via Restrict Anonymous Bypass\n" );
	printf( "-p  -- Get Installed Programs\n" );
	printf( "-c  -- Get Audit Policy Information\n" );
	printf( "-i  -- Get Server Information\n" );
	printf( "-j  -- Get Service Information\n" );
	printf( "-k  -- Get User Rights Information\n" );
	printf( "\n" );
	printf( "<target host(s)> -- Host Name, IP Address, or Input File\n" );
	printf( "\n" );
	printf( "Examples:\n" );
	printf( "AuditorToolkitPE -n -adglsupcijk 15 10.10.10.10\n" );
	printf( "AuditorToolkitPE -n -r MyWindowsMachine\n" );
	printf( "AuditorToolkitPE -adglsupcijk IPInputFile.txt\n" );
	printf( "\n" );
	printf( "(Written by Reed Arvin | reed@canaudit.com)\n" );

	fflush( stdout );
}

VOID ThreadedSub( VOID *pParameter )
{
	CHAR                szTarget[ 128 ];
	BOOL bUseAnonymousConnection;
	BOOL   bGetAccountPolicyInfo;
	BOOL          bGetDomainInfo;
	BOOL           bGetGroupInfo;
	BOOL       bGetLoggedOnUsers;
	BOOL           bGetShareInfo;
	BOOL            bGetUserInfo;
	BOOL          bGetRAUserInfo;
	BOOL   bGetInstalledPrograms;
	BOOL     bGetAuditPolicyInfo;
	BOOL          bGetServerInfo;
	BOOL         bGetServiceInfo;
	BOOL      bGetUserRightsInfo;
	BOOL          bMultipleHosts;

	PTHREAD_ARGS pThreadArgs;

	pThreadArgs = (PTHREAD_ARGS)pParameter;

	strcpy( szTarget, pThreadArgs->Target );

	bUseAnonymousConnection = pThreadArgs->UseAnonymousConnection;
	bGetAccountPolicyInfo   = pThreadArgs->GetAccountPolicyInfo;
	bGetDomainInfo          = pThreadArgs->GetDomainInfo;
	bGetGroupInfo           = pThreadArgs->GetGroupInfo;
	bGetLoggedOnUsers       = pThreadArgs->GetLoggedOnUsers;
	bGetShareInfo           = pThreadArgs->GetShareInfo;
	bGetUserInfo            = pThreadArgs->GetUserInfo;
	bGetRAUserInfo          = pThreadArgs->GetRAUserInfo;
	bGetInstalledPrograms   = pThreadArgs->GetInstalledPrograms;
	bGetAuditPolicyInfo     = pThreadArgs->GetAuditPolicyInfo;
	bGetServerInfo          = pThreadArgs->GetServerInfo;
	bGetServiceInfo         = pThreadArgs->GetServiceInfo;
	bGetUserRightsInfo      = pThreadArgs->GetUserRightsInfo;
	bMultipleHosts          = pThreadArgs->MultipleHosts;

	HeapFree( GetProcessHeap(), 0, pThreadArgs );

	if ( bMultipleHosts )
	{
		printf( "Spawning thread for host %s...\n", szTarget );

		fflush( stdout );
	}

	if ( bUseAnonymousConnection )
	{
		if ( Connect( szTarget, &bMultipleHosts ) )
		{
			if ( bGetAccountPolicyInfo )
			{
				GetAccountPolicyInfo( szTarget, &bMultipleHosts );
			}

			if ( bGetDomainInfo )
			{
				GetDomainInfo( szTarget, &bMultipleHosts );
			}

			if ( bGetGroupInfo )
			{
				GetLocalGroupInfo( szTarget, &bMultipleHosts );
				GetGlobalGroupInfo( szTarget, &bMultipleHosts );
			}

			if ( bGetLoggedOnUsers )
			{
				GetLoggedOnUsers( szTarget, &bMultipleHosts );
			}

			if ( bGetShareInfo )
			{
				GetShareInfo( szTarget, &bMultipleHosts );
			}

			if ( bGetUserInfo )
			{
				GetUserInfo( szTarget, &bMultipleHosts );
			}

			if ( bGetRAUserInfo )
			{
				GetRAUserInfo( szTarget, &bMultipleHosts );
			}

			if ( bGetInstalledPrograms )
			{
				GetInstalledPrograms( szTarget, &bMultipleHosts );
			}

			if ( bGetAuditPolicyInfo )
			{
				GetAuditPolicyInfo( szTarget, &bMultipleHosts );
			}

			if ( bGetServerInfo )
			{
				GetServerInfo( szTarget, &bMultipleHosts );
			}

			if ( bGetServiceInfo )
			{
				GetServiceInfo( szTarget, &bMultipleHosts );
			}

			if ( bGetUserRightsInfo )
			{
				GetUserRightsInfo( szTarget, &bMultipleHosts );
			}

			Disconnect( szTarget, &bMultipleHosts );
		}
	}
	else
	{
		if ( bGetAccountPolicyInfo )
		{
			GetAccountPolicyInfo( szTarget, &bMultipleHosts );
		}

		if ( bGetDomainInfo )
		{
			GetDomainInfo( szTarget, &bMultipleHosts );
		}

		if ( bGetGroupInfo )
		{
			GetLocalGroupInfo( szTarget, &bMultipleHosts );
			GetGlobalGroupInfo( szTarget, &bMultipleHosts );
		}

		if ( bGetLoggedOnUsers )
		{
			GetLoggedOnUsers( szTarget, &bMultipleHosts );
		}

		if ( bGetShareInfo )
		{
			GetShareInfo( szTarget, &bMultipleHosts );
		}

		if ( bGetUserInfo )
		{
			GetUserInfo( szTarget, &bMultipleHosts );
		}

		if ( bGetRAUserInfo )
		{
			GetRAUserInfo( szTarget, &bMultipleHosts );
		}

		if ( bGetInstalledPrograms )
		{
			GetInstalledPrograms( szTarget, &bMultipleHosts );
		}

		if ( bGetAuditPolicyInfo )
		{
			GetAuditPolicyInfo( szTarget, &bMultipleHosts );
		}

		if ( bGetServerInfo )
		{
			GetServerInfo( szTarget, &bMultipleHosts );
		}

		if ( bGetServiceInfo )
		{
			GetServiceInfo( szTarget, &bMultipleHosts );
		}

		if ( bGetUserRightsInfo )
		{
			GetUserRightsInfo( szTarget, &bMultipleHosts );
		}
	}

	WaitForSingleObject( hSemaphore, INFINITE );

	nThreads--;

	ReleaseSemaphore( hSemaphore, 1, NULL );

	_endthread();
}

BOOL Connect( CHAR szTarget[], BOOL *bMultipleHosts )
{
	BOOL                 bReturn;
	CHAR              *pLocation;
	CHAR            szTempTarget[ 128 ];
	CHAR            szRemoteName[ 128 ];
	CHAR            szDomainName[ 128 ];
	CHAR              szUsername[ 128 ];
	CHAR              szPassword[ 128 ];
	WCHAR          wszRemoteName[ 256 ];
	WCHAR          wszDomainName[ 256 ];
	WCHAR            wszUsername[ 256 ];
	WCHAR            wszPassword[ 256 ];
	DWORD                dwLevel;
	USE_INFO_2           ui2Info;
	NET_API_STATUS       nStatus;
	DWORD                dwError;

	bReturn = FALSE;

	pLocation = strstr( szTarget, "\\\\" );

	if ( pLocation != NULL )
	{
		strcpy( szTempTarget, szTarget );
	}
	else
	{
		sprintf( szTempTarget, "\\\\%s", szTarget );
	}

	sprintf( szRemoteName, "%s\\IPC$", szTempTarget );

	strcpy( szDomainName, "" );
	strcpy( szUsername,   "" );
	strcpy( szPassword,   "" );

	MultiByteToWideChar( CP_ACP, 0, szRemoteName, strlen( szRemoteName ) + 1, wszRemoteName, sizeof( wszRemoteName ) / sizeof( wszRemoteName[0] ) );
	MultiByteToWideChar( CP_ACP, 0, szDomainName, strlen( szDomainName ) + 1, wszDomainName, sizeof( wszDomainName ) / sizeof( wszDomainName[0] ) );
	MultiByteToWideChar( CP_ACP, 0, szUsername,   strlen( szUsername ) + 1,   wszUsername,   sizeof( wszUsername ) / sizeof( wszUsername[0] ) );
	MultiByteToWideChar( CP_ACP, 0, szPassword,   strlen( szPassword ) + 1,   wszPassword,   sizeof( wszPassword ) / sizeof( wszPassword[0] ) );

	dwLevel = 2;

	ui2Info.ui2_local      = NULL;
	ui2Info.ui2_remote     = wszRemoteName;
	ui2Info.ui2_password   = wszPassword;
	ui2Info.ui2_asg_type   = USE_IPC;
	ui2Info.ui2_username   = wszUsername;
	ui2Info.ui2_domainname = wszDomainName;

	nStatus = NetUseAdd( NULL, dwLevel, (BYTE *)&ui2Info, NULL );

	if ( nStatus == NERR_Success )
	{
		bReturn = TRUE;
	}
	else
	{
		dwError = nStatus;

		WriteLastErrorToErrorLog( szTarget, "NetUseAdd", &dwError, bMultipleHosts );
	}

	return bReturn;
}

VOID WriteLastErrorToErrorLog( CHAR szTarget[], CHAR szFunction[], DWORD *dwError, BOOL *bMultipleHosts )
{
	DWORD     dwReturn;
	CHAR    szErrorMsg[ 128 ];
	CHAR     *pNewLine;
	FILE  *pOutputFile;

	dwReturn = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, *dwError, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), szErrorMsg, sizeof( szErrorMsg ), NULL );

	if ( dwReturn > 0 )
	{
		pNewLine = strchr( szErrorMsg, '\r' );

		if ( pNewLine != NULL )
		{
			*pNewLine = '\0';
		}

		pNewLine = strchr( szErrorMsg, '\n' );

		if ( pNewLine != NULL )
		{
			*pNewLine = '\0';
		}
	}
	else
	{
		strcpy( szErrorMsg, "Unknown error occurred." );
	}

	if ( !*bMultipleHosts )
	{
		fprintf( stderr, "ERROR! %s - %s\n", szFunction, szErrorMsg );

		fflush( stderr );
	}

	WaitForSingleObject( hSemaphore, INFINITE );

	pOutputFile = fopen( "ErrorLog.txt", "r" );

	if ( pOutputFile != NULL )
	{
		fclose( pOutputFile );
	}
	else
	{
		pOutputFile = fopen( "ErrorLog.txt", "w" );

		if ( pOutputFile != NULL )
		{
			fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
			fprintf( pOutputFile, "\n" );
			fprintf( pOutputFile, "Hostname\tFunction Name\tError Number\tError Message\n" );

			fclose( pOutputFile );
		}
	}

	pOutputFile = fopen( "ErrorLog.txt", "a+" );

	if ( pOutputFile != NULL )
	{
		fprintf( pOutputFile, "%s\t%s\t%d\t%s\n", szTarget, szFunction, *dwError, szErrorMsg );

		fclose( pOutputFile );
	}

	ReleaseSemaphore( hSemaphore, 1, NULL );
}

VOID GetAccountPolicyInfo( CHAR szTarget[], BOOL *bMultipleHosts )
{
	CHAR                       *pLocation;
	CHAR                     szTempTarget[ 128 ];
	WCHAR                       wszTarget[ 256 ];
	DWORD                         dwLevel;
	USER_MODALS_INFO_0            *pInfo0;
	NET_API_STATUS                nStatus;
	CHAR                   szMaxPasswdAge[ 128 ];
	CHAR                   szMinPasswdAge[ 128 ];
	CHAR                    szForceLogoff[ 128 ];
	DWORD                         dwError;
	USER_MODALS_INFO_1            *pInfo1;
	CHAR                      szPrimaryDC[ 128 ];
	USER_MODALS_INFO_2            *pInfo2;
	CHAR                     szDomainName[ 128 ];
	USER_MODALS_INFO_3            *pInfo3;
	CHAR                szLockoutDuration[ 128 ];
	CHAR                    szResetWindow[ 128 ];
	CHAR               szLockoutThreshold[ 128 ];
	FILE                     *pOutputFile;

	pLocation = strstr( szTarget, "\\\\" );

	if ( pLocation != NULL )
	{
		strcpy( szTempTarget, szTarget );
	}
	else
	{
		sprintf( szTempTarget, "\\\\%s", szTarget );
	}

	MultiByteToWideChar( CP_ACP, 0, szTempTarget, strlen( szTempTarget ) + 1, wszTarget, sizeof( wszTarget ) / sizeof( wszTarget[0] ) );

	dwLevel = 0;
	pInfo0  = NULL;

	nStatus = NetUserModalsGet( wszTarget, dwLevel, (PBYTE *)&pInfo0 );

	if ( nStatus == NERR_Success )
	{
		if ( pInfo0 != NULL )
		{
			if ( pInfo0->usrmod0_max_passwd_age == TIMEQ_FOREVER )
			{
				strcpy( szMaxPasswdAge, "Passwords never expire" );
			}
			else
			{
				sprintf( szMaxPasswdAge, "%d days", pInfo0->usrmod0_max_passwd_age / 86400 );
			}

			sprintf( szMinPasswdAge, "%d days", pInfo0->usrmod0_min_passwd_age / 86400 );

			if ( pInfo0->usrmod0_force_logoff == TIMEQ_FOREVER )
			{
				strcpy( szForceLogoff, "Users are not forced to logoff" );
			}
			else
			{
				sprintf( szForceLogoff, "%d seconds", pInfo0->usrmod0_force_logoff );
			}
		}
	}
	else
	{
		if ( nStatus == ERROR_ACCESS_DENIED )
		{
			WriteToErrorLog( szTarget, "NetUserModalsGet", "The user does not have access to the requested information.", bMultipleHosts );
		}
		else if ( nStatus == NERR_InvalidComputer )
		{
			WriteToErrorLog( szTarget, "NetUserModalsGet", "The computer name is invalid.", bMultipleHosts );
		}
		else
		{
			dwError = nStatus;

			WriteLastErrorToErrorLog( szTarget, "NetUserModalsGet", &dwError, bMultipleHosts );
		}
	}

	dwLevel = 1;
	pInfo1  = NULL;

	nStatus = NetUserModalsGet( wszTarget, dwLevel, (PBYTE *)&pInfo1 );

	if ( nStatus == NERR_Success )
	{
		if ( pInfo1 != NULL )
		{
			WideCharToMultiByte( CP_ACP, 0, pInfo1->usrmod1_primary, -1, szPrimaryDC, 128, NULL, NULL );
		}
	}
	else
	{
		if ( nStatus == ERROR_ACCESS_DENIED )
		{
			WriteToErrorLog( szTarget, "NetUserModalsGet", "The user does not have access to the requested information.", bMultipleHosts );
		}
		else if ( nStatus == NERR_InvalidComputer )
		{
			WriteToErrorLog( szTarget, "NetUserModalsGet", "The computer name is invalid.", bMultipleHosts );
		}
		else
		{
			dwError = nStatus;

			WriteLastErrorToErrorLog( szTarget, "NetUserModalsGet", &dwError, bMultipleHosts );
		}
	}

	dwLevel = 2;
	pInfo2  = NULL;

	nStatus = NetUserModalsGet( wszTarget, dwLevel, (PBYTE *)&pInfo2 );

	if ( nStatus == NERR_Success )
	{
		if ( pInfo2 != NULL )
		{
			WideCharToMultiByte( CP_ACP, 0, pInfo2->usrmod2_domain_name, -1, szDomainName, 128, NULL, NULL );
		}
	}
	else
	{
		if ( nStatus == ERROR_ACCESS_DENIED )
		{
			WriteToErrorLog( szTarget, "NetUserModalsGet", "The user does not have access to the requested information.", bMultipleHosts );
		}
		else if ( nStatus == NERR_InvalidComputer )
		{
			WriteToErrorLog( szTarget, "NetUserModalsGet", "The computer name is invalid.", bMultipleHosts );
		}
		else
		{
			dwError = nStatus;

			WriteLastErrorToErrorLog( szTarget, "NetUserModalsGet", &dwError, bMultipleHosts );
		}
	}

	dwLevel = 3;
	pInfo3  = NULL;

	nStatus = NetUserModalsGet( wszTarget, dwLevel, (PBYTE *)&pInfo3 );

	if ( nStatus == NERR_Success )
	{
		if ( pInfo3 != NULL )
		{
			if ( pInfo3->usrmod3_lockout_duration == TIMEQ_FOREVER )
			{
				strcpy( szLockoutDuration, "Forever" );
			}
			else
			{
				sprintf( szLockoutDuration, "%d minutes", pInfo3->usrmod3_lockout_duration / 60 );
			}

			sprintf( szResetWindow, "%d minutes", pInfo3->usrmod3_lockout_observation_window / 60 );

			if ( pInfo3->usrmod3_lockout_threshold == 0 )
			{
				strcpy( szLockoutThreshold, "Accounts do not lockout" );
			}
			else
			{
				sprintf( szLockoutThreshold, "%d attempts", pInfo3->usrmod3_lockout_threshold );
			}
		}
	}
	else
	{
		if ( nStatus == ERROR_ACCESS_DENIED )
		{
			WriteToErrorLog( szTarget, "NetUserModalsGet", "The user does not have access to the requested information.", bMultipleHosts );
		}
		else if ( nStatus == NERR_InvalidComputer )
		{
			WriteToErrorLog( szTarget, "NetUserModalsGet", "The computer name is invalid.", bMultipleHosts );
		}
		else
		{
			dwError = nStatus;

			WriteLastErrorToErrorLog( szTarget, "NetUserModalsGet", &dwError, bMultipleHosts );
		}
	}

	if ( pInfo0 != NULL && pInfo1 != NULL && pInfo2 != NULL && pInfo3 != NULL )
	{
		if ( !*bMultipleHosts )
		{
			printf( "\n" );
			printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
			printf( "+++++       ACCOUNT POLICY INFORMATION        +++++\n" );
			printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
			printf( "\n" );

			printf( "Lockout Duration:          %s\n",            szLockoutDuration );
			printf( "Counter Reset After:       %s\n",            szResetWindow );
			printf( "Lockout Threshold:         %s\n",            szLockoutThreshold );
			printf( "Minimum Password Length:   %d characters\n", pInfo0->usrmod0_min_passwd_len );
			printf( "Maximum Password Age:      %s\n",            szMaxPasswdAge );
			printf( "Minimum Password Age:      %s\n",            szMinPasswdAge );
			printf( "Force Logoff After:        %s\n",            szForceLogoff );
			printf( "Password History Length:   %d passwords\n",  pInfo0->usrmod0_password_hist_len );
			printf( "Primary Domain Controller: %s\n",            szPrimaryDC );
			printf( "Domain Name:               %s\n",            szDomainName );
			printf( "\n" );

			fflush( stdout );
		}

		WaitForSingleObject( hSemaphore, INFINITE );

		pOutputFile = fopen( "AccountPolicyInfo.txt", "r" );

		if ( pOutputFile != NULL )
		{
			fclose( pOutputFile );
		}
		else
		{
			pOutputFile = fopen( "AccountPolicyInfo.txt", "w" );

			if ( pOutputFile != NULL )
			{
				fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
				fprintf( pOutputFile, "\n" );
				fprintf( pOutputFile, "Hostname\tLockout Duration\tCounter Reset After\tLockout Threshold\tMinimum Password Length\tMaximum Password Age\tMinimum Password Age\tForce Logoff After\tPassword History Length\tPrimary Domain Controller\tDomain Name\n" );

				fclose( pOutputFile );
			}
		}

		pOutputFile = fopen( "AccountPolicyInfo.txt", "a+" );

		if ( pOutputFile != NULL )
		{
			fprintf( pOutputFile, "%s\t%s\t%s\t%s\t%d characters\t%s\t%s\t%s\t%d passwords\t%s\t%s\n", szTarget, szLockoutDuration, szResetWindow, szLockoutThreshold, pInfo0->usrmod0_min_passwd_len, szMaxPasswdAge, szMinPasswdAge, szForceLogoff, pInfo0->usrmod0_password_hist_len, szPrimaryDC, szDomainName );

			fclose( pOutputFile );
		}

		ReleaseSemaphore( hSemaphore, 1, NULL );
	}

	if ( pInfo0 != NULL )
	{
		NetApiBufferFree( pInfo0 );
	}

	if ( pInfo1 != NULL )
	{
		NetApiBufferFree( pInfo1 );
	}

	if ( pInfo2 != NULL )
	{
		NetApiBufferFree( pInfo2 );
	}

	if ( pInfo3 != NULL )
	{
		NetApiBufferFree( pInfo3 );
	}
}

VOID WriteToErrorLog( CHAR szTarget[], CHAR szFunction[], CHAR szErrorMsg[], BOOL *bMultipleHosts )
{
	FILE *pOutputFile;

	if ( !*bMultipleHosts )
	{
		fprintf( stderr, "ERROR! %s - %s\n", szFunction, szErrorMsg );

		fflush( stderr );
	}

	WaitForSingleObject( hSemaphore, INFINITE );

	pOutputFile = fopen( "ErrorLog.txt", "r" );

	if ( pOutputFile != NULL )
	{
		fclose( pOutputFile );
	}
	else
	{
		pOutputFile = fopen( "ErrorLog.txt", "w" );

		if ( pOutputFile != NULL )
		{
			fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
			fprintf( pOutputFile, "\n" );
			fprintf( pOutputFile, "Hostname\tFunction Name\tError Number\tError Message\n" );

			fclose( pOutputFile );
		}
	}

	pOutputFile = fopen( "ErrorLog.txt", "a+" );

	if ( pOutputFile != NULL )
	{
		fprintf( pOutputFile, "%s\t%s\t-\t%s\n", szTarget, szFunction, szErrorMsg );

		fclose( pOutputFile );
	}

	ReleaseSemaphore( hSemaphore, 1, NULL );
}

VOID GetDomainInfo( CHAR szTarget[], BOOL *bMultipleHosts )
{
	WCHAR                             wszTarget[ 256 ];
	LSA_OBJECT_ATTRIBUTES         loaAttributes;
	LSA_UNICODE_STRING            lusSystemName;
	NTSTATUS                           ntStatus;
	LSA_HANDLE                 lsahPolicyHandle;
	POLICY_ACCOUNT_DOMAIN_INFO           *pInfo;
	CHAR                           szDomainName[ 128 ];
	FILE                           *pOutputFile;
	DWORD                               dwError;

	MultiByteToWideChar( CP_ACP, 0, szTarget, strlen( szTarget ) + 1, wszTarget, sizeof( wszTarget ) / sizeof( wszTarget[0] ) );

	ZeroMemory( &loaAttributes, sizeof( loaAttributes ) );

	lusSystemName.Buffer        = wszTarget;
	lusSystemName.Length        = wcslen( wszTarget ) * sizeof( WCHAR );
	lusSystemName.MaximumLength = ( wcslen( wszTarget ) + 1 ) * sizeof( WCHAR );

	ntStatus = LsaOpenPolicy( &lusSystemName, &loaAttributes, POLICY_VIEW_LOCAL_INFORMATION, &lsahPolicyHandle );

	if ( ntStatus == 0 )
	{
		pInfo = NULL;

		ntStatus = LsaQueryInformationPolicy( lsahPolicyHandle, PolicyAccountDomainInformation, (PVOID *)&pInfo );

		if ( ntStatus == 0 )
		{
			sprintf( szDomainName, "%.*S", pInfo->DomainName.Length, pInfo->DomainName.Buffer );

			if ( !*bMultipleHosts )
			{
				printf( "\n" );
				printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
				printf( "+++++           DOMAIN INFORMATION            +++++\n" );
				printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
				printf( "\n" );

				printf( "Domain Name: %s\n", szDomainName );
				printf( "\n" );

				fflush( stdout );
			}

			WaitForSingleObject( hSemaphore, INFINITE );

			pOutputFile = fopen( "DomainInfo.txt", "r" );

			if ( pOutputFile != NULL )
			{
				fclose( pOutputFile );
			}
			else
			{
				pOutputFile = fopen( "DomainInfo.txt", "w" );

				if ( pOutputFile != NULL )
				{
					fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
					fprintf( pOutputFile, "\n" );
					fprintf( pOutputFile, "Hostname\tDomain Name\n" );

					fclose( pOutputFile );
				}
			}

			pOutputFile = fopen( "DomainInfo.txt", "a+" );

			if ( pOutputFile != NULL )
			{
				fprintf( pOutputFile, "%s\t%s\n", szTarget, szDomainName );

				fclose( pOutputFile );
			}

			ReleaseSemaphore( hSemaphore, 1, NULL );

			LsaFreeMemory( pInfo );
		}
		else
		{
			dwError = LsaNtStatusToWinError( ntStatus );

			WriteLastErrorToErrorLog( szTarget, "LsaQueryInformationPolicy", &dwError, bMultipleHosts );
		}

		LsaClose( lsahPolicyHandle );
	}
	else
	{
		dwError = LsaNtStatusToWinError( ntStatus );

		WriteLastErrorToErrorLog( szTarget, "LsaOpenPolicy", &dwError, bMultipleHosts );
	}
}

VOID GetLocalGroupInfo( CHAR szTarget[], BOOL *bMultipleHosts )
{
	CHAR                           *pLocation;
	CHAR                         szTempTarget[ 128 ];
	WCHAR                           wszTarget[ 256 ];
	DWORD                                   i;
	DWORD                            dwLevel1;
	DWORD                      dwEntriesRead1;
	DWORD                     dwTotalEntries1;
	DWORD                     dwResumeHandle1;
	LOCALGROUP_INFO_1                 *pInfo1;
	NET_API_STATUS                   nStatus1;
	LOCALGROUP_INFO_1             *pTempInfo1;
	DWORD                                   j;
	WCHAR                        wszGroupName[ 256 ];
	CHAR                          szGroupName[ 128 ];
	DWORD                            dwLevel2;
	DWORD                      dwEntriesRead2;
	DWORD                     dwTotalEntries2;
	DWORD                     dwResumeHandle2;
	LOCALGROUP_MEMBERS_INFO_2         *pInfo2;
	NET_API_STATUS                   nStatus2;
	LOCALGROUP_MEMBERS_INFO_2     *pTempInfo2;
	DWORD                                   k;
	CHAR                           szUsername[ 128 ];
	FILE                         *pOutputFile;
	DWORD                             dwError;

	pLocation = strstr( szTarget, "\\\\" );

	if ( pLocation != NULL )
	{
		strcpy( szTempTarget, szTarget );
	}
	else
	{
		sprintf( szTempTarget, "\\\\%s", szTarget );
	}

	MultiByteToWideChar( CP_ACP, 0, szTempTarget, strlen( szTempTarget ) + 1, wszTarget, sizeof( wszTarget ) / sizeof( wszTarget[0] ) );

	i = 0;

	dwLevel1        = 1;
	dwEntriesRead1  = 0;
	dwTotalEntries1 = 0;
	dwResumeHandle1 = 0;

	do
	{
		pInfo1 = NULL;

		nStatus1 = NetLocalGroupEnum( wszTarget, dwLevel1, (PBYTE *)&pInfo1, 4096, &dwEntriesRead1, &dwTotalEntries1, &dwResumeHandle1 );

		if ( nStatus1 == NERR_Success || nStatus1 == ERROR_MORE_DATA )
		{
			if ( pInfo1 != NULL )
			{
				pTempInfo1 = pInfo1;

				for ( j = 0; j < dwEntriesRead1; j++ )
				{
					wcscpy( wszGroupName, pTempInfo1->lgrpi1_name );

					WideCharToMultiByte( CP_ACP, 0, wszGroupName, -1, szGroupName, 128, NULL, NULL );

					dwLevel2        = 2;
					dwEntriesRead2  = 0;
					dwTotalEntries2 = 0;
					dwResumeHandle2 = 0;

					do
					{
						pInfo2 = NULL;

						nStatus2 = NetLocalGroupGetMembers( wszTarget, wszGroupName, dwLevel2, (PBYTE *)&pInfo2, 4096, &dwEntriesRead2, &dwTotalEntries2, &dwResumeHandle2 );

						if ( nStatus2 == NERR_Success || nStatus2 == ERROR_MORE_DATA )
						{
							if ( pInfo2 != NULL )
							{
								pTempInfo2 = pInfo2;

								for ( k = 0; k < dwEntriesRead2; k++ )
								{
									WideCharToMultiByte( CP_ACP, 0, pTempInfo2->lgrmi2_domainandname, -1, szUsername, 128, NULL, NULL );

									if ( !*bMultipleHosts )
									{
										if ( i == 0 )
										{
											printf( "\n" );
											printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
											printf( "+++++         LOCAL GROUP INFORMATION         +++++\n" );
											printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
											printf( "\n" );

											i++;
										}

										printf( "Group Name: %s\n", szGroupName );
										printf( "Group Type: Local\n" );
										printf( "Username:   %s\n", szUsername );
										printf( "\n" );

										fflush( stdout );
									}

									WaitForSingleObject( hSemaphore, INFINITE );

									pOutputFile = fopen( "GroupInfo.txt", "r" );

									if ( pOutputFile != NULL )
									{
										fclose( pOutputFile );
									}
									else
									{
										pOutputFile = fopen( "GroupInfo.txt", "w" );

										if ( pOutputFile != NULL )
										{
											fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
											fprintf( pOutputFile, "\n" );
											fprintf( pOutputFile, "Hostname\tGroup Name\tGroup Type\tUsername\n" );

											fclose( pOutputFile );
										}
									}

									pOutputFile = fopen( "GroupInfo.txt", "a+" );

									if ( pOutputFile != NULL )
									{
										fprintf( pOutputFile, "%s\t%s\tLocal\t%s\n", szTarget, szGroupName, szUsername );

										fclose( pOutputFile );
									}

									ReleaseSemaphore( hSemaphore, 1, NULL );

									pTempInfo2++;
								}
							}
						}
						else
						{
							if ( nStatus2 == ERROR_ACCESS_DENIED )
							{
								WriteToErrorLog( szTarget, "NetLocalGroupGetMembers", "The user does not have access to the requested information.", bMultipleHosts );
							}
							else if ( nStatus2 == NERR_InvalidComputer )
							{
								WriteToErrorLog( szTarget, "NetLocalGroupGetMembers", "The computer name is invalid.", bMultipleHosts );
							}
							else if ( nStatus2 == ERROR_NO_SUCH_ALIAS )
							{
								WriteToErrorLog( szTarget, "NetLocalGroupGetMembers", "The specified local group does not exist.", bMultipleHosts );
							}
							else
							{
								dwError = nStatus2;

								WriteLastErrorToErrorLog( szTarget, "NetLocalGroupGetMembers", &dwError, bMultipleHosts );
							}
						}

						if ( pInfo2 != NULL )
						{
							NetApiBufferFree( pInfo2 );
						}
					}
					while ( nStatus2 == ERROR_MORE_DATA );

					pTempInfo1++;
				}
			}
		}
		else
		{
			if ( nStatus1 == ERROR_ACCESS_DENIED )
			{
				WriteToErrorLog( szTarget, "NetLocalGroupEnum", "The user does not have access to the requested information.", bMultipleHosts );
			}
			else if ( nStatus1 == NERR_InvalidComputer )
			{
				WriteToErrorLog( szTarget, "NetLocalGroupEnum", "The computer name is invalid.", bMultipleHosts );
			}
			else if ( nStatus1 == NERR_BufTooSmall )
			{
				WriteToErrorLog( szTarget, "NetLocalGroupEnum", "The return buffer is too small.", bMultipleHosts );
			}
			else
			{
				dwError = nStatus1;

				WriteLastErrorToErrorLog( szTarget, "NetLocalGroupEnum", &dwError, bMultipleHosts );
			}
		}

		if ( pInfo1 != NULL )
		{
			NetApiBufferFree( pInfo1 );
		}
	}
	while ( nStatus1 == ERROR_MORE_DATA );
}

VOID GetGlobalGroupInfo( CHAR szTarget[], BOOL *bMultipleHosts )
{
	CHAR                    *pLocation;
	CHAR                  szTempTarget[ 128 ];
	WCHAR                    wszTarget[ 256 ];
	DWORD                            i;
	DWORD                     dwLevel1;
	DWORD               dwEntriesRead1;
	DWORD              dwTotalEntries1;
	DWORD              dwResumeHandle1;
	GROUP_INFO_1               *pInfo1;
	NET_API_STATUS            nStatus1;
	GROUP_INFO_1           *pTempInfo1;
	DWORD                            j;
	WCHAR                 wszGroupName[ 256 ];
	CHAR                   szGroupName[ 128 ];
	DWORD                     dwLevel0;
	DWORD               dwEntriesRead2;
	DWORD              dwTotalEntries2;
	DWORD              dwResumeHandle2;
	GROUP_USERS_INFO_0         *pInfo0;
	NET_API_STATUS            nStatus2;
	GROUP_USERS_INFO_0     *pTempInfo0;
	DWORD                            k;
	CHAR                    szUsername[ 128 ];
	FILE                  *pOutputFile;
	DWORD                      dwError;

	pLocation = strstr( szTarget, "\\\\" );

	if ( pLocation != NULL )
	{
		strcpy( szTempTarget, szTarget );
	}
	else
	{
		sprintf( szTempTarget, "\\\\%s", szTarget );
	}

	MultiByteToWideChar( CP_ACP, 0, szTempTarget, strlen( szTempTarget ) + 1, wszTarget, sizeof( wszTarget ) / sizeof( wszTarget[0] ) );

	i = 0;

	dwLevel1        = 1;
	dwEntriesRead1  = 0;
	dwTotalEntries1 = 0;
	dwResumeHandle1 = 0;

	do
	{
		pInfo1 = NULL;

		nStatus1 = NetGroupEnum( wszTarget, dwLevel1, (PBYTE *)&pInfo1, 4096, &dwEntriesRead1, &dwTotalEntries1, &dwResumeHandle1 );

		if ( nStatus1 == NERR_Success || nStatus1 == ERROR_MORE_DATA )
		{
			if ( pInfo1 != NULL )
			{
				pTempInfo1 = pInfo1;

				for ( j = 0; j < dwEntriesRead1; j++ )
				{
					wcscpy( wszGroupName, pTempInfo1->grpi1_name );

					WideCharToMultiByte( CP_ACP, 0, wszGroupName, -1, szGroupName, 128, NULL, NULL );

					dwLevel0        = 0;
					dwEntriesRead2  = 0;
					dwTotalEntries2 = 0;
					dwResumeHandle2 = 0;

					do
					{
						pInfo0 = NULL;

						nStatus2 = NetGroupGetUsers( wszTarget, wszGroupName, dwLevel0, (PBYTE *)&pInfo0, 4096, &dwEntriesRead2, &dwTotalEntries2, &dwResumeHandle2 );

						if ( nStatus2 == NERR_Success || nStatus2 == ERROR_MORE_DATA )
						{
							if ( pInfo0 != NULL )
							{
								pTempInfo0 = pInfo0;

								for ( k = 0; k < dwEntriesRead2; k++ )
								{
									WideCharToMultiByte( CP_ACP, 0, pTempInfo0->grui0_name, -1, szUsername, 128, NULL, NULL );

									if ( !*bMultipleHosts )
									{
										if ( i == 0 )
										{
											printf( "\n" );
											printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
											printf( "+++++        GLOBAL GROUP INFORMATION         +++++\n" );
											printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
											printf( "\n" );

											i++;
										}

										printf( "Group Name: %s\n", szGroupName );
										printf( "Group Type: Global\n" );
										printf( "Username:   %s\n", szUsername );
										printf( "\n" );

										fflush( stdout );
									}

									WaitForSingleObject( hSemaphore, INFINITE );

									pOutputFile = fopen( "GroupInfo.txt", "r" );

									if ( pOutputFile != NULL )
									{
										fclose( pOutputFile );
									}
									else
									{
										pOutputFile = fopen( "GroupInfo.txt", "w" );

										if ( pOutputFile != NULL )
										{
											fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
											fprintf( pOutputFile, "\n" );
											fprintf( pOutputFile, "Hostname\tGroup Name\tGroup Type\tUsername\n" );

											fclose( pOutputFile );
										}
									}

									pOutputFile = fopen( "GroupInfo.txt", "a+" );

									if ( pOutputFile != NULL )
									{
										fprintf( pOutputFile, "%s\t%s\tGlobal\t%s\n", szTarget, szGroupName, szUsername );

										fclose( pOutputFile );
									}

									ReleaseSemaphore( hSemaphore, 1, NULL );

									pTempInfo0++;
								}
							}
						}
						else
						{
							if ( nStatus2 == ERROR_ACCESS_DENIED )
							{
								WriteToErrorLog( szTarget, "NetGroupGetUsers", "The user does not have access to the requested information.", bMultipleHosts );
							}
							else if ( nStatus2 == ERROR_INVALID_LEVEL )
							{
								WriteToErrorLog( szTarget, "NetGroupGetUsers", "The system call level is not correct.", bMultipleHosts );
							}
							else if ( nStatus2 == ERROR_NOT_ENOUGH_MEMORY )
							{
								WriteToErrorLog( szTarget, "NetGroupGetUsers", "Insufficient memory was available to complete the operation.", bMultipleHosts );
							}
							else if ( nStatus2 == NERR_InvalidComputer )
							{
								WriteToErrorLog( szTarget, "NetGroupGetUsers", "The computer name is invalid.", bMultipleHosts );
							}
							else if ( nStatus2 == NERR_GroupNotFound )
							{
								WriteToErrorLog( szTarget, "NetGroupGetUsers", "The global group name could not be found.", bMultipleHosts );
							}
							else if ( nStatus2 == NERR_InternalError )
							{
								WriteToErrorLog( szTarget, "NetGroupGetUsers", "An internal error occurred.", bMultipleHosts );
							}
							else
							{
								dwError = nStatus2;

								WriteLastErrorToErrorLog( szTarget, "NetGroupGetUsers", &dwError, bMultipleHosts );
							}
						}

						if ( pInfo0 != NULL )
						{
							NetApiBufferFree( pInfo0 );
						}
					}
					while ( nStatus2 == ERROR_MORE_DATA );

					pTempInfo1++;
				}
			}
		}
		else
		{
			if ( nStatus1 == ERROR_ACCESS_DENIED )
			{
				WriteToErrorLog( szTarget, "NetGroupEnum", "The user does not have access to the requested information.", bMultipleHosts );
			}
			else if ( nStatus1 == NERR_InvalidComputer )
			{
				WriteToErrorLog( szTarget, "NetGroupEnum", "The computer name is invalid.", bMultipleHosts );
			}
			else
			{
				dwError = nStatus1;

				WriteLastErrorToErrorLog( szTarget, "NetGroupEnum", &dwError, bMultipleHosts );
			}
		}

		if ( pInfo1 != NULL )
		{
			NetApiBufferFree( pInfo1 );
		}
	}
	while ( nStatus1 == ERROR_MORE_DATA );
}

VOID GetLoggedOnUsers( CHAR szTarget[], BOOL *bMultipleHosts )
{
	CHAR                  *pLocation;
	CHAR                szTempTarget[ 128 ];
	WCHAR                  wszTarget[ 256 ];
	DWORD                          i;
	DWORD                    dwLevel;
	DWORD              dwEntriesRead;
	DWORD             dwTotalEntries;
	DWORD             dwResumeHandle;
	WKSTA_USER_INFO_1         *pInfo;
	NET_API_STATUS           nStatus;
	WKSTA_USER_INFO_1     *pTempInfo;
	DWORD                          j;
	CHAR                  szUsername[ 128 ];
	CHAR               szLogonDomain[ 128 ];
	CHAR              szOtherDomains[ 256 ];
	CHAR               szLogonServer[ 128 ];
	FILE                *pOutputFile;
	DWORD                    dwError;

	pLocation = strstr( szTarget, "\\\\" );

	if ( pLocation != NULL )
	{
		strcpy( szTempTarget, szTarget );
	}
	else
	{
		sprintf( szTempTarget, "\\\\%s", szTarget );
	}

	MultiByteToWideChar( CP_ACP, 0, szTempTarget, strlen( szTempTarget ) + 1, wszTarget, sizeof( wszTarget ) / sizeof( wszTarget[0] ) );

	i = 0;

	dwLevel        = 1;
	dwEntriesRead  = 0;
	dwTotalEntries = 0;
	dwResumeHandle = 0;

	do
	{
		pInfo = NULL;

		nStatus = NetWkstaUserEnum( wszTarget, dwLevel, (PBYTE *)&pInfo, 4096, &dwEntriesRead, &dwTotalEntries, &dwResumeHandle );

		if ( nStatus == NERR_Success || nStatus == ERROR_MORE_DATA )
		{
			if ( pInfo != NULL )
			{
				pTempInfo = pInfo;

				for ( j = 0; j < dwEntriesRead; j++ )
				{
					WideCharToMultiByte( CP_ACP, 0, pTempInfo->wkui1_username,     -1, szUsername,     128, NULL, NULL );
					WideCharToMultiByte( CP_ACP, 0, pTempInfo->wkui1_logon_domain, -1, szLogonDomain,  128, NULL, NULL );
					WideCharToMultiByte( CP_ACP, 0, pTempInfo->wkui1_oth_domains,  -1, szOtherDomains, 256, NULL, NULL );
					WideCharToMultiByte( CP_ACP, 0, pTempInfo->wkui1_logon_server, -1, szLogonServer,  128, NULL, NULL );

					if ( !*bMultipleHosts )
					{
						if ( i == 0 )
						{
							printf( "\n" );
							printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
							printf( "+++++             LOGGED ON USERS             +++++\n" );
							printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
							printf( "\n" );

							i++;
						}

						printf( "Username:      %s\n", szUsername );
						printf( "Logon Domain:  %s\n", szLogonDomain );
						printf( "Other Domains: %s\n", szOtherDomains );
						printf( "Logon Server:  %s\n", szLogonServer );
						printf( "\n" );

						fflush( stdout );
					}

					WaitForSingleObject( hSemaphore, INFINITE );

					pOutputFile = fopen( "LoggedOnUsers.txt", "r" );

					if ( pOutputFile != NULL )
					{
						fclose( pOutputFile );
					}
					else
					{
						pOutputFile = fopen( "LoggedOnUsers.txt", "w" );

						if ( pOutputFile != NULL )
						{
							fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
							fprintf( pOutputFile, "\n" );
							fprintf( pOutputFile, "Hostname\tUsername\tLogon Domain\tOther Domains\tLogon Server\n" );

							fclose( pOutputFile );
						}
					}

					pOutputFile = fopen( "LoggedOnUsers.txt", "a+" );

					if ( pOutputFile != NULL )
					{
						fprintf( pOutputFile, "%s\t%s\t%s\t%s\t%s\n", szTarget, szUsername, szLogonDomain, szOtherDomains, szLogonServer );

						fclose( pOutputFile );
					}

					ReleaseSemaphore( hSemaphore, 1, NULL );

					pTempInfo++;
				}
			}
		}
		else
		{
			if ( nStatus == ERROR_ACCESS_DENIED )
			{
				WriteToErrorLog( szTarget, "NetWkstaUserEnum", "The user does not have access to the requested information.", bMultipleHosts );
			}
			else if ( nStatus == ERROR_INVALID_LEVEL )
			{
				WriteToErrorLog( szTarget, "NetWkstaUserEnum", "The level parameter is invalid.", bMultipleHosts );
			}
			else
			{
				dwError = nStatus;

				WriteLastErrorToErrorLog( szTarget, "NetWkstaUserEnum", &dwError, bMultipleHosts );
			}
		}

		if ( pInfo != NULL )
		{
			NetApiBufferFree( pInfo );
		}
	}
	while ( nStatus == ERROR_MORE_DATA );
}

VOID GetShareInfo( CHAR szTarget[], BOOL *bMultipleHosts )
{
	CHAR               *pLocation;
	CHAR             szTempTarget[ 128 ];
	WCHAR               wszTarget[ 256 ];
	DWORD                       i;
	DWORD                 dwLevel;
	DWORD           dwEntriesRead;
	DWORD          dwTotalEntries;
	DWORD          dwResumeHandle;
	SHARE_INFO_1           *pInfo;
	NET_API_STATUS        nStatus;
	SHARE_INFO_1       *pTempInfo;
	DWORD                       j;
	CHAR              szShareName[ 128 ];
	CHAR              szShareType[ 128 ];
	CHAR                 szRemark[ 512 ];
	FILE             *pOutputFile;
	DWORD                 dwError;

	pLocation = strstr( szTarget, "\\\\" );

	if ( pLocation != NULL )
	{
		strcpy( szTempTarget, szTarget );
	}
	else
	{
		sprintf( szTempTarget, "\\\\%s", szTarget );
	}

	MultiByteToWideChar( CP_ACP, 0, szTempTarget, strlen( szTempTarget ) + 1, wszTarget, sizeof( wszTarget ) / sizeof( wszTarget[0] ) );

	i = 0;

	dwLevel        = 1;
	dwEntriesRead  = 0;
	dwTotalEntries = 0;
	dwResumeHandle = 0;

	do
	{
		pInfo = NULL;

		nStatus = NetShareEnum( wszTarget, dwLevel, (PBYTE *)&pInfo, 4096, &dwEntriesRead, &dwTotalEntries, &dwResumeHandle );

		if ( nStatus == NERR_Success || nStatus == ERROR_MORE_DATA )
		{
			if ( pInfo != NULL )
			{
				pTempInfo = pInfo;

				for ( j = 0; j < dwEntriesRead; j++ )
				{
					WideCharToMultiByte( CP_ACP, 0, (WCHAR *)pTempInfo->shi1_netname, -1, szShareName, 128, NULL, NULL );
					WideCharToMultiByte( CP_ACP, 0, (WCHAR *)pTempInfo->shi1_remark,  -1, szRemark,    512, NULL, NULL );

					strcpy( szShareType, "" );

					if ( pTempInfo->shi1_type == STYPE_DISKTREE )
					{
						strcpy( szShareType, "Disk drive" );
					}
					else if ( pTempInfo->shi1_type == STYPE_PRINTQ )
					{
						strcpy( szShareType, "Print queue" );
					}
					else if ( pTempInfo->shi1_type == STYPE_DEVICE )
					{
						strcpy( szShareType, "Communication device" );
					}
					else if ( pTempInfo->shi1_type == STYPE_IPC )
					{
						strcpy( szShareType, "Interprocess communication (IPC)" );
					}
					else if ( pTempInfo->shi1_type == STYPE_SPECIAL )
					{
						strcpy( szShareType, "Administrative share" );
					}
					else
					{
						strcpy( szShareType, "Unknown share type" );
					}

					if ( !*bMultipleHosts )
					{
						if ( i == 0 )
						{
							printf( "\n" );
							printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
							printf( "+++++            SHARE INFORMATION            +++++\n" );
							printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
							printf( "\n" );

							i++;
						}

						printf( "Share Name: %s\n", szShareName );
						printf( "Share Type: %s\n", szShareType );
						printf( "Remark:     %s\n", szRemark );
						printf( "\n" );

						fflush( stdout );
					}

					WaitForSingleObject( hSemaphore, INFINITE );

					pOutputFile = fopen( "ShareInfo.txt", "r" );

					if ( pOutputFile != NULL )
					{
						fclose( pOutputFile );
					}
					else
					{
						pOutputFile = fopen( "ShareInfo.txt", "w" );

						if ( pOutputFile != NULL )
						{
							fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
							fprintf( pOutputFile, "\n" );
							fprintf( pOutputFile, "Hostname\tShare Name\tShare Type\tRemark\n" );

							fclose( pOutputFile );
						}
					}

					pOutputFile = fopen( "ShareInfo.txt", "a+" );

					if ( pOutputFile != NULL )
					{
						fprintf( pOutputFile, "%s\t%s\t%s\t%s\n", szTarget, szShareName, szShareType, szRemark );

						fclose( pOutputFile );
					}

					ReleaseSemaphore( hSemaphore, 1, NULL );

					pTempInfo++;
				}
			}
		}
		else
		{
			dwError = nStatus;

			WriteLastErrorToErrorLog( szTarget, "NetShareEnum", &dwError, bMultipleHosts );
		}

		if ( pInfo != NULL )
		{
			NetApiBufferFree( pInfo );
		}
	}
	while ( nStatus == ERROR_MORE_DATA );
}

VOID GetUserInfo( CHAR szTarget[], BOOL *bMultipleHosts )
{
	CHAR                *pLocation;
	CHAR              szTempTarget[ 128 ];
	WCHAR               wszTarget1[ 256 ];
	WCHAR               wszTarget2[ 256 ];
	DWORD                        i;
	DWORD                 dwLevel3;
	DWORD            dwEntriesRead;
	DWORD           dwTotalEntries;
	DWORD           dwResumeHandle;
	USER_INFO_3             *pInfo;
	NET_API_STATUS         nStatus;
	USER_INFO_3         *pTempInfo;
	DWORD                        j;
	CHAR                szUsername[ 128 ];
	CHAR                 szHomeDir[ 128 ];
	CHAR                 szComment[ 512 ];
	CHAR              szScriptPath[ 512 ];
	CHAR                szFullName[ 128 ];
	CHAR            szWorkstations[ 128 ];
	CHAR             szLogonServer[ 128 ];
	CHAR                 szProfile[ 128 ];
	CHAR            szHomeDirDrive[ 128 ];
	DWORD                 dwTmpAge;
	DWORD                   dwDays;
	DWORD                  dwHours;
	DWORD                dwMinutes;
	DWORD                dwSeconds;
	CHAR             szPasswordAge[ 128 ];
	CHAR              szPrivileges[ 128 ];
	CHAR                   szFlags[ 128 ];
	CHAR               szLastLogon[ 128 ];
	CHAR              szAcctExpiry[ 128 ];
	CHAR            szPasswdExpiry[ 128 ];
	CHAR               szRASDialIn[ 128 ];
	CHAR             szRASCallBack[ 128 ];
	CHAR           szRASCallBackNo[ 128 ];
	DWORD                 dwLevel0;
	DWORD                 dwStatus;
	RAS_USER_0             ru0Info;
	DWORD                  dwError;
	FILE              *pOutputFile;

	struct tm *pTime;

	pLocation = strstr( szTarget, "\\\\" );

	if ( pLocation != NULL )
	{
		strcpy( szTempTarget, szTarget );
	}
	else
	{
		sprintf( szTempTarget, "\\\\%s", szTarget );
	}

	MultiByteToWideChar( CP_ACP, 0, szTempTarget, strlen( szTempTarget ) + 1, wszTarget1, sizeof( wszTarget1 ) / sizeof( wszTarget1[0] ) );
	MultiByteToWideChar( CP_ACP, 0, szTarget,     strlen( szTarget ) + 1,     wszTarget2, sizeof( wszTarget2 ) / sizeof( wszTarget2[0] ) );

	i = 0;

	dwLevel3       = 3;
	dwEntriesRead  = 0;
	dwTotalEntries = 0;
	dwResumeHandle = 0;

	do
	{
		pInfo = NULL;

		nStatus = NetUserEnum( wszTarget1, dwLevel3, FILTER_NORMAL_ACCOUNT, (PBYTE *)&pInfo, 4096, &dwEntriesRead, &dwTotalEntries, &dwResumeHandle );

		if ( nStatus == NERR_Success || nStatus == ERROR_MORE_DATA )
		{
			if ( pInfo != NULL )
			{
				pTempInfo = pInfo;

				for ( j = 0; j < dwEntriesRead; j++ )
				{
					WideCharToMultiByte( CP_ACP, 0, pTempInfo->usri3_name,           -1, szUsername,     128, NULL, NULL );
					WideCharToMultiByte( CP_ACP, 0, pTempInfo->usri3_home_dir,       -1, szHomeDir,      128, NULL, NULL );
					WideCharToMultiByte( CP_ACP, 0, pTempInfo->usri3_comment,        -1, szComment,      512, NULL, NULL );
					WideCharToMultiByte( CP_ACP, 0, pTempInfo->usri3_script_path,    -1, szScriptPath,   512, NULL, NULL );
					WideCharToMultiByte( CP_ACP, 0, pTempInfo->usri3_full_name,      -1, szFullName,     128, NULL, NULL );
					WideCharToMultiByte( CP_ACP, 0, pTempInfo->usri3_workstations,   -1, szWorkstations, 128, NULL, NULL );
					WideCharToMultiByte( CP_ACP, 0, pTempInfo->usri3_logon_server,   -1, szLogonServer,  128, NULL, NULL );
					WideCharToMultiByte( CP_ACP, 0, pTempInfo->usri3_profile,        -1, szProfile,      128, NULL, NULL );
					WideCharToMultiByte( CP_ACP, 0, pTempInfo->usri3_home_dir_drive, -1, szHomeDirDrive, 128, NULL, NULL );

					dwTmpAge = pTempInfo->usri3_password_age;

					dwDays    = dwTmpAge / 86400;
					dwTmpAge  = dwTmpAge % 86400;
					dwHours   = dwTmpAge / 3600;
					dwTmpAge  = dwTmpAge % 3600;
					dwMinutes = dwTmpAge / 60;
					dwTmpAge  = dwTmpAge % 60;
					dwSeconds = dwTmpAge;

					sprintf( szPasswordAge, "%dd %dh %dm %ds", dwDays, dwHours, dwMinutes, dwSeconds );

					if ( pTempInfo->usri3_priv == USER_PRIV_GUEST )
					{
						strcpy( szPrivileges, "Guest user" );
					}

					if ( pTempInfo->usri3_priv == USER_PRIV_USER )
					{
						strcpy( szPrivileges, "Normal user" );
					}

					if ( pTempInfo->usri3_priv == USER_PRIV_ADMIN )
					{
						strcpy( szPrivileges, "Administrative user" );
					}

					strcpy( szFlags, "" );

					if ( pTempInfo->usri3_flags & UF_LOCKOUT )
					{
						sprintf( szFlags, "%s(Locked out) ", szFlags );
					}

					if ( pTempInfo->usri3_flags & UF_ACCOUNTDISABLE )
					{
						sprintf( szFlags, "%s(Disabled) ", szFlags );
					}

					if ( pTempInfo->usri3_flags & UF_DONT_EXPIRE_PASSWD )
					{
						sprintf( szFlags, "%s(Password never expires) ", szFlags );
					}

					if ( pTempInfo->usri3_last_logon == 0 )
					{
						strcpy( szLastLogon, "Account has never logged on" );
					}
					else
					{
						pTime = localtime( (CONST time_t *)&pTempInfo->usri3_last_logon );

						if ( pTime != NULL )
						{
							strcpy( szLastLogon, asctime( pTime ) );

							pLocation = strchr( szLastLogon, '\n' );

							if ( pLocation != NULL )
							{
								*pLocation = '\0';
							}
						}
						else
						{
							strcpy( szLastLogon, "" );
						}
					}

					if ( pTempInfo->usri3_acct_expires == TIMEQ_FOREVER )
					{
						strcpy( szAcctExpiry, "Account never expires" );
					}
					else
					{
						pTime = localtime( (CONST time_t *)&pTempInfo->usri3_acct_expires );

						if ( pTime != NULL )
						{
							strcpy( szAcctExpiry, asctime( pTime ) );

							pLocation = strchr( szAcctExpiry, '\n' );

							if ( pLocation != NULL )
							{
								*pLocation = '\0';
							}
						}
						else
						{
							strcpy( szAcctExpiry, "" );
						}
					}

					if ( pTempInfo->usri3_password_expired == 0 )
					{
						strcpy( szPasswdExpiry, "Password has not expired" );
					}
					else
					{
						strcpy( szPasswdExpiry, "Password expired" );
					}

					strcpy( szRASDialIn,     "" );
					strcpy( szRASCallBack,   "" );
					strcpy( szRASCallBackNo, "" );

					dwLevel0 = 0;

					dwStatus = MprAdminUserGetInfo( wszTarget2, pTempInfo->usri3_name, dwLevel0, (BYTE *)&ru0Info );

					if ( dwStatus == NO_ERROR )
					{
						if ( ru0Info.bfPrivilege & RASPRIV_DialinPrivilege )
						{
							strcpy( szRASDialIn, "Yes" );
						}
						else
						{
							strcpy( szRASDialIn, "No" );
						}

						if ( ru0Info.bfPrivilege & RASPRIV_NoCallback )
						{
							strcpy( szRASCallBack, "None" );
						}

						if ( ru0Info.bfPrivilege & RASPRIV_CallerSetCallback )
						{
							strcpy( szRASCallBack, "Set by Caller" );
						}

						if ( ru0Info.bfPrivilege & RASPRIV_AdminSetCallback )
						{
							strcpy( szRASCallBack, "Set by Admin" );
						}

						WideCharToMultiByte( CP_ACP, 0, ru0Info.wszPhoneNumber, -1, szRASCallBackNo, 128, NULL, NULL );
					}
					else
					{
						if ( dwStatus == ERROR_ACCESS_DENIED )
						{
							WriteToErrorLog( szTarget, "MprAdminUserGetInfo", "The caller does not have sufficient privileges.", bMultipleHosts );
						}
						else if ( dwStatus == ERROR_INVALID_LEVEL )
						{
							WriteToErrorLog( szTarget, "MprAdminUserGetInfo", "The level parameter is invalid.", bMultipleHosts );
						}
						else if ( dwStatus == ERROR_INVALID_PARAMETER )
						{
							WriteToErrorLog( szTarget, "MprAdminUserGetInfo", "One or more parameters are incorrect.", bMultipleHosts );
						}
						else if ( dwStatus == ERROR_NO_SUCH_USER )
						{
							WriteToErrorLog( szTarget, "MprAdminUserGetInfo", "The specified user does not exist.", bMultipleHosts );
						}
						else
						{
							dwError = nStatus;

							WriteLastErrorToErrorLog( szTarget, "MprAdminUserGetInfo", &dwError, bMultipleHosts );
						}
					}

					if ( !*bMultipleHosts )
					{
						if ( i == 0 )
						{
							printf( "\n" );
							printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
							printf( "+++++            USER INFORMATION             +++++\n" );
							printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
							printf( "\n" );

							i++;
						}

						printf( "Username:             %s\n", szUsername );
						printf( "Full Name:            %s\n", szFullName );
						printf( "Comment:              %s\n", szComment );
						printf( "Privileges:           %s\n", szPrivileges );
						printf( "Flags:                %s\n", szFlags );
						printf( "Password Age:         %s\n", szPasswordAge );
						printf( "Bad Password Count:   %d\n", pTempInfo->usri3_bad_pw_count );
						printf( "Number of Logons:     %d\n", pTempInfo->usri3_num_logons );
						printf( "Last Logon:           %s\n", szLastLogon );
						printf( "Logon Server:         %s\n", szLogonServer );
						printf( "Home Dir:             %s\n", szHomeDir );
						printf( "Home Dir Drive:       %s\n", szHomeDirDrive );
						printf( "Script Path:          %s\n", szScriptPath );
						printf( "Profile:              %s\n", szProfile );
						printf( "Workstations:         %s\n", szWorkstations );
						printf( "User ID:              %d\n", pTempInfo->usri3_user_id );
						printf( "Primary Group ID:     %d\n", pTempInfo->usri3_primary_group_id );
						printf( "Account Expiry:       %s\n", szAcctExpiry );
						printf( "Password Expiry:      %s\n", szPasswdExpiry );
						printf( "RAS Dial-in:          %s\n", szRASDialIn );
						printf( "RAS Call Back:        %s\n", szRASCallBack );
						printf( "RAS Call Back Number: %s\n", szRASCallBackNo );
						printf( "\n" );

						fflush( stdout );
					}

					WaitForSingleObject( hSemaphore, INFINITE );

					pOutputFile = fopen( "UserInfo.txt", "r" );

					if ( pOutputFile != NULL )
					{
						fclose( pOutputFile );
					}
					else
					{
						pOutputFile = fopen( "UserInfo.txt", "w" );

						if ( pOutputFile != NULL )
						{
							fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
							fprintf( pOutputFile, "\n" );
							fprintf( pOutputFile, "Hostname\tUsername\tFull Name\tComment\tPrivileges\tFlags\tPassword Age\tBad Password Count\tNumber of Logons\tLast Logon\tLogon Server\tHome Dir\tHome Dir Drive\tScript Path\tProfile\tWorkstations\tUser ID\tPrimary Group ID\tAccount Expiry\tPassword Expiry\tRAS Dial-in\tRAS Call Back\tRAS Call Back Number\n" );

							fclose( pOutputFile );
						}
					}

					pOutputFile = fopen( "UserInfo.txt", "a+" );

					if ( pOutputFile != NULL )
					{
						fprintf( pOutputFile, "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%d\t%d\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%d\t%d\t%s\t%s\t%s\t%s\t%s\n", szTarget, szUsername, szFullName, szComment, szPrivileges, szFlags, szPasswordAge, pTempInfo->usri3_bad_pw_count, pTempInfo->usri3_num_logons, szLastLogon, szLogonServer, szHomeDir, szHomeDirDrive, szScriptPath, szProfile, szWorkstations, pTempInfo->usri3_user_id, pTempInfo->usri3_primary_group_id, szAcctExpiry, szPasswdExpiry, szRASDialIn, szRASCallBack, szRASCallBackNo );

						fclose( pOutputFile );
					}

					ReleaseSemaphore( hSemaphore, 1, NULL );

					pTempInfo++;
				}
			}
		}
		else
		{
			if ( nStatus == ERROR_ACCESS_DENIED )
			{
				WriteToErrorLog( szTarget, "NetUserEnum", "The user does not have access to the requested information.", bMultipleHosts );
			}
			else if ( nStatus == NERR_InvalidComputer )
			{
				WriteToErrorLog( szTarget, "NetUserEnum", "The computer name is invalid.", bMultipleHosts );
			}
			else
			{
				dwError = nStatus;

				WriteLastErrorToErrorLog( szTarget, "NetUserEnum", &dwError, bMultipleHosts );
			}
		}

		if ( pInfo != NULL )
		{
			NetApiBufferFree( pInfo );
		}
	}
	while ( nStatus == ERROR_MORE_DATA );
}

VOID GetRAUserInfo( CHAR szTarget[], BOOL *bMultipleHosts )
{
	DWORD             i;
	CHAR   szMachineSID[ 128 ];
	CHAR      szUserSID[ 128 ];
	CHAR   szDomainName[ 128 ];
	CHAR  szAccountName[ 128 ];
	DWORD             j;
	FILE   *pOutputFile;

	i = 0;

	if ( GetMachineSID( szTarget, szMachineSID, bMultipleHosts ) )
	{
		for ( j = 500; j < 502; j++ )
		{
			sprintf( szUserSID, "%s-%d", szMachineSID, j );

			strcpy( szDomainName,  "" );
			strcpy( szAccountName, "" );

			if ( GetAccountNameFromSID( szTarget, szUserSID, szDomainName, szAccountName, bMultipleHosts ) )
			{
				if ( !*bMultipleHosts )
				{
					if ( i == 0 )
					{
						printf( "\n" );
						printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
						printf( "+++++     USER INFORMATION VIA RA BYPASS      +++++\n" );
						printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
						printf( "\n" );

						i++;
					}

					printf( "Username: %s\\%s\n", szDomainName, szAccountName );
					printf( "User ID:  %d\n", j );
					printf( "\n" );

					fflush( stdout );
				}

				WaitForSingleObject( hSemaphore, INFINITE );

				pOutputFile = fopen( "RAUserInfo.txt", "r" );

				if ( pOutputFile != NULL )
				{
					fclose( pOutputFile );
				}
				else
				{
					pOutputFile = fopen( "RAUserInfo.txt", "w" );

					if ( pOutputFile != NULL )
					{
						fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
						fprintf( pOutputFile, "\n" );
						fprintf( pOutputFile, "Hostname\tUsername\tUser ID\n" );

						fclose( pOutputFile );
					}
				}

				pOutputFile = fopen( "RAUserInfo.txt", "a+" );

				if ( pOutputFile != NULL )
				{
					fprintf( pOutputFile, "%s\t%s\\%s\t%d\n", szTarget, szDomainName, szAccountName, j );

					fclose( pOutputFile );
				}

				ReleaseSemaphore( hSemaphore, 1, NULL );
			}
		}

		for ( j = 1000; j < 2000; j++ )
		{
			sprintf( szUserSID, "%s-%d", szMachineSID, j );

			strcpy( szDomainName,  "" );
			strcpy( szAccountName, "" );

			if ( GetAccountNameFromSID( szTarget, szUserSID, szDomainName, szAccountName, bMultipleHosts ) )
			{
				if ( !*bMultipleHosts )
				{
					if ( i == 0 )
					{
						printf( "\n" );
						printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
						printf( "+++++     USER INFORMATION VIA RA BYPASS      +++++\n" );
						printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
						printf( "\n" );

						i++;
					}

					printf( "Username: %s\\%s\n", szDomainName, szAccountName );
					printf( "User ID:  %d\n", j );
					printf( "\n" );

					fflush( stdout );
				}

				WaitForSingleObject( hSemaphore, INFINITE );

				pOutputFile = fopen( "RAUserInfo.txt", "r" );

				if ( pOutputFile != NULL )
				{
					fclose( pOutputFile );
				}
				else
				{
					pOutputFile = fopen( "RAUserInfo.txt", "w" );

					if ( pOutputFile != NULL )
					{
						fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
						fprintf( pOutputFile, "\n" );
						fprintf( pOutputFile, "Hostname\tUsername\tUser ID\n" );

						fclose( pOutputFile );
					}
				}

				pOutputFile = fopen( "RAUserInfo.txt", "a+" );

				if ( pOutputFile != NULL )
				{
					fprintf( pOutputFile, "%s\t%s\\%s\t%d\n", szTarget, szDomainName, szAccountName, j );

					fclose( pOutputFile );
				}

				ReleaseSemaphore( hSemaphore, 1, NULL );
			}
		}
	}

}

BOOL GetMachineSID( CHAR szTarget[], CHAR szMachineSID[], BOOL *bMultipleHosts )
{
	BOOL                                bReturn;
	WCHAR                             wszTarget[ 256 ];
	LSA_OBJECT_ATTRIBUTES         loaAttributes;
	LSA_UNICODE_STRING            lusSystemName;
	NTSTATUS                           ntStatus;
	LSA_HANDLE                 lsahPolicyHandle;
	CHAR                            *pStringSID;
	DWORD                               dwError;
	POLICY_ACCOUNT_DOMAIN_INFO           *pInfo;

	bReturn = FALSE;

	MultiByteToWideChar( CP_ACP, 0, szTarget, strlen( szTarget ) + 1, wszTarget, sizeof( wszTarget ) / sizeof( wszTarget[0] ) );

	ZeroMemory( &loaAttributes, sizeof( loaAttributes ) );

	lusSystemName.Buffer        = wszTarget;
	lusSystemName.Length        = wcslen( wszTarget ) * sizeof( WCHAR );
	lusSystemName.MaximumLength = ( wcslen( wszTarget ) + 1 ) * sizeof( WCHAR );

	ntStatus = LsaOpenPolicy( &lusSystemName, &loaAttributes, POLICY_VIEW_LOCAL_INFORMATION, &lsahPolicyHandle );

	if ( ntStatus == 0 )
	{
		pInfo = NULL;

		ntStatus = LsaQueryInformationPolicy( lsahPolicyHandle, PolicyAccountDomainInformation, (PVOID *)&pInfo );

		if ( ntStatus == 0 )
		{
			if ( ConvertSidToStringSid( pInfo->DomainSid, &pStringSID ) )
			{
				strcpy( szMachineSID, pStringSID );

				LocalFree( pStringSID );

				bReturn = TRUE;
			}
			else
			{
				dwError = GetLastError();

				WriteLastErrorToErrorLog( szTarget, "ConvertSidToStringSid", &dwError, bMultipleHosts );
			}

			LsaFreeMemory( pInfo );
		}
		else
		{
			dwError = LsaNtStatusToWinError( ntStatus );

			WriteLastErrorToErrorLog( szTarget, "LsaQueryInformationPolicy", &dwError, bMultipleHosts );
		}

		LsaClose( lsahPolicyHandle );
	}
	else
	{
		dwError = LsaNtStatusToWinError( ntStatus );

		WriteLastErrorToErrorLog( szTarget, "LsaOpenPolicy", &dwError, bMultipleHosts );
	}

	return bReturn;
}

BOOL GetAccountNameFromSID( CHAR szTarget[], CHAR szStringSID[], CHAR szDomainName[], CHAR szAccountName[], BOOL *bMultipleHosts )
{
	BOOL                   bReturn;
	PSID                      pSID;
	DWORD             dwDomainName;
	CHAR          szTempDomainName[ 128 ];
	DWORD            dwAccountName;
	CHAR         szTempAccountName[ 128 ];
	SID_NAME_USE             snUse;
	DWORD                  dwError;

	bReturn = FALSE;

	if ( ConvertStringSidToSid( szStringSID, &pSID ) )
	{
		dwDomainName  = sizeof( szTempDomainName );
		dwAccountName = sizeof( szTempAccountName );

		if ( LookupAccountSid( szTarget, pSID, szTempAccountName, &dwAccountName, szTempDomainName, &dwDomainName, &snUse ) )
		{
			strcpy( szDomainName,  szTempDomainName );
			strcpy( szAccountName, szTempAccountName );

			bReturn = TRUE;
		}

		LocalFree( pSID );
	}
	else
	{
		dwError = GetLastError();

		WriteLastErrorToErrorLog( szTarget, "ConvertStringSidToSid", &dwError, bMultipleHosts );
	}

	return bReturn;
}

VOID GetInstalledPrograms( CHAR szTarget[], BOOL *bMultipleHosts )
{
	CHAR     *pLocation;
	DWORD             i;
	CHAR   szTempTarget[ 128 ];
	LONG       lReturn1;
	HKEY           hKey;
	DWORD             j;
	HKEY       hSubKey1;
	CHAR   szSubKeyName[ 256 ];
	CHAR  szFullKeyName[ 512 ];
	LONG       lReturn2;
	HKEY       hSubKey2;
	CHAR  szDisplayName[ 128 ];
	DWORD  dwBufferSize;
	CHAR  szInstallDate[ 128 ];
	FILE   *pOutputFile;
	DWORD       dwError;

	pLocation = strstr( szTarget, "\\\\" );

	if ( pLocation != NULL )
	{
		strcpy( szTempTarget, szTarget );
	}
	else
	{
		sprintf( szTempTarget, "\\\\%s", szTarget );
	}

	i = 0;

	lReturn1 = RegConnectRegistry( szTempTarget, HKEY_LOCAL_MACHINE, &hKey );

	if ( lReturn1 == ERROR_SUCCESS )
	{
		lReturn1 = RegOpenKeyEx( hKey, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", 0, KEY_ENUMERATE_SUB_KEYS, &hSubKey1 );

		if ( lReturn1 == ERROR_SUCCESS )
		{
			for ( j = 0; TRUE; j++ )
			{
				lReturn1 = RegEnumKey( hSubKey1, j, szSubKeyName, 255 );

				if ( lReturn1 != ERROR_SUCCESS )
				{
					break;
				}

				sprintf( szFullKeyName, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\%s", szSubKeyName );

				lReturn2 = RegOpenKeyEx( hKey, szFullKeyName, 0, KEY_QUERY_VALUE, &hSubKey2 );

				if ( lReturn2 == ERROR_SUCCESS )
				{
					strcpy( szDisplayName, "" );

					dwBufferSize = 128;

					lReturn2 = RegQueryValueEx( hSubKey2, "DisplayName", NULL, NULL, (BYTE *)szDisplayName, &dwBufferSize );

					if ( lReturn2 == ERROR_SUCCESS )
					{
						strcpy( szInstallDate, "" );

						dwBufferSize = 128;

						lReturn2 = RegQueryValueEx( hSubKey2, "InstallDate", NULL, NULL, (BYTE *)szInstallDate, &dwBufferSize );

						if ( !*bMultipleHosts )
						{
							if ( i == 0 )
							{
								printf( "\n" );
								printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
								printf( "+++++      INSTALLED PROGRAMS INFORMATION     +++++\n" );
								printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
								printf( "\n" );

								i++;
							}

							printf( "Display Name: %s\n", szDisplayName );
							printf( "Install Date: %s\n", szInstallDate );
							printf( "\n" );

							fflush( stdout );
						}

						WaitForSingleObject( hSemaphore, INFINITE );

						pOutputFile = fopen( "InstalledPrograms.txt", "r" );

						if ( pOutputFile != NULL )
						{
							fclose( pOutputFile );
						}
						else
						{
							pOutputFile = fopen( "InstalledPrograms.txt", "w" );

							if ( pOutputFile != NULL )
							{
								fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
								fprintf( pOutputFile, "\n" );
								fprintf( pOutputFile, "Hostname\tDisplay Name\tInstall Date\n" );

								fclose( pOutputFile );
							}
						}

						pOutputFile = fopen( "InstalledPrograms.txt", "a+" );

						if ( pOutputFile != NULL )
						{
							fprintf( pOutputFile, "%s\t%s\t%s\n", szTarget, szDisplayName, szInstallDate );

							fclose( pOutputFile );
						}

						ReleaseSemaphore( hSemaphore, 1, NULL );
					}

					RegCloseKey( hSubKey2 );
				}
			}

			RegCloseKey( hSubKey1 );
		}
		else
		{
			dwError = lReturn1;

			WriteLastErrorToErrorLog( szTarget, "RegOpenKeyEx", &dwError, bMultipleHosts );
		}

		RegCloseKey( hKey );
	}
	else
	{
		dwError = lReturn1;

		WriteLastErrorToErrorLog( szTarget, "RegConnectRegistry", &dwError, bMultipleHosts );
	}
}

VOID GetAuditPolicyInfo( CHAR szTarget[], BOOL *bMultipleHosts )
{
	WCHAR                                    wszTarget[ 256 ];
	LSA_OBJECT_ATTRIBUTES                loaAttributes;
	LSA_UNICODE_STRING                   lusSystemName;
	NTSTATUS                                  ntStatus;
	LSA_HANDLE                        lsahPolicyHandle;
	POLICY_AUDIT_EVENTS_INFO                    *pInfo;
	CHAR                             szAuditingEnabled[ 4 ];
	CHAR                     szDirectoryServiceAccess1[ 16 ];
	CHAR                     szDirectoryServiceAccess2[ 4 ];
	CHAR                         szAccountLogonEvents1[ 16 ];
	CHAR                         szAccountLogonEvents2[ 4 ];
	DWORD                                            i;
	CHAR                               szSystemEvents1[ 16 ];
	CHAR                               szSystemEvents2[ 4 ];
	CHAR                                szLogonEvents1[ 16 ];
	CHAR                                szLogonEvents2[ 4 ];
	CHAR                               szObjectAccess1[ 16 ];
	CHAR                               szObjectAccess2[ 4 ];
	CHAR                               szPrivilegeUse1[ 16 ];
	CHAR                               szPrivilegeUse2[ 4 ];
	CHAR                            szProcessTracking1[ 16 ];
	CHAR                            szProcessTracking2[ 4 ];
	CHAR                               szPolicyChange1[ 16 ];
	CHAR                               szPolicyChange2[ 4 ];
	CHAR                          szAccountManagement1[ 16 ];
	CHAR                          szAccountManagement2[ 4 ];
	FILE                                  *pOutputFile;
	DWORD                                      dwError;

	MultiByteToWideChar( CP_ACP, 0, szTarget, strlen( szTarget ) + 1, wszTarget, sizeof( wszTarget ) / sizeof( wszTarget[0] ) );

	ZeroMemory( &loaAttributes, sizeof( loaAttributes ) );

	lusSystemName.Buffer        = wszTarget;
	lusSystemName.Length        = wcslen( wszTarget ) * sizeof( WCHAR );
	lusSystemName.MaximumLength = ( wcslen( wszTarget ) + 1 ) * sizeof( WCHAR );

	ntStatus = LsaOpenPolicy( &lusSystemName, &loaAttributes, POLICY_VIEW_AUDIT_INFORMATION, &lsahPolicyHandle );

	if ( ntStatus == 0 )
	{
		ntStatus = LsaQueryInformationPolicy( lsahPolicyHandle, PolicyAuditEventsInformation, (PVOID *)&pInfo );

		if ( ntStatus == 0 )
		{
			if ( pInfo->AuditingMode )
			{
				strcpy( szAuditingEnabled, "Yes" );
			}
			else
			{
				strcpy( szAuditingEnabled, "No" );
			}

			strcpy( szDirectoryServiceAccess1, "N/A" );
			strcpy( szDirectoryServiceAccess2, "N/A\tN/A" );

			strcpy( szAccountLogonEvents1, "N/A" );
			strcpy( szAccountLogonEvents2, "N/A\tN/A" );

			for ( i = 0; i < pInfo->MaximumAuditEventCount; i++ )
			{
				switch ( pInfo->EventAuditingOptions[i] )
				{
					case POLICY_AUDIT_EVENT_SUCCESS:
						switch ( i )
						{
							case (POLICY_AUDIT_EVENT_TYPE)AuditCategorySystem:
								strcpy( szSystemEvents1, "Success Only" );
								strcpy( szSystemEvents2, "X\t" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryLogon:
								strcpy( szLogonEvents1, "Success Only" );
								strcpy( szLogonEvents2, "X\t" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryObjectAccess:
								strcpy( szObjectAccess1, "Success Only" );
								strcpy( szObjectAccess2, "X\t" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryPrivilegeUse:
								strcpy( szPrivilegeUse1, "Success Only" );
								strcpy( szPrivilegeUse2, "X\t" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryDetailedTracking:
								strcpy( szProcessTracking1, "Success Only" );
								strcpy( szProcessTracking2, "X\t" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryPolicyChange:
								strcpy( szPolicyChange1, "Success Only" );
								strcpy( szPolicyChange2, "X\t" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryAccountManagement:
								strcpy( szAccountManagement1, "Success Only" );
								strcpy( szAccountManagement2, "X\t" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryDirectoryServiceAccess:
								strcpy( szDirectoryServiceAccess1, "Success Only" );
								strcpy( szDirectoryServiceAccess2, "X\t" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryAccountLogon:
								strcpy( szAccountLogonEvents1, "Success Only" );
								strcpy( szAccountLogonEvents2, "X\t" );

								break;
						}

						break;

					case POLICY_AUDIT_EVENT_FAILURE:
						switch ( i )
						{
							case (POLICY_AUDIT_EVENT_TYPE)AuditCategorySystem:
								strcpy( szSystemEvents1, "Failure Only" );
								strcpy( szSystemEvents2, "\tX" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryLogon:
								strcpy( szLogonEvents1, "Failure Only" );
								strcpy( szLogonEvents2, "\tX" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryObjectAccess:
								strcpy( szObjectAccess1, "Failure Only" );
								strcpy( szObjectAccess2, "\tX" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryPrivilegeUse:
								strcpy( szPrivilegeUse1, "Failure Only" );
								strcpy( szPrivilegeUse2, "\tX" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryDetailedTracking:
								strcpy( szProcessTracking1, "Failure Only" );
								strcpy( szProcessTracking2, "\tX" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryPolicyChange:
								strcpy( szPolicyChange1, "Failure Only" );
								strcpy( szPolicyChange2, "\tX" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryAccountManagement:
								strcpy( szAccountManagement1, "Failure Only" );
								strcpy( szAccountManagement2, "\tX" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryDirectoryServiceAccess:
								strcpy( szDirectoryServiceAccess1, "Failure Only" );
								strcpy( szDirectoryServiceAccess2, "\tX" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryAccountLogon:
								strcpy( szAccountLogonEvents1, "Failure Only" );
								strcpy( szAccountLogonEvents2, "\tX" );

								break;
						}

						break;

					case (POLICY_AUDIT_EVENT_SUCCESS | POLICY_AUDIT_EVENT_FAILURE):
						switch ( i )
						{
							case (POLICY_AUDIT_EVENT_TYPE)AuditCategorySystem:
								strcpy( szSystemEvents1, "Success/Failure" );
								strcpy( szSystemEvents2, "X\tX" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryLogon:
								strcpy( szLogonEvents1, "Success/Failure" );
								strcpy( szLogonEvents2, "X\tX" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryObjectAccess:
								strcpy( szObjectAccess1, "Success/Failure" );
								strcpy( szObjectAccess2, "X\tX" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryPrivilegeUse:
								strcpy( szPrivilegeUse1, "Success/Failure" );
								strcpy( szPrivilegeUse2, "X\tX" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryDetailedTracking:
								strcpy( szProcessTracking1, "Success/Failure" );
								strcpy( szProcessTracking2, "X\tX" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryPolicyChange:
								strcpy( szPolicyChange1, "Success/Failure" );
								strcpy( szPolicyChange2, "X\tX" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryAccountManagement:
								strcpy( szAccountManagement1, "Success/Failure" );
								strcpy( szAccountManagement2, "X\tX" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryDirectoryServiceAccess:
								strcpy( szDirectoryServiceAccess1, "Success/Failure" );
								strcpy( szDirectoryServiceAccess2, "X\tX" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryAccountLogon:
								strcpy( szAccountLogonEvents1, "Success/Failure" );
								strcpy( szAccountLogonEvents2, "X\tX" );

								break;
						}

						break;

					default:
						switch ( i )
						{
							case (POLICY_AUDIT_EVENT_TYPE)AuditCategorySystem:
								strcpy( szSystemEvents1, "None" );
								strcpy( szSystemEvents2, "\t" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryLogon:
								strcpy( szLogonEvents1, "None" );
								strcpy( szLogonEvents2, "\t" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryObjectAccess:
								strcpy( szObjectAccess1, "None" );
								strcpy( szObjectAccess2, "\t" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryPrivilegeUse:
								strcpy( szPrivilegeUse1, "None" );
								strcpy( szPrivilegeUse2, "\t" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryDetailedTracking:
								strcpy( szProcessTracking1, "None" );
								strcpy( szProcessTracking2, "\t" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryPolicyChange:
								strcpy( szPolicyChange1, "None" );
								strcpy( szPolicyChange2, "\t" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryAccountManagement:
								strcpy( szAccountManagement1, "None" );
								strcpy( szAccountManagement2, "\t" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryDirectoryServiceAccess:
								strcpy( szDirectoryServiceAccess1, "None" );
								strcpy( szDirectoryServiceAccess2, "\t" );

								break;

							case (POLICY_AUDIT_EVENT_TYPE)AuditCategoryAccountLogon:
								strcpy( szAccountLogonEvents1, "None" );
								strcpy( szAccountLogonEvents2, "\t" );

								break;
						}

						break;
				}
			}

			if ( !*bMultipleHosts )
			{
				printf( "\n" );
				printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
				printf( "+++++        AUDIT POLICY INFORMATION         +++++\n" );
				printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
				printf( "\n" );

				printf( "Auditing Enabled:         %s\n", szAuditingEnabled );
				printf( "System Events:            %s\n", szSystemEvents1 );
				printf( "Logon Events:             %s\n", szLogonEvents1 );
				printf( "Object Access:            %s\n", szObjectAccess1 );
				printf( "Privilege Use:            %s\n", szPrivilegeUse1 );
				printf( "Process Tracking:         %s\n", szProcessTracking1 );
				printf( "Policy Change:            %s\n", szPolicyChange1 );
				printf( "Account Management:       %s\n", szAccountManagement1 );
				printf( "Directory Service Access: %s\n", szDirectoryServiceAccess1 );
				printf( "Account Logon Events      %s\n", szAccountLogonEvents1 );
				printf( "\n" );

				fflush( stdout );
			}

			WaitForSingleObject( hSemaphore, INFINITE );

			pOutputFile = fopen( "AuditPolicyInfo.txt", "r" );

			if ( pOutputFile != NULL )
			{
				fclose( pOutputFile );
			}
			else
			{
				pOutputFile = fopen( "AuditPolicyInfo.txt", "w" );

				if ( pOutputFile != NULL )
				{
					fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
					fprintf( pOutputFile, "\n" );
					fprintf( pOutputFile, "Hostname\tAuditing Enabled\tSystem Events\t\tLogon Events\t\tObject Access\t\tPrivilege Use\t\tProcess Tracking\t\tPolicy Change\t\tAccount Management\t\tDirectory Service Access\t\tAccount Logon Events\n" );
					fprintf( pOutputFile, "\t\tSuccess\tFailure\tSuccess\tFailure\tSuccess\tFailure\tSuccess\tFailure\tSuccess\tFailure\tSuccess\tFailure\tSuccess\tFailure\tSuccess\tFailure\tSuccess\tFailure\n" );

					fclose( pOutputFile );
				}
			}

			pOutputFile = fopen( "AuditPolicyInfo.txt", "a+" );

			if ( pOutputFile != NULL )
			{
				fprintf( pOutputFile, "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n", szTarget, szAuditingEnabled, szSystemEvents2, szLogonEvents2, szObjectAccess2, szPrivilegeUse2, szProcessTracking2, szPolicyChange2, szAccountManagement2, szDirectoryServiceAccess2, szAccountLogonEvents2 );

				fclose( pOutputFile );
			}

			ReleaseSemaphore( hSemaphore, 1, NULL );

			LsaFreeMemory( pInfo );
		}
		else
		{
			dwError = LsaNtStatusToWinError( ntStatus );

			WriteLastErrorToErrorLog( szTarget, "LsaQueryInformationPolicy", &dwError, bMultipleHosts );
		}

		LsaClose( lsahPolicyHandle );
	}
	else
	{
		dwError = LsaNtStatusToWinError( ntStatus );

		WriteLastErrorToErrorLog( szTarget, "LsaOpenPolicy", &dwError, bMultipleHosts );
	}
}

VOID GetServerInfo( CHAR szTarget[], BOOL *bMultipleHosts )
{
	CHAR               *pLocation;
	CHAR             szTempTarget[ 128 ];
	WCHAR               wszTarget[ 256 ];
	DWORD                 dwLevel;
	SERVER_INFO_101        *pInfo;
	NET_API_STATUS        nStatus;
	CHAR                szComment[ 512 ];
	CHAR             szServerType[ 128 ];
	CHAR                 szOSType[ 128 ];
	CHAR            szServerFlags[ 128 ];
	FILE             *pOutputFile;
	DWORD                 dwError;

	pLocation = strstr( szTarget, "\\\\" );

	if ( pLocation != NULL )
	{
		strcpy( szTempTarget, szTarget );
	}
	else
	{
		sprintf( szTempTarget, "\\\\%s", szTarget );
	}

	MultiByteToWideChar( CP_ACP, 0, szTempTarget, strlen( szTempTarget ) + 1, wszTarget, sizeof( wszTarget ) / sizeof( wszTarget[0] ) );

	dwLevel = 101;
	pInfo   = NULL;

	nStatus = NetServerGetInfo( wszTarget, dwLevel, (PBYTE *)&pInfo );

	if ( nStatus == NERR_Success )
	{
		if ( pInfo != NULL )
		{
			WideCharToMultiByte( CP_ACP, 0, pInfo->sv101_comment, -1, szComment, 512, NULL, NULL );

			if ( ( pInfo->sv101_type & SV_TYPE_DOMAIN_CTRL ) || ( pInfo->sv101_type & SV_TYPE_DOMAIN_BAKCTRL ) )
			{
				strcpy( szServerType, "Domain Controller" );
			}
			else if ( ( pInfo->sv101_type & SV_TYPE_SERVER_NT ) )
			{
				strcpy( szServerType, "Server" );
			}
			else
			{
				strcpy( szServerType, "Workstation" );
			}

			if ( pInfo->sv101_version_major == 4 && pInfo->sv101_version_minor == 0 )
			{
				strcpy( szOSType, "Windows NT 4.0" );
			}
			else if ( pInfo->sv101_version_major == 5 && pInfo->sv101_version_minor == 0 )
			{
				strcpy( szOSType, "Windows 2000" );
			}
			else if ( pInfo->sv101_version_major == 5 && pInfo->sv101_version_minor == 1 )
			{
				strcpy( szOSType, "Windows XP" );
			}
			else if ( pInfo->sv101_version_major == 5 && pInfo->sv101_version_minor == 2 )
			{
				strcpy( szOSType, "Windows 2003" );
			}
			else
			{
				sprintf( szOSType, "Other (%d.%d)", pInfo->sv101_version_major, pInfo->sv101_version_minor );
			}

			strcpy( szServerFlags, "" );

			if ( ( pInfo->sv101_type & SV_TYPE_SQLSERVER ) )
			{
				sprintf( szServerFlags, "%s(MS SQL Server) ", szServerFlags );
			}

			if ( ( pInfo->sv101_type & SV_TYPE_NOVELL ) )
			{
				sprintf( szServerFlags, "%s(Novell Server) ", szServerFlags );
			}

			if ( ( pInfo->sv101_type & SV_TYPE_PRINTQ_SERVER ) )
			{
				sprintf( szServerFlags, "%s(Print Server) ", szServerFlags );
			}

			if ( ( pInfo->sv101_type & SV_TYPE_DIALIN_SERVER ) )
			{
				sprintf( szServerFlags, "%s(RAS Server) ", szServerFlags );
			}

			if ( !*bMultipleHosts )
			{
				printf( "\n" );
				printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
				printf( "+++++           SERVER INFORMATION            +++++\n" );
				printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
				printf( "\n" );

				printf( "Server Type:      %s\n", szServerType );
				printf( "Operating System: %s\n", szOSType );
				printf( "Server Comment:   %s\n", szComment );
				printf( "Other Flags:      %s\n", szServerFlags );
				printf( "\n" );

				fflush( stdout );
			}

			WaitForSingleObject( hSemaphore, INFINITE );

			pOutputFile = fopen( "ServerInfo.txt", "r" );

			if ( pOutputFile != NULL )
			{
				fclose( pOutputFile );
			}
			else
			{
				pOutputFile = fopen( "ServerInfo.txt", "w" );

				if ( pOutputFile != NULL )
				{
					fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
					fprintf( pOutputFile, "\n" );
					fprintf( pOutputFile, "Hostname\tServer Type\tOperating System\tServer Comment\tOther Flags\n" );

					fclose( pOutputFile );
				}
			}

			pOutputFile = fopen( "ServerInfo.txt", "a+" );

			if ( pOutputFile != NULL )
			{
				fprintf( pOutputFile, "%s\t%s\t%s\t%s\t%s\n", szTarget, szServerType, szOSType, szComment, szServerFlags );

				fclose( pOutputFile );
			}

			ReleaseSemaphore( hSemaphore, 1, NULL );

			NetApiBufferFree( pInfo );
		}
	}
	else
	{
		if ( nStatus == ERROR_ACCESS_DENIED )
		{
			WriteToErrorLog( szTarget, "NetServerGetInfo", "The user does not have access to the requested information.", bMultipleHosts );
		}
		else if ( nStatus == ERROR_INVALID_LEVEL )
		{
			WriteToErrorLog( szTarget, "NetServerGetInfo", "The value specified for the level parameter is invalid.", bMultipleHosts );
		}
		else if ( nStatus == ERROR_INVALID_PARAMETER )
		{
			WriteToErrorLog( szTarget, "NetServerGetInfo", "The specified parameter is invalid.", bMultipleHosts );
		}
		else if ( nStatus == ERROR_NOT_ENOUGH_MEMORY )
		{
			WriteToErrorLog( szTarget, "NetServerGetInfo", "Insufficient memory is available.", bMultipleHosts );
		}
		else
		{
			dwError = nStatus;

			WriteLastErrorToErrorLog( szTarget, "NetServerGetInfo", &dwError, bMultipleHosts );
		}
	}
}

VOID GetServiceInfo( CHAR szTarget[], BOOL *bMultipleHosts )
{
	DWORD                                   i;
	SC_HANDLE                    schSCManager;
	DWORD                       dwBytesNeeded;
	DWORD                  dwServicesReturned;
	DWORD                      dwResumeHandle;
	DWORD                             dwError;
	ENUM_SERVICE_STATUS       *pServiceStatus;
	DWORD                                   j;
	SC_HANDLE                      schService;
	QUERY_SERVICE_CONFIG      *pServiceConfig;
	CHAR                          szStartType[ 128 ];
	BOOL                     bShowDescription;
	SERVICE_DESCRIPTION  *pServiceDescription;
	FILE                         *pOutputFile;

	i = 0;

	schSCManager = NULL;

	schSCManager = OpenSCManager( szTarget, NULL, SC_MANAGER_ENUMERATE_SERVICE );
 
	if ( schSCManager != NULL )
	{
		dwBytesNeeded      = 0;
		dwServicesReturned = 0;
		dwResumeHandle     = 0;

		EnumServicesStatus( schSCManager, SERVICE_WIN32, SERVICE_STATE_ALL, NULL, 0, &dwBytesNeeded, &dwServicesReturned, &dwResumeHandle );

		dwError = GetLastError();

		if ( dwError == ERROR_MORE_DATA )
		{
			pServiceStatus = NULL;

			pServiceStatus = (ENUM_SERVICE_STATUS *)malloc( dwBytesNeeded * sizeof( ENUM_SERVICE_STATUS ) );

			if ( pServiceStatus != NULL )
			{
				if ( EnumServicesStatus( schSCManager, SERVICE_WIN32, SERVICE_STATE_ALL, pServiceStatus, dwBytesNeeded, &dwBytesNeeded, &dwServicesReturned, &dwResumeHandle ) )
				{
					for ( j = 0; j < dwServicesReturned; j++ )
					{
						schService = NULL;

						schService = OpenService( schSCManager, pServiceStatus[j].lpServiceName, SERVICE_QUERY_CONFIG );

						if ( schService != NULL )
						{
							dwBytesNeeded = 0;

							QueryServiceConfig( schService, NULL, 0, &dwBytesNeeded );

							dwError = GetLastError();

							if ( dwError == ERROR_INSUFFICIENT_BUFFER )
							{
								pServiceConfig = NULL;

								pServiceConfig = (QUERY_SERVICE_CONFIG *)malloc( dwBytesNeeded * sizeof( QUERY_SERVICE_CONFIG ) );

								if ( pServiceConfig != NULL )
								{
									if ( QueryServiceConfig( schService, pServiceConfig, dwBytesNeeded, &dwBytesNeeded ) )
									{
										switch ( pServiceConfig->dwStartType )
										{
											case SERVICE_BOOT_START:
												strcpy( szStartType, "Device Driver (Boot Start)" );

												break;

											case SERVICE_SYSTEM_START:
												strcpy( szStartType, "Device Driver (System Start)" );

												break;

											case SERVICE_AUTO_START:
												strcpy( szStartType, "Auto Start" );

												break;

											case SERVICE_DEMAND_START:
												strcpy( szStartType, "Manual Start" );

												break;

											case SERVICE_DISABLED:
												strcpy( szStartType, "Service Disabled" );

												break;
										}

										bShowDescription = FALSE;

										dwBytesNeeded = 0;

										QueryServiceConfig2( schService, SERVICE_CONFIG_DESCRIPTION, NULL, 0, &dwBytesNeeded );

										dwError = GetLastError();

										if ( dwError == ERROR_INSUFFICIENT_BUFFER )
										{
											pServiceDescription = NULL;

											pServiceDescription = (SERVICE_DESCRIPTION *)malloc( dwBytesNeeded * sizeof( SERVICE_DESCRIPTION ) );

											if ( pServiceDescription != NULL )
											{
												if ( QueryServiceConfig2( schService, SERVICE_CONFIG_DESCRIPTION, (BYTE *)pServiceDescription, dwBytesNeeded, &dwBytesNeeded ) )
												{
													if ( pServiceDescription->lpDescription != NULL )
													{
														bShowDescription = TRUE;
													}
												}
												else
												{
													dwError = GetLastError();

													if ( dwError == ERROR_ACCESS_DENIED )
													{
														WriteToErrorLog( szTarget, "QueryServiceConfig2", "The user does not have access to the requested information.", bMultipleHosts );
													}
													else if ( dwError == ERROR_INVALID_HANDLE )
													{
														WriteToErrorLog( szTarget, "QueryServiceConfig2", "The specified handle is invalid.", bMultipleHosts );
													}
													else
													{
														WriteLastErrorToErrorLog( szTarget, "QueryServiceConfig2", &dwError, bMultipleHosts );
													}
												}
											}
											else
											{
												WriteToErrorLog( szTarget, "malloc", "Unable to allocate memory.", bMultipleHosts );
											}
										}
										else
										{
											dwError = GetLastError();

											if ( dwError == ERROR_ACCESS_DENIED )
											{
												WriteToErrorLog( szTarget, "QueryServiceConfig2", "The user does not have access to the requested information.", bMultipleHosts );
											}
											else if ( dwError == ERROR_INVALID_HANDLE )
											{
												WriteToErrorLog( szTarget, "QueryServiceConfig2", "The specified handle is invalid.", bMultipleHosts );
											}
											else
											{
												WriteLastErrorToErrorLog( szTarget, "QueryServiceConfig2", &dwError, bMultipleHosts );
											}
										}

										if ( !*bMultipleHosts )
										{
											if ( i == 0 )
											{
												printf( "\n" );
												printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
												printf( "+++++           SERVICE INFORMATION           +++++\n" );
												printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
												printf( "\n" );

												i++;
											}

											printf( "Service Name: %s\n", pServiceStatus[j].lpServiceName );
											printf( "Display Name: %s\n", pServiceStatus[j].lpDisplayName );
											printf( "Start Type:   %s\n", szStartType );
											printf( "Account:      %s\n", pServiceConfig->lpServiceStartName );

											if ( bShowDescription )
											{
												printf( "Description:  %s\n", pServiceDescription->lpDescription );
											}
											else
											{
												printf( "Description:  \n" );
											}

											printf( "\n" );

											fflush( stdout );
										}

										WaitForSingleObject( hSemaphore, INFINITE );

										pOutputFile = fopen( "ServiceInfo.txt", "r" );

										if ( pOutputFile != NULL )
										{
											fclose( pOutputFile );
										}
										else
										{
											pOutputFile = fopen( "ServiceInfo.txt", "w" );

											if ( pOutputFile != NULL )
											{
												fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
												fprintf( pOutputFile, "\n" );
												fprintf( pOutputFile, "Hostname\tService Name\tDisplay Name\tStart Type\tAccount\tDescription\n" );

												fclose( pOutputFile );
											}
										}

										pOutputFile = fopen( "ServiceInfo.txt", "a+" );

										if ( pOutputFile != NULL )
										{
											if ( bShowDescription )
											{
												fprintf( pOutputFile, "%s\t%s\t%s\t%s\t%s\t%s\n", szTarget, pServiceStatus[j].lpServiceName, pServiceStatus[j].lpDisplayName, szStartType, pServiceConfig->lpServiceStartName, pServiceDescription->lpDescription );
											}
											else
											{
												fprintf( pOutputFile, "%s\t%s\t%s\t%s\t%s\t\n", szTarget, pServiceStatus[j].lpServiceName, pServiceStatus[j].lpDisplayName, szStartType, pServiceConfig->lpServiceStartName );
											}

											fclose( pOutputFile );
										}

										ReleaseSemaphore( hSemaphore, 1, NULL );

										if ( pServiceDescription != NULL )
										{
											free( pServiceDescription );
										}
									}
									else
									{
										dwError = GetLastError();

										if ( dwError == ERROR_ACCESS_DENIED )
										{
											WriteToErrorLog( szTarget, "QueryServiceConfig", "The user does not have access to the requested information.", bMultipleHosts );
										}
										else if ( dwError == ERROR_INVALID_HANDLE )
										{
											WriteToErrorLog( szTarget, "QueryServiceConfig", "The specified handle is invalid.", bMultipleHosts );
										}
										else
										{
											WriteLastErrorToErrorLog( szTarget, "QueryServiceConfig", &dwError, bMultipleHosts );
										}
									}

									free( pServiceConfig );
								}
								else
								{
									WriteToErrorLog( szTarget, "malloc", "Unable to allocate memory.", bMultipleHosts );
								}
							}
							else
							{
								dwError = GetLastError();

								if ( dwError == ERROR_ACCESS_DENIED )
								{
									WriteToErrorLog( szTarget, "QueryServiceConfig", "The user does not have access to the requested information.", bMultipleHosts );
								}
								else if ( dwError == ERROR_INVALID_HANDLE )
								{
									WriteToErrorLog( szTarget, "QueryServiceConfig", "The specified handle is invalid.", bMultipleHosts );
								}
								else
								{
									WriteLastErrorToErrorLog( szTarget, "QueryServiceConfig", &dwError, bMultipleHosts );
								}
							}

							CloseServiceHandle( schService );
						}
						else
						{
							dwError = GetLastError();

							if ( dwError == ERROR_ACCESS_DENIED )
							{
								WriteToErrorLog( szTarget, "OpenService", "The user does not have access to the requested information.", bMultipleHosts );
							}
							else if ( dwError == ERROR_INVALID_HANDLE )
							{
								WriteToErrorLog( szTarget, "OpenService", "The specified handle is invalid.", bMultipleHosts );
							}
							else if ( dwError == ERROR_INVALID_NAME )
							{
								WriteToErrorLog( szTarget, "OpenService", "The specified service name is invalid.", bMultipleHosts );
							}
							else if ( dwError == ERROR_SERVICE_DOES_NOT_EXIST )
							{
								WriteToErrorLog( szTarget, "OpenService", "The specified service does not exist.", bMultipleHosts );
							}
							else
							{
								WriteLastErrorToErrorLog( szTarget, "OpenService", &dwError, bMultipleHosts );
							}
						}
					}
				}
				else
				{
					dwError = GetLastError();

					if ( dwError == ERROR_ACCESS_DENIED )
					{
						WriteToErrorLog( szTarget, "EnumServicesStatus", "The user does not have access to the requested information.", bMultipleHosts );
					}
					else if ( dwError == ERROR_INVALID_HANDLE )
					{
						WriteToErrorLog( szTarget, "EnumServicesStatus", "The specified handle is invalid.", bMultipleHosts );
					}
					else if ( dwError == ERROR_INVALID_PARAMETER )
					{
						WriteToErrorLog( szTarget, "EnumServicesStatus", "The specified parameter is invalid.", bMultipleHosts );
					}
					else
					{
						WriteLastErrorToErrorLog( szTarget, "EnumServicesStatus", &dwError, bMultipleHosts );
					}
				}

				free( pServiceStatus );
			}
			else
			{
				WriteToErrorLog( szTarget, "malloc", "Unable to allocate memory.", bMultipleHosts );
			}
		}
		else
		{
			dwError = GetLastError();

			if ( dwError == ERROR_ACCESS_DENIED )
			{
				WriteToErrorLog( szTarget, "EnumServicesStatus", "The user does not have access to the requested information.", bMultipleHosts );
			}
			else if ( dwError == ERROR_INVALID_HANDLE )
			{
				WriteToErrorLog( szTarget, "EnumServicesStatus", "The specified handle is invalid.", bMultipleHosts );
			}
			else if ( dwError == ERROR_INVALID_PARAMETER )
			{
				WriteToErrorLog( szTarget, "EnumServicesStatus", "The specified parameter is invalid.", bMultipleHosts );
			}
			else
			{
				WriteLastErrorToErrorLog( szTarget, "EnumServicesStatus", &dwError, bMultipleHosts );
			}
		}

		CloseServiceHandle( schSCManager );
	}
	else
	{
		dwError = GetLastError();

		if ( dwError == ERROR_ACCESS_DENIED )
		{
			WriteToErrorLog( szTarget, "OpenSCManager", "The user does not have access to the requested information.", bMultipleHosts );
		}
		else if ( dwError == ERROR_DATABASE_DOES_NOT_EXIST )
		{
			WriteToErrorLog( szTarget, "OpenSCManager", "The specified database does not exist.", bMultipleHosts );
		}
		else if ( dwError == ERROR_INVALID_PARAMETER )
		{
			WriteToErrorLog( szTarget, "OpenSCManager", "The specified parameter is invalid.", bMultipleHosts );
		}
		else
		{
			WriteLastErrorToErrorLog( szTarget, "OpenSCManager", &dwError, bMultipleHosts );
		}
	}
}

VOID GetUserRightsInfo( CHAR szTarget[], BOOL *bMultipleHosts )
{
	WCHAR                        wszTarget[ 256 ];
	DWORD                                i;
	LSA_OBJECT_ATTRIBUTES    loaAttributes;
	LSA_UNICODE_STRING       lusSystemName;
	NTSTATUS                      ntStatus;
	LSA_HANDLE            lsahPolicyHandle;
	DWORD                                j;
	CHAR                       szUserRight[ 64 ];
	WCHAR                     wszUserRight[ 256 ];
	LSA_UNICODE_STRING        lusUserRight;
	VOID                            *pInfo;
	DWORD                          dwCount;
	DWORD                                k;
	PSID                            *pSIDs;
	DWORD                    dwAccountName;
	CHAR                     szAccountName[ 128 ];
	DWORD                     dwDomainName;
	CHAR                      szDomainName[ 128 ];
        SID_NAME_USE                     snUse;
	FILE                      *pOutputFile;
	DWORD                          dwError;

	MultiByteToWideChar( CP_ACP, 0, szTarget, strlen( szTarget ) + 1, wszTarget, sizeof( wszTarget ) / sizeof( wszTarget[0] ) );

	i = 0;

	ZeroMemory( &loaAttributes, sizeof( loaAttributes ) );

	lusSystemName.Buffer        = wszTarget;
	lusSystemName.Length        = wcslen( wszTarget ) * sizeof( WCHAR );
	lusSystemName.MaximumLength = ( wcslen( wszTarget ) + 1 ) * sizeof( WCHAR );

	ntStatus = LsaOpenPolicy( &lusSystemName, &loaAttributes, POLICY_VIEW_LOCAL_INFORMATION | POLICY_LOOKUP_NAMES, &lsahPolicyHandle );

	if ( ntStatus == 0 )
	{
		for ( j = 0; j < 3; j++ )
		{
			if ( j == 0 )
			{
				strcpy( szUserRight, "SeInteractiveLogonRight" );
			}

			if ( j == 1 )
			{
				strcpy( szUserRight, "SeNetworkLogonRight" );
			}

			if ( j == 2 )
			{
				strcpy( szUserRight, "SeShutdownPrivilege" );
			}

			MultiByteToWideChar( CP_ACP, 0, szUserRight, strlen( szUserRight ) + 1, wszUserRight, sizeof( wszUserRight ) / sizeof( wszUserRight[0] ) );

			lusUserRight.Buffer        = wszUserRight;
			lusUserRight.Length        = wcslen( wszUserRight ) * sizeof( WCHAR );
			lusUserRight.MaximumLength = ( wcslen( wszUserRight ) + 1 ) * sizeof( WCHAR );

			pInfo = NULL;

			ntStatus = LsaEnumerateAccountsWithUserRight( lsahPolicyHandle, &lusUserRight, &pInfo, &dwCount );

			if ( ntStatus == 0 )
			{
				if ( pInfo != NULL )
				{
					pSIDs = (PSID *)pInfo;

					for ( k = 0; k < dwCount; k++ )
					{
						dwAccountName = sizeof( szAccountName );
						dwDomainName  = sizeof( szDomainName );

						if ( LookupAccountSid( szTarget, pSIDs[k], szAccountName, &dwAccountName, szDomainName, &dwDomainName, &snUse ) )
						{
							if ( !*bMultipleHosts )
							{
								if ( i == 0 )
								{
									printf( "\n" );
									printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
									printf( "+++++         USER RIGHTS INFORMATION         +++++\n" );
									printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
									printf( "\n" );

									i++;
								}

								printf( "User Right: %s\n", szUserRight );
								printf( "Username:   %s\\%s\n", szDomainName, szAccountName );
								printf( "\n" );

								fflush( stdout );
							}

							WaitForSingleObject( hSemaphore, INFINITE );

							pOutputFile = fopen( "UserRightsInfo.txt", "r" );

							if ( pOutputFile != NULL )
							{
								fclose( pOutputFile );
							}
							else
							{
								pOutputFile = fopen( "UserRightsInfo.txt", "w" );

								if ( pOutputFile != NULL )
								{
									fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
									fprintf( pOutputFile, "\n" );
									fprintf( pOutputFile, "Hostname\tUser Right\tUsername\n" );

									fclose( pOutputFile );
								}
							}

							pOutputFile = fopen( "UserRightsInfo.txt", "a+" );

							if ( pOutputFile != NULL )
							{
								fprintf( pOutputFile, "%s\t%s\t%s\\%s\n", szTarget, szUserRight, szDomainName, szAccountName );

								fclose( pOutputFile );
							}

							ReleaseSemaphore( hSemaphore, 1, NULL );
						}
						else
						{
							dwError = GetLastError();

							WriteLastErrorToErrorLog( szTarget, "LookupAccountSid", &dwError, bMultipleHosts );
						}
					}

					LsaFreeMemory( pInfo );
				}
			}
			else
			{
				dwError = LsaNtStatusToWinError( ntStatus );

				WriteLastErrorToErrorLog( szTarget, "LsaEnumerateAccountsWithUserRight", &dwError, bMultipleHosts );
			}
		}

		LsaClose( lsahPolicyHandle );
	}
	else
	{
		dwError = LsaNtStatusToWinError( ntStatus );

		WriteLastErrorToErrorLog( szTarget, "LsaOpenPolicy", &dwError, bMultipleHosts );
	}
}

BOOL Disconnect( CHAR szTarget[], BOOL *bMultipleHosts )
{
	BOOL                 bReturn;
	CHAR              *pLocation;
	CHAR            szTempTarget[ 128 ];
	CHAR            szRemoteName[ 128 ];
	WCHAR          wszRemoteName[ 256 ];
	NET_API_STATUS       nStatus;
	DWORD                dwError;

	bReturn = FALSE;

	pLocation = strstr( szTarget, "\\\\" );

	if ( pLocation != NULL )
	{
		strcpy( szTempTarget, szTarget );
	}
	else
	{
		sprintf( szTempTarget, "\\\\%s", szTarget );
	}

	sprintf( szRemoteName, "%s\\IPC$", szTempTarget );

	MultiByteToWideChar( CP_ACP, 0, szRemoteName, strlen( szRemoteName ) + 1, wszRemoteName, sizeof( wszRemoteName ) / sizeof( wszRemoteName[0] ) );

	nStatus = NetUseDel( NULL, wszRemoteName, USE_LOTS_OF_FORCE );

	if ( nStatus == NERR_Success )
	{
		bReturn = TRUE;
	}
	else
	{
		dwError = nStatus;

		WriteLastErrorToErrorLog( szTarget, "NetUseDel", &dwError, bMultipleHosts );
	}

	return bReturn;
}