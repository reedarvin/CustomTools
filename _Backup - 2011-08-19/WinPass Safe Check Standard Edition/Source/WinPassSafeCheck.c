//
// gcc source.c -o source.exe -lnetapi32
//

#define _WIN32_WINNT 0x0500
#define WINVER       0x0500

#include <windows.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h>
#include <process.h>
#include <lm.h>

#define MAX_THREADS 64

VOID                            Usage( VOID );
VOID                      ThreadedSub( VOID *pParameter );
BOOL                          Connect( CHAR *szTarget, CHAR *szUsername, CHAR *szPassword, BOOL *bMultipleHosts );
VOID         WriteLastErrorToErrorLog( CHAR *szTarget, CHAR *szFunction, DWORD *dwError, BOOL *bMultipleHosts );
VOID               StartPasswordCheck( CHAR *szTarget, CHAR *szDictionary, BOOL *bMultipleHosts );
VOID       GetAccountLockoutThreshold( CHAR *szTarget, CHAR *szLockoutThreshold, BOOL *bMultipleHosts );
VOID                  WriteToErrorLog( CHAR *szTarget, CHAR *szFunction, CHAR *szErrorMsg, BOOL *bMultipleHosts );
VOID                      GetUserList( CHAR *szTarget, BOOL *bMultipleHosts );
VOID                      GetUserInfo( CHAR *szTarget, CHAR *szUsername, BOOL *bAccountLocked, BOOL *bAccountDisabled, BOOL *bPasswordCantChange, DWORD *dwBadPWCount, BOOL *bMultipleHosts );
VOID                       WriteToLog( CHAR *szTarget, CHAR *szUsername, CHAR *szAccountFlags, DWORD *dwBadPWCount, CHAR *szLockoutThreshold );
BOOL                    CheckPassword( CHAR *szTarget, CHAR *szUsername, CHAR *szPassword, BOOL *bMultipleHosts );
VOID          WriteToPasswordErrorLog( CHAR *szTarget, CHAR *szUsername, CHAR *szPassword, CHAR *szErrorMsg, BOOL *bMultipleHosts );
VOID WriteLastErrorToPasswordErrorLog( CHAR *szTarget, CHAR *szUsername, CHAR *szPassword, DWORD *dwError, BOOL *bMultipleHosts );
VOID               LogGuessedPassword( CHAR *szTarget, CHAR *szUsername, CHAR *szPassword );
BOOL           ResetAccountAttributes( CHAR *szTarget, CHAR *szUsername, BOOL *bMultipleHosts );
BOOL                       Disconnect( CHAR *szTarget, BOOL *bMultipleHosts );

typedef struct _THREAD_ARGS
{
	CHAR        Target[ 128 ];
	CHAR    Dictionary[ 128 ];
	CHAR      Username[ 128 ];
	CHAR      Password[ 128 ];
	BOOL MultipleHosts;
} THREAD_ARGS, *PTHREAD_ARGS;

HANDLE hSemaphore;

INT nThreads = 0;

INT main( INT argc, CHAR *argv[] )
{
	BOOL      bContinue;
	CHAR   *pDictionary;
	CHAR     *pUsername;
	CHAR     *pPassword;
	INT         nGetOpt;
	FILE    *pInputFile;
	DWORD             i;
	DWORD       dwError;
	CHAR  szTargetInput[ 128 ];
	CHAR     szReadLine[ 128 ];
	CHAR       szTarget[ 128 ];

	PTHREAD_ARGS pThreadArgs;

	hSemaphore = CreateSemaphore( NULL, 1, 1, NULL );

	opterr = 0;

	bContinue = TRUE;

	pDictionary = NULL;
	pUsername   = NULL;
	pPassword   = NULL;

	while ( bContinue )
	{
		nGetOpt = getopt( argc, argv, "d:u:p:" );

		if ( nGetOpt == -1 )
		{
			bContinue = FALSE;
		}

		switch ( nGetOpt )
		{
			case 'd':
				pDictionary = optarg;

				break;

			case 'u':
				pUsername = optarg;

				break;

			case 'p':
				pPassword = optarg;

				break;
		}
	}

	if ( ( pDictionary == NULL ) || ( pUsername == NULL ) || ( pPassword == NULL ) )
	{
		Usage();

		return 1;
	}

	pInputFile = fopen( pDictionary, "r" );

	if ( pInputFile == NULL )
	{
		fprintf( stderr, "ERROR! Cannot open dictionary file \"%s\".\n", pDictionary );

		fflush( stderr );

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

	printf( "Running WinPassSafeCheck v1.0 with the following arguments:\n" );
	printf( "[+] Target Host(s):   \"%s\"\n", szTargetInput );
	printf( "[+] Dictionary File:  \"%s\"\n", pDictionary );
	printf( "[+] Connect Username: \"%s\"\n", pUsername );
	printf( "[+] Connect Passowrd: \"%s\"\n", pPassword );
	printf( "[+] # of Threads:     \"64\"\n" );
	printf( "[+] Output File:      \"WinPassSafeCheckLog.txt\"\n" );
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
				strcpy( pThreadArgs->Target,     szTarget );
				strcpy( pThreadArgs->Dictionary, pDictionary );
				strcpy( pThreadArgs->Username,   pUsername );
				strcpy( pThreadArgs->Password,   pPassword );

				pThreadArgs->MultipleHosts = TRUE;

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
			strcpy( pThreadArgs->Target,     szTarget );
			strcpy( pThreadArgs->Dictionary, pDictionary );
			strcpy( pThreadArgs->Username,   pUsername );
			strcpy( pThreadArgs->Password,   pPassword );

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

VOID Usage( VOID )
{
	printf( "WinPassSafeCheck v1.0 | Canaudit, Inc. http://www.canaudit.com/\n" );
	printf( "\n" );
	printf( "Usage: WinPassWarrior [-d:u:p:] <target host(s)>\n" );
	printf( "\n" );
	printf( "-d <dictionary file>   -- Dictionary file\n" );
	printf( "-u <connect username>  -- Username used for connection\n" );
	printf( "-p <connect password>  -- Password used for connection\n" );
	printf( "\n" );
	printf( "<target host(s)>       -- Host Name, IP Address, or Input File\n" );
	printf( "\n" );
	printf( "If the <connect username> and <connect password> arguments are both plus\n" );
	printf( "signs (+), the existing credentials of the user running this utility will\n" );
	printf( "be used.\n" );
	printf( "\n" );
	printf( "Examples:\n" );
	printf( "WinPassWarrior -d dict.txt -u Administrator -p P@$$w0rd 10.10.10.10\n" );
	printf( "WinPassWarrior -d dict.txt -u Administrator -p P@$$w0rd MyWindowsMachine\n" );
	printf( "WinPassWarrior -d dict.txt -u Administrator -p P@$$w0rd IPInputFile.txt\n" );
	printf( "\n" );
	printf( "WinPassWarrior -d dict.txt -u + -p + 10.10.10.10\n" );
	printf( "WinPassWarrior -d dict.txt -u + -p + MyWindowsMachine\n" );
	printf( "WinPassWarrior -d dict.txt -u + -p + IPInputFile.txt\n" );
	printf( "\n" );
	printf( "(Written by Reed Arvin | reed@canaudit.com)\n" );

	fflush( stdout );
}

VOID ThreadedSub( VOID *pParameter )
{
	CHAR       szTarget[ 128 ];
	CHAR   szDictionary[ 128 ];
	CHAR     szUsername[ 128 ];
	CHAR     szPassword[ 128 ];
	BOOL bMultipleHosts;

	PTHREAD_ARGS pThreadArgs;

	pThreadArgs = (PTHREAD_ARGS)pParameter;

	strcpy( szTarget,     pThreadArgs->Target );
	strcpy( szDictionary, pThreadArgs->Dictionary );
	strcpy( szUsername,   pThreadArgs->Username );
	strcpy( szPassword,   pThreadArgs->Password );

	bMultipleHosts = pThreadArgs->MultipleHosts;

	HeapFree( GetProcessHeap(), 0, pThreadArgs );

	if ( bMultipleHosts )
	{
		printf( "Spawning thread for host %s...\n", szTarget );

		fflush( stdout );
	}

	if ( strcmp( szUsername, "+" ) == 0 && strcmp( szPassword, "+" ) == 0 )
	{
		StartPasswordCheck( szTarget, szDictionary, &bMultipleHosts );
	}
	else
	{
		if ( Connect( szTarget, szUsername, szPassword, &bMultipleHosts ) )
		{
			StartPasswordCheck( szTarget, szDictionary, &bMultipleHosts );

			Disconnect( szTarget, &bMultipleHosts );
		}
	}

	WaitForSingleObject( hSemaphore, INFINITE );

	nThreads--;

	ReleaseSemaphore( hSemaphore, 1, NULL );

	_endthread();
}

VOID StartPasswordCheck( CHAR szTarget[], CHAR szDictionary[], BOOL *bMultipleHosts )
{
	BOOL            bContinue;
	CHAR   szLockoutThreshold[ 128 ];
	CHAR          szCacheFile[ 128 ];
	FILE          *pCacheFile;
	CHAR           szUsername[ 128 ];
	BOOL       bAccountLocked;
	BOOL     bAccountDisabled;
	BOOL  bPasswordCantChange;
	DWORD        dwBadPWCount;
	CHAR       szAccountFlags[ 128 ];
	FILE       *pPasswordFile;
	CHAR           szPassword[ 128 ];
	CHAR        szTmpPassword[ 128 ];
	CHAR        szTmpUsername[ 128 ];

	if ( !*bMultipleHosts )
	{
		printf( " -> Getting account list from host %s...\n", szTarget );

		fflush( stdout );
	}

	GetAccountLockoutThreshold( szTarget, szLockoutThreshold, bMultipleHosts );

	GetUserList( szTarget, bMultipleHosts );

	bContinue = TRUE;

	sprintf( szCacheFile, "UserCache\\%s.users", szTarget );

	pCacheFile = fopen( szCacheFile, "r" );

	if ( pCacheFile != NULL )
	{
		while ( bContinue && ( fgets( szUsername, sizeof( szUsername ), pCacheFile ) != NULL ) )
		{
			if ( szUsername[0] != '\n' )
			{
				if ( szUsername[strlen( szUsername ) - 1] == '\n' )
				{
					szUsername[strlen( szUsername ) - 1] = '\0';
				}

				GetUserInfo( szTarget, szUsername, &bAccountLocked, &bAccountDisabled, &bPasswordCantChange, &dwBadPWCount, bMultipleHosts );

				if ( !*bMultipleHosts )
				{
					printf( " -> Account %s information...\n", szUsername );

					strcpy( szAccountFlags, "" );

					if ( bAccountLocked )
					{
						sprintf( szAccountFlags, "%s(Locked out) ", szAccountFlags );
					}

					if ( bAccountDisabled )
					{
						sprintf( szAccountFlags, "%s(Disabled) ", szAccountFlags );
					}

					if ( bPasswordCantChange )
					{
						sprintf( szAccountFlags, "%s(Cannot change password) ", szAccountFlags );
					}

					printf( "    - Account flags:      %s\n", szAccountFlags );
					printf( "    - Bad password count: %d\n", dwBadPWCount );
					printf( "    - Lockout threshold:  %s\n", szLockoutThreshold );
					printf( "\n" );

					fflush( stdout );
				}

				WriteToLog( szTarget, szUsername, szAccountFlags, &dwBadPWCount, szLockoutThreshold );

				if ( !bAccountLocked && !bAccountDisabled && !bPasswordCantChange )
				{
					pPasswordFile = fopen( szDictionary, "r" );

					if ( pPasswordFile != NULL )
					{
						while ( bContinue && ( fgets( szPassword, sizeof( szPassword ), pPasswordFile ) != NULL ) )
						{
							if ( szPassword[0] != '#' && szPassword[0] != '\n' )
							{
								if ( szPassword[strlen( szPassword ) - 1] == '\n' )
								{
									szPassword[strlen( szPassword ) - 1] = '\0';
								}

								strcpy( szTmpPassword, szPassword );

								strupr( szTmpPassword );

								if ( strcmp( szTmpPassword, "<LCUSERNAME>" ) == 0 )
								{
									strcpy( szTmpUsername, szUsername );

									strlwr( szTmpUsername );

									strcpy( szPassword, szTmpUsername );
								}

								if ( strcmp( szTmpPassword, "<UCUSERNAME>" ) == 0 )
								{
									strcpy( szTmpUsername, szUsername );

									strupr( szTmpUsername );

									strcpy( szPassword, szTmpUsername );
								}

								if ( strcmp( szTmpPassword, "<USERNAME>" ) == 0 )
								{
									strcpy( szPassword, szUsername );
								}

								if ( strcmp( szTmpPassword, "<BLANK>" ) == 0 )
								{
									strcpy( szPassword, "" );
								}

								if ( !*bMultipleHosts )
								{
									if ( strcmp( szPassword, "" ) == 0 )
									{
										printf( "    - Trying password:    <blank>\n" );
									}
									else
									{
										printf( "    - Trying password:    %s\n", szPassword );
									}

									fflush( stdout );
								}

								if ( CheckPassword( szTarget, szUsername, szPassword, bMultipleHosts ) )
								{
									if ( strcmp( szPassword, "" ) == 0 )
									{
										if ( !*bMultipleHosts )
										{
											printf( "\n" );
											printf( "PASSWORD FOUND! %s / <blank>\n", szUsername );

											fflush( stdout );
										}

										LogGuessedPassword( szTarget, szUsername, "<blank>" );
									}
									else
									{
										if ( !*bMultipleHosts )
										{
											printf( "\n" );
											printf( "PASSWORD FOUND! %s / %s\n", szUsername, szPassword );

											fflush( stdout );
										}

										LogGuessedPassword( szTarget, szUsername, szPassword );
									}

									break;
								}

								if ( strcmp( szLockoutThreshold, "None" ) != 0 )
								{
									if ( !ResetAccountAttributes( szTarget, szUsername, bMultipleHosts ) )
									{
										bContinue = FALSE;
									}
								}
							}
						}

						if ( !*bMultipleHosts )
						{
							printf( "\n" );

							fflush( stdout );
						}

						fclose( pPasswordFile );
					}
				}
			}
		}

		fclose( pCacheFile );
	}
}

BOOL Connect( CHAR szTarget[], CHAR szUsername[], CHAR szPassword[], BOOL *bMultipleHosts )
{
	BOOL                  bReturn;
	CHAR               *pLocation;
	CHAR             szTempTarget[ 128 ];
	CHAR             szRemoteName[ 128 ];
	DWORD              dwLocation;
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
		dwLocation = (INT)( pLocation - szUsername + 1 );

		i = 0;

		while ( i < dwLocation - 1 )
		{
			szDomainName[i] = szUsername[i];

			i++;
		}

		szDomainName[i] = '\0';

		i = strlen( szDomainName ) + 1;

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

VOID GetAccountLockoutThreshold( CHAR szTarget[], CHAR szLockoutThreshold[], BOOL *bMultipleHosts )
{
	CHAR                 *pLocation;
	CHAR               szTempTarget[ 128 ];
	WCHAR                 wszTarget[ 256 ];
	DWORD                   dwLevel;
	USER_MODALS_INFO_3       *pInfo;
	NET_API_STATUS          nStatus;
	DWORD                   dwError;

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

	dwLevel = 3;
	pInfo   = NULL;

	nStatus = NetUserModalsGet( wszTarget, dwLevel, (PBYTE *)&pInfo );

	if ( nStatus == NERR_Success )
	{
		if ( pInfo != NULL )
		{
			if ( pInfo->usrmod3_lockout_threshold == 0 )
			{
				strcpy( szLockoutThreshold, "None" );
			}
			else
			{
				sprintf( szLockoutThreshold, "%d attempts", pInfo->usrmod3_lockout_threshold );
			}

			NetApiBufferFree( pInfo );
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

			WriteLastErrorToErrorLog( szTarget, "NetUserModalGet", &dwError, bMultipleHosts );
		}
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

VOID GetUserList( CHAR szTarget[], BOOL *bMultipleHosts )
{
	CHAR               *pLocation;
	CHAR             szTempTarget[ 128 ];
	CHAR              szCacheFile[ 128 ];
	WCHAR               wszTarget[ 256 ];
	DWORD                       i;
	DWORD                 dwLevel;
	DWORD           dwEntriesRead;
	DWORD          dwTotalEntries;
	DWORD          dwResumeHandle;
	USER_INFO_3            *pInfo;
	NET_API_STATUS        nStatus;
	USER_INFO_3        *pTempInfo;
	DWORD                       j;
	CHAR               szUsername[ 128 ];
	FILE              *pCacheFile;
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

	sprintf( szCacheFile, "UserCache\\%s.users", szTarget );

	MultiByteToWideChar( CP_ACP, 0, szTempTarget, strlen( szTempTarget ) + 1, wszTarget, sizeof( wszTarget ) / sizeof( wszTarget[0] ) );

	i = 0;

	dwLevel        = 3;
	dwEntriesRead  = 0;
	dwTotalEntries = 0;
	dwResumeHandle = 0;

	do
	{
		pInfo = NULL;

		nStatus = NetUserEnum( wszTarget, dwLevel, FILTER_NORMAL_ACCOUNT, (PBYTE *)&pInfo, 4096, &dwEntriesRead, &dwTotalEntries, &dwResumeHandle );

		if ( nStatus == NERR_Success || nStatus == ERROR_MORE_DATA )
		{
			if ( pInfo != NULL )
			{
				pTempInfo = pInfo;

				for ( j = 0; j < dwEntriesRead; j++ )
				{
					WideCharToMultiByte( CP_ACP, 0, pTempInfo->usri3_name, -1, szUsername, 128, NULL, NULL );

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

VOID GetUserInfo( CHAR szTarget[], CHAR szUsername[], BOOL *bAccountLocked, BOOL *bAccountDisabled, BOOL *bPasswordCantChange, DWORD *dwBadPWCount, BOOL *bMultipleHosts )
{
	CHAR             *pLocation;
	CHAR           szTempTarget[ 128 ];
	WCHAR             wszTarget[ 256 ];
	WCHAR           wszUsername[ 256 ];
	DWORD               dwLevel;
	USER_INFO_2          *pInfo;
	NET_API_STATUS      nStatus;
	DWORD               dwError;

	pLocation = strstr( szTarget, "\\\\" );

	if ( pLocation != NULL )
	{
		strcpy( szTempTarget, szTarget );
	}
	else
	{
		sprintf( szTempTarget, "\\\\%s", szTarget );
	}

	MultiByteToWideChar( CP_ACP, 0, szTempTarget, strlen( szTempTarget ) + 1, wszTarget,   sizeof( wszTarget ) / sizeof( wszTarget[0] ) );
	MultiByteToWideChar( CP_ACP, 0, szUsername,   strlen( szUsername ) + 1,   wszUsername, sizeof( wszUsername ) / sizeof( wszUsername[0] ) );

	dwLevel = 2;

	pInfo = NULL;

	nStatus = NetUserGetInfo( wszTarget, wszUsername, dwLevel, (PBYTE *)&pInfo );

	if ( nStatus == NERR_Success )
	{
		if ( pInfo != NULL )
		{
			if ( pInfo->usri2_flags & UF_LOCKOUT )
			{
				*bAccountLocked = TRUE;
			}
			else
			{
				*bAccountLocked = FALSE;
			}

			if ( pInfo->usri2_flags & UF_ACCOUNTDISABLE )
			{
				*bAccountDisabled = TRUE;
			}
			else
			{
				*bAccountDisabled = FALSE;
			}

			if ( pInfo->usri2_flags & UF_PASSWD_CANT_CHANGE )
			{
				*bPasswordCantChange = TRUE;
			}
			else
			{
				*bPasswordCantChange = FALSE;
			}

			*dwBadPWCount = pInfo->usri2_bad_pw_count;

			NetApiBufferFree( pInfo );
		}
	}
	else
	{
		if ( nStatus == ERROR_ACCESS_DENIED )
		{
			WriteToErrorLog( szTarget, "NetUserGetInfo", "The user does not have access to the requested information.", bMultipleHosts );
		}
		else if ( nStatus == NERR_InvalidComputer )
		{
			WriteToErrorLog( szTarget, "NetUserGetInfo", "The computer name is invalid.", bMultipleHosts );
		}
		else if ( nStatus == NERR_UserNotFound )
		{
			WriteToErrorLog( szTarget, "NetUserGetInfo", "The user name could not be found.", bMultipleHosts );
		}
		else
		{
			dwError = nStatus;

			WriteLastErrorToErrorLog( szTarget, "NetUserGetInfo", &dwError, bMultipleHosts );
		}
	}
}

VOID WriteToLog( CHAR szTarget[], CHAR szUsername[], CHAR szAccountFlags[], DWORD *dwBadPWCount, CHAR szLockoutThreshold[] )
{
	FILE *pOutputFile;

	WaitForSingleObject( hSemaphore, INFINITE );

	pOutputFile = fopen( "WinPassSafeCheckLog.txt", "r" );

	if ( pOutputFile != NULL )
	{
		fclose( pOutputFile );
	}
	else
	{
		pOutputFile = fopen( "WinPassSafeCheckLog.txt", "w" );

		if ( pOutputFile != NULL )
		{
			fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
			fprintf( pOutputFile, "\n" );
			fprintf( pOutputFile, "Hostname\tUsername\tAccount Flags\tBad Password Count\tAccount Lockout Threshold\n" );

			fclose( pOutputFile );
		}
	}

	pOutputFile = fopen( "WinPassSafeCheckLog.txt", "a+" );

	if ( pOutputFile != NULL )
	{
		fprintf( pOutputFile, "%s\t%s\t%s\t%d\t%s\n", szTarget, szUsername, szAccountFlags, *dwBadPWCount, szLockoutThreshold );

		fclose( pOutputFile );
	}

	ReleaseSemaphore( hSemaphore, 1, NULL );
}

BOOL CheckPassword( CHAR szTarget[], CHAR szUsername[], CHAR szPassword[], BOOL *bMultipleHosts )
{
	BOOL                bReturn;
	CHAR             *pLocation;
	CHAR           szTempTarget[ 128 ];
	WCHAR             wszTarget[ 256 ];
	WCHAR           wszUsername[ 256 ];
	WCHAR           wszPassword[ 256 ];
	NET_API_STATUS      nStatus;
	DWORD               dwError;

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

	MultiByteToWideChar( CP_ACP, 0, szTempTarget, strlen( szTempTarget ) + 1, wszTarget,   sizeof( wszTarget ) / sizeof( wszTarget[0] ) );
	MultiByteToWideChar( CP_ACP, 0, szUsername,   strlen( szUsername ) + 1,   wszUsername, sizeof( wszUsername ) / sizeof( wszUsername[0] ) );
	MultiByteToWideChar( CP_ACP, 0, szPassword,   strlen( szPassword ) + 1,   wszPassword, sizeof( wszPassword ) / sizeof( wszPassword[0] ) );

	nStatus = NetUserChangePassword( wszTarget, wszUsername, wszPassword, wszPassword );

	if ( nStatus == NERR_Success )
	{
		bReturn = TRUE;
	}
	else
	{
		if ( nStatus == ERROR_ACCESS_DENIED )
		{
			WriteToPasswordErrorLog( szTarget, szUsername, szPassword, "The user does not have access to the requested information.", bMultipleHosts );
		}
		else if ( nStatus == ERROR_INVALID_PASSWORD )
		{
			WriteToPasswordErrorLog( szTarget, szUsername, szPassword, "The user has entered an invalid password.", bMultipleHosts );
		}
		else if ( nStatus == NERR_InvalidComputer )
		{
			WriteToPasswordErrorLog( szTarget, szUsername, szPassword, "The computer name is invalid.", bMultipleHosts );
		}
		else if ( nStatus == NERR_NotPrimary )
		{
			WriteToPasswordErrorLog( szTarget, szUsername, szPassword, "The operation is allowed only on the primary domain controller of the domain.", bMultipleHosts );
		}
		else if ( nStatus == NERR_UserNotFound )
		{
			WriteToPasswordErrorLog( szTarget, szUsername, szPassword, "The user name could not be found.", bMultipleHosts );
		}
		else if ( nStatus == NERR_PasswordTooShort )
		{
			WriteToPasswordErrorLog( szTarget, szUsername, szPassword, "The password is shorter than required.", bMultipleHosts );
		}
		else
		{
			dwError = nStatus;

			WriteLastErrorToPasswordErrorLog( szTarget, szUsername, szPassword, &dwError, bMultipleHosts );
		}
	}

	return bReturn;
}

VOID WriteToPasswordErrorLog( CHAR szTarget[], CHAR szUsername[], CHAR szPassword[], CHAR szErrorMsg[], BOOL *bMultipleHosts )
{
	FILE *pOutputFile;

	WaitForSingleObject( hSemaphore, INFINITE );

	pOutputFile = fopen( "PasswordErrorLog.txt", "r" );

	if ( pOutputFile != NULL )
	{
		fclose( pOutputFile );
	}
	else
	{
		pOutputFile = fopen( "PasswordErrorLog.txt", "w" );

		if ( pOutputFile != NULL )
		{
			fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
			fprintf( pOutputFile, "\n" );
			fprintf( pOutputFile, "Hostname\tUsername\tPassword\tError Number\tError Message\n" );

			fclose( pOutputFile );
		}
	}

	pOutputFile = fopen( "PasswordErrorLog.txt", "a+" );

	if ( pOutputFile != NULL )
	{
		fprintf( pOutputFile, "%s\t%s\t%s\t-\t%s\n", szTarget, szUsername, szPassword, szErrorMsg );

		fclose( pOutputFile );
	}

	ReleaseSemaphore( hSemaphore, 1, NULL );
}

VOID WriteLastErrorToPasswordErrorLog( CHAR szTarget[], CHAR szUsername[], CHAR szPassword[], DWORD *dwError, BOOL *bMultipleHosts )
{
	DWORD     dwReturn;
	CHAR    szErrorMsg[ 128 ];
	CHAR     *pNewLine;
	FILE  *pOutputFile;

	dwReturn = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, *dwError, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), szErrorMsg, sizeof( szErrorMsg ), NULL );

	if ( dwReturn > 0 )
	{
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

	pOutputFile = fopen( "PasswordErrorLog.txt", "r" );

	if ( pOutputFile != NULL )
	{
		fclose( pOutputFile );
	}
	else
	{
		pOutputFile = fopen( "PasswordErrorLog.txt", "w" );

		if ( pOutputFile != NULL )
		{
			fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
			fprintf( pOutputFile, "\n" );
			fprintf( pOutputFile, "Hostname\tUsername\tPassword\tError Number\tError Message\n" );

			fclose( pOutputFile );
		}
	}

	pOutputFile = fopen( "PasswordErrorLog.txt", "a+" );

	if ( pOutputFile != NULL )
	{
		fprintf( pOutputFile, "%s\t%s\t%s\t%d\t%s\n", szTarget, szUsername, szPassword, *dwError, szErrorMsg );

		fclose( pOutputFile );
	}

	ReleaseSemaphore( hSemaphore, 1, NULL );
}

VOID LogGuessedPassword( CHAR szTarget[], CHAR szUsername[], CHAR szPassword[] )
{
	FILE *pOutputFile;

	WaitForSingleObject( hSemaphore, INFINITE );

	pOutputFile = fopen( "GuessedPasswords.txt", "r" );

	if ( pOutputFile != NULL )
	{
		fclose( pOutputFile );
	}
	else
	{
		pOutputFile = fopen( "GuessedPasswords.txt", "w" );

		if ( pOutputFile != NULL )
		{
			fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
			fprintf( pOutputFile, "\n" );
			fprintf( pOutputFile, "Hostname\tUsername\tPassword\n" );

			fclose( pOutputFile );
		}
	}

	pOutputFile = fopen( "GuessedPasswords.txt", "a+" );

	if ( pOutputFile != NULL )
	{
		fprintf( pOutputFile, "%s\t%s\t%s\n", szTarget, szUsername, szPassword );

		fclose( pOutputFile );
	}

	ReleaseSemaphore( hSemaphore, 1, NULL );
}

BOOL ResetAccountAttributes( CHAR szTarget[], CHAR szUsername[], BOOL *bMultipleHosts )
{
	CHAR                bReturn;
	CHAR             *pLocation;
	CHAR           szTempTarget[ 128 ];
	WCHAR             wszTarget[ 256 ];
	WCHAR           wszUsername[ 256 ];
	DWORD               dwLevel;
	USER_INFO_2          *pInfo;
	NET_API_STATUS      nStatus;
	USER_INFO_1008   ui1008Info;
	DWORD               dwError;

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

	MultiByteToWideChar( CP_ACP, 0, szTempTarget, strlen( szTempTarget ) + 1, wszTarget,   sizeof( wszTarget ) / sizeof( wszTarget[0] ) );
	MultiByteToWideChar( CP_ACP, 0, szUsername,   strlen( szUsername ) + 1,   wszUsername, sizeof( wszUsername ) / sizeof( wszUsername[0] ) );

	dwLevel = 2;
	pInfo   = NULL;

	nStatus = NetUserGetInfo( wszTarget, wszUsername, dwLevel, (PBYTE *)&pInfo );

	if ( nStatus == NERR_Success )
	{
		if ( pInfo != NULL )
		{
			dwLevel = 1008;

			ui1008Info.usri1008_flags = pInfo->usri2_flags;

			ui1008Info.usri1008_flags &= ~UF_LOCKOUT;

			nStatus = NetUserSetInfo( wszTarget, wszUsername, dwLevel, (BYTE *)&ui1008Info, NULL );

			if ( nStatus == NERR_Success )
			{
				bReturn = TRUE;
			}
			else
			{
				if ( nStatus == ERROR_ACCESS_DENIED )
				{
					WriteToErrorLog( szTarget, "NetUserSetInfo", "The user does not have access to the requested information.", bMultipleHosts );
				}
				else if ( nStatus == ERROR_INVALID_PARAMETER )
				{
					WriteToErrorLog( szTarget, "NetUserSetInfo", "One of the function parameters is invalid.", bMultipleHosts );
				}
				else if ( nStatus == NERR_InvalidComputer )
				{
					WriteToErrorLog( szTarget, "NetUserSetInfo", "The computer name is invalid.", bMultipleHosts );
				}
				else if ( nStatus == NERR_NotPrimary )
				{
					WriteToErrorLog( szTarget, "NetUserSetInfo", "The operation is allowed only on the primary domain controller of the domain.", bMultipleHosts );
				}
				else if ( nStatus == NERR_SpeGroupOp )
				{
					WriteToErrorLog( szTarget, "NetUserSetInfo", "The operation is not allowed on specified special groups, which are user groups, admin groups, local groups, or guest groups.", bMultipleHosts );
				}
				else if ( nStatus == NERR_LastAdmin )
				{
					WriteToErrorLog( szTarget, "NetUserSetInfo", "The operation is not allowed on the last administrative account.", bMultipleHosts );
				}
				else if ( nStatus == NERR_BadPassword )
				{
					WriteToErrorLog( szTarget, "NetUserSetInfo", "The share name or password is invalid.", bMultipleHosts );
				}
				else if ( nStatus == NERR_PasswordTooShort )
				{
					WriteToErrorLog( szTarget, "NetUserSetInfo", "The password is shorter than required.", bMultipleHosts );
				}
				else if ( nStatus == NERR_UserNotFound )
				{
					WriteToErrorLog( szTarget, "NetUserSetInfo", "The user name could not be found.", bMultipleHosts );
				}
				else
				{
					dwError = nStatus;

					WriteLastErrorToErrorLog( szTarget, "NetUserSetInfo", &dwError, bMultipleHosts );
				}
			}

			NetApiBufferFree( pInfo );
		}
	}
	else
	{
		if ( nStatus == ERROR_ACCESS_DENIED )
		{
			WriteToErrorLog( szTarget, "NetUserGetInfo", "The user does not have access to the requested information.", bMultipleHosts );
		}
		else if ( nStatus == NERR_InvalidComputer )
		{
			WriteToErrorLog( szTarget, "NetUserGetInfo", "The computer name is invalid.", bMultipleHosts );
		}
		else if ( nStatus == NERR_UserNotFound )
		{
			WriteToErrorLog( szTarget, "NetUserGetInfo", "The user name could not be found.", bMultipleHosts );
		}
		else
		{
			dwError = nStatus;

			WriteLastErrorToErrorLog( szTarget, "NetUserSetInfo", &dwError, bMultipleHosts );
		}
	}

	return bReturn;
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
