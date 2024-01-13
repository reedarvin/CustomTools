//
// gcc source.c -o source.exe -ladvapi32 -lnetapi32 -lwldap32 -lmprapi -lversion -lsnmpapi -lmgmtapi
//

#define _WIN32_WINNT 0x0500
#define WINVER       0x0500

#define STRICT
#define WIN32_LEAN_AND_MEAN

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <process.h>
#include <lm.h>
#include <dsgetdc.h>
#include <ntsecapi.h>
#include <sddl.h>
#include <winldap.h>
#include <time.h>
#include <winber.h>
#include <stdlib.h>
#include <mprapi.h>
#include <aclapi.h>
#include <mgmtapi.h>
#include <snmp.h>
#include "getopt.h"
#include "d3des.h"

#define MAX_THREADS 64

VOID        WriteLastErrorToErrorLog( CHAR *szTarget, CHAR *szFunction, DWORD *dwError, BOOL *bMultipleHosts );
VOID                 WriteToErrorLog( CHAR *szTarget, CHAR *szFunction, CHAR *szErrorMsg, BOOL *bMultipleHosts );
VOID                           Usage( VOID );
VOID            KeyboardEventMonitor( VOID *pParameter );
BOOL                       IsIPRange( CHAR *szTargetInput, CHAR *szIPNetwork );
VOID                     ThreadedSub( VOID *pParameter );
BOOL                         Connect( CHAR *szTarget, CHAR *szUsername, CHAR *szPassword, BOOL *bMultipleHosts );
VOID WriteLastErrorToConnectErrorLog( CHAR *szTarget, CHAR *szRemoteLocation, CHAR *szDomainName, CHAR *szUsername, CHAR *szPassword, DWORD *dwError );
BOOL                      Disconnect( CHAR *szTarget, BOOL *bMultipleHosts );
VOID            GetAccountPolicyInfo( CHAR *szTarget, BOOL *bMultipleHosts );
VOID            GetInstalledPrograms( CHAR *szTarget, BOOL *bMultipleHosts );
VOID          GetAdminLocalGroupInfo( CHAR *szTarget, BOOL *bMultipleHosts );
VOID         GetAdminGlobalGroupInfo( CHAR *szTarget, BOOL *bMultipleHosts );
VOID              GetAuditPolicyInfo( CHAR *szTarget, BOOL *bMultipleHosts );
VOID           GetAutoAdminLogonInfo( CHAR *szTarget, BOOL *bMultipleHosts );
VOID                   GetDomainInfo( CHAR *szTarget, BOOL *bMultipleHosts );
VOID               GetLocalGroupInfo( CHAR *szTarget, BOOL *bMultipleHosts );
VOID              GetGlobalGroupInfo( CHAR *szTarget, BOOL *bMultipleHosts );
VOID                     GetLDAPInfo( CHAR *szTarget, BOOL *bMultipleHosts );
VOID                       LDAPQuery( CHAR *szTarget, LDAP *pLDAPConnection, CHAR *pBaseDN, CHAR *pFilter, CHAR *szDefaultNamingContext, BOOL *bMultipleHosts );
VOID                GetLoggedOnUsers( CHAR *szTarget, BOOL *bMultipleHosts );
VOID                   GetRAUserInfo( CHAR *szTarget, BOOL *bMultipleHosts );
BOOL                   GetMachineSID( CHAR *szTarget, CHAR *szMachineSID, BOOL *bMultipleHosts );
BOOL           GetAccountNameFromSID( CHAR *szTarget, CHAR *szStringSID, CHAR *szDomainName, CHAR *szAccountName, BOOL *bMultipleHosts );
VOID                   GetServerInfo( CHAR *szTarget, BOOL *bMultipleHosts );
VOID                  GetServiceInfo( CHAR *szTarget, BOOL *bMultipleHosts );
VOID                    GetShareInfo( CHAR *szTarget, BOOL *bMultipleHosts );
VOID                     GetUserInfo( CHAR *szTarget, BOOL *bMultipleHosts );
VOID               GetUserRightsInfo( CHAR *szTarget, BOOL *bMultipleHosts );
VOID                  GetWinVNC3Info( CHAR *szTarget, BOOL *bMultipleHosts );
VOID                  GetWinVNC4Info( CHAR *szTarget, BOOL *bMultipleHosts );
VOID               SaveRegistryHives( CHAR *szTarget, BOOL *bMultipleHosts );
VOID                    GetPatchInfo( CHAR *szTarget, BOOL *bMultipleHosts );
VOID                            Trim( CHAR *szText );
VOID                           Split( CHAR *szText, CHAR *szSplitText, CHAR *szOSVersion, CHAR *szServicePack, CHAR *szMSAdvisory, CHAR *szFilePath, DWORD *dwHMS, DWORD *dwLMS, DWORD *dwHLS, DWORD *dwLLS );
BOOL                  GetFileVersion( CHAR *szTarget, CHAR *szFilePath, DWORD *dwHMS, DWORD *dwLMS, DWORD *dwHLS, DWORD *dwLLS, BOOL *bMultipleHosts );
VOID            GetScheduledTaskInfo( CHAR *szTarget, BOOL *bMultipleHosts );
VOID          ParseScheduledTaskFile( CHAR *szTarget, CHAR *szTaskFile, CHAR *szFilePath, CHAR *szArguments, CHAR *szWorkingDirectory, CHAR *szCreatedBy, CHAR *szDescription );
VOID            GetSNMPCommunityInfo( CHAR *szTarget, BOOL *bMultipleHosts );
VOID               GetODBCDriverInfo( CHAR *szTarget, BOOL *bMultipleHosts );
VOID   GetInteractivelyLoggedOnUsers( CHAR *szTarget, BOOL *bMultipleHosts );
VOID         GetFileSharePermissions( CHAR *szTarget, BOOL *bMultipleHosts );
VOID           GuessWindowsPasswords( CHAR *szTarget, BOOL *bMultipleHosts );
BOOL                  ConnectAsAdmin( CHAR *szTarget, CHAR *szUsername, CHAR *szPassword, BOOL *bMultipleHosts );
BOOL             DisconnectFromAdmin( CHAR *szTarget, BOOL *bMultipleHosts );
VOID      LogGuessedWindowsPasswords( CHAR *szTarget, CHAR *szUsername, CHAR *szPassword, CHAR *szIsAdmin );
VOID       GuessSNMPCommunityStrings( CHAR *szTarget, BOOL *bMultipleHosts );
BOOL                     SNMPConnect( CHAR *szTarget, CHAR *szCommunityString, BOOL *bMultipleHosts );
VOID      LogGuessedCommunityStrings( CHAR *szTarget, CHAR *szCommunityString );

typedef struct _THREAD_ARGS
{
	BOOL              NoOptionSelected;
	BOOL         WindowsOptionSelected;
	BOOL          GetAccountPolicyInfo;
	BOOL          GetInstalledPrograms;
	BOOL             GetAdminGroupInfo;
	BOOL            GetAuditPolicyInfo;
	BOOL         GetAutoAdminLogonInfo;
	BOOL                 GetDomainInfo;
	BOOL                  GetGroupInfo;
	BOOL                   GetLDAPInfo;
	BOOL              GetLoggedOnUsers;
	BOOL                 GetRAUserInfo;
	BOOL                 GetServerInfo;
	BOOL                GetServiceInfo;
	BOOL                  GetShareInfo;
	BOOL                   GetUserInfo;
	BOOL             GetUserRightsInfo;
	BOOL                 GetWinVNCInfo;
	BOOL       SaveRemoteRegistryHives;
	BOOL                  GetPatchInfo;
	BOOL          GetScheduledTaskInfo;
	BOOL          GetSNMPCommunityInfo;
	BOOL             GetODBCDriverInfo;
	BOOL GetInteractivelyLoggedOnUsers;
	BOOL       GetFileSharePermissions;
	BOOL         GuessWindowsPasswords;
	BOOL     GuessSNMPCommunityStrings;
	BOOL          Group1OptionSelected;
	CHAR                        Target[ 128 ];
	CHAR                      Username[ 128 ];
	CHAR                      Password[ 128 ];
	BOOL                 MultipleHosts;
} THREAD_ARGS, *PTHREAD_ARGS;

HANDLE hSemaphore;

INT nThreads = 0;

BOOL bVerboseOptionSelected = FALSE;

INT main( INT argc, CHAR *argv[] )
{
	DWORD                                   dwError;
	BOOL                                  bContinue;
	BOOL                          bNoOptionSelected;
	BOOL                     bWindowsOptionSelected;
	BOOL                      bGetAccountPolicyInfo;
	BOOL                      bGetInstalledPrograms;
	BOOL                         bGetAdminGroupInfo;
	BOOL                        bGetAuditPolicyInfo;
	BOOL                     bGetAutoAdminLogonInfo;
	BOOL                             bGetDomainInfo;
	BOOL                              bGetGroupInfo;
	BOOL                               bGetLDAPInfo;
	BOOL                          bGetLoggedOnUsers;
	BOOL                             bGetRAUserInfo;
	BOOL                             bGetServerInfo;
	BOOL                            bGetServiceInfo;
	BOOL                              bGetShareInfo;
	BOOL                               bGetUserInfo;
	BOOL                         bGetUserRightsInfo;
	BOOL                             bGetWinVNCInfo;
	BOOL                   bSaveRemoteRegistryHives;
	BOOL                              bGetPatchInfo;
	BOOL                      bGetScheduledTaskInfo;
	BOOL                      bGetSNMPCommunityInfo;
	BOOL                         bGetODBCDriverInfo;
	BOOL             bGetInteractivelyLoggedOnUsers;
	BOOL                   bGetFileSharePermissions;
	BOOL                     bGuessWindowsPasswords;
	BOOL                 bGuessSNMPCommunityStrings;
	BOOL                      bGroup1OptionSelected;
	INT                                     nGetOpt;
	BOOL                             bMultipleHosts;
	HANDLE                                   hToken;
	LUID                                   luidInfo;
	TOKEN_PRIVILEGES                         tpInfo;
	DWORD                                         i;
	CHAR                              szTargetInput[ 128 ];
	CHAR                                 szUsername[ 128 ];
	CHAR                                 szPassword[ 128 ];
	FILE                                *pInputFile;
	CHAR                                 szReadLine[ 128 ];
	CHAR                                   szTarget[ 128 ];
	CHAR                                szIPNetwork[ 128 ];

	PTHREAD_ARGS pThreadArgs;

	hSemaphore = CreateSemaphore( NULL, 1, 1, NULL );

	if ( !CreateDirectory( "UserCache", NULL ) )
	{
		dwError = GetLastError();

		if ( dwError != ERROR_ALREADY_EXISTS )
		{
			fprintf( stderr, "ERROR! Cannot create UserCache directory.\n" );

			fflush( stderr );

			return 1;
		}
	}

	opterr = 0;

	bContinue = TRUE;

	bNoOptionSelected              = TRUE;
	bWindowsOptionSelected         = FALSE;
	bGetAccountPolicyInfo          = FALSE;
	bGetInstalledPrograms          = FALSE;
	bGetAdminGroupInfo             = FALSE;
	bGetAuditPolicyInfo            = FALSE;
	bGetAutoAdminLogonInfo         = FALSE;
	bGetDomainInfo                 = FALSE;
	bGetGroupInfo                  = FALSE;
	bGetLDAPInfo                   = FALSE;
	bGetLoggedOnUsers              = FALSE;
	bGetRAUserInfo                 = FALSE;
	bGetServerInfo                 = FALSE;
	bGetServiceInfo                = FALSE;
	bGetShareInfo                  = FALSE;
	bGetUserInfo                   = FALSE;
	bGetUserRightsInfo             = FALSE;
	bGetWinVNCInfo                 = FALSE;
	bSaveRemoteRegistryHives       = FALSE;
	bGetPatchInfo                  = FALSE;
	bGetScheduledTaskInfo          = FALSE;
	bGetSNMPCommunityInfo          = FALSE;
	bGetODBCDriverInfo             = FALSE;
	bGetInteractivelyLoggedOnUsers = FALSE;
	bGetFileSharePermissions       = FALSE;
	bGuessWindowsPasswords         = FALSE;
	bGuessSNMPCommunityStrings     = FALSE;
	bGroup1OptionSelected          = FALSE;

	while ( bContinue )
	{
		nGetOpt = getopt( argc, argv, "apbcedgflrijsukwmnoqtxyWSv1" );

		if ( nGetOpt == -1 )
		{
			bContinue = FALSE;
		}

		switch ( nGetOpt )
		{
			case 'a':
				bNoOptionSelected      = FALSE;
				bWindowsOptionSelected = TRUE;
				bGetAccountPolicyInfo  = TRUE;

				break;

			case 'p':
				bNoOptionSelected      = FALSE;
				bWindowsOptionSelected = TRUE;
				bGetInstalledPrograms  = TRUE;

				break;

			case 'b':
				bNoOptionSelected      = FALSE;
				bWindowsOptionSelected = TRUE;
				bGetAdminGroupInfo     = TRUE;

				break;

			case 'c':
				bNoOptionSelected      = FALSE;
				bWindowsOptionSelected = TRUE;
				bGetAuditPolicyInfo    = TRUE;

				break;

			case 'e':
				bNoOptionSelected      = FALSE;
				bWindowsOptionSelected = TRUE;
				bGetAutoAdminLogonInfo = TRUE;

				break;

			case 'd':
				bNoOptionSelected      = FALSE;
				bWindowsOptionSelected = TRUE;
				bGetDomainInfo         = TRUE;

				break;

			case 'g':
				bNoOptionSelected      = FALSE;
				bWindowsOptionSelected = TRUE;
				bGetGroupInfo          = TRUE;

				break;

			case 'f':
				bNoOptionSelected = FALSE;
				bGetLDAPInfo      = TRUE;

				break;

			case 'l':
				bNoOptionSelected      = FALSE;
				bWindowsOptionSelected = TRUE;
				bGetLoggedOnUsers      = TRUE;

				break;

			case 'r':
				bNoOptionSelected      = FALSE;
				bWindowsOptionSelected = TRUE;
				bGetRAUserInfo         = TRUE;

				break;

			case 'i':
				bNoOptionSelected      = FALSE;
				bWindowsOptionSelected = TRUE;
				bGetServerInfo         = TRUE;

				break;

			case 'j':
				bNoOptionSelected      = FALSE;
				bWindowsOptionSelected = TRUE;
				bGetServiceInfo        = TRUE;

				break;

			case 's':
				bNoOptionSelected      = FALSE;
				bWindowsOptionSelected = TRUE;
				bGetShareInfo          = TRUE;

				break;

			case 'u':
				bNoOptionSelected      = FALSE;
				bWindowsOptionSelected = TRUE;
				bGetUserInfo           = TRUE;

				break;

			case 'k':
				bNoOptionSelected      = FALSE;
				bWindowsOptionSelected = TRUE;
				bGetUserRightsInfo     = TRUE;

				break;

			case 'w':
				bNoOptionSelected      = FALSE;
				bWindowsOptionSelected = TRUE;
				bGetWinVNCInfo         = TRUE;

				break;

			case 'm':
				bMultipleHosts = FALSE;

				if ( OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken ) )
				{
					if ( LookupPrivilegeValue( NULL, SE_BACKUP_NAME, &luidInfo ) )
					{
						tpInfo.PrivilegeCount           = 1;
						tpInfo.Privileges[0].Luid       = luidInfo;
						tpInfo.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

						if ( AdjustTokenPrivileges( hToken, FALSE, &tpInfo, sizeof( TOKEN_PRIVILEGES ), NULL, NULL ) )
						{
							dwError = GetLastError();

							if ( dwError != ERROR_SUCCESS )
							{
								if ( dwError == ERROR_NOT_ALL_ASSIGNED )
								{
									WriteToErrorLog( "localhost", "AdjustTokenPrivileges (SaveRegistryHives)", "The token does not have one or more of the privileges specified.", &bMultipleHosts );
								}
								else
								{
									WriteLastErrorToErrorLog( "localhost", "AdjustTokenPrivileges (SaveRegistryHives)", &dwError, &bMultipleHosts );
								}
							}
						}
					}
					else
					{
						dwError = GetLastError();

						WriteLastErrorToErrorLog( "localhost", "LookupPrivilegeValue (SaveRegistryHives)", &dwError, &bMultipleHosts );
					}

					CloseHandle( hToken );
				}
				else
				{
					dwError = GetLastError();

					WriteLastErrorToErrorLog( "localhost", "OpenProcessToken (SaveRegistryHives)", &dwError, &bMultipleHosts );
				}

				bNoOptionSelected        = FALSE;
				bWindowsOptionSelected   = TRUE;
				bSaveRemoteRegistryHives = TRUE;

				break;

			case 'n':
				bNoOptionSelected      = FALSE;
				bWindowsOptionSelected = TRUE;
				bGetPatchInfo          = TRUE;

				break;

			case 'o':
				bNoOptionSelected      = FALSE;
				bWindowsOptionSelected = TRUE;
				bGetScheduledTaskInfo  = TRUE;

				break;

			case 'q':
				bNoOptionSelected      = FALSE;
				bWindowsOptionSelected = TRUE;
				bGetSNMPCommunityInfo  = TRUE;

				break;

			case 't':
				bNoOptionSelected      = FALSE;
				bWindowsOptionSelected = TRUE;
				bGetODBCDriverInfo     = TRUE;

				break;

			case 'x':
				bNoOptionSelected              = FALSE;
				bWindowsOptionSelected         = TRUE;
				bGetInteractivelyLoggedOnUsers = TRUE;

				break;

			case 'y':
				bNoOptionSelected        = FALSE;
				bWindowsOptionSelected   = TRUE;
				bGetFileSharePermissions = TRUE;

				break;

			case 'W':
				bNoOptionSelected      = FALSE;
				bGuessWindowsPasswords = TRUE;

				break;

			case 'S':
				bNoOptionSelected          = FALSE;
				bGuessSNMPCommunityStrings = TRUE;

				break;

			case 'v':
				bVerboseOptionSelected = TRUE;

				break;

			case '1':
				bNoOptionSelected      = FALSE;
				bWindowsOptionSelected = TRUE;
				bGroup1OptionSelected  = TRUE;

				break;
		}
	}

	i = 0;

	strcpy( szTargetInput, "" );
	strcpy( szUsername,    "" );
	strcpy( szPassword,    "" );

	if ( optind < argc )
	{
		while ( optind < argc )
		{
			if ( i == 0 )
			{
				strcpy( szTargetInput, argv[optind] );
			}

			if ( i == 1 )
			{
				strcpy( szUsername, argv[optind] );
			}

			if ( i == 2 )
			{
				strcpy( szPassword, argv[optind] );
			}

			optind++;

			i++;
		}
	}

	if ( i < 1 || i > 3 )
	{
		Usage();

		return 1;
	}

	_beginthread( KeyboardEventMonitor, 0, NULL );

	printf( "Running CanScan v1.5 with the following arguments:\n" );
	printf( "[+] Host Input:   \"%s\"\n", szTargetInput );
	printf( "[+] Username:     \"%s\"\n", szUsername );
	printf( "[+] Password:     \"%s\"\n", szPassword );
	printf( "[+] # of Threads: \"64\"\n" );
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
				pThreadArgs->NoOptionSelected              = bNoOptionSelected;
				pThreadArgs->WindowsOptionSelected         = bWindowsOptionSelected;
				pThreadArgs->GetAccountPolicyInfo          = bGetAccountPolicyInfo;
				pThreadArgs->GetInstalledPrograms          = bGetInstalledPrograms;
				pThreadArgs->GetAdminGroupInfo             = bGetAdminGroupInfo;
				pThreadArgs->GetAuditPolicyInfo            = bGetAuditPolicyInfo;
				pThreadArgs->GetAutoAdminLogonInfo         = bGetAutoAdminLogonInfo;
				pThreadArgs->GetDomainInfo                 = bGetDomainInfo;
				pThreadArgs->GetGroupInfo                  = bGetGroupInfo;
				pThreadArgs->GetLDAPInfo                   = bGetLDAPInfo;
				pThreadArgs->GetLoggedOnUsers              = bGetLoggedOnUsers;
				pThreadArgs->GetRAUserInfo                 = bGetRAUserInfo;
				pThreadArgs->GetServerInfo                 = bGetServerInfo;
				pThreadArgs->GetServiceInfo                = bGetServiceInfo;
				pThreadArgs->GetShareInfo                  = bGetShareInfo;
				pThreadArgs->GetUserInfo                   = bGetUserInfo;
				pThreadArgs->GetUserRightsInfo             = bGetUserRightsInfo;
				pThreadArgs->GetWinVNCInfo                 = bGetWinVNCInfo;
				pThreadArgs->SaveRemoteRegistryHives       = bSaveRemoteRegistryHives;
				pThreadArgs->GetPatchInfo                  = bGetPatchInfo;
				pThreadArgs->GetScheduledTaskInfo          = bGetScheduledTaskInfo;
				pThreadArgs->GetSNMPCommunityInfo          = bGetSNMPCommunityInfo;
				pThreadArgs->GetODBCDriverInfo             = bGetODBCDriverInfo;
				pThreadArgs->GetInteractivelyLoggedOnUsers = bGetInteractivelyLoggedOnUsers;
				pThreadArgs->GetFileSharePermissions       = bGetFileSharePermissions;
				pThreadArgs->GuessWindowsPasswords         = bGuessWindowsPasswords;
				pThreadArgs->GuessSNMPCommunityStrings     = bGuessSNMPCommunityStrings;
				pThreadArgs->Group1OptionSelected          = bGroup1OptionSelected;

				strcpy( pThreadArgs->Target,   szTarget );
				strcpy( pThreadArgs->Username, szUsername );
				strcpy( pThreadArgs->Password, szPassword );

				pThreadArgs->MultipleHosts = TRUE;

				WaitForSingleObject( hSemaphore, INFINITE );

				nThreads++;

				ReleaseSemaphore( hSemaphore, 1, NULL );

				_beginthread( ThreadedSub, 0, (VOID *)pThreadArgs );
			}
		}

		fclose( pInputFile );

		Sleep( 2000 );

		printf( "Waiting for threads to terminate...\n" );
	}
	else if ( IsIPRange( szTargetInput, szIPNetwork ) )
	{
		for ( i = 1; i < 255; i++ )
		{
			sprintf( szTarget, "%s%d", szIPNetwork, i );

			while ( nThreads >= MAX_THREADS )
			{
				Sleep( 200 );
			}

			pThreadArgs = (PTHREAD_ARGS)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( THREAD_ARGS ) );

			if ( pThreadArgs != NULL )
			{
				pThreadArgs->NoOptionSelected              = bNoOptionSelected;
				pThreadArgs->WindowsOptionSelected         = bWindowsOptionSelected;
				pThreadArgs->GetAccountPolicyInfo          = bGetAccountPolicyInfo;
				pThreadArgs->GetInstalledPrograms          = bGetInstalledPrograms;
				pThreadArgs->GetAdminGroupInfo             = bGetAdminGroupInfo;
				pThreadArgs->GetAuditPolicyInfo            = bGetAuditPolicyInfo;
				pThreadArgs->GetAutoAdminLogonInfo         = bGetAutoAdminLogonInfo;
				pThreadArgs->GetDomainInfo                 = bGetDomainInfo;
				pThreadArgs->GetGroupInfo                  = bGetGroupInfo;
				pThreadArgs->GetLDAPInfo                   = bGetLDAPInfo;
				pThreadArgs->GetLoggedOnUsers              = bGetLoggedOnUsers;
				pThreadArgs->GetRAUserInfo                 = bGetRAUserInfo;
				pThreadArgs->GetServerInfo                 = bGetServerInfo;
				pThreadArgs->GetServiceInfo                = bGetServiceInfo;
				pThreadArgs->GetShareInfo                  = bGetShareInfo;
				pThreadArgs->GetUserInfo                   = bGetUserInfo;
				pThreadArgs->GetUserRightsInfo             = bGetUserRightsInfo;
				pThreadArgs->GetWinVNCInfo                 = bGetWinVNCInfo;
				pThreadArgs->SaveRemoteRegistryHives       = bSaveRemoteRegistryHives;
				pThreadArgs->GetPatchInfo                  = bGetPatchInfo;
				pThreadArgs->GetScheduledTaskInfo          = bGetScheduledTaskInfo;
				pThreadArgs->GetSNMPCommunityInfo          = bGetSNMPCommunityInfo;
				pThreadArgs->GetODBCDriverInfo             = bGetODBCDriverInfo;
				pThreadArgs->GetInteractivelyLoggedOnUsers = bGetInteractivelyLoggedOnUsers;
				pThreadArgs->GetFileSharePermissions       = bGetFileSharePermissions;
				pThreadArgs->GuessWindowsPasswords         = bGuessWindowsPasswords;
				pThreadArgs->GuessSNMPCommunityStrings     = bGuessSNMPCommunityStrings;
				pThreadArgs->Group1OptionSelected          = bGroup1OptionSelected;

				strcpy( pThreadArgs->Target,   szTarget );
				strcpy( pThreadArgs->Username, szUsername );
				strcpy( pThreadArgs->Password, szPassword );

				pThreadArgs->MultipleHosts = TRUE;

				WaitForSingleObject( hSemaphore, INFINITE );

				nThreads++;

				ReleaseSemaphore( hSemaphore, 1, NULL );

				_beginthread( ThreadedSub, 0, (VOID *)pThreadArgs );
			}
		}

		Sleep( 2000 );

		printf( "Waiting for threads to terminate...\n" );
	}
	else
	{
		strcpy( szTarget, szTargetInput );

		pThreadArgs = (PTHREAD_ARGS)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( THREAD_ARGS ) );

		if ( pThreadArgs != NULL )
		{
			pThreadArgs->NoOptionSelected              = bNoOptionSelected;
			pThreadArgs->WindowsOptionSelected         = bWindowsOptionSelected;
			pThreadArgs->GetAccountPolicyInfo          = bGetAccountPolicyInfo;
			pThreadArgs->GetInstalledPrograms          = bGetInstalledPrograms;
			pThreadArgs->GetAdminGroupInfo             = bGetAdminGroupInfo;
			pThreadArgs->GetAuditPolicyInfo            = bGetAuditPolicyInfo;
			pThreadArgs->GetAutoAdminLogonInfo         = bGetAutoAdminLogonInfo;
			pThreadArgs->GetDomainInfo                 = bGetDomainInfo;
			pThreadArgs->GetGroupInfo                  = bGetGroupInfo;
			pThreadArgs->GetLDAPInfo                   = bGetLDAPInfo;
			pThreadArgs->GetLoggedOnUsers              = bGetLoggedOnUsers;
			pThreadArgs->GetRAUserInfo                 = bGetRAUserInfo;
			pThreadArgs->GetServerInfo                 = bGetServerInfo;
			pThreadArgs->GetServiceInfo                = bGetServiceInfo;
			pThreadArgs->GetShareInfo                  = bGetShareInfo;
			pThreadArgs->GetUserInfo                   = bGetUserInfo;
			pThreadArgs->GetUserRightsInfo             = bGetUserRightsInfo;
			pThreadArgs->GetWinVNCInfo                 = bGetWinVNCInfo;
			pThreadArgs->SaveRemoteRegistryHives       = bSaveRemoteRegistryHives;
			pThreadArgs->GetPatchInfo                  = bGetPatchInfo;
			pThreadArgs->GetScheduledTaskInfo          = bGetScheduledTaskInfo;
			pThreadArgs->GetSNMPCommunityInfo          = bGetSNMPCommunityInfo;
			pThreadArgs->GetODBCDriverInfo             = bGetODBCDriverInfo;
			pThreadArgs->GetInteractivelyLoggedOnUsers = bGetInteractivelyLoggedOnUsers;
			pThreadArgs->GetFileSharePermissions       = bGetFileSharePermissions;
			pThreadArgs->GuessWindowsPasswords         = bGuessWindowsPasswords;
			pThreadArgs->GuessSNMPCommunityStrings     = bGuessSNMPCommunityStrings;
			pThreadArgs->Group1OptionSelected          = bGroup1OptionSelected;

			strcpy( pThreadArgs->Target,   szTarget );
			strcpy( pThreadArgs->Username, szUsername );
			strcpy( pThreadArgs->Password, szPassword );

			pThreadArgs->MultipleHosts = FALSE;

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

VOID Usage( VOID )
{
	printf( "CanScan v1.5 | Canaudit, Inc. http://www.canaudit.com/\n" );
	printf( "\n" );
	printf( "Usage: CanScan <hostname | ip range | ip input file> <username> <password>\n" );
	printf( "\n" );
	printf( "<hostname | ip range | ip input file>  -- required argument\n" );
	printf( "<username>                             -- optional argument\n" );
	printf( "<password>                             -- optional argument\n" );
	printf( "\n" );
	printf( "If the <username> and <password> arguments are omitted, this utility\n" );
	printf( "will attempt to establish a NetBIOS null session and gather information\n" );
	printf( "via the null session.\n" );
	printf( "\n" );
	printf( "If the <username> and <password> arguments are both plus signs (+), the\n" );
	printf( "existing credentials of the user running this utility will be used.\n" );
	printf( "\n" );
	printf( "Examples:\n" );
	printf( "CanScan 10.10.10.10\n" );
	printf( "CanScan 10.10.10.10 + +\n" );
	printf( "CanScan 10.10.10.10 administrator password\n" );
	printf( "CanScan 10.10.10.10 domain\\admin password\n" );
	printf( "\n" );
	printf( "CanScan 192.168.1-254\n" );
	printf( "CanScan 192.168.1-254 + +\n" );
	printf( "CanScan 192.168.1-254 administrator password\n" );
	printf( "CanScan 192.168.1-254 domain\\admin password\n" );
	printf( "\n" );
	printf( "CanScan IPInputFile.txt\n" );
	printf( "CanScan IPInputFile.txt + +\n" );
	printf( "CanScan IPInputFile.txt administrator password\n" );
	printf( "CanScan IPInputFile.txt domain\\admin password\n" );
	printf( "\n" );
	printf( "\n" );
	printf( "==== CanScan Advanced Features ====\n" );
	printf( "\n" );
	printf( "Usage: CanScan [-apbcedgflrijsukwmnoqtxyWSv1] <hostname | ip range | ip input file> <username> <password>\n" );
	printf( "\n" );
	printf( "[-apbcedgflrijsukwmnoqtxyWSv1]         -- optional arguments\n" );
	printf( "<hostname | ip range | ip input file>  -- required argument\n" );
	printf( "<username>                             -- optional argument\n" );
	printf( "<password>                             -- optional argument\n" );
	printf( "\n" );
	printf( "-a  -- Get Account Policy Information\n" );
	printf( "-p  -- Get Installed Programs\n" );
	printf( "-b  -- Get Administrative Local & Global Group Information\n" );
	printf( "-c  -- Get Audit Policy Information\n" );
	printf( "-e  -- Get Auto Admin Logon Information\n" );
	printf( "-d  -- Get Domain Information\n" );
	printf( "-g  -- Get Local & Global Group Information\n" );
	printf( "-f  -- Get LDAP Information\n" );
	printf( "-l  -- Get Logged On Users\n" );
	printf( "-r  -- Get User Information via RA Bypass\n" );
	printf( "-i  -- Get Server Information\n" );
	printf( "-j  -- Get Service Information\n" );
	printf( "-s  -- Get Share Information\n" );
	printf( "-u  -- Get User Information\n" );
	printf( "-k  -- Get User Rights Information\n" );
	printf( "-w  -- Get WinVNC3 & WinVNC4 Passwords\n" );
	printf( "-m  -- Save Remote Registry Hives\n" );
	printf( "-n  -- Get Patch Information\n" );
	printf( "-o  -- Get Scheduled Task Information\n" );
	printf( "-q  -- Get SNMP Community Information\n" );
	printf( "-t  -- Get ODBC Driver Information\n" );
	printf( "-x  -- Get Interactively Logged On Users\n" );
	printf( "-y  -- Get Share Permissions\n" );
	printf( "\n" );
	printf( "-W  -- Guess Windows Passwords\n" );
	printf( "-S  -- Guess SNMP Community Strings\n" );
	printf( "\n" );
	printf( "-v  -- Verbose Output\n" );
	printf( "\n" );
	printf( "-1  -- Group 1 (includes -apbcedlijsukwnoqtxy)\n" );
	printf( "\n" );
	printf( "\n" );
	printf( "==== Windows Password Guessing ====\n" );
	printf( "\n" );
	printf( "For Windows password guessing to occur, there must be a matching\n" );
	printf( "<hostname>.users file in the UserCache directory for each host on which\n" );
	printf( "you attempt to guess passwords. To generate <hostname>.users cache files\n" );
	printf( "use CanScan.exe -Sru <hostname | ip range | ip input file>.\n" );
	printf( "\n" );
	printf( "The passwords that are attempted for each user account are included in the\n" );
	printf( "Dictionary.input file.\n" );
	printf( "\n" );
	printf( "The following can also be used in the Dictionary.input file:\n" );
	printf( "\n" );
	printf( "<username>   -- The name of the current user\n" );
	printf( "<lcusername> -- The name of the current user in lower case\n" );
	printf( "<ucusername> -- The name of the current user in upper case\n" );
	printf( "<blank>      -- A blank or null password\n" );
	printf( "\n" );
	printf( "\n" );
	printf( "==== SNMP Community String Guessing ====\n" );
	printf( "\n" );
	printf( "The SNMP community strings that are attempted for each host are included\n" );
	printf( "in the CommunityStrings.input file.\n" );
	printf( "\n" );
	printf( "(Written by Reed Arvin | reed@canaudit.com)\n" );

	fflush( stdout );
}

VOID KeyboardEventMonitor( VOID *pParameter )
{
	HANDLE              hStdin;
	INPUT_RECORD      irBuffer[ 128 ];
	DWORD            dwNumRead;
	DWORD                    i;
	KEY_EVENT_RECORD   kerInfo;

	hStdin = GetStdHandle( STD_INPUT_HANDLE );

	if ( hStdin != INVALID_HANDLE_VALUE )
	{
		while ( TRUE )
		{
			if ( ReadConsoleInput( hStdin, irBuffer, 128, &dwNumRead ) )
			{
				for ( i = 0; i < dwNumRead; i++ )
				{
					if ( irBuffer[i].EventType == KEY_EVENT )
					{
						kerInfo = irBuffer[i].Event.KeyEvent;

						if ( kerInfo.bKeyDown )
						{
							if ( kerInfo.wVirtualKeyCode == 86 )
							{
								if ( bVerboseOptionSelected == FALSE )
								{
									printf( "\n" );
									printf( "Verbose mode ON.\n" );
									printf( "\n" );

									bVerboseOptionSelected = TRUE;
								}
								else
								{
									printf( "\n" );
									printf( "Verbose mode OFF.\n" );
									printf( "\n" );

									bVerboseOptionSelected = FALSE;
								}

								fflush( stdout );
							}

							if ( kerInfo.wVirtualKeyCode == VK_SPACE )
							{
								printf( "\n" );
								printf( "%d threads currently running.\n", nThreads );
								printf( "\n" );

								fflush( stdout );
							}
						}
					}
				}
			}

			Sleep( 10 );
		}
	}
}

BOOL IsIPRange( CHAR szTargetInput[], CHAR szIPNetwork[] )
{
	BOOL  bReturn;
	DWORD       i;
	DWORD       j;

	bReturn = FALSE;

	if ( strstr( szTargetInput, "1-254" ) != NULL )
	{
		strcpy( szIPNetwork, "" );

		i = 0;
		j = 0;

		while ( szTargetInput[i] != '\0' && j != 3 )
		{
			if ( szTargetInput[i] == '.' )
			{
				j++;
			}
			else
			{
				if ( szTargetInput[i] != '0' && szTargetInput[i] != '1' && szTargetInput[i] != '2' && szTargetInput[i] != '3' && szTargetInput[i] != '4' && szTargetInput[i] != '5' && szTargetInput[i] != '6' && szTargetInput[i] != '7' && szTargetInput[i] != '8' && szTargetInput[i] != '9' )
				{
					break;
				}
			}

			szIPNetwork[i] = szTargetInput[i];

			i++;
		}

		szIPNetwork[i] = '\0';

		if ( j == 3 )
		{
			bReturn = TRUE;
		}
	}

	return bReturn;
}

VOID ThreadedSub( VOID *pParameter )
{
	BOOL              bNoOptionSelected;
	BOOL         bWindowsOptionSelected;
	BOOL          bGetAccountPolicyInfo;
	BOOL          bGetInstalledPrograms;
	BOOL             bGetAdminGroupInfo;
	BOOL            bGetAuditPolicyInfo;
	BOOL         bGetAutoAdminLogonInfo;
	BOOL                 bGetDomainInfo;
	BOOL                  bGetGroupInfo;
	BOOL                   bGetLDAPInfo;
	BOOL              bGetLoggedOnUsers;
	BOOL                 bGetRAUserInfo;
	BOOL                 bGetServerInfo;
	BOOL                bGetServiceInfo;
	BOOL                  bGetShareInfo;
	BOOL                   bGetUserInfo;
	BOOL             bGetUserRightsInfo;
	BOOL                 bGetWinVNCInfo;
	BOOL       bSaveRemoteRegistryHives;
	BOOL                  bGetPatchInfo;
	BOOL          bGetScheduledTaskInfo;
	BOOL          bGetSNMPCommunityInfo;
	BOOL             bGetODBCDriverInfo;
	BOOL bGetInteractivelyLoggedOnUsers;
	BOOL       bGetFileSharePermissions;
	BOOL         bGuessWindowsPasswords;
	BOOL     bGuessSNMPCommunityStrings;
	BOOL          bGroup1OptionSelected;
	CHAR                       szTarget[ 128 ];
	CHAR                     szUsername[ 128 ];
	CHAR                     szPassword[ 128 ];
	BOOL                 bMultipleHosts;
	BOOL                   bSkipConnect;
	BOOL                     bConnected;

	PTHREAD_ARGS pThreadArgs;

	pThreadArgs = (PTHREAD_ARGS)pParameter;

	bNoOptionSelected              = pThreadArgs->NoOptionSelected;
	bWindowsOptionSelected         = pThreadArgs->WindowsOptionSelected;
	bGetAccountPolicyInfo          = pThreadArgs->GetAccountPolicyInfo;
	bGetInstalledPrograms          = pThreadArgs->GetInstalledPrograms;
	bGetAdminGroupInfo             = pThreadArgs->GetAdminGroupInfo;
	bGetAuditPolicyInfo            = pThreadArgs->GetAuditPolicyInfo;
	bGetAutoAdminLogonInfo         = pThreadArgs->GetAutoAdminLogonInfo;
	bGetDomainInfo                 = pThreadArgs->GetDomainInfo;
	bGetGroupInfo                  = pThreadArgs->GetGroupInfo;
	bGetLDAPInfo                   = pThreadArgs->GetLDAPInfo;
	bGetLoggedOnUsers              = pThreadArgs->GetLoggedOnUsers;
	bGetRAUserInfo                 = pThreadArgs->GetRAUserInfo;
	bGetServerInfo                 = pThreadArgs->GetServerInfo;
	bGetServiceInfo                = pThreadArgs->GetServiceInfo;
	bGetShareInfo                  = pThreadArgs->GetShareInfo;
	bGetUserInfo                   = pThreadArgs->GetUserInfo;
	bGetUserRightsInfo             = pThreadArgs->GetUserRightsInfo;
	bGetWinVNCInfo                 = pThreadArgs->GetWinVNCInfo;
	bSaveRemoteRegistryHives       = pThreadArgs->SaveRemoteRegistryHives;
	bGetPatchInfo                  = pThreadArgs->GetPatchInfo;
	bGetScheduledTaskInfo          = pThreadArgs->GetScheduledTaskInfo;
	bGetSNMPCommunityInfo          = pThreadArgs->GetSNMPCommunityInfo;
	bGetODBCDriverInfo             = pThreadArgs->GetODBCDriverInfo;
	bGetInteractivelyLoggedOnUsers = pThreadArgs->GetInteractivelyLoggedOnUsers;
	bGetFileSharePermissions       = pThreadArgs->GetFileSharePermissions;
	bGuessWindowsPasswords         = pThreadArgs->GuessWindowsPasswords;
	bGuessSNMPCommunityStrings     = pThreadArgs->GuessSNMPCommunityStrings;
	bGroup1OptionSelected          = pThreadArgs->Group1OptionSelected;

	strcpy( szTarget,   pThreadArgs->Target );
	strcpy( szUsername, pThreadArgs->Username );
	strcpy( szPassword, pThreadArgs->Password );

	bMultipleHosts = pThreadArgs->MultipleHosts;

	HeapFree( GetProcessHeap(), 0, pThreadArgs );

	if ( bMultipleHosts )
	{
		printf( "Spawning thread for host %s...\n", szTarget );

		fflush( stdout );
	}

	bSkipConnect = FALSE;
	bConnected   = FALSE;

	if ( strcmp( szUsername, "+" ) == 0 && strcmp( szPassword, "+" ) == 0 )
	{
		bSkipConnect = TRUE;
	}
	else
	{
		if ( bNoOptionSelected || bWindowsOptionSelected )
		{
			if ( Connect( szTarget, szUsername, szPassword, &bMultipleHosts ) )
			{
				bConnected = TRUE;
			}
		}
	}

	if ( bSkipConnect || bConnected )
	{
		if ( bGetAccountPolicyInfo || bNoOptionSelected || bGroup1OptionSelected )
		{
			GetAccountPolicyInfo( szTarget, &bMultipleHosts );
		}

		if ( bGetInstalledPrograms || bGroup1OptionSelected )
		{
			GetInstalledPrograms( szTarget, &bMultipleHosts );
		}

		if ( bGetAdminGroupInfo || bNoOptionSelected || bGroup1OptionSelected )
		{
			GetAdminLocalGroupInfo( szTarget, &bMultipleHosts );
			GetAdminGlobalGroupInfo( szTarget, &bMultipleHosts );
		}

		if ( bGetAuditPolicyInfo || bGroup1OptionSelected )
		{
			GetAuditPolicyInfo( szTarget, &bMultipleHosts );
		}

		if ( bGetAutoAdminLogonInfo || bGroup1OptionSelected )
		{
			GetAutoAdminLogonInfo( szTarget, &bMultipleHosts );
		}

		if ( bGetDomainInfo || bNoOptionSelected || bGroup1OptionSelected )
		{
			GetDomainInfo( szTarget, &bMultipleHosts );
		}

		if ( bGetGroupInfo )
		{
			GetLocalGroupInfo( szTarget, &bMultipleHosts );
			GetGlobalGroupInfo( szTarget, &bMultipleHosts );
		}

		if ( bGetLoggedOnUsers || bNoOptionSelected || bGroup1OptionSelected )
		{
			GetLoggedOnUsers( szTarget, &bMultipleHosts );
		}

		if ( bGetRAUserInfo || bNoOptionSelected )
		{
			GetRAUserInfo( szTarget, &bMultipleHosts );
		}

		if ( bGetServerInfo || bNoOptionSelected || bGroup1OptionSelected )
		{
			GetServerInfo( szTarget, &bMultipleHosts );
		}

		if ( bGetServiceInfo || bGroup1OptionSelected )
		{
			GetServiceInfo( szTarget, &bMultipleHosts );
		}

		if ( bGetShareInfo || bNoOptionSelected || bGroup1OptionSelected )
		{
			GetShareInfo( szTarget, &bMultipleHosts );
		}

		if ( bGetUserInfo || bNoOptionSelected || bGroup1OptionSelected )
		{
			GetUserInfo( szTarget, &bMultipleHosts );
		}

		if ( bGetUserRightsInfo || bGroup1OptionSelected )
		{
			GetUserRightsInfo( szTarget, &bMultipleHosts );
		}

		if ( bGetWinVNCInfo || bGroup1OptionSelected )
		{
			GetWinVNC3Info( szTarget, &bMultipleHosts );
			GetWinVNC4Info( szTarget, &bMultipleHosts );
		}

		if ( bSaveRemoteRegistryHives )
		{
			SaveRegistryHives( szTarget, &bMultipleHosts );
		}

		if ( bGetPatchInfo || bGroup1OptionSelected )
		{
			GetPatchInfo( szTarget, &bMultipleHosts );
		}

		if ( bGetScheduledTaskInfo || bGroup1OptionSelected )
		{
			GetScheduledTaskInfo( szTarget, &bMultipleHosts );
		}

		if ( bGetSNMPCommunityInfo || bGroup1OptionSelected )
		{
			GetSNMPCommunityInfo( szTarget, &bMultipleHosts );
		}

		if ( bGetODBCDriverInfo || bGroup1OptionSelected )
		{
			GetODBCDriverInfo( szTarget, &bMultipleHosts );
		}

		if ( bGetInteractivelyLoggedOnUsers || bGroup1OptionSelected )
		{
			GetInteractivelyLoggedOnUsers( szTarget, &bMultipleHosts );
		}

		if ( bGetFileSharePermissions || bGroup1OptionSelected )
		{
			GetFileSharePermissions( szTarget, &bMultipleHosts );
		}
	}

	if ( bConnected )
	{
		Disconnect( szTarget, &bMultipleHosts );
	}

	if ( bGetLDAPInfo )
	{
		GetLDAPInfo( szTarget, &bMultipleHosts );
	}

	if ( bGuessWindowsPasswords )
	{
		GuessWindowsPasswords( szTarget, &bMultipleHosts );
	}

	if ( bGuessSNMPCommunityStrings )
	{
		GuessSNMPCommunityStrings( szTarget, &bMultipleHosts );
	}

	WaitForSingleObject( hSemaphore, INFINITE );

	nThreads--;

	ReleaseSemaphore( hSemaphore, 1, NULL );

	_endthread();
}

BOOL Connect( CHAR szTarget[], CHAR szUsername[], CHAR szPassword[], BOOL *bMultipleHosts )
{
	BOOL                  bReturn;
	CHAR               *pLocation;
	CHAR             szTempTarget[ 128 ];
	CHAR             szRemoteName[ 128 ];
	DWORD          dwTextLocation;
	DWORD                       i;
	CHAR             szDomainName[ 128 ];
	DWORD                       j;
	CHAR           szTempUsername[ 128 ];
	WCHAR           wszRemoteName[ 256 ];
	WCHAR           wszDomainName[ 256 ];
	WCHAR             wszUsername[ 256 ];
	WCHAR             wszPassword[ 256 ];
	DWORD                 dwLevel;
	USE_INFO_2            ui2Info;
	NET_API_STATUS        nStatus;
	DWORD                 dwError;

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

	pLocation = strstr( szUsername, "\\" );

	if ( pLocation != NULL )
	{
		dwTextLocation = (INT)( pLocation - szUsername );

		i = 0;

		while ( i < dwTextLocation )
		{
			szDomainName[i] = szUsername[i];

			i++;
		}

		szDomainName[i] = '\0';

		i = dwTextLocation + 1;

		j = 0;

		while ( i < strlen( szUsername ) )
		{
			szTempUsername[j] = szUsername[i];

			i++;
			j++;
		}

		szTempUsername[j] = '\0';
	}
	else
	{
		if ( strcmp( szUsername, "" ) != 0 )
		{
			strcpy( szDomainName, szTarget );
		}
		else
		{
			strcpy( szDomainName, "" );
		}

		strcpy( szTempUsername, szUsername );
	}

	MultiByteToWideChar( CP_ACP, 0, szRemoteName,   strlen( szRemoteName ) + 1,   wszRemoteName, sizeof( wszRemoteName ) / sizeof( wszRemoteName[0] ) );
	MultiByteToWideChar( CP_ACP, 0, szDomainName,   strlen( szDomainName ) + 1,   wszDomainName, sizeof( wszDomainName ) / sizeof( wszDomainName[0] ) );
	MultiByteToWideChar( CP_ACP, 0, szTempUsername, strlen( szTempUsername ) + 1, wszUsername,   sizeof( wszUsername ) / sizeof( wszUsername[0] ) );
	MultiByteToWideChar( CP_ACP, 0, szPassword,     strlen( szPassword ) + 1,     wszPassword,   sizeof( wszPassword ) / sizeof( wszPassword[0] ) );

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

		WriteLastErrorToErrorLog( szTarget, "NetUseAdd (Connect)", &dwError, bMultipleHosts );

		WriteLastErrorToConnectErrorLog( szTarget, szRemoteName, szDomainName, szTempUsername, szPassword, &dwError );
	}

	return bReturn;
}

VOID WriteLastErrorToConnectErrorLog( CHAR szTarget[], CHAR szRemoteLocation[], CHAR szDomainName[], CHAR szUsername[], CHAR szPassword[], DWORD *dwError )
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

	WaitForSingleObject( hSemaphore, INFINITE );

	pOutputFile = fopen( "ConnectErrorLog.txt", "r" );

	if ( pOutputFile != NULL )
	{
		fclose( pOutputFile );
	}
	else
	{
		pOutputFile = fopen( "ConnectErrorLog.txt", "w" );

		if ( pOutputFile != NULL )
		{
			fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
			fprintf( pOutputFile, "\n" );
			fprintf( pOutputFile, "Hostname\tRemote Location\tAccount Name\tPassword\tError Number\tError Message\n" );

			fclose( pOutputFile );
		}
	}

	pOutputFile = fopen( "ConnectErrorLog.txt", "a+" );

	if ( pOutputFile != NULL )
	{
		if ( strcmp( szDomainName, "" ) == 0 && strcmp( szUsername, "" ) == 0 )
		{
			if ( strcmp( szPassword, "" ) == 0 )
			{
				fprintf( pOutputFile, "%s\t%s\t(Null)\t<blank>\t%d\t%s\n", szTarget, szRemoteLocation, *dwError, szErrorMsg );
			}
			else
			{
				fprintf( pOutputFile, "%s\t%s\t(Null)\t%s\t%d\t%s\n", szTarget, szRemoteLocation, szPassword, *dwError, szErrorMsg );
			}
		}
		else
		{
			if ( strcmp( szPassword, "" ) == 0 )
			{
				fprintf( pOutputFile, "%s\t%s\t%s\\%s\t<blank>\t%d\t%s\n", szTarget, szRemoteLocation, szDomainName, szUsername, *dwError, szErrorMsg );
			}
			else
			{
				fprintf( pOutputFile, "%s\t%s\t%s\\%s\t%s\t%d\t%s\n", szTarget, szRemoteLocation, szDomainName, szUsername, szPassword, *dwError, szErrorMsg );
			}
		}

		fclose( pOutputFile );
	}

	ReleaseSemaphore( hSemaphore, 1, NULL );
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

		WriteLastErrorToErrorLog( szTarget, "NetUseDel (Disconnect)", &dwError, bMultipleHosts );
	}

	return bReturn;
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
			WriteToErrorLog( szTarget, "NetUserModalsGet (GetAccountPolicyInfo)", "The user does not have access to the requested information.", bMultipleHosts );
		}
		else if ( nStatus == NERR_InvalidComputer )
		{
			WriteToErrorLog( szTarget, "NetUserModalsGet (GetAccountPolicyInfo)", "The computer name is invalid.", bMultipleHosts );
		}
		else
		{
			dwError = nStatus;

			WriteLastErrorToErrorLog( szTarget, "NetUserModalsGet (GetAccountPolicyInfo)", &dwError, bMultipleHosts );
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
			WriteToErrorLog( szTarget, "NetUserModalsGet (GetAccountPolicyInfo)", "The user does not have access to the requested information.", bMultipleHosts );
		}
		else if ( nStatus == NERR_InvalidComputer )
		{
			WriteToErrorLog( szTarget, "NetUserModalsGet (GetAccountPolicyInfo)", "The computer name is invalid.", bMultipleHosts );
		}
		else
		{
			dwError = nStatus;

			WriteLastErrorToErrorLog( szTarget, "NetUserModalsGet (GetAccountPolicyInfo)", &dwError, bMultipleHosts );
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
			WriteToErrorLog( szTarget, "NetUserModalsGet (GetAccountPolicyInfo)", "The user does not have access to the requested information.", bMultipleHosts );
		}
		else if ( nStatus == NERR_InvalidComputer )
		{
			WriteToErrorLog( szTarget, "NetUserModalsGet (GetAccountPolicyInfo)", "The computer name is invalid.", bMultipleHosts );
		}
		else
		{
			dwError = nStatus;

			WriteLastErrorToErrorLog( szTarget, "NetUserModalsGet (GetAccountPolicyInfo)", &dwError, bMultipleHosts );
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
			WriteToErrorLog( szTarget, "NetUserModalsGet (GetAccountPolicyInfo)", "The user does not have access to the requested information.", bMultipleHosts );
		}
		else if ( nStatus == NERR_InvalidComputer )
		{
			WriteToErrorLog( szTarget, "NetUserModalsGet (GetAccountPolicyInfo)", "The computer name is invalid.", bMultipleHosts );
		}
		else
		{
			dwError = nStatus;

			WriteLastErrorToErrorLog( szTarget, "NetUserModalsGet (GetAccountPolicyInfo)", &dwError, bMultipleHosts );
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

		if ( bVerboseOptionSelected && *bMultipleHosts )
		{
			printf( "%s -> Logging account policy information.\n", szTarget );

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
								printf( "+++++            INSTALLED PROGRAMS           +++++\n" );
								printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
								printf( "\n" );

								i++;
							}

							printf( "Display Name: %s\n", szDisplayName );
							printf( "Install Date: %s\n", szInstallDate );
							printf( "\n" );

							fflush( stdout );
						}

						if ( bVerboseOptionSelected && *bMultipleHosts )
						{
							printf( "%s -> Logging installed programs.\n", szTarget );

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
				else
				{
					dwError = lReturn1;

					WriteLastErrorToErrorLog( szTarget, "RegOpenKeyEx (GetInstalledPrograms)", &dwError, bMultipleHosts );
				}
			}

			RegCloseKey( hSubKey1 );
		}
		else
		{
			dwError = lReturn1;

			WriteLastErrorToErrorLog( szTarget, "RegOpenKeyEx (GetInstalledPrograms)", &dwError, bMultipleHosts );
		}

		RegCloseKey( hKey );
	}
	else
	{
		dwError = lReturn1;

		WriteLastErrorToErrorLog( szTarget, "RegConnectRegistry (GetInstalledPrograms)", &dwError, bMultipleHosts );
	}
}

VOID GetAdminLocalGroupInfo( CHAR szTarget[], BOOL *bMultipleHosts )
{
	CHAR                          *pLocation;
	CHAR                        szTempTarget[ 128 ];
	WCHAR                          wszTarget[ 256 ];
	DWORD                                  i;
	CHAR                         szGroupName[ 128 ];
	WCHAR                       wszGroupName[ 256 ];
	DWORD                            dwLevel;
	DWORD                      dwEntriesRead;
	DWORD                     dwTotalEntries;
	DWORD                     dwResumeHandle;
	LOCALGROUP_MEMBERS_INFO_2         *pInfo;
	NET_API_STATUS                   nStatus;
	LOCALGROUP_MEMBERS_INFO_2     *pTempInfo;
	DWORD                                  j;
	CHAR                          szUsername[ 128 ];
	FILE                        *pOutputFile;
	DWORD                            dwError;

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

	strcpy( szGroupName, "Administrators" );

	MultiByteToWideChar( CP_ACP, 0, szGroupName, strlen( szGroupName ) + 1, wszGroupName, sizeof( wszGroupName ) / sizeof( wszGroupName[0] ) );

	dwLevel        = 2;
	dwEntriesRead  = 0;
	dwTotalEntries = 0;
	dwResumeHandle = 0;

	do
	{
		pInfo = NULL;

		nStatus = NetLocalGroupGetMembers( wszTarget, wszGroupName, dwLevel, (PBYTE *)&pInfo, MAX_PREFERRED_LENGTH, &dwEntriesRead, &dwTotalEntries, &dwResumeHandle );

		if ( nStatus == NERR_Success || nStatus == ERROR_MORE_DATA )
		{
			if ( pInfo != NULL )
			{
				pTempInfo = pInfo;

				for ( j = 0; j < dwEntriesRead; j++ )
				{
					WideCharToMultiByte( CP_ACP, 0, pTempInfo->lgrmi2_domainandname, -1, szUsername, 128, NULL, NULL );

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

					if ( bVerboseOptionSelected && *bMultipleHosts )
					{
						printf( "%s -> Logging local group information.\n", szTarget );

						fflush( stdout );
					}

					WaitForSingleObject( hSemaphore, INFINITE );

					pOutputFile = fopen( "AdminGroupInfo.txt", "r" );

					if ( pOutputFile != NULL )
					{
						fclose( pOutputFile );
					}
					else
					{
						pOutputFile = fopen( "AdminGroupInfo.txt", "w" );

						if ( pOutputFile != NULL )
						{
							fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
							fprintf( pOutputFile, "\n" );
							fprintf( pOutputFile, "Hostname\tGroup Name\tGroup Type\tUsername\n" );

							fclose( pOutputFile );
						}
					}

					pOutputFile = fopen( "AdminGroupInfo.txt", "a+" );

					if ( pOutputFile != NULL )
					{
						fprintf( pOutputFile, "%s\t%s\tLocal\t%s\n", szTarget, szGroupName, szUsername );

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
				WriteToErrorLog( szTarget, "NetLocalGroupGetMembers (GetAdminLocalGroupInfo)", "The user does not have access to the requested information.", bMultipleHosts );
			}
			else if ( nStatus == NERR_InvalidComputer )
			{
				WriteToErrorLog( szTarget, "NetLocalGroupGetMembers (GetAdminLocalGroupInfo)", "The computer name is invalid.", bMultipleHosts );
			}
			else if ( nStatus == ERROR_NO_SUCH_ALIAS )
			{
				WriteToErrorLog( szTarget, "NetLocalGroupGetMembers (GetAdminLocalGroupInfo)", "The specified local group does not exist.", bMultipleHosts );
			}
			else
			{
				dwError = nStatus;

				WriteLastErrorToErrorLog( szTarget, "NetLocalGroupGetMembers (GetAdminLocalGroupInfo)", &dwError, bMultipleHosts );
			}
		}

		if ( pInfo != NULL )
		{
			NetApiBufferFree( pInfo );
		}
	}
	while ( nStatus == ERROR_MORE_DATA );
}

VOID GetAdminGlobalGroupInfo( CHAR szTarget[], BOOL *bMultipleHosts )
{
	CHAR                   *pLocation;
	CHAR                 szTempTarget[ 128 ];
	WCHAR                   wszTarget[ 256 ];
	DWORD                           i;
	DWORD                           j;
	CHAR                  szGroupName[ 128 ];
	WCHAR                wszGroupName[ 256 ];
	DWORD                     dwLevel;
	DWORD               dwEntriesRead;
	DWORD              dwTotalEntries;
	DWORD              dwResumeHandle;
	GROUP_USERS_INFO_0         *pInfo;
	NET_API_STATUS            nStatus;
	GROUP_USERS_INFO_0     *pTempInfo;
	DWORD                           k;
	CHAR                   szUsername[ 128 ];
	FILE                 *pOutputFile;
	DWORD                     dwError;

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

	for ( j = 0; j < 2; j++ )
	{
		if ( j == 0 )
		{
			strcpy( szGroupName, "Enterprise Admins" );
		}

		if ( j == 1 )
		{
			strcpy( szGroupName, "Domain Admins" );
		}

		MultiByteToWideChar( CP_ACP, 0, szGroupName, strlen( szGroupName ) + 1, wszGroupName, sizeof( wszGroupName ) / sizeof( wszGroupName[0] ) );

		dwLevel        = 0;
		dwEntriesRead  = 0;
		dwTotalEntries = 0;
		dwResumeHandle = 0;

		do
		{
			pInfo = NULL;

			nStatus = NetGroupGetUsers( wszTarget, wszGroupName, dwLevel, (PBYTE *)&pInfo, MAX_PREFERRED_LENGTH, &dwEntriesRead, &dwTotalEntries, &dwResumeHandle );

			if ( nStatus == NERR_Success || nStatus == ERROR_MORE_DATA )
			{
				if ( pInfo != NULL )
				{
					pTempInfo = pInfo;

					for ( k = 0; k < dwEntriesRead; k++ )
					{
						WideCharToMultiByte( CP_ACP, 0, pTempInfo->grui0_name, -1, szUsername, 128, NULL, NULL );

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

						if ( bVerboseOptionSelected && *bMultipleHosts )
						{
							printf( "%s -> Logging global group information.\n", szTarget );

							fflush( stdout );
						}

						WaitForSingleObject( hSemaphore, INFINITE );

						pOutputFile = fopen( "AdminGroupInfo.txt", "r" );

						if ( pOutputFile != NULL )
						{
							fclose( pOutputFile );
						}
						else
						{
							pOutputFile = fopen( "AdminGroupInfo.txt", "w" );

							if ( pOutputFile != NULL )
							{
								fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
								fprintf( pOutputFile, "\n" );
								fprintf( pOutputFile, "Hostname\tGroup Name\tGroup Type\tUsername\n" );

								fclose( pOutputFile );
							}
						}

						pOutputFile = fopen( "AdminGroupInfo.txt", "a+" );

						if ( pOutputFile != NULL )
						{
							fprintf( pOutputFile, "%s\t%s\tGlobal\t%s\n", szTarget, szGroupName, szUsername );

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
					WriteToErrorLog( szTarget, "NetGroupGetUsers (GetAdminGlobalGroupInfo)", "The user does not have access to the requested information.", bMultipleHosts );
				}
				else if ( nStatus == ERROR_INVALID_LEVEL )
				{
					WriteToErrorLog( szTarget, "NetGroupGetUsers (GetAdminGlobalGroupInfo)", "The system call level is not correct.", bMultipleHosts );
				}
				else if ( nStatus == ERROR_NOT_ENOUGH_MEMORY )
				{
					WriteToErrorLog( szTarget, "NetGroupGetUsers (GetAdminGlobalGroupInfo)", "Insufficient memory was available to complete the operation.", bMultipleHosts );
				}
				else if ( nStatus == NERR_InvalidComputer )
				{
					WriteToErrorLog( szTarget, "NetGroupGetUsers (GetAdminGlobalGroupInfo)", "The computer name is invalid.", bMultipleHosts );
				}
				else if ( nStatus == NERR_GroupNotFound )
				{
					WriteToErrorLog( szTarget, "NetGroupGetUsers (GetAdminGlobalGroupInfo)", "The global group name could not be found.", bMultipleHosts );
				}
				else if ( nStatus == NERR_InternalError )
				{
					WriteToErrorLog( szTarget, "NetGroupGetUsers (GetAdminGlobalGroupInfo)", "An internal error occurred.", bMultipleHosts );
				}
				else
				{
					dwError = nStatus;

					WriteLastErrorToErrorLog( szTarget, "NetGroupGetUsers (GetAdminGlobalGroupInfo)", &dwError, bMultipleHosts );
				}
			}

			if ( pInfo != NULL )
			{
				NetApiBufferFree( pInfo );
			}
		}
		while ( nStatus == ERROR_MORE_DATA );
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

			if ( bVerboseOptionSelected && *bMultipleHosts )
			{
				printf( "%s -> Logging audit policy information.\n", szTarget );

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

			WriteLastErrorToErrorLog( szTarget, "LsaQueryInformationPolicy (GetAuditPolicyInfo)", &dwError, bMultipleHosts );
		}

		LsaClose( lsahPolicyHandle );
	}
	else
	{
		dwError = LsaNtStatusToWinError( ntStatus );

		WriteLastErrorToErrorLog( szTarget, "LsaOpenPolicy (GetAuditPolicyInfo)", &dwError, bMultipleHosts );
	}
}

VOID GetAutoAdminLogonInfo( CHAR szTarget[], BOOL *bMultipleHosts )
{
	CHAR    *pLocation;
	CHAR  szTempTarget[ 128 ];
	LONG       lReturn;
	HKEY          hKey;
	HKEY       hSubKey;
	CHAR  szDomainName[ 128 ];
	CHAR    szUsername[ 128 ];
	CHAR    szPassword[ 128 ];
	DWORD dwBufferSize;
	FILE  *pOutputFile;
	DWORD      dwError;

	pLocation = strstr( szTarget, "\\\\" );

	if ( pLocation != NULL )
	{
		strcpy( szTempTarget, szTarget );
	}
	else
	{
		sprintf( szTempTarget, "\\\\%s", szTarget );
	}

	lReturn = RegConnectRegistry( szTempTarget, HKEY_LOCAL_MACHINE, &hKey );

	if ( lReturn == ERROR_SUCCESS )
	{
		lReturn = RegOpenKeyEx( hKey, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", 0, KEY_QUERY_VALUE, &hSubKey );

		if ( lReturn == ERROR_SUCCESS )
		{
			strcpy( szDomainName, "" );
			strcpy( szUsername,   "" );
			strcpy( szPassword,   "" );

			dwBufferSize = 128;

			lReturn = RegQueryValueEx( hSubKey, "DefaultDomainName", NULL, NULL, (BYTE *)szDomainName, &dwBufferSize );

			dwBufferSize = 128;

			lReturn = RegQueryValueEx( hSubKey, "DefaultUserName", NULL, NULL, (BYTE *)szUsername, &dwBufferSize );

			dwBufferSize = 128;

			lReturn = RegQueryValueEx( hSubKey, "DefaultPassword", NULL, NULL, (BYTE *)szPassword, &dwBufferSize );

			if ( lReturn == ERROR_SUCCESS )
			{
				if ( strcmp( szPassword, "" ) == 0 )
				{
					strcpy( szPassword, "<blank>" );
				}

				if ( !*bMultipleHosts )
				{
					printf( "\n" );
					printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
					printf( "+++++      AUTO ADMIN LOGON INFORMATION       +++++\n" );
					printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
					printf( "\n" );

					printf( "Domain Name: %s\n", szDomainName );
					printf( "Username:    %s\n", szUsername );
					printf( "Password:    %s\n", szPassword );
					printf( "\n" );

					fflush( stdout );
				}

				if ( bVerboseOptionSelected && *bMultipleHosts )
				{
					printf( "%s -> Logging Auto Admin Logon information.\n", szTarget );

					fflush( stdout );
				}

				WaitForSingleObject( hSemaphore, INFINITE );

				pOutputFile = fopen( "AutoAdminLogonInfo.txt", "r" );

				if ( pOutputFile != NULL )
				{
					fclose( pOutputFile );
				}
				else
				{
					pOutputFile = fopen( "AutoAdminLogonInfo.txt", "w" );

					if ( pOutputFile != NULL )
					{
						fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
						fprintf( pOutputFile, "\n" );
						fprintf( pOutputFile, "Hostname\tDomain Name\tUsername\tPassword\n" );

						fclose( pOutputFile );
					}
				}

				pOutputFile = fopen( "AutoAdminLogonInfo.txt", "a+" );

				if ( pOutputFile != NULL )
				{
					fprintf( pOutputFile, "%s\t%s\t%s\t%s\n", szTarget, szDomainName, szUsername, szPassword );

					fclose( pOutputFile );
				}

				ReleaseSemaphore( hSemaphore, 1, NULL );
			}

			RegCloseKey( hSubKey );
		}
		else
		{
			dwError = lReturn;

			WriteLastErrorToErrorLog( szTarget, "RegOpenKeyEx (GetAutoAdminLogonInfo)", &dwError, bMultipleHosts );
		}

		RegCloseKey( hKey );
	}
	else
	{
		dwError = lReturn;

		WriteLastErrorToErrorLog( szTarget, "RegConnectRegistry (GetAutoAdminLogonInfo)", &dwError, bMultipleHosts );
	}
}

VOID GetDomainInfo( CHAR szTarget[], BOOL *bMultipleHosts )
{
	WCHAR                             wszTarget[ 256 ];
	LSA_OBJECT_ATTRIBUTES         loaAttributes;
	LSA_UNICODE_STRING            lusSystemName;
	NTSTATUS                           ntStatus;
	LSA_HANDLE                 lsahPolicyHandle;
	POLICY_ACCOUNT_DOMAIN_INFO           *pInfo1;
	CHAR                           szDomainName[ 128 ];
	CHAR                       szTrustedDomains[ 512 ];
	DWORD                               lReturn;
	DS_DOMAIN_TRUSTS                    *pInfo2;
	ULONG                          uDomainCount;
	DWORD                                     i;
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
		pInfo1 = NULL;

		ntStatus = LsaQueryInformationPolicy( lsahPolicyHandle, PolicyAccountDomainInformation, (PVOID *)&pInfo1 );

		if ( ntStatus == 0 )
		{
			sprintf( szDomainName, "%.*S", pInfo1->DomainName.Length, pInfo1->DomainName.Buffer );

			strcpy( szTrustedDomains, "" );

			lReturn = DsEnumerateDomainTrusts( szTarget, DS_DOMAIN_DIRECT_INBOUND | DS_DOMAIN_DIRECT_OUTBOUND | DS_DOMAIN_DIRECT_OUTBOUND | DS_DOMAIN_NATIVE_MODE | DS_DOMAIN_PRIMARY | DS_DOMAIN_TREE_ROOT, &pInfo2, &uDomainCount );

			if ( lReturn == ERROR_SUCCESS )
			{
				for ( i = 0; i < uDomainCount; i++ )
				{
					sprintf( szTrustedDomains, "%s%s(%s) ", szTrustedDomains, pInfo2[i].DnsDomainName, pInfo2[i].NetbiosDomainName );
				}

				NetApiBufferFree( pInfo2 );
			}

			if ( !*bMultipleHosts )
			{
				printf( "\n" );
				printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
				printf( "+++++           DOMAIN INFORMATION            +++++\n" );
				printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
				printf( "\n" );

				printf( "Domain Name:     %s\n", szDomainName );
				printf( "Trusted Domains: %s\n", szTrustedDomains );
				printf( "\n" );

				fflush( stdout );
			}

			if ( bVerboseOptionSelected && *bMultipleHosts )
			{
				printf( "%s -> Logging domain information.\n", szTarget );

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
					fprintf( pOutputFile, "Hostname\tDomain Name\tTrusted Domains\n" );

					fclose( pOutputFile );
				}
			}

			pOutputFile = fopen( "DomainInfo.txt", "a+" );

			if ( pOutputFile != NULL )
			{
				fprintf( pOutputFile, "%s\t%s\t%s\n", szTarget, szDomainName, szTrustedDomains );

				fclose( pOutputFile );
			}

			ReleaseSemaphore( hSemaphore, 1, NULL );

			LsaFreeMemory( pInfo1 );
		}
		else
		{
			dwError = LsaNtStatusToWinError( ntStatus );

			WriteLastErrorToErrorLog( szTarget, "LsaQueryInformationPolicy (GetDomainInfo)", &dwError, bMultipleHosts );
		}

		LsaClose( lsahPolicyHandle );
	}
	else
	{
		dwError = LsaNtStatusToWinError( ntStatus );

		WriteLastErrorToErrorLog( szTarget, "LsaOpenPolicy (GetDomainInfo)", &dwError, bMultipleHosts );
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

		nStatus1 = NetLocalGroupEnum( wszTarget, dwLevel1, (PBYTE *)&pInfo1, MAX_PREFERRED_LENGTH, &dwEntriesRead1, &dwTotalEntries1, &dwResumeHandle1 );

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

						nStatus2 = NetLocalGroupGetMembers( wszTarget, wszGroupName, dwLevel2, (PBYTE *)&pInfo2, MAX_PREFERRED_LENGTH, &dwEntriesRead2, &dwTotalEntries2, &dwResumeHandle2 );

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

									if ( bVerboseOptionSelected && *bMultipleHosts )
									{
										printf( "%s -> Logging local group information.\n", szTarget );

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
								WriteToErrorLog( szTarget, "NetLocalGroupGetMembers (GetLocalGroupInfo)", "The user does not have access to the requested information.", bMultipleHosts );
							}
							else if ( nStatus2 == NERR_InvalidComputer )
							{
								WriteToErrorLog( szTarget, "NetLocalGroupGetMembers (GetLocalGroupInfo)", "The computer name is invalid.", bMultipleHosts );
							}
							else if ( nStatus2 == ERROR_NO_SUCH_ALIAS )
							{
								WriteToErrorLog( szTarget, "NetLocalGroupGetMembers (GetLocalGroupInfo)", "The specified local group does not exist.", bMultipleHosts );
							}
							else
							{
								dwError = nStatus2;

								WriteLastErrorToErrorLog( szTarget, "NetLocalGroupGetMembers (GetLocalGroupInfo)", &dwError, bMultipleHosts );
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
				WriteToErrorLog( szTarget, "NetLocalGroupEnum (GetLocalGroupInfo)", "The user does not have access to the requested information.", bMultipleHosts );
			}
			else if ( nStatus1 == NERR_InvalidComputer )
			{
				WriteToErrorLog( szTarget, "NetLocalGroupEnum (GetLocalGroupInfo)", "The computer name is invalid.", bMultipleHosts );
			}
			else if ( nStatus1 == NERR_BufTooSmall )
			{
				WriteToErrorLog( szTarget, "NetLocalGroupEnum (GetLocalGroupInfo)", "The return buffer is too small.", bMultipleHosts );
			}
			else
			{
				dwError = nStatus1;

				WriteLastErrorToErrorLog( szTarget, "NetLocalGroupEnum (GetLocalGroupInfo)", &dwError, bMultipleHosts );
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

		nStatus1 = NetGroupEnum( wszTarget, dwLevel1, (PBYTE *)&pInfo1, MAX_PREFERRED_LENGTH, &dwEntriesRead1, &dwTotalEntries1, &dwResumeHandle1 );

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

						nStatus2 = NetGroupGetUsers( wszTarget, wszGroupName, dwLevel0, (PBYTE *)&pInfo0, MAX_PREFERRED_LENGTH, &dwEntriesRead2, &dwTotalEntries2, &dwResumeHandle2 );

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

									if ( bVerboseOptionSelected && *bMultipleHosts )
									{
										printf( "%s -> Logging global group information.\n", szTarget );

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
								WriteToErrorLog( szTarget, "NetGroupGetUsers (GetGlobalGroupInfo)", "The user does not have access to the requested information.", bMultipleHosts );
							}
							else if ( nStatus2 == ERROR_INVALID_LEVEL )
							{
								WriteToErrorLog( szTarget, "NetGroupGetUsers (GetGlobalGroupInfo)", "The system call level is not correct.", bMultipleHosts );
							}
							else if ( nStatus2 == ERROR_NOT_ENOUGH_MEMORY )
							{
								WriteToErrorLog( szTarget, "NetGroupGetUsers (GetGlobalGroupInfo)", "Insufficient memory was available to complete the operation.", bMultipleHosts );
							}
							else if ( nStatus2 == NERR_InvalidComputer )
							{
								WriteToErrorLog( szTarget, "NetGroupGetUsers (GetGlobalGroupInfo)", "The computer name is invalid.", bMultipleHosts );
							}
							else if ( nStatus2 == NERR_GroupNotFound )
							{
								WriteToErrorLog( szTarget, "NetGroupGetUsers (GetGlobalGroupInfo)", "The global group name could not be found.", bMultipleHosts );
							}
							else if ( nStatus2 == NERR_InternalError )
							{
								WriteToErrorLog( szTarget, "NetGroupGetUsers (GetGlobalGroupInfo)", "An internal error occurred.", bMultipleHosts );
							}
							else
							{
								dwError = nStatus2;

								WriteLastErrorToErrorLog( szTarget, "NetGroupGetUsers (GetGlobalGroupInfo)", &dwError, bMultipleHosts );
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
				WriteToErrorLog( szTarget, "NetGroupEnum (GetGlobalGroupInfo)", "The user does not have access to the requested information.", bMultipleHosts );
			}
			else if ( nStatus1 == NERR_InvalidComputer )
			{
				WriteToErrorLog( szTarget, "NetGroupEnum (GetGlobalGroupInfo)", "The computer name is invalid.", bMultipleHosts );
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

VOID GetLDAPInfo( CHAR szTarget[], BOOL *bMultipleHosts )
{
	CHAR                       *pTarget;
	LDAP               *pLDAPConnection;
	ULONG                       lReturn;
	LDAP_TIMEVAL                    ltv;
	CHAR                       *pBaseDN;
	CHAR                       szFilter[ 128 ];
	CHAR                       *pFilter;
	CHAR         szDefaultNamingContext[ 256 ];

	pTarget = szTarget;

	pLDAPConnection = ldap_init( pTarget, LDAP_PORT );
    
	if ( pLDAPConnection != NULL )
	{
		lReturn = ldap_set_option( pLDAPConnection, LDAP_OPT_PROTOCOL_VERSION, (VOID *)LDAP_VERSION3 );

		if ( lReturn == LDAP_SUCCESS )
		{
			ltv.tv_sec  = 2;
			ltv.tv_usec = 0;

			lReturn = ldap_connect( pLDAPConnection, &ltv );

			if ( lReturn == LDAP_SUCCESS )
			{
				lReturn = ldap_simple_bind( pLDAPConnection, NULL, NULL );

				if ( lReturn != -1 )
				{
					if ( !*bMultipleHosts )
					{
						printf( "\n" );
						printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
						printf( "+++++            LDAP INFORMATION             +++++\n" );
						printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
						printf( "\n" );

						fflush( stdout );
					}

					if ( bVerboseOptionSelected && *bMultipleHosts )
					{
						printf( "%s -> Logging LDAP information.\n", szTarget );

						fflush( stdout );
					}

					pBaseDN = NULL;

					strcpy( szFilter, "(objectClass=*)" );

					pFilter = szFilter;

					strcpy( szDefaultNamingContext, "" );

					LDAPQuery( szTarget, pLDAPConnection, pBaseDN, pFilter, szDefaultNamingContext, bMultipleHosts );

					if ( strcmp( szDefaultNamingContext, "" ) != 0 )
					{
						pBaseDN = szDefaultNamingContext;

						LDAPQuery( szTarget, pLDAPConnection, pBaseDN, pFilter, szDefaultNamingContext, bMultipleHosts );
					}
				}

				ldap_unbind( pLDAPConnection );
			}
			else
			{
				if ( lReturn == LDAP_INSUFFICIENT_RIGHTS )
				{
					WriteToErrorLog( szTarget, "ldap_connect (GetLDAPInfo)", "The user does not have access to the requested information.", bMultipleHosts );
				}
				else if ( lReturn == LDAP_SERVER_DOWN )
				{
					WriteToErrorLog( szTarget, "ldap_connect (GetLDAPInfo)", "Cannot contact the LDAP server.", bMultipleHosts );
				}
				else if ( lReturn == LDAP_TIMEOUT )
				{
					WriteToErrorLog( szTarget, "ldap_connect (GetLDAPInfo)", "The search was aborted due to exceeding the limit of the client side timeout.", bMultipleHosts );
				}
				else
				{
					WriteToErrorLog( szTarget, "ldap_connect (GetLDAPInfo)", "Unable to connect to the LDAP service.", bMultipleHosts );
				}
			}
		}
	}
}

VOID LDAPQuery( CHAR szTarget[], LDAP *pLDAPConnection, CHAR pBaseDN[], CHAR pFilter[], CHAR szDefaultNamingContext[], BOOL *bMultipleHosts )
{
	ULONG           lMessageID;
	LDAPMessage *pSearchResult;
	ULONG              lReturn;
	ULONG             lEntries;
	ULONG                    i;
	LDAPMessage        *pEntry;
	CHAR           *pAttribute;
	BerElement    *pBerElement;
	CHAR            **ppValues;
	ULONG              lValues;
	ULONG                    j;
	FILE          *pOutputFile;

	struct l_timeval tv;

	lMessageID = ldap_search( pLDAPConnection, pBaseDN, LDAP_SCOPE_BASE, pFilter, NULL, 0 );

	if ( lMessageID != -1 )
	{
		tv.tv_sec  = 2;
		tv.tv_usec = 0;

		lReturn = ldap_result( pLDAPConnection, lMessageID, LDAP_MSG_ALL, &tv, &pSearchResult );

		if ( lReturn > 0 )
		{
			lEntries = ldap_count_entries( pLDAPConnection, pSearchResult );

			if ( lEntries != -1 )
			{
				for ( i = 0; i < lEntries; i++ )
				{
					pEntry = NULL;

					if ( i == 0 )
					{
						pEntry = ldap_first_entry( pLDAPConnection, pSearchResult );
					}
					else
					{
						pEntry = ldap_next_entry( pLDAPConnection, pEntry );
					}

					if ( pEntry != NULL )
					{
						pAttribute = ldap_first_attribute( pLDAPConnection, pEntry, &pBerElement );

						while ( pAttribute != NULL )
						{
							ppValues = ldap_get_values( pLDAPConnection, pEntry, pAttribute );

							if ( ppValues != NULL )
							{
								lValues = 0;

								lValues = ldap_count_values( ppValues );

								if ( lValues > 0 )
								{
									if ( !*bMultipleHosts )
									{
										printf( "Base DN:   %s\n", pBaseDN );
										printf( "Attribute: %s\n", pAttribute );
										printf( "Value:     %s\n", *ppValues );

										fflush( stdout );
									}

									if ( bVerboseOptionSelected && *bMultipleHosts )
									{
										printf( "%s -> Logging LDAP information.\n", szTarget );

										fflush( stdout );
									}

									WaitForSingleObject( hSemaphore, INFINITE );

									pOutputFile = fopen( "LDAPInfo.txt", "r" );

									if ( pOutputFile != NULL )
									{
										fclose( pOutputFile );
									}
									else
									{
										pOutputFile = fopen( "LDAPInfo.txt", "w" );

										if ( pOutputFile != NULL )
										{
											fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
											fprintf( pOutputFile, "\n" );
											fprintf( pOutputFile, "Hostname\tBase DN\tLDAP Attribute\tValue\n" );

											fclose( pOutputFile );
										}
									}

									pOutputFile = fopen( "LDAPInfo.txt", "a+" );

									if ( pOutputFile != NULL )
									{
										fprintf( pOutputFile, "%s\t%s\t%s\t%s\n", szTarget, pBaseDN, pAttribute, *ppValues );

										fclose( pOutputFile );
									}

									ReleaseSemaphore( hSemaphore, 1, NULL );

									for ( j = 0; j < lValues; j++ )
									{
										if ( !*bMultipleHosts )
										{
											printf( "Value:     %s\n", ppValues[j] );

											fflush( stdout );
										}

										if ( bVerboseOptionSelected && *bMultipleHosts )
										{
											printf( "%s -> Logging LDAP information.\n", szTarget );

											fflush( stdout );
										}

										WaitForSingleObject( hSemaphore, INFINITE );

										pOutputFile = fopen( "LDAPInfo.txt", "a+" );

										if ( pOutputFile != NULL )
										{
											fprintf( pOutputFile, "%s\t%s\t%s\t%s\n", szTarget, pBaseDN, pAttribute, ppValues[j] );

											fclose( pOutputFile );
										}

										ReleaseSemaphore( hSemaphore, 1, NULL );
									}

									if ( !*bMultipleHosts )
									{
										printf( "\n" );

										fflush( stdout );
									}
								}

								if ( strcmp( pAttribute, "defaultNamingContext" ) == 0 )
								{
									strcpy( szDefaultNamingContext, *ppValues );
								}

								ldap_value_free( ppValues );
							}

							ldap_memfree( pAttribute );

							pAttribute = ldap_next_attribute( pLDAPConnection, pEntry, pBerElement );
						}

						if ( pAttribute != NULL )
						{
							ldap_memfree( pAttribute );
						}

						if ( pBerElement != NULL )
						{
							ber_free( pBerElement, 0 );
						}
					}
				}
			}

			ldap_msgfree( pSearchResult );
		}
	}
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

		nStatus = NetWkstaUserEnum( wszTarget, dwLevel, (PBYTE *)&pInfo, MAX_PREFERRED_LENGTH, &dwEntriesRead, &dwTotalEntries, &dwResumeHandle );

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

					if ( bVerboseOptionSelected && *bMultipleHosts )
					{
						printf( "%s -> Logging logged on users.\n", szTarget );

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
				WriteToErrorLog( szTarget, "NetWkstaUserEnum (GetLoggedOnUsers)", "The user does not have access to the requested information.", bMultipleHosts );
			}
			else if ( nStatus == ERROR_INVALID_LEVEL )
			{
				WriteToErrorLog( szTarget, "NetWkstaUserEnum (GetLoggedOnUsers)", "The level parameter is invalid.", bMultipleHosts );
			}
			else
			{
				dwError = nStatus;

				WriteLastErrorToErrorLog( szTarget, "NetWkstaUserEnum (GetLoggedOnUsers)", &dwError, bMultipleHosts );
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
	CHAR    szCacheFile[ 128 ];
	DWORD             i;
	DWORD             j;
	CHAR   szMachineSID[ 128 ];
	CHAR      szUserSID[ 128 ];
	CHAR   szDomainName[ 128 ];
	CHAR  szAccountName[ 128 ];
	DWORD             k;
	FILE   *pOutputFile;
	FILE    *pCacheFile;

	sprintf( szCacheFile, "UserCache\\%s.users", szTarget );

	i = 0;
	j = 0;

	if ( GetMachineSID( szTarget, szMachineSID, bMultipleHosts ) )
	{
		for ( k = 500; k < 502; k++ )
		{
			sprintf( szUserSID, "%s-%d", szMachineSID, k );

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
					printf( "User ID:  %d\n", k );
					printf( "\n" );

					fflush( stdout );
				}

				if ( bVerboseOptionSelected && *bMultipleHosts )
				{
					printf( "%s -> Logging user information.\n", szTarget );

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
					fprintf( pOutputFile, "%s\t%s\\%s\t%d\n", szTarget, szDomainName, szAccountName, k );

					fclose( pOutputFile );
				}

				if ( j == 0 )
				{
					pCacheFile = fopen( szCacheFile, "r" );

					if ( pCacheFile != NULL )
					{
						fclose( pCacheFile );
					}
					else
					{
						pCacheFile = fopen( szCacheFile, "w" );

						if ( pCacheFile != NULL )
						{
							fclose( pCacheFile );
						}

						j++;
					}
				}

				if ( j > 0 )
				{
					pCacheFile = fopen( szCacheFile, "a+" );

					if ( pCacheFile != NULL )
					{
						fprintf( pCacheFile, "%s\n", szAccountName );

						fclose( pCacheFile );
					}
				}

				ReleaseSemaphore( hSemaphore, 1, NULL );
			}
		}

		for ( k = 1000; k < 2000; k++ )
		{
			sprintf( szUserSID, "%s-%d", szMachineSID, k );

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
					printf( "User ID:  %d\n", k );
					printf( "\n" );

					fflush( stdout );
				}

				if ( bVerboseOptionSelected && *bMultipleHosts )
				{
					printf( "%s -> Logging user information.\n", szTarget );

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
					fprintf( pOutputFile, "%s\t%s\\%s\t%d\n", szTarget, szDomainName, szAccountName, k );

					fclose( pOutputFile );
				}

				if ( j == 0 )
				{
					pCacheFile = fopen( szCacheFile, "r" );

					if ( pCacheFile != NULL )
					{
						fclose( pCacheFile );
					}
					else
					{
						pCacheFile = fopen( szCacheFile, "w" );

						if ( pCacheFile != NULL )
						{
							fclose( pCacheFile );
						}

						j++;
					}
				}

				if ( j > 0 )
				{
					pCacheFile = fopen( szCacheFile, "a+" );

					if ( pCacheFile != NULL )
					{
						fprintf( pCacheFile, "%s\n", szAccountName );

						fclose( pCacheFile );
					}
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

				WriteLastErrorToErrorLog( szTarget, "ConvertSidToStringSid (GetMachineSID)", &dwError, bMultipleHosts );
			}

			LsaFreeMemory( pInfo );
		}
		else
		{
			dwError = LsaNtStatusToWinError( ntStatus );

			WriteLastErrorToErrorLog( szTarget, "LsaQueryInformationPolicy (GetMachineSID)", &dwError, bMultipleHosts );
		}

		LsaClose( lsahPolicyHandle );
	}
	else
	{
		dwError = LsaNtStatusToWinError( ntStatus );

		WriteLastErrorToErrorLog( szTarget, "LsaOpenPolicy (GetMachineSID)", &dwError, bMultipleHosts );
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

		WriteLastErrorToErrorLog( szTarget, "ConvertStringSidToSid (GetAccountNameFromSID)", &dwError, bMultipleHosts );
	}

	return bReturn;
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

			if ( bVerboseOptionSelected && *bMultipleHosts )
			{
				printf( "%s -> Logging server information.\n", szTarget );

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
			WriteToErrorLog( szTarget, "NetServerGetInfo (GetServerInfo)", "The user does not have access to the requested information.", bMultipleHosts );
		}
		else if ( nStatus == ERROR_INVALID_LEVEL )
		{
			WriteToErrorLog( szTarget, "NetServerGetInfo (GetServerInfo)", "The value specified for the level parameter is invalid.", bMultipleHosts );
		}
		else if ( nStatus == ERROR_INVALID_PARAMETER )
		{
			WriteToErrorLog( szTarget, "NetServerGetInfo (GetServerInfo)", "The specified parameter is invalid.", bMultipleHosts );
		}
		else if ( nStatus == ERROR_NOT_ENOUGH_MEMORY )
		{
			WriteToErrorLog( szTarget, "NetServerGetInfo (GetServerInfo)", "Insufficient memory is available.", bMultipleHosts );
		}
		else
		{
			dwError = nStatus;

			WriteLastErrorToErrorLog( szTarget, "NetServerGetInfo (GetServerInfo)", &dwError, bMultipleHosts );
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
														WriteToErrorLog( szTarget, "QueryServiceConfig2 (GetServiceInfo)", "The user does not have access to the requested information.", bMultipleHosts );
													}
													else if ( dwError == ERROR_INVALID_HANDLE )
													{
														WriteToErrorLog( szTarget, "QueryServiceConfig2 (GetServiceInfo)", "The specified handle is invalid.", bMultipleHosts );
													}
													else
													{
														WriteLastErrorToErrorLog( szTarget, "QueryServiceConfig2 (GetServiceInfo)", &dwError, bMultipleHosts );
													}
												}
											}
											else
											{
												WriteToErrorLog( szTarget, "malloc (GetServiceInfo)", "Unable to allocate memory.", bMultipleHosts );
											}
										}
										else
										{
											dwError = GetLastError();

											if ( dwError == ERROR_ACCESS_DENIED )
											{
												WriteToErrorLog( szTarget, "QueryServiceConfig2 (GetServiceInfo)", "The user does not have access to the requested information.", bMultipleHosts );
											}
											else if ( dwError == ERROR_INVALID_HANDLE )
											{
												WriteToErrorLog( szTarget, "QueryServiceConfig2 (GetServiceInfo)", "The specified handle is invalid.", bMultipleHosts );
											}
											else
											{
												WriteLastErrorToErrorLog( szTarget, "QueryServiceConfig2 (GetServiceInfo)", &dwError, bMultipleHosts );
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

										if ( bVerboseOptionSelected && *bMultipleHosts )
										{
											printf( "%s -> Logging service information.\n", szTarget );

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
											WriteToErrorLog( szTarget, "QueryServiceConfig (GetServiceInfo)", "The user does not have access to the requested information.", bMultipleHosts );
										}
										else if ( dwError == ERROR_INVALID_HANDLE )
										{
											WriteToErrorLog( szTarget, "QueryServiceConfig (GetServiceInfo)", "The specified handle is invalid.", bMultipleHosts );
										}
										else
										{
											WriteLastErrorToErrorLog( szTarget, "QueryServiceConfig (GetServiceInfo)", &dwError, bMultipleHosts );
										}
									}

									free( pServiceConfig );
								}
								else
								{
									WriteToErrorLog( szTarget, "malloc (GetServiceInfo)", "Unable to allocate memory.", bMultipleHosts );
								}
							}
							else
							{
								dwError = GetLastError();

								if ( dwError == ERROR_ACCESS_DENIED )
								{
									WriteToErrorLog( szTarget, "QueryServiceConfig (GetServiceInfo)", "The user does not have access to the requested information.", bMultipleHosts );
								}
								else if ( dwError == ERROR_INVALID_HANDLE )
								{
									WriteToErrorLog( szTarget, "QueryServiceConfig (GetServiceInfo)", "The specified handle is invalid.", bMultipleHosts );
								}
								else
								{
									WriteLastErrorToErrorLog( szTarget, "QueryServiceConfig (GetServiceInfo)", &dwError, bMultipleHosts );
								}
							}

							CloseServiceHandle( schService );
						}
						else
						{
							dwError = GetLastError();

							if ( dwError == ERROR_ACCESS_DENIED )
							{
								WriteToErrorLog( szTarget, "OpenService (GetServiceInfo)", "The user does not have access to the requested information.", bMultipleHosts );
							}
							else if ( dwError == ERROR_INVALID_HANDLE )
							{
								WriteToErrorLog( szTarget, "OpenService (GetServiceInfo)", "The specified handle is invalid.", bMultipleHosts );
							}
							else if ( dwError == ERROR_INVALID_NAME )
							{
								WriteToErrorLog( szTarget, "OpenService (GetServiceInfo)", "The specified service name is invalid.", bMultipleHosts );
							}
							else if ( dwError == ERROR_SERVICE_DOES_NOT_EXIST )
							{
								WriteToErrorLog( szTarget, "OpenService (GetServiceInfo)", "The specified service does not exist.", bMultipleHosts );
							}
							else
							{
								WriteLastErrorToErrorLog( szTarget, "OpenService (GetServiceInfo)", &dwError, bMultipleHosts );
							}
						}
					}
				}
				else
				{
					dwError = GetLastError();

					if ( dwError == ERROR_ACCESS_DENIED )
					{
						WriteToErrorLog( szTarget, "EnumServicesStatus (GetServiceInfo)", "The user does not have access to the requested information.", bMultipleHosts );
					}
					else if ( dwError == ERROR_INVALID_HANDLE )
					{
						WriteToErrorLog( szTarget, "EnumServicesStatus (GetServiceInfo)", "The specified handle is invalid.", bMultipleHosts );
					}
					else if ( dwError == ERROR_INVALID_PARAMETER )
					{
						WriteToErrorLog( szTarget, "EnumServicesStatus (GetServiceInfo)", "The specified parameter is invalid.", bMultipleHosts );
					}
					else
					{
						WriteLastErrorToErrorLog( szTarget, "EnumServicesStatus (GetServiceInfo)", &dwError, bMultipleHosts );
					}
				}

				free( pServiceStatus );
			}
			else
			{
				WriteToErrorLog( szTarget, "malloc (GetServiceInfo)", "Unable to allocate memory.", bMultipleHosts );
			}
		}
		else
		{
			dwError = GetLastError();

			if ( dwError == ERROR_ACCESS_DENIED )
			{
				WriteToErrorLog( szTarget, "EnumServicesStatus (GetServiceInfo)", "The user does not have access to the requested information.", bMultipleHosts );
			}
			else if ( dwError == ERROR_INVALID_HANDLE )
			{
				WriteToErrorLog( szTarget, "EnumServicesStatus (GetServiceInfo)", "The specified handle is invalid.", bMultipleHosts );
			}
			else if ( dwError == ERROR_INVALID_PARAMETER )
			{
				WriteToErrorLog( szTarget, "EnumServicesStatus (GetServiceInfo)", "The specified parameter is invalid.", bMultipleHosts );
			}
			else
			{
				WriteLastErrorToErrorLog( szTarget, "EnumServicesStatus (GetServiceInfo)", &dwError, bMultipleHosts );
			}
		}

		CloseServiceHandle( schSCManager );
	}
	else
	{
		dwError = GetLastError();

		if ( dwError == ERROR_ACCESS_DENIED )
		{
			WriteToErrorLog( szTarget, "OpenSCManager (GetServiceInfo)", "The user does not have access to the requested information.", bMultipleHosts );
		}
		else if ( dwError == ERROR_DATABASE_DOES_NOT_EXIST )
		{
			WriteToErrorLog( szTarget, "OpenSCManager (GetServiceInfo)", "The specified database does not exist.", bMultipleHosts );
		}
		else if ( dwError == ERROR_INVALID_PARAMETER )
		{
			WriteToErrorLog( szTarget, "OpenSCManager (GetServiceInfo)", "The specified parameter is invalid.", bMultipleHosts );
		}
		else
		{
			WriteLastErrorToErrorLog( szTarget, "OpenSCManager (GetServiceInfo)", &dwError, bMultipleHosts );
		}
	}
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

		nStatus = NetShareEnum( wszTarget, dwLevel, (PBYTE *)&pInfo, MAX_PREFERRED_LENGTH, &dwEntriesRead, &dwTotalEntries, &dwResumeHandle );

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

					if ( bVerboseOptionSelected && *bMultipleHosts )
					{
						printf( "%s -> Logging share information.\n", szTarget );

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

			WriteLastErrorToErrorLog( szTarget, "NetShareEnum (GetShareInfo)", &dwError, bMultipleHosts );
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
	CHAR               szCacheFile[ 128 ];
	DWORD                        i;
	DWORD                        j;
	DWORD                 dwLevel3;
	DWORD            dwEntriesRead;
	DWORD           dwTotalEntries;
	DWORD           dwResumeHandle;
	USER_INFO_3             *pInfo;
	NET_API_STATUS         nStatus;
	USER_INFO_3         *pTempInfo;
	DWORD                        k;
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
	FILE               *pCacheFile;

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

	sprintf( szCacheFile, "UserCache\\%s.users", szTarget );

	i = 0;
	j = 0;

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

				for ( k = 0; k < dwEntriesRead; k++ )
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
							WriteToErrorLog( szTarget, "MprAdminUserGetInfo (GetUserInfo)", "The caller does not have sufficient privileges.", bMultipleHosts );
						}
						else if ( dwStatus == ERROR_INVALID_LEVEL )
						{
							WriteToErrorLog( szTarget, "MprAdminUserGetInfo (GetUserInfo)", "The level parameter is invalid.", bMultipleHosts );
						}
						else if ( dwStatus == ERROR_INVALID_PARAMETER )
						{
							WriteToErrorLog( szTarget, "MprAdminUserGetInfo (GetUserInfo)", "One or more parameters are incorrect.", bMultipleHosts );
						}
						else if ( dwStatus == ERROR_NO_SUCH_USER )
						{
							WriteToErrorLog( szTarget, "MprAdminUserGetInfo (GetUserInfo)", "The specified user does not exist.", bMultipleHosts );
						}
						else
						{
							dwError = nStatus;

							WriteLastErrorToErrorLog( szTarget, "MprAdminUserGetInfo (GetUserInfo)", &dwError, bMultipleHosts );
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

					if ( bVerboseOptionSelected && *bMultipleHosts )
					{
						printf( "%s -> Logging user information.\n", szTarget );

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

					if ( j == 0 )
					{
						pCacheFile = fopen( szCacheFile, "w" );

						if ( pCacheFile != NULL )
						{
							fclose( pCacheFile );
						}

						j++;
					}

					pCacheFile = fopen( szCacheFile, "a+" );

					if ( pCacheFile != NULL )
					{
						fprintf( pCacheFile, "%s\n", szUsername);

						fclose( pCacheFile );
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
				WriteToErrorLog( szTarget, "NetUserEnum (GetUserInfo)", "The user does not have access to the requested information.", bMultipleHosts );
			}
			else if ( nStatus == NERR_InvalidComputer )
			{
				WriteToErrorLog( szTarget, "NetUserEnum (GetUserInfo)", "The computer name is invalid.", bMultipleHosts );
			}
			else
			{
				dwError = nStatus;

				WriteLastErrorToErrorLog( szTarget, "NetUserEnum (GetUserInfo)", &dwError, bMultipleHosts );
			}
		}

		if ( pInfo != NULL )
		{
			NetApiBufferFree( pInfo );
		}
	}
	while ( nStatus == ERROR_MORE_DATA );
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

							if ( bVerboseOptionSelected && *bMultipleHosts )
							{
								printf( "%s -> Logging user rights information.\n", szTarget );

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

							WriteLastErrorToErrorLog( szTarget, "LookupAccountSid (GetUserRightsInfo)", &dwError, bMultipleHosts );
						}
					}

					LsaFreeMemory( pInfo );
				}
			}
			else
			{
				dwError = LsaNtStatusToWinError( ntStatus );

				WriteLastErrorToErrorLog( szTarget, "LsaEnumerateAccountsWithUserRight (GetUserRightsInfo)", &dwError, bMultipleHosts );
			}
		}

		LsaClose( lsahPolicyHandle );
	}
	else
	{
		dwError = LsaNtStatusToWinError( ntStatus );

		WriteLastErrorToErrorLog( szTarget, "LsaOpenPolicy (GetUserRightsInfo)", &dwError, bMultipleHosts );
	}
}

VOID GetWinVNC3Info( CHAR szTarget[], BOOL *bMultipleHosts )
{
	CHAR             *pLocation;
	CHAR           szTempTarget[ 128 ];
	LONG                lReturn;
	HKEY                   hKey;
	HKEY                hSubKey;
	CHAR          szEncPassword[ 128 ];
	DWORD          dwBufferSize;
	unsigned char    szPassword[ 128 ];
	FILE           *pOutputFile;
	DWORD               dwError;

	unsigned char ObfKey[8] = { 23, 82, 107, 6, 35, 78, 88, 7 };

	pLocation = strstr( szTarget, "\\\\" );

	if ( pLocation != NULL )
	{
		strcpy( szTempTarget, szTarget );
	}
	else
	{
		sprintf( szTempTarget, "\\\\%s", szTarget );
	}

	lReturn = RegConnectRegistry( szTempTarget, HKEY_LOCAL_MACHINE, &hKey );

	if ( lReturn == ERROR_SUCCESS )
	{
		lReturn = RegOpenKeyEx( hKey, "SOFTWARE\\ORL\\WinVNC3\\Default", 0, KEY_QUERY_VALUE, &hSubKey );

		if ( lReturn == ERROR_SUCCESS )
		{
			strcpy( szEncPassword, "" );

			dwBufferSize = 128;

			lReturn = RegQueryValueEx( hSubKey, "Password", NULL, NULL, (BYTE *)szEncPassword, &dwBufferSize );

			if ( lReturn == ERROR_SUCCESS )
			{
				deskey( ObfKey, DE1 );

				des( szEncPassword, szPassword );

				szPassword[8] = '\0';

				if ( !*bMultipleHosts )
				{
					printf( "\n" );
					printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
					printf( "+++++           WinVNC3 INFORMATION           +++++\n" );
					printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
					printf( "\n" );

					printf( "WinVNC Password Hash: %02X %02X %02X %02X %02X %02X %02X %02X\n", szEncPassword[0] & 0xff, szEncPassword[1] & 0xff, szEncPassword[2] & 0xff, szEncPassword[3] & 0xff, szEncPassword[4] & 0xff, szEncPassword[5] & 0xff, szEncPassword[6] & 0xff, szEncPassword[7] & 0xff );
					printf( "WinVNC Password:      %s\n", szPassword );
					printf( "\n" );

					fflush( stdout );
				}

				if ( bVerboseOptionSelected && *bMultipleHosts )
				{
					printf( "%s -> Logging WinVNC3 information.\n", szTarget );

					fflush( stdout );
				}

				WaitForSingleObject( hSemaphore, INFINITE );

				pOutputFile = fopen( "WinVNCInfo.txt", "r" );

				if ( pOutputFile != NULL )
				{
					fclose( pOutputFile );
				}
				else
				{
					pOutputFile = fopen( "WinVNCInfo.txt", "w" );

					if ( pOutputFile != NULL )
					{
						fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
						fprintf( pOutputFile, "\n" );
						fprintf( pOutputFile, "Hostname\tWinVNC Password Hash\tWinVNC Password\n" );

						fclose( pOutputFile );
					}
				}

				pOutputFile = fopen( "WinVNCInfo.txt", "a+" );

				if ( pOutputFile != NULL )
				{
					fprintf( pOutputFile, "%s\t%02X %02X %02X %02X %02X %02X %02X %02X\t%s\n", szTarget, szEncPassword[0] & 0xff, szEncPassword[1] & 0xff, szEncPassword[2] & 0xff, szEncPassword[3] & 0xff, szEncPassword[4] & 0xff, szEncPassword[5] & 0xff, szEncPassword[6] & 0xff, szEncPassword[7] & 0xff, szPassword );

					fclose( pOutputFile );
				}

				ReleaseSemaphore( hSemaphore, 1, NULL );
			}

			RegCloseKey( hSubKey );
		}
		else
		{
			dwError = lReturn;

			WriteLastErrorToErrorLog( szTarget, "RegOpenKeyEx (GetWinVNC3Info)", &dwError, bMultipleHosts );
		}

		RegCloseKey( hKey );
	}
	else
	{
		dwError = lReturn;

		WriteLastErrorToErrorLog( szTarget, "RegConnectRegistry (GetWinVNC3Info)", &dwError, bMultipleHosts );
	}
}

VOID GetWinVNC4Info( CHAR szTarget[], BOOL *bMultipleHosts )
{
	CHAR             *pLocation;
	CHAR           szTempTarget[ 128 ];
	LONG                lReturn;
	HKEY                   hKey;
	HKEY                hSubKey;
	CHAR          szEncPassword[ 128 ];
	DWORD          dwBufferSize;
	unsigned char    szPassword[ 128 ];
	FILE           *pOutputFile;
	DWORD               dwError;

	unsigned char ObfKey[8] = { 23, 82, 107, 6, 35, 78, 88, 7 };

	pLocation = strstr( szTarget, "\\\\" );

	if ( pLocation != NULL )
	{
		strcpy( szTempTarget, szTarget );
	}
	else
	{
		sprintf( szTempTarget, "\\\\%s", szTarget );
	}

	lReturn = RegConnectRegistry( szTempTarget, HKEY_LOCAL_MACHINE, &hKey );

	if ( lReturn == ERROR_SUCCESS )
	{
		lReturn = RegOpenKeyEx( hKey, "SOFTWARE\\RealVNC\\WinVNC4", 0, KEY_QUERY_VALUE, &hSubKey );

		if ( lReturn == ERROR_SUCCESS )
		{
			strcpy( szEncPassword, "" );

			dwBufferSize = 128;

			lReturn = RegQueryValueEx( hSubKey, "Password", NULL, NULL, (BYTE *)szEncPassword, &dwBufferSize );

			if ( lReturn == ERROR_SUCCESS )
			{
				deskey( ObfKey, DE1 );

				des( szEncPassword, szPassword );

				szPassword[8] = '\0';

				if ( !*bMultipleHosts )
				{
					printf( "\n" );
					printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
					printf( "+++++           WinVNC4 INFORMATION           +++++\n" );
					printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
					printf( "\n" );

					printf( "WinVNC Password Hash: %02X %02X %02X %02X %02X %02X %02X %02X\n", szEncPassword[0] & 0xff, szEncPassword[1] & 0xff, szEncPassword[2] & 0xff, szEncPassword[3] & 0xff, szEncPassword[4] & 0xff, szEncPassword[5] & 0xff, szEncPassword[6] & 0xff, szEncPassword[7] & 0xff );
					printf( "WinVNC Password:      %s\n", szPassword );
					printf( "\n" );

					fflush( stdout );
				}

				if ( bVerboseOptionSelected && *bMultipleHosts )
				{
					printf( "%s -> Logging WinVNC4 information.\n", szTarget );

					fflush( stdout );
				}

				WaitForSingleObject( hSemaphore, INFINITE );

				pOutputFile = fopen( "WinVNCInfo.txt", "r" );

				if ( pOutputFile != NULL )
				{
					fclose( pOutputFile );
				}
				else
				{
					pOutputFile = fopen( "WinVNCInfo.txt", "w" );

					if ( pOutputFile != NULL )
					{
						fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
						fprintf( pOutputFile, "\n" );
						fprintf( pOutputFile, "Hostname\tWinVNC Password Hash\tWinVNC Password\n" );

						fclose( pOutputFile );
					}
				}

				pOutputFile = fopen( "WinVNCInfo.txt", "a+" );

				if ( pOutputFile != NULL )
				{
					fprintf( pOutputFile, "%s\t%02X %02X %02X %02X %02X %02X %02X %02X\t%s\n", szTarget, szEncPassword[0] & 0xff, szEncPassword[1] & 0xff, szEncPassword[2] & 0xff, szEncPassword[3] & 0xff, szEncPassword[4] & 0xff, szEncPassword[5] & 0xff, szEncPassword[6] & 0xff, szEncPassword[7] & 0xff, szPassword );

					fclose( pOutputFile );
				}

				ReleaseSemaphore( hSemaphore, 1, NULL );
			}

			RegCloseKey( hSubKey );
		}
		else
		{
			dwError = lReturn;

			WriteLastErrorToErrorLog( szTarget, "RegOpenKeyEx (GetWinVNC4Info)", &dwError, bMultipleHosts );
		}

		RegCloseKey( hKey );
	}
	else
	{
		dwError = lReturn;

		WriteLastErrorToErrorLog( szTarget, "RegConnectRegistry (GetWinVNC4Info)", &dwError, bMultipleHosts );
	}
}

VOID SaveRegistryHives( CHAR szTarget[], BOOL *bMultipleHosts )
{
	CHAR      *pLocation;
	CHAR    szTempTarget[ 128 ];
	LONG         lReturn;
	HKEY            hKey;
	DWORD              i;
	DWORD              j;
	CHAR       szKeyName[ 128 ];
	CHAR  szSaveFileName[ 128 ];
	HKEY      hKeyToSave;
	DWORD  dwDisposition;
	CHAR  szSaveFilePath[ 256 ];
	DWORD        dwError;

	pLocation = strstr( szTarget, "\\\\" );

	if ( pLocation != NULL )
	{
		strcpy( szTempTarget, szTarget );
	}
	else
	{
		sprintf( szTempTarget, "\\\\%s", szTarget );
	}

	lReturn = RegConnectRegistry( szTempTarget, HKEY_LOCAL_MACHINE, &hKey );

	if ( lReturn == ERROR_SUCCESS )
	{
		i = 0;

		for ( j = 0; j < 4; j++ )
		{
			if ( j == 0 )
			{
				strcpy( szKeyName, "SAM" );

				sprintf( szSaveFileName, "%s-%s", szTarget, szKeyName );
			}

			if ( j == 1 )
			{
				strcpy( szKeyName, "SECURITY" );

				sprintf( szSaveFileName, "%s-%s", szTarget, szKeyName );
			}

			if ( j == 2 )
			{
				strcpy( szKeyName, "SYSTEM" );

				sprintf( szSaveFileName, "%s-%s", szTarget, szKeyName );
			}

			if ( j == 3 )
			{
				strcpy( szKeyName, "SOFTWARE" );

				sprintf( szSaveFileName, "%s-%s", szTarget, szKeyName );
			}

			lReturn = RegCreateKeyEx( hKey, szKeyName, 0, NULL, REG_OPTION_BACKUP_RESTORE, KEY_QUERY_VALUE, NULL, &hKeyToSave, &dwDisposition );

			if ( lReturn == ERROR_SUCCESS )
			{
				lReturn = RegSaveKey( hKeyToSave, szSaveFileName, NULL );

				if ( lReturn == ERROR_SUCCESS )
				{
					if ( !*bMultipleHosts )
					{
						if ( i == 0 )
						{
							printf( "\n" );
							printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
							printf( "+++++           SAVE REGISTRY HIVES           +++++\n" );
							printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
							printf( "\n" );

							i++;
						}

						printf( "Saved Registry Hive: %s\n", szSaveFileName );

						fflush( stdout );
					}

					if ( bVerboseOptionSelected && *bMultipleHosts )
					{
						printf( "%s -> Saving registry hives.\n", szTarget );

						fflush( stdout );
					}

					DeleteFile( szSaveFileName );

					sprintf( szSaveFilePath, "\\\\%s\\ADMIN$\\system32\\%s", szTarget, szSaveFileName );

					MoveFile( szSaveFilePath, szSaveFileName );
				}

				RegCloseKey( hKeyToSave );
			}
			else
			{
				dwError = lReturn;

				WriteLastErrorToErrorLog( szTarget, "RegCreateKeyEx (SaveRegistryHives)", &dwError, bMultipleHosts );
			}
		}

		if ( i > 0 )
		{
			if ( !*bMultipleHosts )
			{
				printf( "\n" );

				fflush( stdout );
			}
		}

		RegCloseKey( hKey );
	}
	else
	{
		dwError = lReturn;

		WriteLastErrorToErrorLog( szTarget, "RegConnectRegistry (SaveRegistryHives)", &dwError, bMultipleHosts );
	}
}

VOID GetPatchInfo( CHAR szTarget[], BOOL *bMultipleHosts )
{
	CHAR        *pLocation;
	CHAR      szTempTarget[ 128 ];
	LONG           lReturn;
	HKEY              hKey;
	HKEY           hSubKey;
	CHAR  szCurrentVersion[ 128 ];
	CHAR     szServicePack[ 128 ];
	DWORD     dwBufferSize;
	CHAR   szOSServicePack[ 128 ];
	FILE   *pPatchInfoFile;
	DWORD                i;
	CHAR            szLine[ 512 ];
	CHAR       szTempText1[ 128 ];
	CHAR       szTempText2[ 128 ];
	CHAR      szMSAdvisory[ 128 ];
	CHAR    szTempFilePath[ 128 ];
	DWORD     dwPatchedHMS;
	DWORD     dwPatchedLMS;
	DWORD     dwPatchedHLS;
	DWORD     dwPatchedLLS;
	CHAR        szFilePath[ 128 ];
	DWORD            dwHMS;
	DWORD            dwLMS;
	DWORD            dwHLS;
	DWORD            dwLLS;
	CHAR       szIsPatched[ 64 ];
	FILE      *pOutputFile;
	DWORD          dwError;

	pLocation = strstr( szTarget, "\\\\" );

	if ( pLocation != NULL )
	{
		strcpy( szTempTarget, szTarget );
	}
	else
	{
		sprintf( szTempTarget, "\\\\%s", szTarget );
	}

	lReturn = RegConnectRegistry( szTempTarget, HKEY_LOCAL_MACHINE, &hKey );

	if ( lReturn == ERROR_SUCCESS )
	{
		lReturn = RegOpenKeyEx( hKey, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_QUERY_VALUE, &hSubKey );

		if ( lReturn == ERROR_SUCCESS )
		{
			strcpy( szCurrentVersion, "" );
			strcpy( szServicePack,    "" );

			dwBufferSize = 128;

			lReturn = RegQueryValueEx( hSubKey, "CurrentVersion", NULL, NULL, (BYTE *)szCurrentVersion, &dwBufferSize );

			dwBufferSize = 128;

			lReturn = RegQueryValueEx( hSubKey, "CSDVersion", NULL, NULL, (BYTE *)szServicePack, &dwBufferSize );

			sprintf( szOSServicePack, "%s:%s:", szCurrentVersion, szServicePack );

			pPatchInfoFile = fopen( "PatchInfo.input", "r" );

			if ( pPatchInfoFile != NULL )
			{
				i = 0;

				while ( fgets( szLine, sizeof( szLine ), pPatchInfoFile ) != NULL )
				{
					Trim( szLine );

					if ( szLine[0] != '#' && szLine[0] != '\n' )
					{
						if ( szLine[strlen( szLine ) - 1] == '\n' )
						{
							szLine[strlen( szLine ) - 1] = '\0';
						}

						pLocation = strstr( szLine, szOSServicePack );

						if ( pLocation != NULL )
						{
							Split( szLine, ":", szTempText1, szTempText2, szMSAdvisory, szTempFilePath, &dwPatchedHMS, &dwPatchedLMS, &dwPatchedHLS, &dwPatchedLLS );

							sprintf( szFilePath, "%s%s", szTempTarget, szTempFilePath );

							if ( GetFileVersion( szTarget, szFilePath, &dwHMS, &dwLMS, &dwHLS, &dwLLS, bMultipleHosts ) )
							{
								strcpy( szIsPatched, "No" );

								if ( dwHMS == dwPatchedHMS )
								{
									if ( dwLMS == dwPatchedLMS )
									{
										if ( dwHLS == dwPatchedHLS )
										{
											if ( dwLLS == dwPatchedLLS )
											{
												strcpy( szIsPatched, "Yes" );
											}
											else
											{
												if ( dwLLS > dwPatchedLLS )
												{
													strcpy( szIsPatched, "Yes" );
												}
											}
										}
										else
										{
											if ( dwHLS > dwPatchedHLS )
											{
												strcpy( szIsPatched, "Yes" );
											}
										}
									}
									else
									{
										if ( dwLMS > dwPatchedLMS )
										{
											strcpy( szIsPatched, "Yes" );
										}
									}
								}
								else
								{
									if ( dwHMS > dwPatchedHMS )
									{
										strcpy( szIsPatched, "Yes" );
									}
								}

								if ( !*bMultipleHosts )
								{
									if ( i == 0 )
									{
										printf( "\n" );
										printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
										printf( "+++++            PATCH INFORMATION            +++++\n" );
										printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
										printf( "\n" );

										i++;
									}

									printf( "MS Advisory:          %s\n", szMSAdvisory );
									printf( "OS Version:           %s\n", szCurrentVersion );
									printf( "Service Pack:         %s\n", szServicePack );
									printf( "File Path:            %s\n", szFilePath );
									printf( "File Version:         %d.%d.%d.%d\n", dwHMS, dwLMS, dwHLS, dwLLS );
									printf( "Patched File Version: %d.%d.%d.%d\n", dwPatchedHMS, dwPatchedLMS, dwPatchedHLS, dwPatchedLLS );
									printf( "Is Patched?:          %s\n", szIsPatched );
									printf( "\n" );

									fflush( stdout );
								}

								if ( bVerboseOptionSelected && *bMultipleHosts )
								{
									printf( "%s -> Logging patch information.\n", szTarget );

									fflush( stdout );
								}

								WaitForSingleObject( hSemaphore, INFINITE );

								pOutputFile = fopen( "PatchInfo.txt", "r" );

								if ( pOutputFile != NULL )
								{
									fclose( pOutputFile );
								}
								else
								{
									pOutputFile = fopen( "PatchInfo.txt", "w" );

									if ( pOutputFile != NULL )
									{
										fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
										fprintf( pOutputFile, "\n" );
										fprintf( pOutputFile, "Hostname\tMS Advisory\tOS Version\tService Pack\tFile Path\tFile Version\tPatched File Version\tIs Patched?\n" );

										fclose( pOutputFile );
									}
								}

								pOutputFile = fopen( "PatchInfo.txt", "a+" );

								if ( pOutputFile != NULL )
								{
									fprintf( pOutputFile, "%s\t%s\t%s\t%s\t%s\t%d.%d.%d.%d\t%d.%d.%d.%d\t%s\n", szTarget, szMSAdvisory, szCurrentVersion, szServicePack, szFilePath, dwHMS, dwLMS, dwHLS, dwLLS, dwPatchedHMS, dwPatchedLMS, dwPatchedHLS, dwPatchedLLS, szIsPatched );

									fclose( pOutputFile );
								}

								ReleaseSemaphore( hSemaphore, 1, NULL );
							}
						}
					}
				}

				fclose( pPatchInfoFile );
			}
			else
			{
				WriteToErrorLog( szTarget, "fopen (GetPatchInfo)", "Cannot open file PatchInfo.input.", bMultipleHosts );
			}

			RegCloseKey( hSubKey );
		}
		else
		{
			dwError = lReturn;

			WriteLastErrorToErrorLog( szTarget, "RegOpenKeyEx (GetPatchInfo)", &dwError, bMultipleHosts );
		}

		RegCloseKey( hKey );
	}
	else
	{
		dwError = lReturn;

		WriteLastErrorToErrorLog( szTarget, "RegConnectRegistry (GetPatchInfo)", &dwError, bMultipleHosts );
	}
}

VOID Trim( CHAR szText[] )
{
	DWORD           i;
	DWORD dwStartChar;
	DWORD   dwEndChar;
	CHAR   szTempText[ 10240 ];
	DWORD           j;

	i = 0;

	while ( i < strlen( szText ) )
	{
		if ( szText[i] == ' ' )
		{
			i++;
		}
		else
		{
			break;
		}
	}

	dwStartChar = i;

	i = strlen( szText ) - 1;

	while ( i > 0 )
	{
		if ( szText[i] == ' ' )
		{
			i--;
		}
		else
		{
			break;
		}
	}

	dwEndChar = i;

	i = dwStartChar;
	j = 0;

	while ( i <= dwEndChar )
	{
		szTempText[j] = szText[i];

		i++;
		j++;
	}

	szTempText[j] = '\0';

	strcpy( szText, szTempText );
}

VOID Split( CHAR szText[], CHAR szSplitText[], CHAR szOSVersion[], CHAR szServicePack[], CHAR szMSAdvisory[], CHAR szFilePath[], DWORD *dwHMS, DWORD *dwLMS, DWORD *dwHLS, DWORD *dwLLS )
{
	CHAR      *pLocation;
	DWORD dwTextLocation;
	DWORD              i;
	DWORD              j;
	CHAR     szStartText[ 128 ];
	DWORD              k;
	CHAR       szEndText[ 128 ];
	CHAR           szHMS[ 64 ];
	CHAR           szLMS[ 64 ];
	CHAR           szHLS[ 64 ];
	CHAR           szLLS[ 64 ];

	pLocation = strstr( szText, szSplitText );

	dwTextLocation = (INT)( pLocation - szText );

	i = 0;

	while ( pLocation != NULL )
	{
		j = 0;

		while ( j < dwTextLocation )
		{
			szStartText[j] = szText[j];

			j++;
		}

		szStartText[j] = '\0';

		j = dwTextLocation + strlen( szSplitText );

		k = 0;

		while ( j < strlen( szText ) )
		{
			szEndText[k] = szText[j];

			j++;
			k++;
		}

		szEndText[k] = '\0';

		strcpy( szText, szEndText );

		if ( i == 0 )
		{
			strcpy( szOSVersion, szStartText );
		}

		if ( i == 1 )
		{
			strcpy( szServicePack, szStartText );
		}

		if ( i == 2 )
		{
			strcpy( szMSAdvisory, szStartText );
		}

		if ( i == 3 )
		{
			strcpy( szFilePath, szStartText );
		}

		if ( i == 4 )
		{
			strcpy( szHMS, szStartText );

			*dwHMS = atoi( szHMS );
		}

		if ( i == 5 )
		{
			strcpy( szLMS, szStartText );

			*dwLMS = atoi( szLMS );
		}

		if ( i == 6 )
		{
			strcpy( szHLS, szStartText );
			strcpy( szLLS, szEndText );

			*dwHLS = atoi( szHLS );
			*dwLLS = atoi( szLLS );
		}

		i++;

		pLocation = strstr( szText, szSplitText );

		dwTextLocation = (INT)( pLocation - szText );
	}
}

BOOL GetFileVersion( CHAR szTarget[], CHAR szFilePath[], DWORD *dwHMS, DWORD *dwLMS, DWORD *dwHLS, DWORD *dwLLS, BOOL *bMultipleHosts )
{
	BOOL                     bResult;
	DWORD              dwVersionSize;
	DWORD                   dwHandle;
	CHAR                   *pVersion;
	VS_FIXEDFILEINFO          *pInfo;
	UINT                        uLen;
	DWORD            dwFileVersionMS;
	DWORD            dwFileVersionLS;
	DWORD                    dwError;

	bResult = FALSE;

	dwVersionSize = GetFileVersionInfoSize( szFilePath, &dwHandle );

	if ( dwVersionSize != 0 )
	{
		pVersion = NULL;

		pVersion = (CHAR *)malloc( dwVersionSize * sizeof( CHAR ) );

		if ( pVersion != NULL )
		{
			if ( GetFileVersionInfo( szFilePath, dwHandle, dwVersionSize, pVersion ) )
			{
				pInfo = NULL;

				if ( VerQueryValue( pVersion, "\\", (PVOID *)&pInfo, &uLen ) )
				{
					if ( pInfo != NULL )
					{
						dwFileVersionMS = pInfo->dwFileVersionMS;
						dwFileVersionLS = pInfo->dwFileVersionLS;

						*dwHMS = HIWORD( dwFileVersionMS );
						*dwLMS = LOWORD( dwFileVersionMS );
						*dwHLS = HIWORD( dwFileVersionLS );
						*dwLLS = LOWORD( dwFileVersionLS );

						bResult = TRUE;
					}
				}
			}
			else
			{
				dwError = GetLastError();

				WriteLastErrorToErrorLog( szTarget, "GetFileVersionInfo (GetFileVersion)", &dwError, bMultipleHosts );
			}

			free( pVersion );
		}
		else
		{
			WriteToErrorLog( szTarget, "malloc (GetFileVersion)", "Unable to allocate memory.", bMultipleHosts );
		}
	}
	else
	{
		dwError = GetLastError();

		WriteLastErrorToErrorLog( szTarget, "GetFileVersionInfoSize (GetFileVersion)", &dwError, bMultipleHosts );
	}

	return bResult;
}

VOID GetScheduledTaskInfo( CHAR szTarget[], BOOL *bMultipleHosts )
{
	CHAR                   szDirectory[ 512 ];
	HANDLE                       hFind;
	WIN32_FIND_DATA       FindFileData;
	CHAR                    szTaskFile[ 128 ];
	DWORD                            i;
	CHAR                    *pLocation;
	CHAR                    szFilePath[ 512 ];
	CHAR                   szArguments[ 1024 ];
	CHAR            szWorkingDirectory[ 512 ];
	CHAR                   szCreatedBy[ 128 ];
	CHAR                 szDescription[ 1024 ];
	FILE                  *pOutputFile;
	DWORD                      dwError;

	sprintf( szDirectory, "\\\\%s\\ADMIN$\\Tasks\\*", szTarget );

	hFind = FindFirstFile( szDirectory, &FindFileData );

	if ( hFind != INVALID_HANDLE_VALUE ) 
	{
		strcpy( szTaskFile, FindFileData.cFileName );

		_strupr( szTaskFile );

		i = 0;

		pLocation = strstr( szTaskFile, ".JOB" );

		if ( pLocation != NULL )
		{
			strcpy( szTaskFile, FindFileData.cFileName );

			strcpy( szFilePath,         "" );
			strcpy( szArguments,        "" );
			strcpy( szWorkingDirectory, "" );
			strcpy( szCreatedBy,        "" );
			strcpy( szDescription,      "" );

			ParseScheduledTaskFile( szTarget, szTaskFile, szFilePath, szArguments, szWorkingDirectory, szCreatedBy, szDescription );

			if ( !*bMultipleHosts )
			{
				if ( i == 0 )
				{
					printf( "\n" );
					printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
					printf( "+++++       SCHEDULED TASK INFORMATION        +++++\n" );
					printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
					printf( "\n" );

					i++;
				}

				printf( "Task Name:         %s\n", szTaskFile );
				printf( "Target File:       %s\n", szFilePath );
				printf( "Arguments:         %s\n", szArguments );
				printf( "Working Directory: %s\n", szWorkingDirectory );
				printf( "Created By:        %s\n", szCreatedBy );
				printf( "Description:       %s\n", szDescription );
				printf( "\n" );

				fflush( stdout );
			}

			if ( bVerboseOptionSelected && *bMultipleHosts )
			{
				printf( "%s -> Logging scheduled task information.\n", szTarget );

				fflush( stdout );
			}

			WaitForSingleObject( hSemaphore, INFINITE );

			pOutputFile = fopen( "ScheduledTaskInfo.txt", "r" );

			if ( pOutputFile != NULL )
			{
				fclose( pOutputFile );
			}
			else
			{
				pOutputFile = fopen( "ScheduledTaskInfo.txt", "w" );

				if ( pOutputFile != NULL )
				{
					fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
					fprintf( pOutputFile, "\n" );
					fprintf( pOutputFile, "Hostname\tTask Name\tTarget File\tArguments\tWorking Directory\tCreated By\tDescription\n" );

					fclose( pOutputFile );
				}
			}

			pOutputFile = fopen( "ScheduledTaskInfo.txt", "a+" );

			if ( pOutputFile != NULL )
			{
				fprintf( pOutputFile, "%s\t%s\t%s\t%s\t%s\t%s\t%s\n", szTarget, szTaskFile, szFilePath, szArguments, szWorkingDirectory, szCreatedBy, szDescription );

				fclose( pOutputFile );
			}

			ReleaseSemaphore( hSemaphore, 1, NULL );
		}

		while ( FindNextFile( hFind, &FindFileData ) != 0 )
		{
			strcpy( szTaskFile, FindFileData.cFileName );

			_strupr( szTaskFile );

			pLocation = strstr( szTaskFile, ".JOB" );

			if ( pLocation != NULL )
			{
				strcpy( szTaskFile, FindFileData.cFileName );

				strcpy( szFilePath,         "" );
				strcpy( szArguments,        "" );
				strcpy( szWorkingDirectory, "" );
				strcpy( szCreatedBy,        "" );
				strcpy( szDescription,      "" );

				ParseScheduledTaskFile( szTarget, szTaskFile, szFilePath, szArguments, szWorkingDirectory, szCreatedBy, szDescription );

				if ( !*bMultipleHosts )
				{
					if ( i == 0 )
					{
						printf( "\n" );
						printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
						printf( "+++++       SCHEDULED TASK INFORMATION        +++++\n" );
						printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
						printf( "\n" );

						i++;
					}

					printf( "Task Name:         %s\n", szTaskFile );
					printf( "Target File:       %s\n", szFilePath );
					printf( "Arguments:         %s\n", szArguments );
					printf( "Working Directory: %s\n", szWorkingDirectory );
					printf( "Created By:        %s\n", szCreatedBy );
					printf( "Description:       %s\n", szDescription );
					printf( "\n" );

					fflush( stdout );
				}

				if ( bVerboseOptionSelected && *bMultipleHosts )
				{
					printf( "%s -> Logging scheduled task information.\n", szTarget );

					fflush( stdout );
				}

				WaitForSingleObject( hSemaphore, INFINITE );

				pOutputFile = fopen( "ScheduledTaskInfo.txt", "r" );

				if ( pOutputFile != NULL )
				{
					fclose( pOutputFile );
				}
				else
				{
					pOutputFile = fopen( "ScheduledTaskInfo.txt", "w" );

					if ( pOutputFile != NULL )
					{
						fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
						fprintf( pOutputFile, "\n" );
						fprintf( pOutputFile, "Hostname\tTask Name\tTarget File\tArguments\tWorking Directory\tCreated By\tDescription\n" );

						fclose( pOutputFile );
					}
				}

				pOutputFile = fopen( "ScheduledTaskInfo.txt", "a+" );

				if ( pOutputFile != NULL )
				{
					fprintf( pOutputFile, "%s\t%s\t%s\t%s\t%s\t%s\t%s\n", szTarget, szTaskFile, szFilePath, szArguments, szWorkingDirectory, szCreatedBy, szDescription );

					fclose( pOutputFile );
				}

				ReleaseSemaphore( hSemaphore, 1, NULL );
			}
		}

		FindClose( hFind );
	}
	else
	{
		dwError = GetLastError();

		WriteLastErrorToErrorLog( szTarget, "FindFirstFile (GetScheduledTaskInfo)", &dwError, bMultipleHosts );
	}
}

VOID ParseScheduledTaskFile( CHAR szTarget[], CHAR szTaskFile[], CHAR szFilePath[], CHAR szArguments[], CHAR szWorkingDirectory[], CHAR szCreatedBy[], CHAR szDescription[] )
{
	CHAR           szTaskPath[ 512 ];
	FILE          *pInputFile;
	unsigned char    szBuffer[ 1024 ];
	DWORD            dwLength;
	DWORD                   i;
	DWORD                   j;

	sprintf( szTaskPath, "\\\\%s\\ADMIN$\\Tasks\\%s", szTarget, szTaskFile );

	pInputFile = fopen( szTaskPath, "rb" );

	if ( pInputFile != NULL )
	{
		if ( fread( szBuffer, 1, 70, pInputFile ) > 0 )
		{
			if ( fread( szBuffer, 1, 2, pInputFile ) > 0 )
			{
				if ( (DWORD)szBuffer[1] > 0 )
				{
					dwLength = ( (DWORD)szBuffer[0] + 255 + (DWORD)szBuffer[1] ) * 2;
				}
				else
				{
					dwLength = (DWORD)szBuffer[0] * 2;
				}

				if ( dwLength > 0 )
				{
					if ( fread( szBuffer, 1, dwLength, pInputFile ) > 0 )
					{
						i = 0;

						for ( j = 0; j < dwLength; j = j + 2 )
						{
							szFilePath[i] = szBuffer[j];

							i++;
						}
					}
				}
			}

			if ( fread( szBuffer, 1, 2, pInputFile ) > 0 )
			{
				if ( (DWORD)szBuffer[1] > 0 )
				{
					dwLength = ( (DWORD)szBuffer[0] + 255 + (DWORD)szBuffer[1] ) * 2;
				}
				else
				{
					dwLength = (DWORD)szBuffer[0] * 2;
				}

				if ( dwLength > 0 )
				{
					if ( fread( szBuffer, 1, dwLength, pInputFile ) > 0 )
					{
						i = 0;

						for ( j = 0; j < dwLength; j = j + 2 )
						{
							szArguments[i] = szBuffer[j];

							i++;
						}
					}
				}
			}

			if ( fread( szBuffer, 1, 2, pInputFile ) > 0 )
			{
				if ( (DWORD)szBuffer[1] > 0 )
				{
					dwLength = ( (DWORD)szBuffer[0] + 255 + (DWORD)szBuffer[1] ) * 2;
				}
				else
				{
					dwLength = (DWORD)szBuffer[0] * 2;
				}

				if ( dwLength > 0 )
				{
					if ( fread( szBuffer, 1, dwLength, pInputFile ) > 0 )
					{
						i = 0;

						for ( j = 0; j < dwLength; j = j + 2 )
						{
							szWorkingDirectory[i] = szBuffer[j];

							i++;
						}
					}
				}
			}

			if ( fread( szBuffer, 1, 2, pInputFile ) > 0 )
			{
				if ( (DWORD)szBuffer[1] > 0 )
				{
					dwLength = ( (DWORD)szBuffer[0] + 255 + (DWORD)szBuffer[1] ) * 2;
				}
				else
				{
					dwLength = (DWORD)szBuffer[0] * 2;
				}

				if ( dwLength > 0 )
				{
					if ( fread( szBuffer, 1, dwLength, pInputFile ) > 0 )
					{
						i = 0;

						for ( j = 0; j < dwLength; j = j + 2 )
						{
							szCreatedBy[i] = szBuffer[j];

							i++;
						}
					}
				}
			}

			if ( fread( szBuffer, 1, 2, pInputFile ) > 0 )
			{
				if ( (DWORD)szBuffer[1] > 0 )
				{
					dwLength = ( (DWORD)szBuffer[0] + 255 + (DWORD)szBuffer[1] ) * 2;
				}
				else
				{
					dwLength = (DWORD)szBuffer[0] * 2;
				}

				if ( dwLength > 0 )
				{
					if ( fread( szBuffer, 1, dwLength, pInputFile ) > 0 )
					{
						i = 0;

						for ( j = 0; j < dwLength; j = j + 2 )
						{
							szDescription[i] = szBuffer[j];

							i++;
						}
					}
				}
			}
		}

		fclose( pInputFile );
	}
}

VOID GetSNMPCommunityInfo( CHAR szTarget[], BOOL *bMultipleHosts )
{
	CHAR       *pLocation;
	CHAR     szTempTarget[ 128 ];
	DWORD               i;
	LONG          lReturn;
	HKEY             hKey;
	HKEY          hSubKey;
	DWORD               j;
	DWORD    dwBufferSize;
	CHAR  szCommunityName[ 128 ];
	FILE     *pOutputFile;
	DWORD         dwError;

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

	lReturn = RegConnectRegistry( szTempTarget, HKEY_LOCAL_MACHINE, &hKey );

	if ( lReturn == ERROR_SUCCESS )
	{
		lReturn = RegOpenKeyEx( hKey, "SYSTEM\\CurrentControlSet\\Services\\SNMP\\Parameters\\ValidCommunities", 0, KEY_READ, &hSubKey );

		if ( lReturn == ERROR_SUCCESS )
		{
			for ( j = 0; TRUE; j++ )
			{
				dwBufferSize = 128;

				lReturn = RegEnumValue( hSubKey, j, szCommunityName, &dwBufferSize, NULL, NULL, NULL, NULL );

				if ( lReturn != ERROR_SUCCESS )
				{
					break;
				}

				if ( !*bMultipleHosts )
				{
					if ( i == 0 )
					{
						printf( "\n" );
						printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
						printf( "+++++        SNMP COMMUNITY INFORMATION       +++++\n" );
						printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
						printf( "\n" );

						i++;
					}

					printf( "SNMP Community Name: %s\n", szCommunityName );

					fflush( stdout );
				}

				if ( bVerboseOptionSelected && *bMultipleHosts )
				{
					printf( "%s -> Logging SNMP community information.\n", szTarget );

					fflush( stdout );
				}

				WaitForSingleObject( hSemaphore, INFINITE );

				pOutputFile = fopen( "SNMPCommunityInfo.txt", "r" );

				if ( pOutputFile != NULL )
				{
					fclose( pOutputFile );
				}
				else
				{
					pOutputFile = fopen( "SNMPCommunityInfo.txt", "w" );

					if ( pOutputFile != NULL )
					{
						fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
						fprintf( pOutputFile, "\n" );
						fprintf( pOutputFile, "Hostname\tSNMP Community Name\n" );

						fclose( pOutputFile );
					}
				}

				pOutputFile = fopen( "SNMPCommunityInfo.txt", "a+" );

				if ( pOutputFile != NULL )
				{
					fprintf( pOutputFile, "%s\t%s\n", szTarget, szCommunityName );

					fclose( pOutputFile );
				}

				ReleaseSemaphore( hSemaphore, 1, NULL );
			}

			if ( !*bMultipleHosts )
			{
				printf( "\n" );

				fflush( stdout );
			}

			RegCloseKey( hSubKey );
		}
		else
		{
			dwError = lReturn;

			WriteLastErrorToErrorLog( szTarget, "RegOpenKeyEx (GetSNMPCommunityInfo)", &dwError, bMultipleHosts );
		}

		RegCloseKey( hKey );
	}
	else
	{
		dwError = lReturn;

		WriteLastErrorToErrorLog( szTarget, "RegConnectRegistry (GetSNMPCommunityInfo)", &dwError, bMultipleHosts );
	}
}

VOID GetODBCDriverInfo( CHAR szTarget[], BOOL *bMultipleHosts )
{
	CHAR    *pLocation;
	CHAR  szTempTarget[ 128 ];
	DWORD            i;
	LONG       lReturn;
	HKEY          hKey;
	HKEY       hSubKey;
	DWORD            j;
	CHAR  szSubKeyName[ 256 ];
	FILE  *pOutputFile;
	DWORD      dwError;

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

	lReturn = RegConnectRegistry( szTempTarget, HKEY_LOCAL_MACHINE, &hKey );

	if ( lReturn == ERROR_SUCCESS )
	{
		lReturn = RegOpenKeyEx( hKey, "SOFTWARE\\ODBC\\ODBCINST.INI", 0, KEY_ENUMERATE_SUB_KEYS, &hSubKey );

		if ( lReturn == ERROR_SUCCESS )
		{
			for ( j = 0; TRUE; j++ )
			{
				lReturn = RegEnumKey( hSubKey, j, szSubKeyName, 255 );

				if ( lReturn != ERROR_SUCCESS )
				{
					break;
				}

				if ( !*bMultipleHosts )
				{
					if ( i == 0 )
					{
						printf( "\n" );
						printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
						printf( "+++++         ODBC DRIVER INFORMATION         +++++\n" );
						printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
						printf( "\n" );

						i++;
					}

					printf( "ODBC Driver: %s\n", szSubKeyName );

					fflush( stdout );
				}

				if ( bVerboseOptionSelected && *bMultipleHosts )
				{
					printf( "%s -> Logging ODBC driver information.\n", szTarget );

					fflush( stdout );
				}

				WaitForSingleObject( hSemaphore, INFINITE );

				pOutputFile = fopen( "ODBCDriverInfo.txt", "r" );

				if ( pOutputFile != NULL )
				{
					fclose( pOutputFile );
				}
				else
				{
					pOutputFile = fopen( "ODBCDriverInfo.txt", "w" );

					if ( pOutputFile != NULL )
					{
						fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
						fprintf( pOutputFile, "\n" );
						fprintf( pOutputFile, "Hostname\tODBC Driver\n" );

						fclose( pOutputFile );
					}
				}

				pOutputFile = fopen( "ODBCDriverInfo.txt", "a+" );

				if ( pOutputFile != NULL )
				{
					fprintf( pOutputFile, "%s\t%s\n", szTarget, szSubKeyName );

					fclose( pOutputFile );
				}

				ReleaseSemaphore( hSemaphore, 1, NULL );
			}

			if ( !*bMultipleHosts )
			{
				printf( "\n" );

				fflush( stdout );
			}

			RegCloseKey( hSubKey );
		}
		else
		{
			dwError = lReturn;

			WriteLastErrorToErrorLog( szTarget, "RegOpenKeyEx (GetODBCDriverInfo)", &dwError, bMultipleHosts );
		}

		RegCloseKey( hKey );
	}
	else
	{
		dwError = lReturn;

		WriteLastErrorToErrorLog( szTarget, "RegConnectRegistry (GetODBCDriverInfo)", &dwError, bMultipleHosts );
	}
}

VOID GetInteractivelyLoggedOnUsers( CHAR szTarget[], BOOL *bMultipleHosts )
{
	CHAR     *pLocation;
	CHAR   szTempTarget[ 128 ];
	DWORD             i;
	LONG        lReturn;
	HKEY           hKey;
	DWORD             j;
	CHAR   szSubKeyName[ 256 ];
	CHAR   szDomainName[ 128 ];
	CHAR  szAccountName[ 128 ];
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

	lReturn = RegConnectRegistry( szTempTarget, HKEY_USERS, &hKey );

	if ( lReturn == ERROR_SUCCESS )
	{
		for ( j = 0; TRUE; j++ )
		{
			lReturn = RegEnumKey( hKey, j, szSubKeyName, 255 );

			if ( lReturn != ERROR_SUCCESS )
			{
				break;
			}

			if ( strstr( szSubKeyName, "S-" ) != NULL && strstr( szSubKeyName, "_Classes" ) == NULL )
			{
				if ( GetAccountNameFromSID( szTarget, szSubKeyName, szDomainName, szAccountName, bMultipleHosts ) )
				{
					if ( !*bMultipleHosts )
					{
						if ( i == 0 )
						{
							printf( "\n" );
							printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
							printf( "+++++      INTERACTIVELY LOGGED ON USERS      +++++\n" );
							printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
							printf( "\n" );

							i++;
						}

						printf( "Username: %s\\%s\n", szDomainName, szAccountName );

						fflush( stdout );
					}

					if ( bVerboseOptionSelected && *bMultipleHosts )
					{
						printf( "%s -> Logging interactively logged on users.\n", szTarget );

						fflush( stdout );
					}

					WaitForSingleObject( hSemaphore, INFINITE );

					pOutputFile = fopen( "InteractivelyLoggedOnUsers.txt", "r" );

					if ( pOutputFile != NULL )
					{
						fclose( pOutputFile );
					}
					else
					{
						pOutputFile = fopen( "InteractivelyLoggedOnUsers.txt", "w" );

						if ( pOutputFile != NULL )
						{
							fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
							fprintf( pOutputFile, "\n" );
							fprintf( pOutputFile, "Hostname\tUsername\n" );

							fclose( pOutputFile );
						}
					}

					pOutputFile = fopen( "InteractivelyLoggedOnUsers.txt", "a+" );

					if ( pOutputFile != NULL )
					{
						fprintf( pOutputFile, "%s\t%s\\%s\n", szTarget, szDomainName, szAccountName );

						fclose( pOutputFile );
					}

					ReleaseSemaphore( hSemaphore, 1, NULL );
				}
			}
		}

		if ( !*bMultipleHosts )
		{
			printf( "\n" );

			fflush( stdout );
		}

		RegCloseKey( hKey );
	}
	else
	{
		dwError = lReturn;

		WriteLastErrorToErrorLog( szTarget, "RegConnectRegistry (GetInteractivelyLoggedOnUsers)", &dwError, bMultipleHosts );
	}
}

VOID GetFileSharePermissions( CHAR szTarget[], BOOL *bMultipleHosts )
{
	CHAR                *pLocation;
	CHAR              szTempTarget[ 128 ];
	WCHAR                wszTarget[ 256 ];
	DWORD                        i;
	DWORD                  dwLevel;
	DWORD            dwEntriesRead;
	DWORD           dwTotalEntries;
	DWORD           dwResumeHandle;
	SHARE_INFO_2            *pInfo;
	NET_API_STATUS         nStatus;
	SHARE_INFO_2        *pTempInfo;
	DWORD                        j;
	CHAR               szShareName[ 128 ];
	CHAR               szSharePath[ 512 ];
	CHAR           szFullShareName[ 256 ];
	ACL                     *pDACL;
	DWORD                  lReturn;
	DWORD                        k;
	ACE_HEADER               *pACE;
	CHAR              szAccessType[ 128 ];
	CHAR             szPermissions[ 128 ];
	PSID                      pSID;
	DWORD             dwDomainName;
	CHAR              szDomainName[ 128 ];
	DWORD            dwAccountName;
	CHAR             szAccountName[ 128 ];
	SID_NAME_USE             snUse;
	FILE              *pOutputFile;
	DWORD                  dwError;

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

	dwLevel        = 2;
	dwEntriesRead  = 0;
	dwTotalEntries = 0;
	dwResumeHandle = 0;

	do
	{
		pInfo = NULL;

		nStatus = NetShareEnum( wszTarget, dwLevel, (PBYTE *)&pInfo, MAX_PREFERRED_LENGTH, &dwEntriesRead, &dwTotalEntries, &dwResumeHandle );

		if ( nStatus == NERR_Success || nStatus == ERROR_MORE_DATA )
		{
			if ( pInfo != NULL )
			{
				pTempInfo = pInfo;

				for ( j = 0; j < dwEntriesRead; j++ )
				{
					if ( pTempInfo->shi2_type == STYPE_DISKTREE || pTempInfo->shi2_type == STYPE_SPECIAL )
					{
						WideCharToMultiByte( CP_ACP, 0, (WCHAR *)pTempInfo->shi2_netname, -1, szShareName, 128, NULL, NULL );
						WideCharToMultiByte( CP_ACP, 0, (WCHAR *)pTempInfo->shi2_path,    -1, szSharePath, 512, NULL, NULL );

						sprintf( szFullShareName, "\\\\%s\\%s", szTarget, szShareName );

						pDACL = NULL;

						lReturn = GetNamedSecurityInfo( szFullShareName, SE_LMSHARE, DACL_SECURITY_INFORMATION, NULL, NULL, &pDACL, NULL, NULL );

						if ( lReturn == ERROR_SUCCESS )
						{
							if ( pDACL != NULL )
							{
								if ( !*bMultipleHosts )
								{
									if ( i == 0 )
									{
										printf( "\n" );
										printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
										printf( "+++++            SHARE PERMISSIONS            +++++\n" );
										printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
										printf( "\n" );

										i++;
									}

									printf( "Share Name:        %s\n", szShareName );
									printf( "Share Path:        %s\n", szSharePath );

									fflush( stdout );
								}

								if ( bVerboseOptionSelected && *bMultipleHosts )
								{
									printf( "%s -> Logging share permissions.\n", szTarget );

									fflush( stdout );
								}

								for ( k = 0; k < pDACL->AceCount; k++ )
								{
									if ( GetAce( pDACL, k, (PVOID *)&pACE ) )
									{
										strcpy( szAccessType,  "" );
										strcpy( szPermissions, "" );

										if ( pACE->AceType == ACCESS_ALLOWED_ACE_TYPE )
										{
											strcpy( szAccessType, "Allow" );

											if ( ( (ACCESS_ALLOWED_ACE *)pACE )->Mask & FILE_SHARE_READ )
											{
												sprintf( szPermissions, "%sR", szPermissions );
											}

											if ( ( (ACCESS_ALLOWED_ACE *)pACE )->Mask & FILE_SHARE_WRITE )
											{
												sprintf( szPermissions, "%sW", szPermissions );
											}

											if ( ( (ACCESS_ALLOWED_ACE *)pACE )->Mask & FILE_SHARE_DELETE )
											{
												sprintf( szPermissions, "%sD", szPermissions );
											}

											pSID = &( (ACCESS_ALLOWED_ACE *)pACE )->SidStart;
										}

										if ( pACE->AceType == ACCESS_DENIED_ACE_TYPE )
										{
											strcpy( szAccessType, "Deny" );

											if ( ( (ACCESS_DENIED_ACE *)pACE )->Mask & FILE_SHARE_READ )
											{
												sprintf( szPermissions, "%sR", szPermissions );
											}

											if ( ( (ACCESS_DENIED_ACE *)pACE )->Mask & FILE_SHARE_WRITE )
											{
												sprintf( szPermissions, "%sW", szPermissions );
											}

											if ( ( (ACCESS_DENIED_ACE *)pACE )->Mask & FILE_SHARE_DELETE )
											{
												sprintf( szPermissions, "%sD", szPermissions );
											}

											pSID = &( (ACCESS_DENIED_ACE *)pACE )->SidStart;
										}		

										dwDomainName  = sizeof( szDomainName );
										dwAccountName = sizeof( szAccountName );

										if ( LookupAccountSid( szTarget, pSID, szAccountName, &dwAccountName, szDomainName, &dwDomainName, &snUse ) )
										{
											if ( strcmp( szAccessType, "" ) == 0 )
											{
												strcpy( szAccessType, "Other" );
											}

											if ( strcmp( szPermissions, "" ) == 0 )
											{
												strcpy( szPermissions, "Other (Not R,W or D)" );
											}

											if ( !*bMultipleHosts )
											{
												printf( "Account Name:      %s\\%s\n", szDomainName, szAccountName );
												printf( "Access Type:       %s\n", szAccessType );
												printf( "Share Permissions: %s\n", szPermissions );

												fflush( stdout );
											}

											if ( bVerboseOptionSelected && *bMultipleHosts )
											{
												printf( "%s -> Logging share permissions.\n", szTarget );

												fflush( stdout );
											}

											WaitForSingleObject( hSemaphore, INFINITE );

											pOutputFile = fopen( "FileSharePermissions.txt", "r" );

											if ( pOutputFile != NULL )
											{
												fclose( pOutputFile );
											}
											else
											{
												pOutputFile = fopen( "FileSharePermissions.txt", "w" );

												if ( pOutputFile != NULL )
												{
													fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
													fprintf( pOutputFile, "\n" );
													fprintf( pOutputFile, "Hostname\tShare Name\tShare Path\tPermission Type\tAccount Name\tAccess Type\tPermissions\n" );

													fclose( pOutputFile );
												}
											}

											pOutputFile = fopen( "FileSharePermissions.txt", "a+" );

											if ( pOutputFile != NULL )
											{
												fprintf( pOutputFile, "%s\t%s\t%s\tShare\t%s\\%s\t%s\t%s\n", szTarget, szShareName, szSharePath, szDomainName, szAccountName, szAccessType, szPermissions );

												fclose( pOutputFile );
											}

											ReleaseSemaphore( hSemaphore, 1, NULL );
										}
									}
									else
									{
										dwError = GetLastError();

										WriteLastErrorToErrorLog( szTarget, "GetAce (GetFileSharePermissions)", &dwError, bMultipleHosts );
									}
								}

								LocalFree( pDACL );
							}
						}
						else
						{
							dwError = lReturn;

							WriteLastErrorToErrorLog( szTarget, "GetNamedSecurityInfo (GetFileSharePermissions)", &dwError, bMultipleHosts );
						}

						pDACL = NULL;

						lReturn = GetNamedSecurityInfo( szFullShareName, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, &pDACL, NULL, NULL );

						if ( lReturn == ERROR_SUCCESS )
						{
							if ( pDACL != NULL )
							{
								if ( !*bMultipleHosts )
								{
									if ( i == 0 )
									{
										printf( "\n" );
										printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
										printf( "+++++         FILE SHARE PERMISSIONS          +++++\n" );
										printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
										printf( "\n" );

										i++;
									}

									printf( "Share Name:        %s\n", szShareName );
									printf( "Share Path:        %s\n", szSharePath );

									fflush( stdout );
								}

								for ( k = 0; k < pDACL->AceCount; k++ )
								{
									if ( GetAce( pDACL, k, (PVOID *)&pACE ) )
									{
										strcpy( szAccessType,  "" );
										strcpy( szPermissions, "" );

										if ( pACE->AceType == ACCESS_ALLOWED_ACE_TYPE )
										{
											strcpy( szAccessType, "Allow" );

											if ( ( (ACCESS_ALLOWED_ACE *)pACE )->Mask & FILE_GENERIC_READ )
											{
												sprintf( szPermissions, "%sR", szPermissions );
											}

											if ( ( (ACCESS_ALLOWED_ACE *)pACE )->Mask & FILE_GENERIC_WRITE )
											{
												sprintf( szPermissions, "%sW", szPermissions );
											}

											if ( ( (ACCESS_ALLOWED_ACE *)pACE )->Mask & FILE_GENERIC_EXECUTE )
											{
												sprintf( szPermissions, "%sX", szPermissions );
											}

											if ( ( (ACCESS_ALLOWED_ACE *)pACE )->Mask & DELETE )
											{
												sprintf( szPermissions, "%sD", szPermissions );
											}

											pSID = &( (ACCESS_ALLOWED_ACE *)pACE )->SidStart;
										}

										if ( pACE->AceType == ACCESS_DENIED_ACE_TYPE )
										{
											strcpy( szAccessType, "Deny" );

											if ( ( (ACCESS_DENIED_ACE *)pACE )->Mask & FILE_GENERIC_READ )
											{
												sprintf( szPermissions, "%sR", szPermissions );
											}

											if ( ( (ACCESS_DENIED_ACE *)pACE )->Mask & FILE_GENERIC_WRITE )
											{
												sprintf( szPermissions, "%sW", szPermissions );
											}

											if ( ( (ACCESS_DENIED_ACE *)pACE )->Mask & FILE_GENERIC_EXECUTE )
											{
												sprintf( szPermissions, "%sX", szPermissions );
											}

											if ( ( (ACCESS_DENIED_ACE *)pACE )->Mask & DELETE )
											{
												sprintf( szPermissions, "%sD", szPermissions );
											}

											pSID = &( (ACCESS_DENIED_ACE *)pACE )->SidStart;
										}

										dwDomainName  = sizeof( szDomainName );
										dwAccountName = sizeof( szAccountName );

										if ( LookupAccountSid( szTarget, pSID, szAccountName, &dwAccountName, szDomainName, &dwDomainName, &snUse ) )
										{
											if ( strcmp( szAccessType, "" ) == 0 )
											{
												strcpy( szAccessType, "Other" );
											}

											if ( strcmp( szPermissions, "" ) == 0 )
											{
												strcpy( szPermissions, "Other (Not R,W,X or D)" );
											}

											if ( !*bMultipleHosts )
											{
												printf( "Account Name:      %s\\%s\n", szDomainName, szAccountName );
												printf( "Access Type:       %s\n", szAccessType );
												printf( "NTFS Permissions:  %s\n", szPermissions );

												fflush( stdout );
											}

											if ( bVerboseOptionSelected && *bMultipleHosts )
											{
												printf( "%s -> Logging file share permissions.\n", szTarget );

												fflush( stdout );
											}

											WaitForSingleObject( hSemaphore, INFINITE );

											pOutputFile = fopen( "FileSharePermissions.txt", "r" );

											if ( pOutputFile != NULL )
											{
												fclose( pOutputFile );
											}
											else
											{
												pOutputFile = fopen( "FileSharePermissions.txt", "w" );

												if ( pOutputFile != NULL )
												{
													fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
													fprintf( pOutputFile, "\n" );
													fprintf( pOutputFile, "Hostname\tShare Name\tShare Path\tPermission Type\tAccount Name\tAccess Type\tPermissions\n" );

													fclose( pOutputFile );
												}
											}

											pOutputFile = fopen( "FileSharePermissions.txt", "a+" );

											if ( pOutputFile != NULL )
											{
												fprintf( pOutputFile, "%s\t%s\t%s\tNTFS\t%s\\%s\t%s\t%s\n", szTarget, szShareName, szSharePath, szDomainName, szAccountName, szAccessType, szPermissions );

												fclose( pOutputFile );
											}

											ReleaseSemaphore( hSemaphore, 1, NULL );
										}
									}
									else
									{
										dwError = GetLastError();

										WriteLastErrorToErrorLog( szTarget, "GetAce (GetFileSharePermissions)", &dwError, bMultipleHosts );
									}
								}

								LocalFree( pDACL );
							}
						}
						else
						{
							dwError = lReturn;

							WriteLastErrorToErrorLog( szTarget, "GetNamedSecurityInfo (GetFileSharePermissions)", &dwError, bMultipleHosts );
						}

						if ( !*bMultipleHosts )
						{
							printf( "\n" );

							fflush( stdout );
						}
					}

					pTempInfo++;
				}
			}
		}
		else
		{
			dwError = nStatus;

			WriteLastErrorToErrorLog( szTarget, "NetShareEnum (GetFileSharePermissions)", &dwError, bMultipleHosts );
		}

		if ( pInfo != NULL )
		{
			NetApiBufferFree( pInfo );
		}
	}
	while ( nStatus == ERROR_MORE_DATA );
}

VOID GuessWindowsPasswords( CHAR szTarget[], BOOL *bMultipleHosts )
{
	BOOL bSuppressErrors;
	CHAR     szCacheFile[ 512 ];
	FILE     *pCacheFile;
	CHAR      szUsername[ 128 ];
	FILE  *pPasswordFile;
	CHAR      szPassword[ 128 ];
	CHAR   szTmpPassword[ 128 ];
	CHAR   szTmpUsername[ 128 ];

	bSuppressErrors = TRUE;

	sprintf( szCacheFile, "UserCache\\%s.users", szTarget );

	pCacheFile = fopen( szCacheFile, "r" );

	if ( pCacheFile != NULL )
	{
		if ( !*bMultipleHosts )
		{
			printf( "\n" );
			printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
			printf( "+++++             GUESS PASSWORDS             +++++\n" );
			printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
			printf( "\n" );

			fflush( stdout );
		}

		while ( fgets( szUsername, sizeof( szUsername ), pCacheFile ) != NULL )
		{
			Trim( szUsername );

			if ( szUsername[0] != '\n' )
			{
				if ( szUsername[strlen( szUsername ) - 1] == '\n' )
				{
					szUsername[strlen( szUsername ) - 1] = '\0';
				}

				pPasswordFile = fopen( "Dictionary.input", "r" );

				if ( pPasswordFile != NULL )
				{
					while ( fgets( szPassword, sizeof( szPassword ), pPasswordFile ) != NULL )
					{
						Trim( szPassword );

						if ( szPassword[0] != '#' && szPassword[0] != '\n' )
						{
							if ( szPassword[strlen( szPassword ) - 1] == '\n' )
							{
								szPassword[strlen( szPassword ) - 1] = '\0';
							}

							strcpy( szTmpPassword, szPassword );

							_strupr( szTmpPassword );

							if ( strcmp( szTmpPassword, "<USERNAME>" ) == 0 )
							{
								if ( bVerboseOptionSelected || !*bMultipleHosts )
								{
									printf( "Trying username:password... %s:%s\n", szUsername, szUsername );

									fflush( stdout );
								}

								if ( Connect( szTarget, szUsername, szUsername, &bSuppressErrors ) )
								{
									Disconnect( szTarget, &bSuppressErrors );

									if ( ConnectAsAdmin( szTarget, szUsername, szUsername, &bSuppressErrors ) )
									{
										DisconnectFromAdmin( szTarget, &bSuppressErrors );

										LogGuessedWindowsPasswords( szTarget, szUsername, szUsername, "Yes" );
									}
									else
									{
										LogGuessedWindowsPasswords( szTarget, szUsername, szUsername, "No" );
									}

									if ( bVerboseOptionSelected || !*bMultipleHosts )
									{
										printf( "\n" );
										printf( "PASSWORD GUESSED! %s's password is %s\n", szUsername, szUsername );

										fflush( stdout );
									}

									break;
								}
							}
							else if ( strcmp( szTmpPassword, "<UCUSERNAME>" ) == 0 )
							{
								strcpy( szTmpUsername, szUsername );

								_strupr( szTmpUsername );

								if ( bVerboseOptionSelected || !*bMultipleHosts )
								{
									printf( "Trying username:password... %s:%s\n", szUsername, szTmpUsername );

									fflush( stdout );
								}

								if ( Connect( szTarget, szUsername, szTmpUsername, &bSuppressErrors ) )
								{
									Disconnect( szTarget, &bSuppressErrors );

									if ( ConnectAsAdmin( szTarget, szUsername, szTmpUsername, &bSuppressErrors ) )
									{
										DisconnectFromAdmin( szTarget, &bSuppressErrors );

										LogGuessedWindowsPasswords( szTarget, szUsername, szTmpUsername, "Yes" );
									}
									else
									{
										LogGuessedWindowsPasswords( szTarget, szUsername, szTmpUsername, "No" );
									}

									if ( bVerboseOptionSelected || !*bMultipleHosts )
									{
										printf( "\n" );
										printf( "PASSWORD GUESSED! %s's password is %s\n", szUsername, szTmpUsername );

										fflush( stdout );
									}

									break;
								}
							}
							else if ( strcmp( szTmpPassword, "<LCUSERNAME>" ) == 0 )
							{
								strcpy( szTmpUsername, szUsername );

								_strlwr( szTmpUsername );

								if ( bVerboseOptionSelected || !*bMultipleHosts )
								{
									printf( "Trying username:password... %s:%s\n", szUsername, szTmpUsername );

									fflush( stdout );
								}

								if ( Connect( szTarget, szUsername, szTmpUsername, &bSuppressErrors ) )
								{
									Disconnect( szTarget, &bSuppressErrors );

									if ( ConnectAsAdmin( szTarget, szUsername, szTmpUsername, &bSuppressErrors ) )
									{
										DisconnectFromAdmin( szTarget, &bSuppressErrors );

										LogGuessedWindowsPasswords( szTarget, szUsername, szTmpUsername, "Yes" );
									}
									else
									{
										LogGuessedWindowsPasswords( szTarget, szUsername, szTmpUsername, "No" );
									}

									if ( bVerboseOptionSelected || !*bMultipleHosts )
									{
										printf( "\n" );
										printf( "PASSWORD GUESSED! %s's password is %s\n", szUsername, szTmpUsername );

										fflush( stdout );
									}

									break;
								}
							}
							else if ( strcmp( szTmpPassword, "<BLANK>" ) == 0 )
							{
								if ( bVerboseOptionSelected || !*bMultipleHosts )
								{
									printf( "Trying username:password... %s:<blank>\n", szUsername );

									fflush( stdout );
								}

								if ( Connect( szTarget, szUsername, "", &bSuppressErrors ) )
								{
									Disconnect( szTarget, &bSuppressErrors );

									if ( ConnectAsAdmin( szTarget, szUsername, "", &bSuppressErrors ) )
									{
										DisconnectFromAdmin( szTarget, &bSuppressErrors );

										LogGuessedWindowsPasswords( szTarget, szUsername, "<blank>", "Yes" );
									}
									else
									{
										LogGuessedWindowsPasswords( szTarget, szUsername, "<blank>", "No" );
									}

									if ( bVerboseOptionSelected || !*bMultipleHosts )
									{
										printf( "\n" );
										printf( "PASSWORD GUESSED! %s's password is <blank>\n", szUsername );

										fflush( stdout );
									}

									break;
								}
							}
							else
							{
								if ( bVerboseOptionSelected || !*bMultipleHosts )
								{
									printf( "Trying username:password... %s:%s\n", szUsername, szPassword );

									fflush( stdout );
								}

								if ( Connect( szTarget, szUsername, szPassword, &bSuppressErrors ) )
								{
									Disconnect( szTarget, &bSuppressErrors );

									if ( ConnectAsAdmin( szTarget, szUsername, szPassword, &bSuppressErrors ) )
									{
										DisconnectFromAdmin( szTarget, &bSuppressErrors );

										LogGuessedWindowsPasswords( szTarget, szUsername, szPassword, "Yes" );
									}
									else
									{
										LogGuessedWindowsPasswords( szTarget, szUsername, szPassword, "No" );
									}

									if ( bVerboseOptionSelected || !*bMultipleHosts )
									{
										printf( "\n" );
										printf( "PASSWORD GUESSED! %s's password is %s\n", szUsername, szPassword );

										fflush( stdout );
									}

									break;
								}
							}
						}
					}

					fclose( pPasswordFile );
				}
				else
				{
					WriteToErrorLog( szTarget, "fopen (GuessWindowsPasswords)", "Cannot open file Dictionary.input.", bMultipleHosts );
				}
			}
		}

		if ( bVerboseOptionSelected || !*bMultipleHosts )
		{
			printf( "\n" );

			fflush( stdout );
		}

		fclose( pCacheFile );
	}
}

BOOL ConnectAsAdmin( CHAR szTarget[], CHAR szUsername[], CHAR szPassword[], BOOL *bMultipleHosts )
{
	BOOL                  bReturn;
	CHAR               *pLocation;
	CHAR             szTempTarget[ 128 ];
	CHAR             szRemoteName[ 128 ];
	DWORD          dwTextLocation;
	DWORD                       i;
	CHAR             szDomainName[ 128 ];
	DWORD                       j;
	CHAR           szTempUsername[ 128 ];
	WCHAR           wszRemoteName[ 256 ];
	WCHAR           wszDomainName[ 256 ];
	WCHAR             wszUsername[ 256 ];
	WCHAR             wszPassword[ 256 ];
	DWORD                 dwLevel;
	USE_INFO_2            ui2Info;
	NET_API_STATUS        nStatus;
	DWORD                 dwError;

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

	sprintf( szRemoteName, "%s\\ADMIN$", szTempTarget );

	pLocation = strstr( szUsername, "\\" );

	if ( pLocation != NULL )
	{
		dwTextLocation = (INT)( pLocation - szUsername );

		i = 0;

		while ( i < dwTextLocation )
		{
			szDomainName[i] = szUsername[i];

			i++;
		}

		szDomainName[i] = '\0';

		i = dwTextLocation + 1;

		j = 0;

		while ( i < strlen( szUsername ) )
		{
			szTempUsername[j] = szUsername[i];

			i++;
			j++;
		}

		szTempUsername[j] = '\0';
	}
	else
	{
		if ( strcmp( szUsername, "" ) != 0 )
		{
			strcpy( szDomainName, szTarget );
		}
		else
		{
			strcpy( szDomainName, "" );
		}

		strcpy( szTempUsername, szUsername );
	}

	MultiByteToWideChar( CP_ACP, 0, szRemoteName,   strlen( szRemoteName ) + 1,   wszRemoteName, sizeof( wszRemoteName ) / sizeof( wszRemoteName[0] ) );
	MultiByteToWideChar( CP_ACP, 0, szDomainName,   strlen( szDomainName ) + 1,   wszDomainName, sizeof( wszDomainName ) / sizeof( wszDomainName[0] ) );
	MultiByteToWideChar( CP_ACP, 0, szTempUsername, strlen( szTempUsername ) + 1, wszUsername,   sizeof( wszUsername ) / sizeof( wszUsername[0] ) );
	MultiByteToWideChar( CP_ACP, 0, szPassword,     strlen( szPassword ) + 1,     wszPassword,   sizeof( wszPassword ) / sizeof( wszPassword[0] ) );

	dwLevel = 2;

	ui2Info.ui2_local      = NULL;
	ui2Info.ui2_remote     = wszRemoteName;
	ui2Info.ui2_password   = wszPassword;
	ui2Info.ui2_asg_type   = USE_WILDCARD;
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

		WriteLastErrorToErrorLog( szTarget, "NetUseAdd (ConnectAsAdmin)", &dwError, bMultipleHosts );
	}

	return bReturn;
}

BOOL DisconnectFromAdmin( CHAR szTarget[], BOOL *bMultipleHosts )
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

	sprintf( szRemoteName, "%s\\ADMIN$", szTempTarget );

	MultiByteToWideChar( CP_ACP, 0, szRemoteName, strlen( szRemoteName ) + 1, wszRemoteName, sizeof( wszRemoteName ) / sizeof( wszRemoteName[0] ) );

	nStatus = NetUseDel( NULL, wszRemoteName, USE_LOTS_OF_FORCE );

	if ( nStatus == NERR_Success )
	{
		bReturn = TRUE;
	}
	else
	{
		dwError = nStatus;

		WriteLastErrorToErrorLog( szTarget, "NetUseDel (DisconnectFromAdmin)", &dwError, bMultipleHosts );
	}

	return bReturn;
}

VOID LogGuessedWindowsPasswords( CHAR szTarget[], CHAR szUsername[], CHAR szPassword[], CHAR szIsAdmin[] )
{
	FILE *pOutputFile;

	WaitForSingleObject( hSemaphore, INFINITE );

	pOutputFile = fopen( "GuessedWindowsPasswords.txt", "r" );

	if ( pOutputFile != NULL )
	{
		fclose( pOutputFile );
	}
	else
	{
		pOutputFile = fopen( "GuessedWindowsPasswords.txt", "w" );

		if ( pOutputFile != NULL )
		{
			fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
			fprintf( pOutputFile, "\n" );
			fprintf( pOutputFile, "Hostname\tUsername\tPassword\tIs Admin?\n" );

			fclose( pOutputFile );
		}
	}

	pOutputFile = fopen( "GuessedWindowsPasswords.txt", "a+" );

	if ( pOutputFile != NULL )
	{
		fprintf( pOutputFile, "%s\t%s\t%s\t%s\n", szTarget, szUsername, szPassword, szIsAdmin );

		fclose( pOutputFile );
	}

	ReleaseSemaphore( hSemaphore, 1, NULL );
}

VOID GuessSNMPCommunityStrings( CHAR szTarget[], BOOL *bMultipleHosts )
{
	BOOL   bSuppressErrors;
	FILE  *pCommStringFile;
	CHAR szCommunityString[ 128 ];

	bSuppressErrors = TRUE;

	pCommStringFile = fopen( "CommunityStrings.input", "r" );

	if ( pCommStringFile != NULL )
	{
		if ( !*bMultipleHosts )
		{
			printf( "\n" );
			printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
			printf( "+++++      GUESS SNMP COMMUNITY STRINGS       +++++\n" );
			printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
			printf( "\n" );

			fflush( stdout );
		}

		while ( fgets( szCommunityString, sizeof( szCommunityString ), pCommStringFile ) != NULL )
		{
			Trim( szCommunityString );

			if ( szCommunityString[0] != '#' && szCommunityString[0] != '\n' )
			{
				if ( szCommunityString[strlen( szCommunityString ) - 1] == '\n' )
				{
					szCommunityString[strlen( szCommunityString ) - 1] = '\0';
				}

				if ( bVerboseOptionSelected || !*bMultipleHosts )
				{
					printf( "Trying community string... %s\n", szCommunityString );

					fflush( stdout );
				}

				if ( SNMPConnect( szTarget, szCommunityString, &bSuppressErrors ) )
				{
					LogGuessedCommunityStrings( szTarget, szCommunityString );

					if ( bVerboseOptionSelected || !*bMultipleHosts )
					{
						printf( "\n" );
						printf( "COMMUNITY STRING GUESSED! %s\n", szCommunityString );

						fflush( stdout );
					}
				}
			}
		}

		if ( bVerboseOptionSelected || !*bMultipleHosts )
		{
			printf( "\n" );

			fflush( stdout );
		}

		fclose( pCommStringFile );
	}
	else
	{
		WriteToErrorLog( szTarget, "fopen (GuessSNMPCommunityStrings)", "Cannot open file CommunityStrings.input.", bMultipleHosts );
	}
}

BOOL SNMPConnect( CHAR szTarget[], CHAR szCommunityString[], BOOL *bMultipleHosts )
{
	BOOL                bLogonSuccess;
	CHAR                  szCacheFile[ 128 ];
	DWORD                           i;
	LPSNMP_MGR_SESSION     smsSession;
	AsnObjectIdentifier        aoiOID;
	RFC1157VarBindList         vbInfo;
	DWORD                    dwResult;
	AsnInteger          aiErrorStatus;
	AsnInteger           aiErrorIndex;
	CHAR                   szUsername[ 128 ];
	AsnAny                     *pInfo;
	DWORD                           j;
	FILE                  *pCacheFile;
	DWORD                     dwError;

	bLogonSuccess = FALSE;

	sprintf( szCacheFile, "UserCache\\%s.users", szTarget );

	i = 0;

	smsSession = NULL;

	smsSession = SnmpMgrOpen( szTarget, szCommunityString, 1000, 1 );

	if ( smsSession != NULL )
	{
		if ( SnmpMgrStrToOid( ".1.3.6.1.4.1.77.1.2.25.1.1", &aoiOID ) )
		{
			vbInfo.len  = 1;
			vbInfo.list = NULL;

			vbInfo.list = (RFC1157VarBind *)SnmpUtilMemReAlloc( vbInfo.list, sizeof( RFC1157VarBind ) * vbInfo.len );

			vbInfo.list[0].name = aoiOID;

			while ( TRUE )
			{
				vbInfo.list[0].value.asnType = ASN_NULL;

				dwResult = SnmpMgrRequest( smsSession, ASN_RFC1157_GETNEXTREQUEST, &vbInfo, &aiErrorStatus, &aiErrorIndex );

				if ( dwResult != 0 )
				{
					if ( aiErrorStatus == SNMP_ERRORSTATUS_NOERROR )
					{
						bLogonSuccess = TRUE;

						strcpy( szUsername, "" );

						pInfo = &vbInfo.list[0].value;

						j = 0;

						while ( j < pInfo->asnValue.string.length )
						{
							szUsername[j] = pInfo->asnValue.string.stream[j];

							j++;
						}

						szUsername[j] = '\0';

						if ( strcmp( szUsername, "" ) == 0 )
						{
							break;
						}

						WaitForSingleObject( hSemaphore, INFINITE );

						if ( i == 0 )
						{
							pCacheFile = fopen( szCacheFile, "w" );

							if ( pCacheFile != NULL )
							{
								fclose( pCacheFile );
							}

							i++;
						}

						pCacheFile = fopen( szCacheFile, "a+" );

						if ( pCacheFile != NULL )
						{
							fprintf( pCacheFile, "%s\n", szUsername );

							fclose( pCacheFile );
						}

						ReleaseSemaphore( hSemaphore, 1, NULL );
					}
					else if ( aiErrorStatus == SNMP_ERRORSTATUS_NOSUCHNAME )
					{
						bLogonSuccess = TRUE;

						break;
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}

			SnmpUtilVarBindFree( &vbInfo.list[0] );

			SnmpUtilVarBindListFree( &vbInfo );
		}

		SnmpMgrClose( smsSession );
	}
	else
	{
		dwError = GetLastError();

		WriteLastErrorToErrorLog( szTarget, "SNMPMgrOpen (SNMPConnect)", &dwError, bMultipleHosts );
	}

	return bLogonSuccess;
}

VOID LogGuessedCommunityStrings( CHAR szTarget[], CHAR szCommunityString[] )
{
	FILE *pOutputFile;

	WaitForSingleObject( hSemaphore, INFINITE );

	pOutputFile = fopen( "GuessedSNMPCommunityStrings.txt", "r" );

	if ( pOutputFile != NULL )
	{
		fclose( pOutputFile );
	}
	else
	{
		pOutputFile = fopen( "GuessedSNMPCommunityStrings.txt", "w" );

		if ( pOutputFile != NULL )
		{
			fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
			fprintf( pOutputFile, "\n" );
			fprintf( pOutputFile, "Hostname\tSNMP Community String\n" );

			fclose( pOutputFile );
		}
	}

	pOutputFile = fopen( "GuessedSNMPCommunityStrings.txt", "a+" );

	if ( pOutputFile != NULL )
	{
		fprintf( pOutputFile, "%s\t%s\n", szTarget, szCommunityString );

		fclose( pOutputFile );
	}

	ReleaseSemaphore( hSemaphore, 1, NULL );
}

// Written by Reed Arvin | reed@canaudit.com