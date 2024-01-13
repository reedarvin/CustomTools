//
// gcc source.c -o source.exe -lws2_32
//

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <process.h>
#include <winsock2.h>
#include <time.h>

BOOL         IsTCPPort( DWORD *dwTCPPort );
BOOL         TCPListen( SOCKET *s, DWORD *dwTCPPort );
VOID AcceptConnections( SOCKET *s, DWORD *dwTCPPorts );
VOID       ThreadedSub( VOID *pParameter );
VOID      TalkToClient( SOCKET *s, CHAR *szIPAddress, DWORD *dwTCPPort, CHAR *szRemoteIP, DWORD *dwRemoteTCPPort );
VOID         LogToFile( CHAR *szRemoteIPAddress, DWORD *dwTCPPort, CHAR *szLogText );
VOID           Replace( CHAR *szText, CHAR *szFindText, CHAR *szReplaceText );

typedef struct _THREAD_ARGS
{
	SOCKET          Socket;
	CHAR         IPAddress[ 128 ];
	DWORD          TCPPort;
	CHAR   RemoteIPAddress[ 128 ];
	DWORD    RemoteTCPPort;
} THREAD_ARGS, *PTHREAD_ARGS;

HANDLE hSemaphore;

INT nTCPPorts = 0;

INT main( INT argc, CHAR *argv[] )
{
	DWORD           i;
	DWORD   dwTCPPort;
	SOCKET          s[ 6 ];
	DWORD  dwTCPPorts[ 6 ];

	hSemaphore = CreateSemaphore( NULL, 1, 1, NULL );

	if ( argc > 1 && argc < 8 )
	{
		for ( i = 1; i < argc; i++ )
		{
			dwTCPPort = atoi( argv[i] );

			if ( IsTCPPort( &dwTCPPort ) )
			{
				if ( TCPListen( &s[nTCPPorts], &dwTCPPort ) )
				{
					dwTCPPorts[nTCPPorts] = dwTCPPort;

					nTCPPorts++;
				}
			}
		}

		if ( nTCPPorts > 0 )
		{
			AcceptConnections( s, dwTCPPorts );
		}
	}
	else
	{
		printf( "SimpleHoneyPot v1.0 | http://reedarvin.thearvins.com/\n" );
		printf( "\n" );
		printf( "Usage: SimpleHoneyPot <tcp port 1> ... <tcp port 6>\n" );
		printf( "\n" );
		printf( "<tcp port 1>  -- required argument\n" );
		printf( "<tcp port 2>  -- optional argument\n" );
		printf( "<tcp port 3>  -- optional argument\n" );
		printf( "<tcp port 4>  -- optional argument\n" );
		printf( "<tcp port 5>  -- optional argument\n" );
		printf( "<tcp port 6>  -- optional argument\n" );
		printf( "\n" );
		printf( "Examples:\n" );
		printf( "SimpleHoneyPot 21\n" );
		printf( "SimpleHoneyPot 23 79\n" );
		printf( "SimpleHoneyPot 21 23 25 79 80 81\n" );
		printf( "\n" );
		printf( "(Written by Reed Arvin | reedarvin@gmail.com)\n" );
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

BOOL TCPListen( SOCKET *s, DWORD *dwTCPPort )
{
	BOOL    bSuccess;
	INT      nResult;
	WSADATA  wsaData;
	LONG           l;

	struct sockaddr_in sin;

	bSuccess = FALSE;

	nResult = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );

	if ( nResult == NO_ERROR )
	{
		*s = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

		if ( *s != INVALID_SOCKET )
		{
			sin.sin_family      = AF_INET;
			sin.sin_port        = htons( *dwTCPPort );
			sin.sin_addr.s_addr = INADDR_ANY;

			l = 1;

			if ( ioctlsocket( *s, FIONBIO, &l ) == 0 )
			{
				if ( bind( *s, (SOCKADDR*)&sin, sizeof( sin ) ) != SOCKET_ERROR )
				{
					if ( listen( *s, 1 ) != SOCKET_ERROR )
					{
						printf( "Listening on TCP port %d...\n", *dwTCPPort );

						bSuccess = TRUE;
					}
				}
				else
				{
					fprintf( stderr, "ERROR! Cannot bind to TCP port %d.\n", *dwTCPPort );
				}
			}
		}
	}
	else
	{
		fprintf( stderr, "ERROR! Function WSAStartup failed.\n" );
	}

	if ( bSuccess )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

VOID AcceptConnections( SOCKET s[], DWORD dwTCPPorts[] )
{
	DWORD              i;
	SOCKET             c;
	HOSTENT   *localhost;
	CHAR    *szIPAddress;

	struct sockaddr_in sin;

	PTHREAD_ARGS pThreadArgs;

	while ( 1 )
	{
		for ( i = 0; i < nTCPPorts; i++ )
		{
			c = accept( s[i], (struct sockaddr *)&sin, NULL );

			if ( c != INVALID_SOCKET )
			{
				localhost = gethostbyname( "" );

				szIPAddress = inet_ntoa( *(struct in_addr *)*localhost->h_addr_list );

				pThreadArgs = (PTHREAD_ARGS)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( THREAD_ARGS ) );

				if ( pThreadArgs != NULL )
				{
					pThreadArgs->Socket = c;

					strcpy( pThreadArgs->IPAddress, szIPAddress );

					pThreadArgs->TCPPort = dwTCPPorts[i];

					strcpy( pThreadArgs->RemoteIPAddress, inet_ntoa( sin.sin_addr ) );

					pThreadArgs->RemoteTCPPort = ntohs( sin.sin_port );

					_beginthread( ThreadedSub, 0, (VOID *)pThreadArgs );
				}
			}
		}

		Sleep( 10 );
	}
}

VOID ThreadedSub( VOID *pParameter )
{
	SOCKET                 s;
	CHAR         szIPAddress[ 128 ];
	DWORD          dwTCPPort;
	CHAR   szRemoteIPAddress[ 128 ];
	DWORD    dwRemoteTCPPort;

	PTHREAD_ARGS pThreadArgs;

	pThreadArgs = (PTHREAD_ARGS)pParameter;

	s = pThreadArgs->Socket;

	strcpy( szIPAddress, pThreadArgs->IPAddress );

	dwTCPPort = pThreadArgs->TCPPort;

	strcpy( szRemoteIPAddress, pThreadArgs->RemoteIPAddress );

	dwRemoteTCPPort = pThreadArgs->RemoteTCPPort;

	HeapFree( GetProcessHeap(), 0, pThreadArgs );

	TalkToClient( &s, szIPAddress, &dwTCPPort, szRemoteIPAddress, &dwRemoteTCPPort );

	_endthread();
}

VOID TalkToClient( SOCKET *s, CHAR szIPAddress[], DWORD *dwTCPPort, CHAR szRemoteIPAddress[], DWORD *dwRemoteTCPPort )
{
	BOOL      bDisconnect;
	DWORD          dwWait;
	CHAR   szTemplateFile[ 128 ];
	FILE   *pTemplateFile;
	CHAR           szLine[ 4096 ];
	CHAR       szTempLine[ 4096 ];
	fd_set       writefds;
	fd_set        readfds;
	INT           nResult;
	CHAR        szLogText[ 4096 ];
	CHAR       szFullRecv[ 4096 ];
	BOOL        bContinue;
	DWORD               i;
	CHAR           szRecv[ 4096 ];

	struct timeval tv;

	bDisconnect = FALSE;

	dwWait = 15;

	printf( "[%s:%d -> %s:%d] Connection established\n", szRemoteIPAddress, *dwRemoteTCPPort, szIPAddress, *dwTCPPort );

	sprintf( szTemplateFile, "Templates\\%d.template", *dwTCPPort );

	pTemplateFile = fopen( szTemplateFile, "r" );

	if ( pTemplateFile != NULL )
	{
		while ( fgets( szLine, sizeof( szLine ), pTemplateFile ) != NULL && !bDisconnect )
		{
			if ( szLine[0] != '#' && szLine[0] != '\n' )
			{
				if ( szLine[strlen( szLine ) - 1] == '\n' )
				{
					szLine[strlen( szLine ) - 1] = '\0';
				}

				strcpy( szTempLine, szLine );

				if ( strcmp( szTempLine, "\\r\\n" ) == 0 )
				{
					strcpy( szTempLine, "" );
				}

				Replace( szTempLine, "\\r", "\r" );
				Replace( szTempLine, "\\n", "\n" );

				FD_ZERO( &writefds );
				FD_SET( *s, &writefds );

				tv.tv_sec  = dwWait;
				tv.tv_usec = 0;

				if ( select( 0, NULL, &writefds, NULL, &tv ) > 0 )
				{
					nResult = send( *s, szTempLine, strlen( szTempLine ), 0 );

					if ( nResult != SOCKET_ERROR )
					{
						sprintf( szLogText, "[%s:%d -> %s:%d] >> \"%s\"\n", szRemoteIPAddress, *dwRemoteTCPPort, szIPAddress, *dwTCPPort, szLine );

						printf( "%s", szLogText );

						LogToFile( szRemoteIPAddress, dwTCPPort, szLogText );

						FD_ZERO( &readfds );
						FD_SET( *s, &readfds );

						tv.tv_sec  = dwWait;
						tv.tv_usec = 0;

						if ( select( 0, &readfds, NULL, NULL, &tv ) > 0 )
						{
							strcpy( szFullRecv, "" );

							bContinue = TRUE;

							i = 0;

							while ( bContinue && !bDisconnect )
							{
								nResult = recv( *s, szRecv, sizeof( szRecv ), 0 );

								if ( nResult > 0 )
								{
									szRecv[nResult] = '\0';

									if ( szRecv[0] == '\r' || szRecv[0] == '\n' )
									{
										bContinue = FALSE;

										sprintf( szLogText, "[%s:%d -> %s:%d] << \"%s\"\n", szRemoteIPAddress, *dwRemoteTCPPort, szIPAddress, *dwTCPPort, szFullRecv );

										printf( "%s", szLogText );

										LogToFile( szRemoteIPAddress, dwTCPPort, szLogText );
									}
									else
									{
										strcat( szFullRecv, szRecv );
									}

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
				else
				{
					bDisconnect = TRUE;
				}
			}
		}

		fclose( pTemplateFile );
	}
	else
	{
		fprintf( stderr, "ERROR! Cannot open template file \"Templates\\%d.txt\".\n", *dwTCPPort );
	}

	closesocket( *s );
}

VOID LogToFile( CHAR szRemoteIPAddress[], DWORD *dwTCPPort, CHAR szLogText[] )
{
	CHAR    szLogFile[ 128 ];
	FILE *pOutputFile;
	CHAR       szDate[ 64 ];
	CHAR       szTime[ 64 ];

	sprintf( szLogFile, "LogFiles\\%s - TCP Port %d.txt", szRemoteIPAddress, *dwTCPPort );

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
		_strdate( szDate );
		_strtime( szTime );

		fprintf( pOutputFile, "%s %s: %s", szDate, szTime, szLogText );

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

// Written by Reed Arvin | reedarvin@gmail.com
