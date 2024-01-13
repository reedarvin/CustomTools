//
// gcc RegQueryInfo.c -o RegQueryInfo.exe -lmpr -ladvapi32
//

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <process.h>
#include <winnetwk.h>

#define MAX_THREADS 64

VOID     ThreadedSub( VOID *pParameter );
BOOL         Connect( CHAR *szTarget, CHAR *szUsername, CHAR *szPassword, BOOL *bMultipleHosts );
VOID RunRegQueryInfo( CHAR *szTarget, BOOL *bMultipleHosts );
VOID            Trim( CHAR *szText );
BOOL           Split( CHAR *szText, CHAR *szFindText, CHAR *szOutputText1, CHAR *szOutputText2 );
VOID      Disconnect( CHAR *szTarget, BOOL *bMultipleHosts );

typedef struct _THREAD_ARGS
{
	CHAR        Target[ 128 ];
	CHAR      Username[ 128 ];
	CHAR      Password[ 128 ];
	BOOL MultipleHosts;
} THREAD_ARGS, *PTHREAD_ARGS;

HANDLE hSemaphore;

INT nThreads = 0;

INT main( INT argc, CHAR *argv[] )
{
	CHAR  szTargetInput[ 128 ];
	CHAR     szUsername[ 128 ];
	CHAR     szPassword[ 128 ];
	FILE    *pInputFile;
	CHAR     szReadLine[ 128 ];
	CHAR       szTarget[ 128 ];
	CHAR szFullUsername[ 128 ];

	PTHREAD_ARGS pThreadArgs;

	hSemaphore = CreateSemaphore( NULL, 1, 1, NULL );

	if ( argc == 4 )
	{
		strcpy( szTargetInput, argv[1] );
		strcpy( szUsername,    argv[2] );
		strcpy( szPassword,    argv[3] );

		printf( "Running RegQueryInfo v1.0 with the following arguments:\n" );
		printf( "[+] Host Input:   \"%s\"\n", szTargetInput );
		printf( "[+] Username:     \"%s\"\n", szUsername );
		printf( "[+] Password:     \"%s\"\n", szPassword );
		printf( "[+] # of Threads: \"64\"\n" );
		printf( "\n" );

		pInputFile = fopen( szTargetInput, "r" );

		if ( pInputFile != NULL )
		{
			while ( fscanf( pInputFile, "%s", szReadLine ) != EOF )
			{
				if ( strstr( szUsername, "\\" ) == NULL && strcmp( szUsername, "+" ) != 0 && strcmp( szUsername, "" ) != 0 )
				{
					strcpy( szTarget, szReadLine );

					sprintf( szFullUsername, "%s\\%s", szTarget, szUsername );

					while ( nThreads >= MAX_THREADS )
					{
						Sleep( 200 );
					}

					pThreadArgs = (PTHREAD_ARGS)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( THREAD_ARGS ) );

					if ( pThreadArgs != NULL )
					{
						strcpy( pThreadArgs->Target,   szTarget );
						strcpy( pThreadArgs->Username, szFullUsername );
						strcpy( pThreadArgs->Password, szPassword );

						pThreadArgs->MultipleHosts = TRUE;

						WaitForSingleObject( hSemaphore, INFINITE );

						nThreads++;

						ReleaseSemaphore( hSemaphore, 1, NULL );

						_beginthread( ThreadedSub, 0, (VOID *)pThreadArgs );
					}
				}
				else
				{
					strcpy( szTarget, szReadLine );

					while ( nThreads >= MAX_THREADS )
					{
						Sleep( 200 );
					}

					pThreadArgs = (PTHREAD_ARGS)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( THREAD_ARGS ) );

					if ( pThreadArgs != NULL )
					{
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
			}

			fclose( pInputFile );

			Sleep( 1000 );

			printf( "Waiting for threads to terminate...\n" );
		}
		else
		{
			if ( strstr( szUsername, "\\" ) == NULL && strcmp( szUsername, "+" ) != 0 && strcmp( szUsername, "" ) != 0 )
			{
				strcpy( szTarget, szTargetInput );

				sprintf( szFullUsername, "%s\\%s", szTarget, szUsername );

				pThreadArgs = (PTHREAD_ARGS)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( THREAD_ARGS ) );

				if ( pThreadArgs != NULL )
				{
					strcpy( pThreadArgs->Target,   szTarget );
					strcpy( pThreadArgs->Username, szFullUsername );
					strcpy( pThreadArgs->Password, szPassword );

					pThreadArgs->MultipleHosts = FALSE;

					WaitForSingleObject( hSemaphore, INFINITE );

					nThreads++;

					ReleaseSemaphore( hSemaphore, 1, NULL );

					_beginthread( ThreadedSub, 0, (VOID *)pThreadArgs );
				}
			}
			else
			{
				strcpy( szTarget, szTargetInput );

				pThreadArgs = (PTHREAD_ARGS)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( THREAD_ARGS ) );

				if ( pThreadArgs != NULL )
				{
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
		}

		while ( nThreads > 0 )
		{
			Sleep( 200 );
		}
	}
	else
	{
		printf( "RegQueryInfo v1.0 | http://reedarvin.thearvins.com/\n" );
		printf( "\n" );
		printf( "Usage: RegQueryInfo <hostname | ip input file> <username> <password>\n" );
		printf( "\n" );
		printf( "<hostname | ip input file>  -- required argument\n" );
		printf( "<username>                  -- required argument\n" );
		printf( "<password>                  -- required argument\n" );
		printf( "\n" );
		printf( "If the <username> and <password> arguments are both plus signs (+), the\n" );
		printf( "existing credentials of the user running this utility will be used.\n" );
		printf( "\n" );
		printf( "Examples:\n" );
		printf( "RegQueryInfo 10.10.10.10 + +\n" );
		printf( "RegQueryInfo 10.10.10.10 administrator password\n" );
		printf( "\n" );
		printf( "RegQueryInfo MyWindowsMachine + +\n" );
		printf( "RegQueryInfo MyWindowsMachine administrator password\n" );
		printf( "\n" );
		printf( "RegQueryInfo IPInputFile.txt + +\n" );
		printf( "RegQueryInfo IPInputFile.txt administrator password\n" );
		printf( "\n" );
		printf( "(Written by Reed Arvin | reedarvin@gmail.com)\n" );
	}

	CloseHandle( hSemaphore );

	return 0;
}

VOID ThreadedSub( VOID *pParameter )
{
	CHAR       szTarget[ 128 ];
	CHAR     szUsername[ 128 ];
	CHAR     szPassword[ 128 ];
	BOOL bMultipleHosts;

	PTHREAD_ARGS pThreadArgs;

	pThreadArgs = (PTHREAD_ARGS)pParameter;

	strcpy( szTarget,   pThreadArgs->Target );
	strcpy( szUsername, pThreadArgs->Username );
	strcpy( szPassword, pThreadArgs->Password );

	bMultipleHosts = pThreadArgs->MultipleHosts;

	HeapFree( GetProcessHeap(), 0, pThreadArgs );

	if ( bMultipleHosts == TRUE )
	{
		printf( "Spawning thread for host %s...\n", szTarget );
	}

	if ( strcmp( szUsername, "+" ) == 0 && strcmp( szPassword, "+" ) == 0 )
	{
		RunRegQueryInfo( szTarget, &bMultipleHosts );
	}
	else
	{
		if ( Connect( szTarget, szUsername, szPassword, &bMultipleHosts ) )
		{
			RunRegQueryInfo( szTarget, &bMultipleHosts );

			Disconnect( szTarget, &bMultipleHosts );
		}
	}

	WaitForSingleObject( hSemaphore, INFINITE );

	nThreads--;

	ReleaseSemaphore( hSemaphore, 1, NULL );

	_endthread();
}

BOOL Connect( CHAR szTarget[], CHAR szUsername[], CHAR szPassword[], BOOL *bMultipleHosts )
{
	CHAR        szRemoteName[ 128 ];
	NETRESOURCE           nr;
	DWORD           dwResult;

	sprintf( szRemoteName, "\\\\%s\\IPC$", szTarget );

	nr.dwType       = RESOURCETYPE_ANY;
	nr.lpLocalName  = NULL;
	nr.lpRemoteName = szRemoteName;
	nr.lpProvider   = NULL;

	dwResult = WNetAddConnection2( &nr, szPassword, szUsername, 0 );

	if ( dwResult == NO_ERROR )
	{
		return TRUE;
	}
	else
	{
		if ( *bMultipleHosts == FALSE )
		{
			fprintf( stderr, "ERROR! Cannot connect to \\\\%s\\IPC$.\n", szTarget );
		}

		return FALSE;
	}
}

VOID RunRegQueryInfo( CHAR szTarget[], BOOL *bMultipleHosts )
{
	FILE  *pRegQueryKeysFile;
	CHAR       szQueryString[ 512 ];
	CHAR       szRegQueryKey[ 256 ];
	CHAR     szRegQueryValue[ 128 ];
	CHAR        szTempTarget[ 128 ];
	LONG             lReturn;
	HKEY                hKey;
	HKEY             hSubKey;
	CHAR          szRegValue[ 256 ];
	DWORD         dwRegValue;
	DWORD       dwBufferSize;
	DWORD             dwType;
	FILE        *pOutputFile;

	pRegQueryKeysFile = fopen( "RegQueryKeys.input", "r" );

	if ( pRegQueryKeysFile != NULL )
	{
		while ( fgets( szQueryString, sizeof( szQueryString ), pRegQueryKeysFile ) != NULL )
		{
			Trim( szQueryString );

			if ( szQueryString[0] != '#' && szQueryString[0] != '\n' )
			{
				if ( szQueryString[strlen( szQueryString ) - 1] == '\n' )
				{
					szQueryString[strlen( szQueryString ) - 1] = '\0';
				}

				if ( Split( szQueryString, ":", szRegQueryKey, szRegQueryValue ) )
				{
					Trim( szRegQueryKey );
					Trim( szRegQueryValue );

					sprintf( szTempTarget, "\\\\%s", szTarget );

					lReturn = RegConnectRegistry( szTempTarget, HKEY_LOCAL_MACHINE, &hKey );

					if ( lReturn == ERROR_SUCCESS )
					{
						lReturn = RegOpenKeyEx( hKey, szRegQueryKey, 0, KEY_QUERY_VALUE, &hSubKey );

						if ( lReturn == ERROR_SUCCESS )
						{
							strcpy( szRegValue, "" );

							dwBufferSize = 256;

							lReturn = RegQueryValueEx( hSubKey, szRegQueryValue, NULL, &dwType, (BYTE *)szRegValue, &dwBufferSize );

							if ( lReturn == ERROR_SUCCESS )
							{
								if ( dwType == REG_DWORD )
								{
									strcpy( szRegValue, "" );

									dwBufferSize = 256;

									lReturn = RegQueryValueEx( hSubKey, szRegQueryValue, NULL, NULL, (BYTE *)&dwRegValue, &dwBufferSize );

									if ( lReturn == ERROR_SUCCESS )
									{
										sprintf( szRegValue, "%d", dwRegValue );
									}
								}

								if ( *bMultipleHosts == FALSE )
								{
									printf( "%s -> Logging registry information.\n", szTarget );
								}

								WaitForSingleObject( hSemaphore, INFINITE );

								pOutputFile = fopen( "RegQueryInfo.txt", "r" );

								if ( pOutputFile != NULL )
								{
									fclose( pOutputFile );
								}
								else
								{
									pOutputFile = fopen( "RegQueryInfo.txt", "w" );

									if ( pOutputFile != NULL )
									{
										fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
										fprintf( pOutputFile, "\n" );
										fprintf( pOutputFile, "Hostname\tRegistry Key\tValue\n" );

										fclose( pOutputFile );
									}
								}

								pOutputFile = fopen( "RegQueryInfo.txt", "a+" );

								if ( pOutputFile != NULL )
								{
									fprintf( pOutputFile, "%s\tHKLM\\%s\\\\%s\t%s\n", szTarget, szRegQueryKey, szRegQueryValue, szRegValue );

									fclose( pOutputFile );
								}

								ReleaseSemaphore( hSemaphore, 1, NULL );
							}

							RegCloseKey( hSubKey );
						}

						RegCloseKey( hKey );
					}
					else
					{
						if ( *bMultipleHosts == FALSE )
						{
							fprintf( stderr, "ERROR! Cannot retrieve registry information from host %s.\n", szTarget );
						}
					}

				}
			}
		}

		fclose( pRegQueryKeysFile );
	}
	else
	{
		if ( *bMultipleHosts == FALSE )
		{
			fprintf( stderr, "ERROR! Cannot open RegQueryKeys.input file.\n" );
		}
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

BOOL Split( CHAR szText[], CHAR szFindText[], CHAR szOutputText1[], CHAR szOutputText2[] )
{
	BOOL  bFindTextLocated;
	CHAR    *pTextLocation;
	DWORD   dwTextLocation;
	DWORD                i;
	DWORD                j;

	bFindTextLocated = FALSE;

	pTextLocation = strstr( szText, szFindText );

	dwTextLocation = (INT)( pTextLocation - szText );

	if ( pTextLocation != NULL )
	{
		bFindTextLocated = TRUE;

		i = 0;

		while ( i < dwTextLocation )
		{
			szOutputText1[i] = szText[i];

			i++;
		}

		szOutputText1[i] = '\0';

		i = dwTextLocation + strlen( szFindText );

		j = 0;

		while ( i < strlen( szText ) )
		{
			szOutputText2[j] = szText[i];

			i++;
			j++;
		}

		szOutputText2[j] = '\0';
	}

	return bFindTextLocated;
}

VOID Disconnect( CHAR szTarget[], BOOL *bMultipleHosts )
{
	CHAR  szRemoteName[ 128 ];
	DWORD     dwResult;

	sprintf( szRemoteName, "\\\\%s\\IPC$", szTarget );

	dwResult = WNetCancelConnection2( szRemoteName, 0, TRUE );

	if ( dwResult != NO_ERROR )
	{
		if ( *bMultipleHosts == FALSE )
		{
			fprintf( stderr, "ERROR! Cannot disconnect from \\\\%s\\IPC$.\n", szTarget );
		}
	}
}

// Written by Reed Arvin | reedarvin@gmail.com
