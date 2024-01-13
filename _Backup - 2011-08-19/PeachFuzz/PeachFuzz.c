//
// gcc source.c -o source.exe -lws2_32
//

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <winsock2.h>
#include <time.h>

BOOL         IsTCPPort( DWORD *dwTCPPort );
BOOL     AreValidFiles( CHAR *szOverflowStrings, CHAR *szLoginStrings, CHAR *szCommandStrings );
VOID         StartFuzz( CHAR *szTarget, DWORD *dwTCPPort, CHAR *szOverflowStrings, CHAR *szLoginStrings, CHAR *szCommandStrings, DWORD *dwSequence );
BOOL             Split( CHAR *szText, CHAR *szSplitText, CHAR *szStartText, CHAR *szEndText );
DWORD       TCPConnect( SOCKET *s, CHAR *szTarget, DWORD *dwTCPPort );
VOID      GetWaitValue( DWORD *dwWait );
BOOL          IsTelnet( SOCKET *s, CHAR *szRecv, DWORD *dwWait, BOOL *bDisconnect );
VOID  GeneratePayload1( CHAR *szLoginCommand, CHAR *szOverflowChars, CHAR *szNumRepeats, CHAR *szPayloadData );
VOID           TCPSend( SOCKET *s, DWORD *dwWait, CHAR *szPayloadData );
VOID     TCPDisconnect( SOCKET *s );
VOID  GeneratePayload2( CHAR *szCommandString, CHAR *szOverflowChars, CHAR *szNumRepeats, CHAR *szPayloadData );
VOID           Replace( CHAR *szText, CHAR *szFindText, CHAR *szReplaceText );

INT main( INT argc, CHAR *argv[] )
{
	CHAR            szTarget[ 128 ];
	DWORD          dwTCPPort;
	CHAR  szProtocolTemplate[ 128 ];
	CHAR  szOverflowTemplate[ 128 ];
	DWORD         dwSequence;
	CHAR   szOverflowStrings[ 128 ];
	CHAR      szLoginStrings[ 128 ];
	CHAR    szCommandStrings[ 128 ];

	if ( argc > 4 && argc < 7 )
	{
		strcpy( szTarget, argv[1] );

		dwTCPPort = atoi( argv[2] );

		strcpy( szProtocolTemplate, argv[3] );
		strcpy( szOverflowTemplate, argv[4] );

		if ( argc == 6 )
		{
			dwSequence = atoi( argv[5] );
		}
		else
		{
			dwSequence = 0;
		}

		strupr( szOverflowTemplate );
		strupr( szProtocolTemplate );

		strcpy( szOverflowStrings, szOverflowTemplate );

		sprintf( szLoginStrings,   "%s.LOGIN",    szProtocolTemplate );
		sprintf( szCommandStrings, "%s.COMMANDS", szProtocolTemplate );

		if ( IsTCPPort( &dwTCPPort ) && AreValidFiles( szOverflowStrings, szLoginStrings, szCommandStrings ) )
		{
			printf( "Running PeachFuzz v1.0 with the following arguments:\n" );
			printf( "[+] Host Input:        \"%s\"\n", szTarget );
			printf( "[+] TCP Port:          \"%d\"\n", dwTCPPort );
			printf( "[+] Protocol Template: \"%s\"\n", szProtocolTemplate );
			printf( "[+] Overflow Template: \"%s\"\n", szOverflowTemplate );

			if ( dwSequence != 0 )
			{
				printf( "[+] Sequence #:        \"%d\"\n", dwSequence );
			}

			printf( "\n" );

			StartFuzz( szTarget, &dwTCPPort, szOverflowStrings, szLoginStrings, szCommandStrings, &dwSequence );
		}
	}
	else
	{
		printf( "PeachFuzz v1.0 | http://reedarvin.thearvins.com/\n" );
		printf( "\n" );
		printf( "Usage: PeachFuzz <hostname | ip address> <tcp port> <protocol template> <overflow template> <sequence #>\n" );
		printf( "\n" );
		printf( "<hostname | ip address>  -- required argument\n" );
		printf( "<tcp port>               -- required argument\n" );
		printf( "<protocol template>      -- required argument\n" );
		printf( "<overflow template>      -- required argument\n" );
		printf( "<sequence #>             -- optional argument\n" );
		printf( "\n" );
		printf( "Examples:\n" );
		printf( "PeachFuzz 10.10.10.10 25 smtp overflow.strings\n" );
		printf( "PeachFuzz 10.10.10.10 110 pop3 overflow.strings 130\n" );
		printf( "PeachFuzz localhost 21 ftp overflow.strings\n" );
		printf( "\n" );
		printf( "(Written by Reed Arvin | reedarvin@gmail.com)\n" );
	}

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

BOOL AreValidFiles( CHAR szOverflowStrings[], CHAR szLoginStrings[], CHAR szCommandStrings[] )
{
	BOOL bValidFiles;
	FILE      *pFile;

	bValidFiles = TRUE;

	pFile = fopen( szOverflowStrings, "r" );

	if ( pFile != NULL )
	{
		fclose( pFile );
	}
	else
	{
		fprintf( stderr, "ERROR! Cannot open overflow template %s.\n", szOverflowStrings );

		bValidFiles = FALSE;
	}

	pFile = fopen( szLoginStrings, "r" );

	if ( pFile != NULL )
	{
		fclose( pFile );
	}
	else
	{
		fprintf( stderr, "ERROR! Cannot open protocol template %s.\n", szLoginStrings );

		bValidFiles = FALSE;
	}

	pFile = fopen( szCommandStrings, "r" );

	if ( pFile != NULL )
	{
		fclose( pFile );
	}
	else
	{
		fprintf( stderr, "ERROR! Cannot open protocol template %s.\n", szCommandStrings );

		bValidFiles = FALSE;
	}

	if ( bValidFiles )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

VOID StartFuzz( CHAR szTarget[], DWORD *dwTCPPort, CHAR szOverflowStrings[], CHAR szLoginStrings[], CHAR szCommandStrings[], DWORD *dwSequence )
{
	FILE   *pOverflowStrings;
	DWORD  dwCurrentSequence;
	DWORD                  i;
	CHAR    szOverflowString[ 1024 ];
	CHAR     szOverflowChars[ 128 ];
	CHAR        szNumRepeats[ 128 ];
	FILE      *pLoginStrings;
	DWORD                  j;
	DWORD                  k;
	CHAR       szLoginString[ 1024 ];
	CHAR      szLoginCommand[ 128 ];
	CHAR         szLoginData[ 128 ];
	DWORD             dwWait;
	SOCKET                 s;
	FILE    *pCommandStrings;
	CHAR     szCommandString[ 1024 ];
	CHAR       szPayloadData[ 102400 ];

	pOverflowStrings = fopen( szOverflowStrings, "r" );

	if ( pOverflowStrings != NULL )
	{
		dwCurrentSequence = 1;

		i = 0;

		while ( fgets( szOverflowString, sizeof( szOverflowString ), pOverflowStrings ) != NULL )
		{
			if ( szOverflowString[0] != '#' && szOverflowString[0] != '\n' )
			{
				if ( szOverflowString[strlen( szOverflowString ) - 1] == '\n' )
				{
					szOverflowString[strlen( szOverflowString ) - 1] = '\0';
				}

				if ( Split( szOverflowString, ":", szOverflowChars, szNumRepeats ) )
				{
					do {
						pLoginStrings = fopen( szLoginStrings, "r" );

						if ( pLoginStrings != NULL )
						{
							if ( i == j )
							{
								i = 0;
							}

							j = 0;

							if ( *dwSequence == 0 || *dwSequence == dwCurrentSequence )
							{
								k = 0;

								while ( fgets( szLoginString, sizeof( szLoginString ), pLoginStrings ) != NULL )
								{
									if ( szLoginString[0] != '#' && szLoginString[0] != '\n' )
									{
										if ( szLoginString[strlen( szLoginString ) - 1] == '\n' )
										{
											szLoginString[strlen( szLoginString ) - 1] = '\0';
										}

										if ( Split( szLoginString, ":", szLoginCommand, szLoginData ) )
										{
											if ( k == 0 )
											{
												printf( "#### Sequence #%d\n", dwCurrentSequence );

												dwWait = TCPConnect( &s, szTarget, dwTCPPort );

												k++;
											}

											if ( j == i )
											{
												GeneratePayload1( szLoginCommand, szOverflowChars, szNumRepeats, szPayloadData );

												printf( "#### Sending string %s\\r\\n\n", szLoginCommand );

												TCPSend( &s, &dwWait, szPayloadData );
											}
											else
											{
												printf( "#### Sending string %s %s\\r\\n\n", szLoginCommand, szLoginData );

												sprintf( szPayloadData, "%s %s\r\n", szLoginCommand, szLoginData );

												TCPSend( &s, &dwWait, szPayloadData );
											}

											j++;
										}
									}
								}

								if ( k == 1 )
								{
									TCPDisconnect( &s );

									printf( "\n" );
								}
							}
							else
							{
								while ( fgets( szLoginString, sizeof( szLoginString ), pLoginStrings ) != NULL )
								{
									if ( szLoginString[0] != '#' && szLoginString[0] != '\n' )
									{
										if ( szLoginString[strlen( szLoginString ) - 1] == '\n' )
										{
											szLoginString[strlen( szLoginString ) - 1] = '\0';
										}

										if ( Split( szLoginString, ":", szLoginCommand, szLoginData ) )
										{
											j++;

											k = 1;
										}
									}
								}
							}

							fclose( pLoginStrings );

							if ( k == 1 )
							{
								dwCurrentSequence++;
							}

							if ( j == 0 )
							{
								break;
							}
						}

						i++;
					} while ( i != j );
				}
			}

			if ( j == 0 )
			{
				break;
			}
		}

		fclose( pOverflowStrings );
	}

	pOverflowStrings = fopen( szOverflowStrings, "r" );

	if ( pOverflowStrings != NULL )
	{
		while ( fgets( szOverflowString, sizeof( szOverflowString ), pOverflowStrings ) != NULL )
		{
			if ( szOverflowString[0] != '#' && szOverflowString[0] != '\n' )
			{
				if ( szOverflowString[strlen( szOverflowString ) - 1] == '\n' )
				{
					szOverflowString[strlen( szOverflowString ) - 1] = '\0';
				}

				if ( Split( szOverflowString, ":", szOverflowChars, szNumRepeats ) )
				{
					pCommandStrings = fopen( szCommandStrings, "r" );

					if ( pCommandStrings != NULL )
					{
						while ( fgets( szCommandString, sizeof( szCommandString ), pCommandStrings ) != NULL )
						{
							if ( szCommandString[0] != '#' && szCommandString[0] != '\n' )
							{
								if ( szCommandString[strlen( szCommandString ) - 1] == '\n' )
								{
									szCommandString[strlen( szCommandString ) - 1] = '\0';
								}

								pLoginStrings = fopen( szLoginStrings, "r" );

								if ( pLoginStrings != NULL )
								{
									if ( *dwSequence == 0 || *dwSequence == dwCurrentSequence )
									{
										printf( "#### Sequence #%d\n", dwCurrentSequence );

										dwWait = TCPConnect( &s, szTarget, dwTCPPort );

										while ( fgets( szLoginString, sizeof( szLoginString ), pLoginStrings ) != NULL )
										{
											if ( szLoginString[0] != '#' && szLoginString[0] != '\n' )
											{
												if ( szLoginString[strlen( szLoginString ) - 1] == '\n' )
												{
													szLoginString[strlen( szLoginString ) - 1] = '\0';
												}

												if ( Split( szLoginString, ":", szLoginCommand, szLoginData ) )
												{
													printf( "#### Sending string %s %s\\r\\n\n", szLoginCommand, szLoginData );

													sprintf( szPayloadData, "%s %s\r\n", szLoginCommand, szLoginData );

													TCPSend( &s, &dwWait, szPayloadData );
												}
											}
										}

										GeneratePayload2( szCommandString, szOverflowChars, szNumRepeats, szPayloadData );

										printf( "#### Sending string %s\\r\\n\n", szCommandString );

										TCPSend( &s, &dwWait, szPayloadData );

										TCPDisconnect( &s );

										printf( "\n" );
									}

									dwCurrentSequence++;

									fclose( pLoginStrings );
								}
							}
						}

						fclose( pCommandStrings );
					}
				}
			}
		}

		fclose( pOverflowStrings );
	}
}

DWORD TCPConnect( SOCKET *s, CHAR szTarget[], DWORD *dwTCPPort )
{
	BOOL      bPortOpen;
	BOOL    bDisconnect;
	INT         nResult;
	WSADATA     wsaData;
	CHAR    szIPAddress[ 16 ];
	LONG              l;
	DWORD        dwWait;
	fd_set     writefds;
	fd_set      readfds;
	BOOL      bContinue;
	DWORD             i;
	CHAR         szRecv[ 4096 ];
	CHAR         szTemp[ 128 ];

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

			*s = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

			if ( *s != INVALID_SOCKET )
			{
				sin.sin_family      = AF_INET;
				sin.sin_port        = htons( *dwTCPPort );
				sin.sin_addr.s_addr = inet_addr( szIPAddress );

				l = 1;

				if ( ioctlsocket( *s, FIONBIO, &l ) == 0 )
				{
					GetWaitValue( &dwWait );

					connect( *s, (struct sockaddr *)&sin, sizeof( sin ) );

					FD_ZERO( &writefds );
					FD_SET( *s, &writefds );

					tv.tv_sec  = dwWait;
					tv.tv_usec = 0;

					if ( select( 0, NULL, &writefds, NULL, &tv ) > 0 )
					{
						bPortOpen = TRUE;

						FD_ZERO( &readfds );
						FD_SET( *s, &readfds );

						tv.tv_sec  = dwWait;
						tv.tv_usec = 0;

						if ( select( 0, &readfds, NULL, NULL, &tv ) > 0 )
						{
							bContinue = TRUE;

							i = 0;

							while ( bContinue && !bDisconnect )
							{
								nResult = recv( *s, szRecv, sizeof( szRecv ), 0 );

								if ( nResult > 0 )
								{
									szRecv[nResult] = '\0';

									if ( !IsTelnet( s, szRecv, &dwWait, &bDisconnect ) )
									{
										printf( "%s", szRecv );

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
			}
		}
		else
		{
			fprintf( stderr, "ERROR! Cannot resolve host %s to an IP address.\n", szTarget );
		}
	}

	if ( bDisconnect )
	{
		fprintf( stderr, "WARNING! Remote host disconnected.\n" );
	}

	if ( !bPortOpen )
	{
		fprintf( stderr, "ERROR! Cannot connect to %s over TCP port %d.\n", szTarget, *dwTCPPort );

		printf( "\n" );
		printf( "Press ENTER to continue or press CTRL+C to quit...\n" );

		gets( szTemp );
	}

	return dwWait;
}

VOID GetWaitValue( DWORD *dwWait )
{
	FILE *pInputFile;
	CHAR      szLine[ 1024 ];

	*dwWait = 5;

	pInputFile = fopen( "PeachFuzz.ini", "r" );

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

BOOL Split( CHAR szText[], CHAR szSplitText[], CHAR szStartText[], CHAR szEndText[] )
{
	BOOL  bSplitTextFound;
	CHAR   *pTextLocation;
	DWORD  dwTextLocation;
	DWORD               i;
	DWORD               j;

	bSplitTextFound = FALSE;

	pTextLocation = strstr( szText, szSplitText );

	dwTextLocation = (INT)( pTextLocation - szText + 1 );

	if ( pTextLocation != NULL )
	{
		bSplitTextFound = TRUE;

		i = 0;

		while ( i < dwTextLocation - 1 )
		{
			szStartText[i] = szText[i];

			i++;
		}

		szStartText[i] = '\0';

		i = strlen( szStartText ) + strlen( szSplitText );

		j = 0;

		while ( i < strlen( szText ) )
		{
			szEndText[j] = szText[i];

			i++;
			j++;
		}

		szEndText[j] = '\0';
	}

	if ( bSplitTextFound )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

VOID GeneratePayload1( CHAR szLoginCommand[], CHAR szOverflowChars[], CHAR szNumRepeats[], CHAR szPayloadData[] )
{
	CHAR  szTempLoginCommand[ 128 ];
	CHAR       szPayloadDesc[ 128 ];
	DWORD       dwNumRepeats;
	CHAR           szNOPData[ 102400 ];
	DWORD                  i;
	DWORD                  j;

	strcpy( szTempLoginCommand, szLoginCommand );

	sprintf( szPayloadDesc, "(%s x %s)", szOverflowChars, szNumRepeats );

	dwNumRepeats = atoi( szNumRepeats );

	sprintf( szLoginCommand, "%s %s", szTempLoginCommand, szPayloadDesc );

	strcpy( szNOPData, "" );

	if ( strlen( szOverflowChars ) == 1 )
	{
		memset( szNOPData, szOverflowChars[0], dwNumRepeats );

		szNOPData[dwNumRepeats] = '\0';
	}
	else
	{
		i = 0;
		j = 0;

		while ( i < dwNumRepeats )
		{
			strcat( szNOPData, szOverflowChars );

			j = j + strlen( szOverflowChars );

			i++;
		}

		szNOPData[j] = '\0';
	}

	sprintf( szPayloadData, "%s %s", szTempLoginCommand, szNOPData );

	strcat( szPayloadData, "\r\n" );
}

VOID TCPSend( SOCKET *s, DWORD *dwWait, CHAR szPayloadData[] )
{
	BOOL   bDisconnect;
	fd_set    writefds;
	INT        nResult;
	fd_set     readfds;
	BOOL     bContinue;
	DWORD            i;
	CHAR        szRecv[ 4096 ];

	struct timeval tv;

	bDisconnect = FALSE;

	FD_ZERO( &writefds );
	FD_SET( *s, &writefds );

	tv.tv_sec  = *dwWait;
	tv.tv_usec = 0;

	if ( select( 0, NULL, &writefds, NULL, &tv ) > 0 )
	{
		Replace( szPayloadData, "\\r", "\r" );
		Replace( szPayloadData, "\\n", "\n" );

		nResult = send( *s, szPayloadData, strlen( szPayloadData ), 0 );

		if ( nResult != SOCKET_ERROR )
		{
			FD_ZERO( &readfds );
			FD_SET( *s, &readfds );

			tv.tv_sec  = *dwWait;
			tv.tv_usec = 0;

			if ( select( 0, &readfds, NULL, NULL, &tv ) > 0 )
			{
				bContinue = TRUE;

				i = 0;

				while ( bContinue && !bDisconnect )
				{
					nResult = recv( *s, szRecv, sizeof( szRecv ), 0 );

					if ( nResult > 0 )
					{
						szRecv[nResult] = '\0';

						printf( "%s", szRecv );

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

	if ( bDisconnect )
	{
		fprintf( stderr, "WARNING! Remote host disconnected.\n" );
	}
}

VOID TCPDisconnect( SOCKET *s )
{
	closesocket( *s );

	WSACleanup();
}

VOID GeneratePayload2( CHAR szCommandString[], CHAR szOverflowChars[], CHAR szNumRepeats[], CHAR szPayloadData[] )
{
	CHAR  szPayloadDesc[ 128 ];
	DWORD  dwNumRepeats;
	CHAR      szNOPData[ 102400 ];
	DWORD             i;
	DWORD             j;

	strcpy( szPayloadData, szCommandString );

	sprintf( szPayloadDesc, "(%s x %s)", szOverflowChars, szNumRepeats );

	dwNumRepeats = atoi( szNumRepeats );

	Replace( szCommandString, "$$$", szPayloadDesc );

	strcpy( szNOPData, "" );

	if ( strlen( szOverflowChars ) == 1 )
	{
		memset( szNOPData, szOverflowChars[0], dwNumRepeats );

		szNOPData[dwNumRepeats] = '\0';
	}
	else
	{
		i = 0;
		j = 0;

		while ( i < dwNumRepeats )
		{
			strcat( szNOPData, szOverflowChars );

			j = j + strlen( szOverflowChars );

			i++;
		}

		szNOPData[j] = '\0';
	}

	Replace( szPayloadData, "$$$", szNOPData );

	strcat( szPayloadData, "\r\n" );
}

VOID Replace( CHAR szText[], CHAR szFindText[], CHAR szReplaceText[] )
{
	CHAR   *pTextLocation;
	DWORD  dwTextLocation;
	DWORD   dwTempTextLen;
	DWORD               i;
	CHAR       szTempText[ 102400 ];
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
