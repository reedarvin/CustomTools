//
// gcc source.c d3des.c -o source.exe -lws2_32
//

#define _WIN32_WINNT 0x0500
#define WINVER       0x0500

#define STRICT
#define WIN32_LEAN_AND_MEAN

#define _WINSOCKAPI_

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <process.h>
#include <winsock2.h>
#include <time.h>
#include "d3des.h"

#define MAX_THREADS 4

BOOL              IsTCPPort( DWORD *dwTCPPort );
VOID                   Trim( CHAR *szText );
VOID            ThreadedSub( VOID *pParameter );
VOID             TCPConnect( CHAR *szTarget, DWORD *dwTCPPort, CHAR *szPassword, BOOL *bMultipleHosts );
VOID           GetWaitValue( DWORD *dwWait );
VOID         RecvSocketData( SOCKET *s, DWORD *dwWait, BOOL *bPortOpen, BOOL *bDisconnect, unsigned char *szFullRecv, DWORD *dwFullRecv );
VOID     SendRecvSocketData( SOCKET *s, DWORD *dwWait, BOOL *bPortOpen, BOOL *bDisconnect, unsigned char *szSend, DWORD *dwSend, unsigned char *szFullRecv, DWORD *dwFullRecv );
VOID          LogVNCBanners( CHAR *szTarget, CHAR *szVersion );
VOID            LogErrorMsg( CHAR *szTarget, CHAR *szVersion, CHAR *szErrorMsg );
VOID         LogVNCAuthType( CHAR *szTarget, CHAR *szAuthType );
VOID LogGuessedVNCPasswords( CHAR *szTarget, CHAR *szPassword );

typedef struct _THREAD_ARGS
{
	CHAR         Target[ 128 ];
	DWORD       TCPPort;
	CHAR       Password[ 128 ];
	BOOL  MultipleHosts;
} THREAD_ARGS, *PTHREAD_ARGS;

HANDLE hSemaphore;

INT nThreads = 0;

INT main( INT argc, CHAR *argv[] )
{
	CHAR    szTargetInput[ 128 ];
	DWORD       dwTCPPort;
	CHAR  szPasswordInput[ 128 ];
	FILE      *pInputFile;
	CHAR         szTarget[ 128 ];
	FILE       *pDictFile;
	CHAR       szPassword[ 128 ];

	PTHREAD_ARGS pThreadArgs;

	hSemaphore = CreateSemaphore( NULL, 1, 1, NULL );

	if ( argc == 4 )
	{
		strcpy( szTargetInput, argv[1] );

		dwTCPPort = atoi( argv[2] );

		strcpy( szPasswordInput, argv[3] );

		printf( "Running Godzilla v1.0 with the following arguments:\n" );
		printf( "[+] Host Input:     \"%s\"\n", szTargetInput );
		printf( "[+] TCP Port:       \"%d\"\n", dwTCPPort );
		printf( "[+] Password Input: \"%s\"\n", szPasswordInput );
		printf( "[+] # of Threads:   \"64\"\n" );
		printf( "\n" );

		if ( IsTCPPort( &dwTCPPort ) )
		{
			pInputFile = fopen( szTargetInput, "r" );

			if ( pInputFile != NULL )
			{
				while ( fscanf( pInputFile, "%s", szTarget ) != EOF )
				{
					pDictFile = fopen( szPasswordInput, "r" );

					if ( pDictFile != NULL )
					{
						while ( fgets( szPassword, sizeof( szPassword ), pDictFile ) != NULL )
						{
							Trim( szPassword );

							if ( szPassword[0] != '#' && szPassword[0] != '\n' )
							{
								if ( szPassword[strlen( szPassword ) - 1] == '\n' )
								{
									szPassword[strlen( szPassword ) - 1] = '\0';
								}

								while ( nThreads >= MAX_THREADS )
								{
									Sleep( 200 );
								}

								pThreadArgs = (PTHREAD_ARGS)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( THREAD_ARGS ) );

								if ( pThreadArgs != NULL )
								{
									strcpy( pThreadArgs->Target, szTarget );

									pThreadArgs->TCPPort = dwTCPPort;

									strcpy( pThreadArgs->Password, szPassword );

									pThreadArgs->MultipleHosts = TRUE;

									WaitForSingleObject( hSemaphore, INFINITE );

									nThreads++;

									ReleaseSemaphore( hSemaphore, 1, NULL );

									_beginthread( ThreadedSub, 0, (VOID *)pThreadArgs );
								}
							}
						}

						fclose( pDictFile );
					}
					else
					{
						strcpy( szPassword, szPasswordInput );

						while ( nThreads >= MAX_THREADS )
						{
							Sleep( 200 );
						}

						pThreadArgs = (PTHREAD_ARGS)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( THREAD_ARGS ) );

						if ( pThreadArgs != NULL )
						{
							strcpy( pThreadArgs->Target, szTarget );

							pThreadArgs->TCPPort = dwTCPPort;

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
				strcpy( szTarget, szTargetInput );

				pDictFile = fopen( szPasswordInput, "r" );

				if ( pDictFile != NULL )
				{
					while ( fgets( szPassword, sizeof( szPassword ), pDictFile ) != NULL )
					{
						Trim( szPassword );

						if ( szPassword[0] != '#' && szPassword[0] != '\n' )
						{
							if ( szPassword[strlen( szPassword ) - 1] == '\n' )
							{
								szPassword[strlen( szPassword ) - 1] = '\0';
							}

							while ( nThreads >= MAX_THREADS )
							{
								Sleep( 200 );
							}

							pThreadArgs = (PTHREAD_ARGS)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( THREAD_ARGS ) );

							if ( pThreadArgs != NULL )
							{
								strcpy( pThreadArgs->Target, szTarget );

								pThreadArgs->TCPPort = dwTCPPort;

								strcpy( pThreadArgs->Password, szPassword );

								pThreadArgs->MultipleHosts = TRUE;

								WaitForSingleObject( hSemaphore, INFINITE );

								nThreads++;

								ReleaseSemaphore( hSemaphore, 1, NULL );

								_beginthread( ThreadedSub, 0, (VOID *)pThreadArgs );
							}
						}
					}

					fclose( pDictFile );
				}
				else
				{
					strcpy( szPassword, szPasswordInput );

					pThreadArgs = (PTHREAD_ARGS)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( THREAD_ARGS ) );

					if ( pThreadArgs != NULL )
					{
						strcpy( pThreadArgs->Target, szTarget );

						pThreadArgs->TCPPort = dwTCPPort;

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
	}
	else
	{
		printf( "Godzilla v1.0 | Canaudit, Inc. http://www.canaudit.com/\n" );
		printf( "\n" );
		printf( "Usage: Godzilla <hostname | ip input file> <tcp port> <password | dictionary>\n" );
		printf( "\n" );
		printf( "<hostname | ip input file>  -- required argument\n" );
		printf( "<tcp port>                  -- required argument\n" );
		printf( "<password | dictionary>     -- required argument\n" );
		printf( "\n" );
		printf( "Examples:\n" );
		printf( "Godzilla 10.10.10.10 5900 password\n" );
		printf( "Godzilla IPInputFile.txt 5900 dict.txt\n" );
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

VOID ThreadedSub( VOID *pParameter )
{
	CHAR        szTarget[ 128 ];
	DWORD      dwTCPPort;
	CHAR      szPassword[ 128 ];
	BOOL  bMultipleHosts;

	PTHREAD_ARGS pThreadArgs;

	pThreadArgs = (PTHREAD_ARGS)pParameter;

	strcpy( szTarget, pThreadArgs->Target );

	dwTCPPort = pThreadArgs->TCPPort;

	strcpy( szPassword, pThreadArgs->Password );

	bMultipleHosts = pThreadArgs->MultipleHosts;

	HeapFree( GetProcessHeap(), 0, pThreadArgs );

	if ( bMultipleHosts == TRUE )
	{
		printf( "Trying password %s on host %s...\n", szPassword, szTarget );
	}

	TCPConnect( szTarget, &dwTCPPort, szPassword, &bMultipleHosts );

	WaitForSingleObject( hSemaphore, INFINITE );

	nThreads--;

	ReleaseSemaphore( hSemaphore, 1, NULL );

	_endthread();
}

VOID TCPConnect( CHAR szTarget[], DWORD *dwTCPPort, CHAR szPassword[], BOOL *bMultipleHosts )
{
	BOOL            bPortOpen;
	BOOL          bDisconnect;
	INT               nResult;
	CHAR          szIPAddress[ 16 ];
	WSADATA           wsaData;
	SOCKET                  s;
	LONG                    l;
	DWORD              dwWait;
	unsigned char  szFullRecv[ 4096 ];
	DWORD          dwFullRecv;
	BOOL               bError;
	DWORD                   i;
	DWORD                   j;
	DWORD                   k;
	CHAR            szVersion[ 16 ];
	CHAR           szErrorMsg[ 128 ];
	unsigned char      szSend[ 1024 ];
	DWORD              dwSend;
	CHAR            szVNCAuth[ 128 ];
	unsigned char szChallenge[ 16 ];
	unsigned char       szKey[ 8 ];

	struct hostent     *remoteHost;
	struct sockaddr_in         sin;

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

					RecvSocketData( &s, &dwWait, &bPortOpen, &bDisconnect, szFullRecv, &dwFullRecv );

					if ( dwFullRecv > 3 )
					{
						if ( szFullRecv[0] == 'R' && szFullRecv[1] == 'F' && szFullRecv[2] == 'B' )
						{
							bError = FALSE;

							i = 0;
							j = 0;

							for ( k = 0; k < dwFullRecv; k++ )
							{
								if ( szFullRecv[k] != '\0' )
								{
									if ( !bError )
									{
										if ( szFullRecv[k] != '\r' && szFullRecv[k] != '\n' )
										{
											szVersion[i] = szFullRecv[k];

											i++;
										}
									}
									else
									{
										if ( j > 0 )
										{
											if ( szFullRecv[k] != '\r' && szFullRecv[k] != '\n' )
											{
												szErrorMsg[j - 1] = szFullRecv[k];
											}
											else
											{
												szErrorMsg[j - 1] = ' ';
											}
										}

										j++;
									}
								}
								else
								{
									bError = TRUE;
								}
							}

							szVersion[i]      = '\0';
							szErrorMsg[j - 1] = '\0';

							if ( *bMultipleHosts == FALSE )
							{
								printf( "VNC Banner/Version:  %s\n", szVersion );
							}

							LogVNCBanners( szTarget, szVersion );

							if ( bError )
							{
								bDisconnect = TRUE;

								if ( *bMultipleHosts == FALSE )
								{
									fprintf( stderr, "ERROR! %s\n", szErrorMsg );
								}

								LogErrorMsg( szTarget, szVersion, szErrorMsg );
							}

							if ( !bDisconnect )
							{
								for ( i = 0; i < dwFullRecv; i++ )
								{
									szSend[i] = szFullRecv[i];
								}

								dwSend = dwFullRecv;

								SendRecvSocketData( &s, &dwWait, &bPortOpen, &bDisconnect, szSend, &dwSend, szFullRecv, &dwFullRecv );

								if ( !bDisconnect )
								{
									strcpy( szVNCAuth, "" );

									if ( dwFullRecv >= 4 )
									{
										for ( i = 1; i < 4; i++ )
										{
											switch ( szFullRecv[i] )
											{
												case 0:
													break;

												case 1:
													if ( *bMultipleHosts == FALSE )
													{
														printf( "VNC Authentication:  None\n" );
													}

													strcpy( szVNCAuth, "None" );

													LogVNCAuthType( szTarget, szVNCAuth );

													break;

												case 2:
													if ( *bMultipleHosts == FALSE )
													{
														printf( "VNC Authentication:  VNC\n" );
													}

													strcpy( szVNCAuth, "VNC" );

													LogVNCAuthType( szTarget, szVNCAuth );

													break;

												default:
													if ( *bMultipleHosts == FALSE )
													{
														printf( "VNC Authentication: Other - 0x%.2X\n", szFullRecv[i] );
													}

													sprintf( szVNCAuth, "Other - 0x%.2X", szFullRecv[i] );

													LogVNCAuthType( szTarget, szVNCAuth );

													break;
											}
										}
									}

									if ( dwFullRecv >= 20 )
									{
										i = 0;

										for ( j = 4; j < 20; j++ )
										{
											szChallenge[i] = szFullRecv[j];

											i++;
										}
									}

									if ( strcmp( szVNCAuth, "VNC" ) == 0 )
									{
										if ( *bMultipleHosts == FALSE )
										{
											printf( "Trying VNC Password: %s\n", szPassword );
										}

										for ( i = 0; i < 8; i++ )
										{
											if ( i < strlen( szPassword ) )
											{
												szKey[i] = szPassword[i];
											}
											else
											{
												szKey[i] = '\0';
											}
										}

										deskey( szKey, EN0 );

										for ( i = 0; i < 16; i = i + 8 )
										{
											des( szChallenge + i, szChallenge + i );
										}

										for ( i = 0; i < 16; i++ )
										{
											szSend[i] = szChallenge[i];
										}

										dwSend = 16;

										SendRecvSocketData( &s, &dwWait, &bPortOpen, &bDisconnect, szSend, &dwSend, szFullRecv, &dwFullRecv );

										if ( !bDisconnect )
										{
											if ( dwFullRecv >= 4 )
											{
												if ( szFullRecv[3] == 0 )
												{
													printf( "\n" );
													printf( "PASSWORD GUESSED! VNC password for host %s is %s\n", szTarget, szPassword );

													LogGuessedVNCPasswords( szTarget, szPassword );
												}
											}
										}

										if ( *bMultipleHosts == FALSE )
										{
											printf( "\n" );
										}
									}
								}
							}
						}
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

	pInputFile = fopen( "Godzilla.ini", "r" );

	if ( pInputFile != NULL )
	{
		while ( fgets( szLine, sizeof( szLine ), pInputFile ) != NULL )
		{
			Trim( szLine );

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

VOID RecvSocketData( SOCKET *s, DWORD *dwWait, BOOL *bPortOpen, BOOL *bDisconnect, unsigned char szFullRecv[], DWORD *dwFullRecv )
{
	DWORD                 i;
	fd_set         writefds;
	fd_set          readfds;
	BOOL          bContinue;
	DWORD                 j;
	DWORD                 k;
	INT             nResult;
	unsigned char    szRecv[ 1024 ];

	struct timeval tv;

	for ( i = 0; i < sizeof( szFullRecv ); i++ )
	{
		szFullRecv[i] = '\0';
	}

	FD_ZERO( &writefds );
	FD_SET( *s, &writefds );

	tv.tv_sec  = *dwWait;
	tv.tv_usec = 0;

	if ( select( 0, NULL, &writefds, NULL, &tv ) > 0 )
	{
		*bPortOpen = TRUE;

		FD_ZERO( &readfds );
		FD_SET( *s, &readfds );

		tv.tv_sec  = *dwWait;
		tv.tv_usec = 0;

		if ( select( 0, &readfds, NULL, NULL, &tv ) > 0 )
		{
			bContinue = TRUE;

			*dwFullRecv = 0;

			i = 0;
			j = 0;
			k = 0;

			while ( bContinue && !*bDisconnect )
			{
				nResult = recv( *s, szRecv, sizeof( szRecv ), 0 );

				if ( nResult > 0 )
				{
					for ( i = 0; i < nResult; i++ )
					{
						szFullRecv[j] = szRecv[i];

						j++;
					}

					*dwFullRecv = j;

					k = 0;
				}

				if ( nResult == 0 )
				{
					*bDisconnect = TRUE;
				}

				if ( nResult == SOCKET_ERROR )
				{
					if ( WSAGetLastError() == 10035 )
					{
						if ( k > 50 )
						{
							bContinue = FALSE;
						}

						k++;

						Sleep( 10 );
					}
					else
					{
						*bDisconnect = TRUE;
					}
				}
			}
		}
	}
	else
	{
		*bDisconnect = TRUE;
	}
}

VOID SendRecvSocketData( SOCKET *s, DWORD *dwWait, BOOL *bPortOpen, BOOL *bDisconnect, unsigned char szSend[], DWORD *dwSend, unsigned char szFullRecv[], DWORD *dwFullRecv )
{
	DWORD                 i;
	fd_set         writefds;
	fd_set          readfds;
	INT             nResult;
	BOOL          bContinue;
	DWORD                 j;
	DWORD                 k;
	unsigned char    szRecv[ 1024 ];

	struct timeval tv;

	for ( i = 0; i < sizeof( szFullRecv ); i++ )
	{
		szFullRecv[i] = '\0';
	}

	FD_ZERO( &writefds );
	FD_SET( *s, &writefds );

	tv.tv_sec  = *dwWait;
	tv.tv_usec = 0;

	if ( select( 0, NULL, &writefds, NULL, &tv ) > 0 )
	{
		nResult = send( *s, szSend, *dwSend, 0 );

		if ( nResult != SOCKET_ERROR )
		{
			FD_ZERO( &readfds );
			FD_SET( *s, &readfds );

			tv.tv_sec  = *dwWait;
			tv.tv_usec = 0;

			if ( select( 0, &readfds, NULL, NULL, &tv ) > 0 )
			{
				bContinue = TRUE;

				*dwFullRecv = 0;

				i = 0;
				j = 0;
				k = 0;

				while ( bContinue && !*bDisconnect )
				{
					nResult = recv( *s, szRecv, sizeof( szRecv ), 0 );

					if ( nResult > 0 )
					{
						for ( i = 0; i < nResult; i++ )
						{
							szFullRecv[j] = szRecv[i];

							j++;
						}

						*dwFullRecv = j;

						k = 0;
					}

					if ( nResult == 0 )
					{
						*bDisconnect = TRUE;
					}

					if ( nResult == SOCKET_ERROR )
					{
						if ( WSAGetLastError() == 10035 )
						{
							if ( k > 50 )
							{
								bContinue = FALSE;
							}

							k++;

							Sleep( 10 );
						}
						else
						{
							*bDisconnect = TRUE;
						}
					}
				}
			}
		}
		else
		{
			*bDisconnect = TRUE;
		}
	}
	else
	{
		*bDisconnect = TRUE;
	}
}

VOID LogVNCBanners( CHAR szTarget[], CHAR szVersion[] )
{
	FILE *pOutputFile;

	WaitForSingleObject( hSemaphore, INFINITE );

	pOutputFile = fopen( "VNCBanners.txt", "r" );

	if ( pOutputFile != NULL )
	{
		fclose( pOutputFile );
	}
	else
	{
		pOutputFile = fopen( "VNCBanners.txt", "w" );

		if ( pOutputFile != NULL )
		{
			fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
			fprintf( pOutputFile, "\n" );
			fprintf( pOutputFile, "Hostname\tVNC Banner / Version\n" );

			fclose( pOutputFile );
		}
	}

	pOutputFile = fopen( "VNCBanners.txt", "a+" );

	if ( pOutputFile != NULL )
	{
		fprintf( pOutputFile, "%s\t%s\n", szTarget, szVersion );

		fclose( pOutputFile );
	}

	ReleaseSemaphore( hSemaphore, 1, NULL );
}

VOID LogErrorMsg( CHAR szTarget[], CHAR szVersion[], CHAR szErrorMsg[] )
{
	FILE *pOutputFile;

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
			fprintf( pOutputFile, "Hostname\tVNC Banner / Version\tError Message\n" );

			fclose( pOutputFile );
		}
	}

	pOutputFile = fopen( "ErrorLog.txt", "a+" );

	if ( pOutputFile != NULL )
	{
		fprintf( pOutputFile, "%s\t%s\t%s\n", szTarget, szVersion, szErrorMsg );

		fclose( pOutputFile );
	}

	ReleaseSemaphore( hSemaphore, 1, NULL );
}

VOID LogVNCAuthType( CHAR szTarget[], CHAR szAuthType[] )
{
	FILE *pOutputFile;

	WaitForSingleObject( hSemaphore, INFINITE );

	pOutputFile = fopen( "VNCAuthType.txt", "r" );

	if ( pOutputFile != NULL )
	{
		fclose( pOutputFile );
	}
	else
	{
		pOutputFile = fopen( "VNCAuthType.txt", "w" );

		if ( pOutputFile != NULL )
		{
			fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
			fprintf( pOutputFile, "\n" );
			fprintf( pOutputFile, "Hostname\tVNC Authentication Type\n" );

			fclose( pOutputFile );
		}
	}

	pOutputFile = fopen( "VNCAuthType.txt", "a+" );

	if ( pOutputFile != NULL )
	{
		fprintf( pOutputFile, "%s\t%s\n", szTarget, szAuthType );

		fclose( pOutputFile );
	}

	ReleaseSemaphore( hSemaphore, 1, NULL );
}

VOID LogGuessedVNCPasswords( CHAR szTarget[], CHAR szPassword[] )
{
	FILE *pOutputFile;

	WaitForSingleObject( hSemaphore, INFINITE );

	pOutputFile = fopen( "GuessedVNCPasswords.txt", "r" );

	if ( pOutputFile != NULL )
	{
		fclose( pOutputFile );
	}
	else
	{
		pOutputFile = fopen( "GuessedVNCPasswords.txt", "w" );

		if ( pOutputFile != NULL )
		{
			fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
			fprintf( pOutputFile, "\n" );
			fprintf( pOutputFile, "Hostname\tVNC Password\n" );

			fclose( pOutputFile );
		}
	}

	pOutputFile = fopen( "GuessedVNCPasswords.txt", "a+" );

	if ( pOutputFile != NULL )
	{
		fprintf( pOutputFile, "%s\t%s\n", szTarget, szPassword );

		fclose( pOutputFile );
	}

	ReleaseSemaphore( hSemaphore, 1, NULL );
}

// Written by Reed Arvin | reed@canaudit.com
