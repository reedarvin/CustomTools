//
// gcc source.c -o source.exe -ladvapi32 -lnetapi32
//

#define _WIN32_WINNT 0x0500
#define WINVER       0x0500

#include <windows.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h>
#include <process.h>
#include <stdlib.h>
#include <time.h>
#include <ntsecapi.h>
#include <sddl.h>
#include <lm.h>

#define MAX_THREADS 64

VOID                    Usage( VOID );
VOID              ThreadedSub( VOID *pParameter );
VOID               WriteToLog( CHAR *szTarget, CHAR *szChangeType, CHAR *szInformation );
BOOL            GetMachineSID( CHAR *szTarget, CHAR *szMachineSID, BOOL *bMultipleHosts );
VOID WriteLastErrorToErrorLog( CHAR *szTarget, CHAR *szFunction, DWORD *dwError, BOOL *bMultipleHosts );
BOOL    GetAccountNameFromSID( CHAR *szTarget, CHAR *szStringSID, CHAR *szAccountName, BOOL *bMultipleHosts );
BOOL         GetAccountStatus( CHAR *szTarget, CHAR *szAccountName, BOOL *bAccountEnabled, BOOL *bMultipleHosts );
VOID          WriteToErrorLog( CHAR *szTarget, CHAR *szFunction, CHAR *szErrorMsg, BOOL *bMultipleHosts );
BOOL       SetAccountPassword( CHAR *szTarget, CHAR *szAccountName, CHAR *szPassword, BOOL *bMultipleHosts );
BOOL       SetAccountDisabled( CHAR *szTarget, CHAR *szAccountName, BOOL *bMultipleHosts );
BOOL           SetAccountName( CHAR *szTarget, CHAR *szAccountName, CHAR *szNewAccountName, BOOL *bMultipleHosts );

typedef struct _THREAD_ARGS
{
	CHAR               Target[ 128 ];
	BOOL   RenameAdminAccount;
	CHAR     AdminAccountName[ 128 ];
	BOOL   RenameGuestAccount;
	CHAR     GuestAccountName[ 128 ];
	BOOL  DisableGuestAccount;
	BOOL          SetPassword;
	CHAR             Password[ 128 ];
	BOOL    SetRandomPassword;
	DWORD      PasswordLength;
	BOOL        MultipleHosts;
} THREAD_ARGS, *PTHREAD_ARGS;

HANDLE hSemaphore;

INT nThreads = 0;

INT main( INT argc, CHAR *argv[] )
{
	BOOL             bContinue;
	BOOL   bRenameAdminAccount;
	BOOL   bRenameGuestAccount;
	BOOL  bDisableGuestAccount;
	BOOL          bSetPassword;
	BOOL    bSetRandomPassword;
	BOOL    bSetPasswordLength;
	CHAR    *pAdminAccountName;
	CHAR    *pGuestAccountName;
	CHAR            *pPassword;
	CHAR      *pPasswordLength;
	INT                nGetOpt;
	DWORD     dwPasswordLength;
	DWORD                    i;
	CHAR         szTargetInput[ 128 ];
	FILE           *pInputFile;
	CHAR            szReadLine[ 128 ];
	CHAR              szTarget[ 128 ];

	PTHREAD_ARGS pThreadArgs;

	hSemaphore = CreateSemaphore( NULL, 1, 1, NULL );

	opterr = 0;

	bContinue = TRUE;

	bRenameAdminAccount  = FALSE;
	bRenameGuestAccount  = FALSE;
	bDisableGuestAccount = FALSE;
	bSetPassword         = FALSE;
	bSetRandomPassword   = FALSE;
	bSetPasswordLength   = FALSE;

	pAdminAccountName = NULL;
	pGuestAccountName = NULL;
	pPassword         = NULL;
	pPasswordLength   = NULL;

	while ( bContinue )
	{
		nGetOpt = getopt( argc, argv, "a:g:dp:rl:" );

		if ( nGetOpt == -1 )
		{
			bContinue = FALSE;
		}

		switch ( nGetOpt )
		{
			case 'a':
				bRenameAdminAccount = TRUE;

				pAdminAccountName = optarg;

				break;

			case 'g':
				bRenameGuestAccount = TRUE;

				pGuestAccountName = optarg;

				break;

			case 'd':
				bDisableGuestAccount = TRUE;

				break;

			case 'p':
				bSetPassword = TRUE;

				pPassword = optarg;

				break;

			case 'r':
				bSetRandomPassword = TRUE;

				break;

			case 'l':
				bSetPasswordLength = TRUE;

				pPasswordLength = optarg;

				dwPasswordLength = atoi( pPasswordLength );

				break;
		}
	}

	if ( bRenameAdminAccount && ( pAdminAccountName == NULL ) )
	{
		Usage();

		return 1;
	}

	if ( bRenameGuestAccount && ( pGuestAccountName == NULL ) )
	{
		Usage();

		return 1;
	}

	if ( !bSetPassword && ( !bSetRandomPassword && !bSetPasswordLength ) )
	{
		Usage();

		return 1;
	}

	if ( bSetPassword && ( bSetRandomPassword || bSetPasswordLength ) )
	{
		Usage();

		return 1;
	}

	if ( ( bSetRandomPassword && !bSetPasswordLength ) || ( !bSetRandomPassword && bSetPasswordLength ) )
	{
		Usage();

		return 1;
	}

	if ( bSetPasswordLength && ( dwPasswordLength < 1 ) )
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

	printf( "Running WinPassWarrior v1.0 with the following arguments:\n" );
	printf( "[+] Target Host(s):           \"%s\"\n", szTargetInput );

	if ( bRenameAdminAccount )
	{
		printf( "[+] Rename Admin Account:     \"%s\"\n", pAdminAccountName );
	}

	if ( bRenameGuestAccount )
	{
		printf( "[+] Rename Guest Account:     \"%s\"\n", pGuestAccountName );
	}

	if ( bDisableGuestAccount )
	{
		printf( "[+] Disable Guest Account:    \"Yes\"\n" );
	}

	if ( bSetPassword )
	{
		printf( "[+] Set Admin/Guest Password: \"%s\"\n", pPassword );
	}

	if ( bSetRandomPassword )
	{
		printf( "[+] Set Random Password:      \"%d characters\"\n", dwPasswordLength );
	}

	printf( "[+] # of Threads:             \"64\"\n" );
	printf( "[+] Output File:              \"WinPassWarriorLog.txt\"\n" );
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

				pThreadArgs->RenameAdminAccount = bRenameAdminAccount;

				if ( bRenameAdminAccount )
				{
					strcpy( pThreadArgs->AdminAccountName, pAdminAccountName );
				}
				else
				{
					strcpy( pThreadArgs->AdminAccountName, "" );
				}

				pThreadArgs->RenameGuestAccount = bRenameGuestAccount;

				if ( bRenameGuestAccount )
				{
					strcpy( pThreadArgs->GuestAccountName, pGuestAccountName );
				}
				else
				{
					strcpy( pThreadArgs->GuestAccountName, "" );
				}

				pThreadArgs->DisableGuestAccount = bDisableGuestAccount;
				pThreadArgs->SetPassword         = bSetPassword;

				if ( bSetPassword )
				{
					strcpy( pThreadArgs->Password, pPassword );
				}
				else
				{
					strcpy( pThreadArgs->Password, "" );
				}

				pThreadArgs->SetRandomPassword = bSetRandomPassword;
				pThreadArgs->PasswordLength    = dwPasswordLength;
				pThreadArgs->MultipleHosts     = TRUE;

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

			pThreadArgs->RenameAdminAccount = bRenameAdminAccount;

			if ( bRenameAdminAccount )
			{
				strcpy( pThreadArgs->AdminAccountName, pAdminAccountName );
			}
			else
			{
				strcpy( pThreadArgs->AdminAccountName, "" );
			}

			pThreadArgs->RenameGuestAccount = bRenameGuestAccount;

			if ( bRenameGuestAccount )
			{
				strcpy( pThreadArgs->GuestAccountName, pGuestAccountName );
			}
			else
			{
				strcpy( pThreadArgs->GuestAccountName, "" );
			}

			pThreadArgs->DisableGuestAccount = bDisableGuestAccount;
			pThreadArgs->SetPassword         = bSetPassword;

			if ( bSetPassword )
			{
				strcpy( pThreadArgs->Password, pPassword );
			}
			else
			{
				strcpy( pThreadArgs->Password, "" );
			}

			pThreadArgs->SetRandomPassword = bSetRandomPassword;
			pThreadArgs->PasswordLength    = dwPasswordLength;
			pThreadArgs->MultipleHosts     = FALSE;

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
	printf( "WinPassWarrior v1.0 | Canaudit, Inc. http://www.canaudit.com/\n" );
	printf( "\n" );
	printf( "Usage: WinPassWarrior [-a:g:dp:rl:] <target host(s)>\n" );
	printf( "\n" );
	printf( "-a <account name>     -- Rename builtin administrator account\n" );
	printf( "-g <account name>     -- Rename builtin guest account\n" );
	printf( "-d                    -- Disable builtin guest account\n" );
	printf( "-p <password>         -- Set builtin administrator and guest password\n" );
	printf( "-r                    -- Randomize builtin administrator and guest password\n" );
	printf( "-l <password length>  -- Set length of randomized password\n" );
	printf( "\n" );
	printf( "<target host(s)>      -- Host Name, IP Address, or Input File\n" );
	printf( "\n" );
	printf( "Examples:\n" );
	printf( "WinPassWarrior -a Admin$XP -g Guest$XP -d -r -l 15 10.10.10.10\n" );
	printf( "WinPassWarrior -a Admin$XP -g Guest$XP -d -p MyP@$$w0rd 10.10.10.10\n" );
	printf( "\n" );
	printf( "WinPassWarrior -d -r -l 15 MyWindowsMachine\n" );
	printf( "WinPassWarrior -d -p MyP@$$w0rd MyWindowsMachine\n" );
	printf( "\n" );
	printf( "WinPassWarrior -a Admin$XP -d -r -l 15 IPInputFile.txt\n" );
	printf( "WinPassWarrior -a Admin$XP -d -p MyP@$$w0rd IPInputFile.txt\n" );
	printf( "\n" );
	printf( "(Written by Reed Arvin | reed@canaudit.com)\n" );

	fflush( stdout );
}

VOID ThreadedSub( VOID *pParameter )
{
	CHAR               szTarget[ 128 ];
	BOOL    bRenameAdminAccount;
	CHAR     szAdminAccountName[ 128 ];
	BOOL    bRenameGuestAccount;
	CHAR     szGuestAccountName[ 128 ];
	BOOL   bDisableGuestAccount;
	BOOL           bSetPassword;
	CHAR             szPassword[ 128 ];
	BOOL     bSetRandomPassword;
	BOOL     bSetPasswordLength;
	DWORD      dwPasswordLength;
	BOOL         bMultipleHosts;
	CHAR           szMachineSID[ 128 ];
	CHAR              szLogText[ 128 ];
	CHAR            szRID500SID[ 128 ];
	CHAR    szRID500AccountName[ 128 ];
	CHAR            szRID501SID[ 128 ];
	CHAR    szRID501AccountName[ 128 ];
	CHAR        szPasswordChars[ 128 ];
	DWORD            dwRangeMax;
	DWORD            dwRangeMin;
	DWORD                     i;
	DWORD                     j;
	BOOL  bRID501AccountEnabled;

	PTHREAD_ARGS pThreadArgs;

	pThreadArgs = (PTHREAD_ARGS)pParameter;

	strcpy( szTarget, pThreadArgs->Target );

	bRenameAdminAccount = pThreadArgs->RenameAdminAccount;

	strcpy( szAdminAccountName, pThreadArgs->AdminAccountName );

	bRenameGuestAccount = pThreadArgs->RenameGuestAccount;

	strcpy( szGuestAccountName, pThreadArgs->GuestAccountName );

	bDisableGuestAccount = pThreadArgs->DisableGuestAccount;
	bSetPassword         = pThreadArgs->SetPassword;

	strcpy( szPassword, pThreadArgs->Password );

	bSetRandomPassword = pThreadArgs->SetRandomPassword;
	dwPasswordLength   = pThreadArgs->PasswordLength;
	bMultipleHosts     = pThreadArgs->MultipleHosts;

	HeapFree( GetProcessHeap(), 0, pThreadArgs );

	if ( bMultipleHosts )
	{
		printf( "Spawning thread for host %s...\n", szTarget );

		fflush( stdout );
	}

	if ( GetMachineSID( szTarget, szMachineSID, &bMultipleHosts ) )
	{
		if ( !bMultipleHosts )
		{
			printf( " -> Machine SID is: %s\n", szMachineSID );

			fflush( stdout );
		}

		sprintf( szLogText, "Machine SID is %s", szMachineSID );

		WriteToLog( szTarget, "SID Query: Machine SID", szLogText );

		sprintf( szRID500SID, "%s-500", szMachineSID );

		if ( GetAccountNameFromSID( szTarget, szRID500SID, szRID500AccountName, &bMultipleHosts ) )
		{
			if ( !bMultipleHosts )
			{
				printf( " -> Builtin administrator account is: %s\n", szRID500AccountName );

				fflush( stdout );
			}

			sprintf( szLogText, "Builtin administrator account is %s", szRID500AccountName );

			WriteToLog( szTarget, "Name Query: Builtin Administrator Account", szLogText );
		}

		sprintf( szRID501SID, "%s-501", szMachineSID );

		if ( GetAccountNameFromSID( szTarget, szRID501SID, szRID501AccountName, &bMultipleHosts ) )
		{
			if ( !bMultipleHosts )
			{
				printf( " -> Builtin guest account is: %s\n", szRID501AccountName );

				fflush( stdout );
			}

			sprintf( szLogText, "Builtin guest account is %s", szRID501AccountName );

			WriteToLog( szTarget, "Name Query: Builtin Guest Account", szLogText );
		}

		if ( bSetPassword )
		{
			if ( SetAccountPassword( szTarget, szRID500AccountName, szPassword, &bMultipleHosts ) )
			{
				if ( !bMultipleHosts )
				{
					printf( " -> Password for account %s set to %s\n", szRID500AccountName, szPassword );

					fflush( stdout );
				}

				sprintf( szLogText, "Password for account %s set to %s", szRID500AccountName, szPassword );

				WriteToLog( szTarget, "Password Change: Builtin Administrator Account", szLogText );
			}

			if ( SetAccountPassword( szTarget, szRID501AccountName, szPassword, &bMultipleHosts ) )
			{
				if ( !bMultipleHosts )
				{
					printf( " -> Password for account %s set to %s\n", szRID501AccountName, szPassword );

					fflush( stdout );
				}

				sprintf( szLogText, "Password for account %s set to %s", szRID501AccountName, szPassword );

				WriteToLog( szTarget, "Password Change: Builtin Guest Account", szLogText );
			}
		}

		if ( bSetRandomPassword )
		{
			srand( (unsigned)time( NULL ) );

			strcpy( szPasswordChars, "~a!b@c#d$e%f^g&h*i(j)k_l+m`n-o=p{q}r|s:t\"u<v>w?x[y]z~A!B@C#D$E%F^G&H*I(J)K_L+M`N-O=P{Q}R|S:T\"U<V>W?X[Y]Z~0!1@2#3$4%5^6&7*8(9)" );

			dwRangeMax = strlen( szPasswordChars );
			dwRangeMin = 0;

			i = 0;

			while ( i < dwPasswordLength )
			{
				j = (double)rand() / ( RAND_MAX + 1 ) * ( dwRangeMax - dwRangeMin ) + dwRangeMin;

				szPassword[i] = szPasswordChars[j];

				i++;
			}

			szPassword[i] = '\0';

			if ( SetAccountPassword( szTarget, szRID500AccountName, szPassword, &bMultipleHosts ) )
			{
				if ( !bMultipleHosts )
				{
					printf( " -> Password for account %s set to %s\n", szRID500AccountName, szPassword );

					fflush( stdout );
				}

				sprintf( szLogText, "Password for account %s set to %s", szRID500AccountName, szPassword );

				WriteToLog( szTarget, "Password Change: Builtin Administrator Account", szLogText );
			}

			if ( SetAccountPassword( szTarget, szRID501AccountName, szPassword, &bMultipleHosts ) )
			{
				if ( !bMultipleHosts )
				{
					printf( " -> Password for account %s set to %s\n", szRID501AccountName, szPassword );

					fflush( stdout );
				}

				sprintf( szLogText, "Password for account %s set to %s", szRID501AccountName, szPassword );

				WriteToLog( szTarget, "Password Change: Builtin Guest Account", szLogText );
			}
		}

		if ( bDisableGuestAccount )
		{
			if ( GetAccountStatus( szTarget, szRID501AccountName, &bRID501AccountEnabled, &bMultipleHosts ) )
			{
				if ( bRID501AccountEnabled )
				{
					if ( !bMultipleHosts )
					{
						printf( " -> Builtin guest account %s is enabled\n", szRID501AccountName );

						fflush( stdout );
					}

					sprintf( szLogText, "Builtin guest account %s is enabled", szRID501AccountName );

					WriteToLog( szTarget, "Status Query: Builtin Guest Account", szLogText );

					if ( SetAccountDisabled( szTarget, szRID501AccountName, &bMultipleHosts ) )
					{
						if ( !bMultipleHosts )
						{
							printf( " -> Account %s has been disabled\n", szRID501AccountName );

							fflush( stdout );
						}

						sprintf( szLogText, "Account %s has been disabled", szRID501AccountName );

						WriteToLog( szTarget, "Account Status Change: Builtin Guest Account", szLogText );
					}
				}
				else
				{
					if ( !bMultipleHosts )
					{
						printf( " -> Builtin guest account %s is disabled\n", szRID501AccountName );

						fflush( stdout );
					}

					sprintf( szLogText, "Builtin guest account %s is disabled", szRID501AccountName );

					WriteToLog( szTarget, "Status Query: Builtin Guest Account", szLogText );
				}
			}
		}

		if ( bRenameAdminAccount )
		{
			if ( SetAccountName( szTarget, szRID500AccountName, szAdminAccountName, &bMultipleHosts ) )
			{
				if ( !bMultipleHosts )
				{
					printf( " -> Account %s has been renamed to %s\n", szRID500AccountName, szAdminAccountName );

					fflush( stdout );
				}

				sprintf( szLogText, "Account %s has been renamed to %s", szRID500AccountName, szAdminAccountName );

				WriteToLog( szTarget, "Account Rename: Builtin Administrator Account", szLogText );
			}
		}

		if ( bRenameGuestAccount )
		{
			if ( SetAccountName( szTarget, szRID501AccountName, szGuestAccountName, &bMultipleHosts ) )
			{
				if ( !bMultipleHosts )
				{
					printf( " -> Account %s has been renamed to %s\n", szRID501AccountName, szGuestAccountName );

					fflush( stdout );
				}

				sprintf( szLogText, "Account %s has been renamed to %s", szRID501AccountName, szGuestAccountName );

				WriteToLog( szTarget, "Account Rename: Builtin Guest Account", szLogText );
			}
		}
	}

	WaitForSingleObject( hSemaphore, INFINITE );

	nThreads--;

	ReleaseSemaphore( hSemaphore, 1, NULL );

	_endthread();
}

VOID WriteToLog( CHAR szTarget[], CHAR szChangeType[], CHAR szInformation[] )
{
	FILE *pOutputFile;

	WaitForSingleObject( hSemaphore, INFINITE );

	pOutputFile = fopen( "WinPassWarriorLog.txt", "r" );

	if ( pOutputFile != NULL )
	{
		fclose( pOutputFile );
	}
	else
	{
		pOutputFile = fopen( "WinPassWarriorLog.txt", "w" );

		if ( pOutputFile != NULL )
		{
			fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
			fprintf( pOutputFile, "\n" );
			fprintf( pOutputFile, "Hostname\tChange Type\tDetailed Information\n" );

			fclose( pOutputFile );
		}
	}

	pOutputFile = fopen( "WinPassWarriorLog.txt", "a+" );

	if ( pOutputFile != NULL )
	{
		fprintf( pOutputFile, "%s\t%s\t%s\n", szTarget, szChangeType, szInformation );

		fclose( pOutputFile );
	}

	ReleaseSemaphore( hSemaphore, 1, NULL );
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
		fprintf( pOutputFile, "%s\t%s\t%d\t%s\n", szTarget, szFunction, &dwError, szErrorMsg );

		fclose( pOutputFile );
	}

	ReleaseSemaphore( hSemaphore, 1, NULL );
}

BOOL GetAccountNameFromSID( CHAR szTarget[], CHAR szStringSID[], CHAR szAccountName[], BOOL *bMultipleHosts )
{
	BOOL                   bReturn;
	PSID                      pSID;
	DWORD            dwAccountName;
	CHAR         szTempAccountName[ 128 ];
	DWORD             dwDomainName;
	CHAR              szDomainName[ 128 ];
	SID_NAME_USE             snUse;
	DWORD                  dwError;

	bReturn = FALSE;

	if ( ConvertStringSidToSid( szStringSID, &pSID ) )
	{
		dwAccountName = sizeof( szTempAccountName );
		dwDomainName  = sizeof( szDomainName );

		if ( LookupAccountSid( szTarget, pSID, szTempAccountName, &dwAccountName, szDomainName, &dwDomainName, &snUse ) )
		{
			strcpy( szAccountName, szTempAccountName );

			bReturn = TRUE;
		}
		else
		{
			dwError = GetLastError();

			WriteLastErrorToErrorLog( szTarget, "LookupAccountSid", &dwError, bMultipleHosts );
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

BOOL GetAccountStatus( CHAR szTarget[], CHAR szAccountName[], BOOL *bAccountEnabled, BOOL *bMultipleHosts )
{
	BOOL                  bReturn;
	CHAR               *pLocation;
	CHAR             szTempTarget[ 128 ];
	WCHAR               wszTarget[ 256 ];
	WCHAR          wszAccountName[ 256 ];
	DWORD                 dwLevel;
	USER_INFO_1            *pInfo;
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

	MultiByteToWideChar( CP_ACP, 0, szTempTarget,  strlen( szTempTarget ) + 1,  wszTarget,      sizeof( wszTarget ) / sizeof( wszTarget[0] ) );
	MultiByteToWideChar( CP_ACP, 0, szAccountName, strlen( szAccountName ) + 1, wszAccountName, sizeof( wszAccountName ) / sizeof( wszAccountName[0] ) );

	dwLevel = 1;
	pInfo   = NULL;

	nStatus = NetUserGetInfo( wszTarget, wszAccountName, dwLevel, (PBYTE *)&pInfo );

	if ( nStatus == NERR_Success )
	{
		if ( pInfo != NULL )
		{
			if ( pInfo->usri1_flags & UF_ACCOUNTDISABLE )
			{
				*bAccountEnabled = FALSE;
			}
			else
			{
				*bAccountEnabled = TRUE;
			}

			bReturn = TRUE;

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

	return bReturn;
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

BOOL SetAccountPassword( CHAR szTarget[], CHAR szAccountName[], CHAR szPassword[], BOOL *bMultipleHosts )
{
	BOOL                  bReturn;
	CHAR               *pLocation;
	CHAR             szTempTarget[ 128 ];
	WCHAR               wszTarget[ 256 ];
	WCHAR          wszAccountName[ 256 ];
	WCHAR             wszPassword[ 256 ];
	DWORD                 dwLevel;
	USER_INFO_1003     ui1003Info;
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

	MultiByteToWideChar( CP_ACP, 0, szTempTarget,  strlen( szTempTarget ) + 1,  wszTarget,      sizeof( wszTarget ) / sizeof( wszTarget[0] ) );
	MultiByteToWideChar( CP_ACP, 0, szAccountName, strlen( szAccountName ) + 1, wszAccountName, sizeof( wszAccountName ) / sizeof( wszAccountName[0] ) );
	MultiByteToWideChar( CP_ACP, 0, szPassword,    strlen( szPassword ) + 1,    wszPassword,    sizeof( wszPassword ) / sizeof( wszPassword[0] ) );

	dwLevel = 1003;

	ui1003Info.usri1003_password = wszPassword;

	nStatus = NetUserSetInfo( wszTarget, wszAccountName, dwLevel, (BYTE *)&ui1003Info, NULL );

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

	return bReturn;
}

BOOL SetAccountDisabled( CHAR szTarget[], CHAR szAccountName[], BOOL *bMultipleHosts )
{
	BOOL                  bReturn;
	CHAR               *pLocation;
	CHAR             szTempTarget[ 128 ];
	WCHAR               wszTarget[ 256 ];
	WCHAR          wszAccountName[ 256 ];
	DWORD                 dwLevel;
	USER_INFO_1            *pInfo;
	NET_API_STATUS        nStatus;
	USER_INFO_1008     ui1008Info;
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

	MultiByteToWideChar( CP_ACP, 0, szTempTarget,  strlen( szTempTarget ) + 1,  wszTarget,      sizeof( wszTarget ) / sizeof( wszTarget[0] ) );
	MultiByteToWideChar( CP_ACP, 0, szAccountName, strlen( szAccountName ) + 1, wszAccountName, sizeof( wszAccountName ) / sizeof( wszAccountName[0] ) );

	dwLevel = 1;
	pInfo   = NULL;

	nStatus = NetUserGetInfo( wszTarget, wszAccountName, dwLevel, (PBYTE *)&pInfo );

	if ( nStatus == NERR_Success )
	{
		if ( pInfo != NULL )
		{
			dwLevel = 1008;

			ui1008Info.usri1008_flags = pInfo->usri1_flags;

			ui1008Info.usri1008_flags |= UF_ACCOUNTDISABLE;

			nStatus = NetUserSetInfo( wszTarget, wszAccountName, dwLevel, (BYTE *)&ui1008Info, NULL );

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

			WriteLastErrorToErrorLog( szTarget, "NetUserGetInfo", &dwError, bMultipleHosts );
		}
	}

	return bReturn;
}

BOOL SetAccountName( CHAR szTarget[], CHAR szAccountName[], CHAR szNewAccountName[], BOOL *bMultipleHosts )
{
	BOOL                     bReturn;
	CHAR                  *pLocation;
	CHAR                szTempTarget[ 128 ];
	WCHAR                  wszTarget[ 256 ];
	WCHAR             wszAccountName[ 256 ];
	WCHAR          wszNewAccountName[ 256 ];
	DWORD                    dwLevel;
	USER_INFO_0              ui0Info;
	NET_API_STATUS           nStatus;
	DWORD                    dwError;

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

	MultiByteToWideChar( CP_ACP, 0, szTempTarget,     strlen( szTempTarget ) + 1,     wszTarget,         sizeof( wszTarget ) / sizeof( wszTarget[0] ) );
	MultiByteToWideChar( CP_ACP, 0, szAccountName,    strlen( szAccountName ) + 1,    wszAccountName,    sizeof( wszAccountName ) / sizeof( wszAccountName[0] ) );
	MultiByteToWideChar( CP_ACP, 0, szNewAccountName, strlen( szNewAccountName ) + 1, wszNewAccountName, sizeof( wszNewAccountName ) / sizeof( wszNewAccountName[0] ) );

	dwLevel = 0;

	ui0Info.usri0_name = wszNewAccountName;

	nStatus = NetUserSetInfo( wszTarget, wszAccountName, dwLevel, (BYTE *)&ui0Info, NULL );

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

	return bReturn;
}
