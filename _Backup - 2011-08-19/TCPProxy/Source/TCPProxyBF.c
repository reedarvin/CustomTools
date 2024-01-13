//
// gcc source.c -o source.exe -lws2_32
//

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <process.h>
#include <winsock2.h>
#include <time.h>

#define MAX_THREADS 64

BOOL    IsTCPPort( DWORD *dwTCPPort );
BOOL  IsValidFile( CHAR *szCommandFile );
VOID  ThreadedSub( VOID *pParameter );
VOID   TCPConnect( CHAR *szTarget, DWORD *dwTCPPort, CHAR *szCommand, DWORD *dwCommandNum, BOOL *bGetBanner, BOOL *bMultipleHosts );
VOID GetWaitValue( DWORD *dwWait );
BOOL     IsTelnet( SOCKET *s, CHAR *szRecv, DWORD *dwWait, BOOL *bDisconnect );
VOID    LogToFile( CHAR *szTarget, DWORD *dwTCPPort, DWORD *dwCommandNum, CHAR *szLogText );
VOID      Replace( CHAR *szText, CHAR *szFindText, CHAR *szReplaceText );

typedef struct _THREAD_ARGS
{
	CHAR         Target[ 128 ];
	DWORD       TCPPort;
	CHAR        Command[ 128 ];
	DWORD    CommandNum;
	BOOL      GetBanner;
	BOOL  MultipleHosts;
} THREAD_ARGS, *PTHREAD_ARGS;

HANDLE hSemaphore;

INT nThreads = 0;

INT main( INT argc, CHAR *argv[] )
{
	DWORD       dwError;
	CHAR  szTargetInput[ 128 ];
	DWORD     dwTCPPort;
	CHAR  szCommandFile[ 128 ];
	BOOL     bGetBanner;
	CHAR      szTempArg[ 128 ];
	FILE    *pInputFile;
	CHAR     szReadLine[ 128 ];
	CHAR       szTarget[ 128 ];
	FILE  *pCommandFile;
	CHAR      szCommand[ 4096 ];
	DWORD             i;

	PTHREAD_ARGS pThreadArgs;

	hSemaphore = CreateSemaphore( NULL, 1, 1, NULL );

	if ( !CreateDirectory( "LogFiles", NULL ) )
	{
		dwError = GetLastError();

		if ( dwError != ERROR_ALREADY_EXISTS )
		{
			fprintf( stderr, "ERROR! Cannot create LogFiles directory.\n" );

			return 1;
		}
	}

	if ( argc > 3 && argc < 6 )
	{
		strcpy( szTargetInput, argv[1] );

		dwTCPPort = atoi( argv[2] );

		strcpy( szCommandFile, "" );

		printf( "Running TCPProxyBF v1.1 with the following arguments:\n" );
		printf( "[+] Host Input:   \"%s\"\n", szTargetInput );
		printf( "[+] TCP Port:     \"%d\"\n", dwTCPPort );

		if ( argc == 4 )
		{
			bGetBanner = TRUE;

			strcpy( szCommandFile, argv[3] );

			strupr( szCommandFile );

			printf( "[+] Get Banner:   \"Yes\"\n" );
			printf( "[+] Command File: \"%s\"\n", szCommandFile );
		}

		if ( argc == 5 )
		{
			strcpy( szCommandFile, argv[3] );
			strcpy( szTempArg,     argv[4] );

			strupr( szCommandFile );
			strupr( szTempArg );

			if ( strcmp( szTempArg, "-NOBANNER" ) == 0 )
			{
				bGetBanner = FALSE;

				printf( "[+] Get Banner:   \"No\"\n" );
			}
			else
			{
				bGetBanner = TRUE;

				printf( "[+] Get Banner:   \"Yes\"\n" );
			}

			printf( "[+] Command File: \"%s\"\n", szCommandFile );
		}

		printf( "[+] # of Threads: \"64\"\n" );
		printf( "\n" );

		if ( IsTCPPort( &dwTCPPort ) && IsValidFile( szCommandFile ) )
		{
			pInputFile = fopen( szTargetInput, "r" );

			if ( pInputFile != NULL )
			{
				while ( fscanf( pInputFile, "%s", szReadLine ) != EOF )
				{
					strcpy( szTarget, szReadLine );

					pCommandFile = fopen( szCommandFile, "r" );

					if ( pCommandFile != NULL )
					{
						i = 1;

						while ( fgets( szCommand, sizeof( szCommand ), pCommandFile ) != NULL )
						{
							if ( szCommand[0] != '#' && szCommand[0] != '\n' )
							{
								while ( nThreads >= MAX_THREADS )
								{
									Sleep( 200 );
								}

								pThreadArgs = (PTHREAD_ARGS)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( THREAD_ARGS ) );

								if ( pThreadArgs != NULL )
								{
									strcpy( pThreadArgs->Target, szTarget );

									pThreadArgs->TCPPort = dwTCPPort;

									strcpy( pThreadArgs->Command, szCommand );

									pThreadArgs->CommandNum    = i;
									pThreadArgs->GetBanner     = bGetBanner;
									pThreadArgs->MultipleHosts = TRUE;

									WaitForSingleObject( hSemaphore, INFINITE );

									nThreads++;

									ReleaseSemaphore( hSemaphore, 1, NULL );

									_beginthread( ThreadedSub, 0, (VOID *)pThreadArgs );
								}

								i++;
							}
						}

						fclose( pCommandFile );
					}
				}

				fclose( pInputFile );

				Sleep( 1000 );

				printf( "Waiting for threads to terminate...\n" );
			}
			else
			{
				strcpy( szTarget, szTargetInput );

				pCommandFile = fopen( szCommandFile, "r" );

				if ( pCommandFile != NULL )
				{
					i = 1;

					while ( fgets( szCommand, sizeof( szCommand ), pCommandFile ) != NULL )
					{
						if ( szCommand[0] != '#' && szCommand[0] != '\n' )
						{
							while ( nThreads >= MAX_THREADS )
							{
								Sleep( 200 );
							}

							pThreadArgs = (PTHREAD_ARGS)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( THREAD_ARGS ) );

							if ( pThreadArgs != NULL )
							{
								strcpy( pThreadArgs->Target, szTarget );

								pThreadArgs->TCPPort = dwTCPPort;

								strcpy( pThreadArgs->Command, szCommand );

								pThreadArgs->CommandNum    = i;
								pThreadArgs->GetBanner     = bGetBanner;
								pThreadArgs->MultipleHosts = FALSE;

								WaitForSingleObject( hSemaphore, INFINITE );

								nThreads++;

								ReleaseSemaphore( hSemaphore, 1, NULL );

								_beginthread( ThreadedSub, 0, (VOID *)pThreadArgs );
							}

							i++;
						}
					}

					fclose( pCommandFile );
				}
			}

			while ( nThreads > 0 )
			{
				Sleep( 200 );
			}
		}
	}
	else
	{
		printf( "TCPProxyBF v1.1 | Canaudit, Inc. http://www.canaudit.com/\n" );
		printf( "\n" );
		printf( "Usage: TCPProxyBF <ip address | ip input file> <tcp port> <command file> -nobanner\n" );
		printf( "\n" );
		printf( "<hostname | ip input file>  -- required argument\n" );
		printf( "<tcp port>                  -- required argument\n" );
		printf( "<command file>              -- required argument\n" );
		printf( "-nobanner                   -- optional argument\n" );
		printf( "\n" );
		printf( "Examples:\n" );
		printf( "TCPProxyBF 10.10.10.10 79 fingercommands.txt\n" );
		printf( "TCPProxyBF IPInputFile.txt 23 ciscocommands.txt\n" );
		printf( "TCPProxyBF IPInputFile.txt 80 ciscohttpcommands.txt -nobanner\n" );
		printf( "\n" );
		printf( "(Written by Reed Arvin | reed@canaudit.com)\n" );
	}

	CloseHandle( hSemaphore );

	return 0;
}

BOOL IsTCPPort( DWORD *dwTCPPort )
{
	if ( *dwTCPPort > 0 && *dwTCPPort < 65536 )
	{
		return TRUE;
	}
	else
	{
		fprintf( stderr, "ERROR! %d is not a valid TCP port.\n", *dwTCPPort );

		return FALSE;
	}
}

BOOL IsValidFile( CHAR szCommandFile[] )
{
	BOOL bValidFile;
	FILE     *pFile;

	bValidFile = TRUE;

	pFile = fopen( szCommandFile, "r" );

	if ( pFile != NULL )
	{
		fclose( pFile );
	}
	else
	{
		if ( strcmp( szCommandFile, "" ) != 0 )
		{
			fprintf( stderr, "ERROR! Cannot open command file %s\n", szCommandFile );

			bValidFile = FALSE;
		}
	}

	if ( bValidFile )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

VOID ThreadedSub( VOID *pParameter )
{
	CHAR        szTarget[ 128 ];
	DWORD      dwTCPPort;
	CHAR       szCommand[ 128 ];
	DWORD   dwCommandNum;
	BOOL      bGetBanner;
	BOOL  bMultipleHosts;

	PTHREAD_ARGS pThreadArgs;

	pThreadArgs = (PTHREAD_ARGS)pParameter;

	strcpy( szTarget, pThreadArgs->Target );

	dwTCPPort = pThreadArgs->TCPPort;

	strcpy( szCommand, pThreadArgs->Command );

	dwCommandNum   = pThreadArgs->CommandNum;
	bGetBanner     = pThreadArgs->GetBanner;
	bMultipleHosts = pThreadArgs->MultipleHosts;

	HeapFree( GetProcessHeap(), 0, pThreadArgs );

	if ( bMultipleHosts == TRUE )
	{
		printf( "Spawning thread for host %s command %d...\n", szTarget, dwCommandNum );
	}

	TCPConnect( szTarget, &dwTCPPort, szCommand, &dwCommandNum, &bGetBanner, &bMultipleHosts );

	WaitForSingleObject( hSemaphore, INFINITE );

	nThreads--;

	ReleaseSemaphore( hSemaphore, 1, NULL );

	_endthread();
}

VOID TCPConnect( CHAR szTarget[], DWORD *dwTCPPort, CHAR szCommand[], DWORD *dwCommandNum, BOOL *bGetBanner, BOOL *bMultipleHosts )
{
	BOOL        bPortOpen;
	INT           nResult;
	CHAR      szIPAddress[ 16 ];
	WSADATA       wsaData;
	BOOL      bDisconnect;
	SOCKET              s;
	LONG                l;
	DWORD          dwWait;
	fd_set       writefds;
	fd_set        readfds;
	BOOL        bContinue;
	DWORD               i;
	CHAR           szRecv[ 4096 ];
	CHAR    szTempCommand[ 4096 ];
	CHAR     szLogCommand[ 4096 ];

	struct hostent     *remoteHost;
	struct sockaddr_in         sin;
	struct timeval              tv;

	bPortOpen   = FALSE;
	bDisconnect = FALSE;

	nResult = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );

	if ( nResult == NO_ERROR )
	{
		remoteHost = gethostbyname( szTarget );

		if ( remoteHost != NULL )
		{
			strcpy( szIPAddress, inet_ntoa( *(struct in_addr *)remoteHost->h_addr_list[0] ) );

			s = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

			if ( s != INVALID_SOCKET )
			{
				sin.sin_family      = AF_INET;
				sin.sin_port        = htons( *dwTCPPort );
				sin.sin_addr.s_addr = inet_addr( szIPAddress );

				l = 1;

				if ( ioctlsocket( s, FIONBIO, &l ) == 0 )
				{
					GetWaitValue( &dwWait );

					connect( s, (struct sockaddr *)&sin, sizeof( sin ) );

					FD_ZERO( &writefds );
					FD_SET( s, &writefds );

					tv.tv_sec  = dwWait;
					tv.tv_usec = 0;

					if ( select( 0, NULL, &writefds, NULL, &tv ) > 0 )
					{
						bPortOpen = TRUE;

						if ( *bGetBanner )
						{
							FD_ZERO( &readfds );
							FD_SET( s, &readfds );

							tv.tv_sec  = dwWait;
							tv.tv_usec = 0;

							if ( select( 0, &readfds, NULL, NULL, &tv ) > 0 )
							{
								bContinue = TRUE;

								i = 0;

								while ( bContinue && !bDisconnect )
								{
									nResult = recv( s, szRecv, sizeof( szRecv ), 0 );

									if ( nResult > 0 )
									{
										szRecv[nResult] = '\0';

										if ( !IsTelnet( &s, szRecv, &dwWait, &bDisconnect ) )
										{
											if ( *bMultipleHosts == FALSE )
											{
												printf( "%s", szRecv );
											}

											LogToFile( szTarget, dwTCPPort, dwCommandNum, szRecv );

											i = 0;
										}
									}

									if ( nResult == 0 )
									{
										bDisconnect = TRUE;
									}

									if ( nResult == SOCKET_ERROR )
									{
										if ( WSAGetLastError() == 10035 )
										{
											if ( i > 50 )
											{
												bContinue = FALSE;
											}

											i++;

											Sleep( 10 );
										}
										else
										{
											bDisconnect = TRUE;
										}
									}
								}
							}
						}
					}
					else
					{
						bDisconnect = TRUE;
					}

					if ( szCommand[strlen( szCommand ) - 1] == '\n' )
					{
						szCommand[strlen( szCommand ) - 1] = '\0';
					}

					strcpy( szTempCommand, szCommand );

					if ( strcmp( szTempCommand, "\\r\\n" ) == 0 )
					{
						strcpy( szTempCommand, "" );
					}

					Replace( szTempCommand, "%IP%", szTarget );

					if ( *bMultipleHosts == FALSE )
					{
						printf( "\n[+] Sending string %s\\r\\n\n", szTempCommand );
						printf( "\n" );
					}

					sprintf( szLogCommand, "\n[+] Sending %s:%d string %s\\r\\n\n", szTarget, *dwTCPPort, szTempCommand );

					LogToFile( szTarget, dwTCPPort, dwCommandNum, szLogCommand );

					Replace( szTempCommand, "\\r", "\r" );
					Replace( szTempCommand, "\\n", "\n" );

					sprintf( szCommand, "%s\r\n", szTempCommand );

					FD_ZERO( &writefds );
					FD_SET( s, &writefds );

					tv.tv_sec  = dwWait;
					tv.tv_usec = 0;

					if ( select( 0, NULL, &writefds, NULL, &tv ) > 0 )
					{
						nResult = send( s, szCommand, strlen( szCommand ), 0 );

						if ( nResult != SOCKET_ERROR )
						{
							FD_ZERO( &readfds );
							FD_SET( s, &readfds );

							tv.tv_sec  = dwWait;
							tv.tv_usec = 0;

							if ( select( 0, &readfds, NULL, NULL, &tv ) > 0 )
							{
								bContinue = TRUE;

								i = 0;

								while ( bContinue && !bDisconnect )
								{
									nResult = recv( s, szRecv, sizeof( szRecv ), 0 );

									if ( nResult > 0 )
									{
										szRecv[nResult] = '\0';

										if ( *bMultipleHosts == FALSE )
										{
											printf( "%s", szRecv );
										}

										LogToFile( szTarget, dwTCPPort, dwCommandNum, szRecv );

										i = 0;
									}

									if ( nResult == 0 )
									{
										bDisconnect = TRUE;
									}

									if ( nResult == SOCKET_ERROR )
									{
										if ( WSAGetLastError() == 10035 )
										{
											if ( i > 50 )
											{
												bContinue = FALSE;
											}

											i++;

											Sleep( 10 );
										}
										else
										{
											bDisconnect = TRUE;
										}
									}
								}
							}
						}
						else
						{
							bDisconnect = TRUE;
						}
					}
					else
					{
						bDisconnect = TRUE;
					}
				}
			}

			closesocket( s );
		}
		else
		{
			if ( *bMultipleHosts == FALSE )
			{
				fprintf( stderr, "ERROR! Cannot resolve host %s to an IP address.\n", szTarget );
			}
		}
	}

	WSACleanup();

	if ( !bPortOpen )
	{
		if ( *bMultipleHosts == FALSE )
		{
			fprintf( stderr, "ERROR! Could not connect to %s over TCP port %d.\n", szTarget, *dwTCPPort );
		}
	}
}

VOID GetWaitValue( DWORD *dwWait )
{
	FILE *pInputFile;
	CHAR      szLine[ 1024 ];

	*dwWait = 5;

	pInputFile = fopen( "TCPProxy.ini", "r" );

	if ( pInputFile != NULL )
	{
		while ( fgets( szLine, sizeof( szLine ), pInputFile ) != NULL )
		{
			if ( szLine[0] != '#' && szLine[0] != '\n' )
			{
				if ( szLine[strlen( szLine ) - 1] == '\n' )
				{
					szLine[strlen( szLine ) - 1] = '\0';
				}

				*dwWait = atoi( szLine );

				break;
			}
		}

		fclose( pInputFile );
	}
}

BOOL IsTelnet( SOCKET *s, CHAR szRecv[], DWORD *dwWait, BOOL *bDisconnect )
{
	BOOL             bIsTelnet;
	unsigned char       *pRecv;
	DWORD         dwRecvLength;
	DWORD                    i;
	CHAR                szSend[ 4096 ];
	fd_set            writefds;
	INT                nResult;

	struct timeval tv;

	bIsTelnet = FALSE;

	pRecv = szRecv;

	dwRecvLength = strlen( szRecv );

	i = 0;

	while ( *pRecv == 255 )
	{
		bIsTelnet = TRUE;

		szSend[i] = *pRecv;

		pRecv++;

		i++;

		if ( ( *pRecv == 251 ) || ( *pRecv == 252 ) )
		{
			szSend[i] = 254;
		}
		else if ( ( *pRecv == 253 ) || ( *pRecv == 254 ) )
		{
			szSend[i] = 252;
		}
		else
		{
			break;
		}

		pRecv++;

		i++;

		szSend[i] = *pRecv;

		pRecv++;

		i++;
	}

	if ( bIsTelnet )
	{
		szSend[i] = '\0';

		FD_ZERO( &writefds );
		FD_SET( *s, &writefds );

		tv.tv_sec  = *dwWait;
		tv.tv_usec = 0;

		if ( select( 0, NULL, &writefds, NULL, &tv ) > 0 )
		{
			nResult = send( *s, szSend, strlen( szSend ), 0 );

			if ( nResult == SOCKET_ERROR )
			{
				*bDisconnect = TRUE;
			}
		}
		else
		{
			*bDisconnect = TRUE;
		}

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

VOID LogToFile( CHAR *szTarget, DWORD *dwTCPPort, DWORD *dwCommandNum, CHAR *szLogText )
{
	CHAR    szLogFile[ 128 ];
	FILE *pOutputFile;

	sprintf( szLogFile, "LogFiles\\%s-TCPPort%d-Cmd%d.txt", szTarget, *dwTCPPort, *dwCommandNum );

	WaitForSingleObject( hSemaphore, INFINITE );

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
			fclose( pOutputFile );
		}
	}

	pOutputFile = fopen( szLogFile, "a+" );

	if ( pOutputFile != NULL )
	{
		fprintf( pOutputFile, "%s", szLogText );

		fclose( pOutputFile );
	}

	ReleaseSemaphore( hSemaphore, 1, NULL );
}

VOID Replace( CHAR szText[], CHAR szFindText[], CHAR szReplaceText[] )
{
	CHAR   *pTextLocation;
	DWORD  dwTextLocation;
	DWORD   dwTempTextLen;
	CHAR       szTempText[ 4096 ];
	DWORD               i;
	DWORD               j;
	DWORD  dwLastLocation;

	pTextLocation = strstr( szText, szFindText );

	dwTextLocation = (INT)( pTextLocation - szText + 1 );

	dwTempTextLen = 0;

	while ( pTextLocation != NULL )
	{
		if ( dwTempTextLen < dwTextLocation )
		{
			i = 0;

			while ( i < dwTextLocation - 1 )
			{
				szTempText[i] = szText[i];

				i++;
			}

			szTempText[i] = '\0';

			strcat( szTempText, szReplaceText );

			dwTempTextLen = strlen( szTempText );

			i = i + strlen( szFindText );

			j = strlen( szTempText );

			while ( szText[i] != '\0' )
			{
				szTempText[j] = szText[i];

				i++;
				j++;
			}

			szTempText[j] = '\0';

			strcpy( szText, szTempText );
		}

		dwLastLocation = dwTextLocation;

		pTextLocation = strstr( szText, szFindText );

		dwTextLocation = (INT)( pTextLocation - szText + 1);

		if ( dwTextLocation == dwLastLocation )
		{
			break;
		}
	}
}

// Written by Reed Arvin | reed@canaudit.com
