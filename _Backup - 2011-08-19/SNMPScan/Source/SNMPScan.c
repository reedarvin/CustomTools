//
// gcc source.c -o source.exe -lws2_32 -lsnmpapi -lmgmtapi
//

#define _WIN32_WINNT 0x0500
#define WINVER       0x0500

#define STRICT
#define WIN32_LEAN_AND_MEAN

#define _WINSOCKAPI_

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <process.h>
#include <winsock2.h>
#include <stdlib.h>
#include <mgmtapi.h>
#include <snmp.h>

#define MAX_THREADS 64

BOOL                  IsIPRange( CHAR *szTargetInput, CHAR *szIPNetwork );
VOID                ThreadedSub( VOID *pParameter );
BOOL            IsHostExclusion( CHAR *szTarget );
VOID                       Trim( CHAR *szText );
BOOL                 IsPingable( CHAR *szTarget, BOOL *bMultipleHosts );
VOID  GuessSNMPCommunityStrings( CHAR *szTarget, BOOL *bMultipleHosts );
VOID                SNMPConnect( CHAR *szTarget, CHAR *szCommunityString, BOOL *bMultipleHosts );
VOID LogGuessedCommunityStrings( CHAR *szTarget, CHAR *szCommunityString, CHAR *szHostname, CHAR *szDescription );

typedef struct _IP_OPTION_INFORMATION
{
	unsigned char          Ttl;          // Time To Live
	unsigned char          Tos;          // Type Of Service
	unsigned char        Flags;          // IP header flags
	unsigned char  OptionsSize;          // Size in bytes of options data
	unsigned char *OptionsData;          // Pointer to options data
} IP_OPTION_INFORMATION;

typedef struct _ICMP_ECHO_REPLY
{
	DWORD                       Address; // Replying address
	unsigned long                Status; // Reply IP_STATUS
	unsigned long         RoundTripTime; // RTT in milliseconds
	unsigned short             DataSize; // Reply data size in bytes
	unsigned short             Reserved; // Reserved for system use
	VOID                          *Data; // Pointer to the reply data
	IP_OPTION_INFORMATION       Options; // Reply options
} ICMP_ECHO_REPLY;

typedef HANDLE (WINAPI *fnIcmpCreateFile)( VOID );
typedef DWORD  (WINAPI *fnIcmpSendEcho)( HANDLE, DWORD, VOID *, WORD, IP_OPTION_INFORMATION *, VOID *, DWORD, DWORD );
typedef BOOL   (WINAPI *fnIcmpCloseHandle)( HANDLE );

typedef struct _THREAD_ARGS
{
	CHAR        Target[ 128 ];
	BOOL MultipleHosts;
} THREAD_ARGS, *PTHREAD_ARGS;

HANDLE hSemaphore;

INT nThreads = 0;

INT main( INT argc, CHAR *argv[] )
{
	CHAR  szTargetInput[ 128 ];
	FILE    *pInputFile;
	CHAR     szReadLine[ 128 ];
	CHAR       szTarget[ 128 ];
	CHAR    szIPNetwork[ 128 ];
	DWORD             i;

	PTHREAD_ARGS pThreadArgs;

	hSemaphore = CreateSemaphore( NULL, 1, 1, NULL );

	if ( argc == 2 )
	{
		strcpy( szTargetInput, argv[1] );

		printf( "Running SNMPScan v1.1 with the following arguments:\n" );
		printf( "[+] Host Input:   \"%s\"\n", szTargetInput );
		printf( "[+] # of Threads: \"64\"\n" );
		printf( "[+] Output File:  \"PingableHosts.txt\"\n" );
		printf( "[+] Output File:  \"SNMPCommunityStringInfo.txt\"\n" );
		printf( "\n" );

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
					strcpy( pThreadArgs->Target, szTarget );

					pThreadArgs->MultipleHosts = TRUE;

					WaitForSingleObject( hSemaphore, INFINITE );

					nThreads++;

					ReleaseSemaphore( hSemaphore, 1, NULL );

					_beginthread( ThreadedSub, 0, (VOID *)pThreadArgs );
				}
			}
		}
		else
		{
			strcpy( szTarget, szTargetInput );

			pThreadArgs = (PTHREAD_ARGS)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( THREAD_ARGS ) );

			if ( pThreadArgs != NULL )
			{
				strcpy( pThreadArgs->Target, szTarget );

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
	}
	else
	{
		printf( "SNMPScan v1.1 | Canaudit, Inc. http://www.canaudit.com/\n" );
		printf( "\n" );
		printf( "Usage: SNMPScan <hostname | ip input file>\n" );
		printf( "\n" );
		printf( "<hostname | ip input file>  -- required argument\n" );
		printf( "\n" );
		printf( "The SNMP community strings that are attempted for each host are\n" );
		printf( "included in the CommunityStrings.input file.\n" );
		printf( "\n" );
		printf( "The hosts included in the HostExclusions.input file are not\n" );
		printf( "scanned.\n" );
		printf( "\n" );
		printf( "Examples:\n" );
		printf( "SNMPScan 10.10.10.10\n" );
		printf( "SNMPScan MyWindowsMachine\n" );
		printf( "SNMPScan 10.10.10.1-254\n" );
		printf( "SNMPScan IPInputFile.txt\n" );
		printf( "\n" );
		printf( "(Written by Reed Arvin | reed@canaudit.com)\n" );
	}

	CloseHandle( hSemaphore );

	return 0;
}

BOOL IsIPRange( CHAR szTargetInput[], CHAR szIPNetwork[] )
{
	BOOL  bIsIPRange;
	DWORD          i;
	DWORD          j;

	bIsIPRange = FALSE;

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
			bIsIPRange = TRUE;
		}
	}

	if ( bIsIPRange )
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
	CHAR       szTarget[ 128 ];
	BOOL bMultipleHosts;

	PTHREAD_ARGS pThreadArgs;

	pThreadArgs = (PTHREAD_ARGS)pParameter;

	strcpy( szTarget, pThreadArgs->Target );

	bMultipleHosts = pThreadArgs->MultipleHosts;

	HeapFree( GetProcessHeap(), 0, pThreadArgs );

	if ( !IsHostExclusion( szTarget ) )
	{
		if ( bMultipleHosts )
		{
			printf( "Spawning thread for host %s...\n", szTarget );
		}

		if ( IsPingable( szTarget, &bMultipleHosts ) )
		{
			GuessSNMPCommunityStrings( szTarget, &bMultipleHosts );
		}
	}

	WaitForSingleObject( hSemaphore, INFINITE );

	nThreads--;

	ReleaseSemaphore( hSemaphore, 1, NULL );

	_endthread();
}

BOOL IsHostExclusion( CHAR *szTarget )
{
	BOOL      bHostExclusion;
	FILE *pHostExclusionFile;
	CHAR     szHostExclusion[ 128 ];
	CHAR        szTempTarget[ 128 ];

	bHostExclusion = FALSE;

	pHostExclusionFile = fopen( "HostExclusions.input", "r" );

	if ( pHostExclusionFile != NULL )
	{
		while ( fgets( szHostExclusion, sizeof( szHostExclusion ), pHostExclusionFile ) != NULL )
		{
			Trim( szHostExclusion );

			if ( szHostExclusion[0] != '#' && szHostExclusion[0] != '\n' )
			{
				if ( szHostExclusion[strlen( szHostExclusion ) - 1] == '\n' )
				{
					szHostExclusion[strlen( szHostExclusion ) - 1] = '\0';
				}

				strcpy( szTempTarget, szTarget );

				_strupr( szTempTarget );
				_strupr( szHostExclusion );

				if ( strcmp( szTempTarget, szHostExclusion ) == 0 )
				{
					bHostExclusion = TRUE;
				}
			}
		}

		fclose( pHostExclusionFile );
	}

	return bHostExclusion;
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

BOOL IsPingable( CHAR szTarget[], BOOL *bMultipleHosts )
{
	BOOL                     bPingable;
	INT                        nResult;
	WSADATA                    wsaData;
	CHAR                   szIPAddress[ 16 ];
	HANDLE                   hIPHLPAPI;
	HANDLE                       hICMP;
	fnIcmpCreateFile   pIcmpCreateFile;
	fnIcmpSendEcho       pIcmpSendEcho;
	fnIcmpCloseHandle pIcmpCloseHandle;
	HANDLE                   hICMPFile;
	CHAR                    szSendData[ 32 ];
	DWORD                  dwReplySize;
	VOID                 *pReplyBuffer;
	DWORD                     dwStatus;
	ICMP_ECHO_REPLY        *pEchoReply;
	FILE                  *pOutputFile;

	struct hostent *remoteHost;

	bPingable = FALSE;

	nResult = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );

	if ( nResult == NO_ERROR )
	{
		remoteHost = gethostbyname( szTarget );

		if ( remoteHost != NULL )
		{
			strcpy( szIPAddress, inet_ntoa( *(struct in_addr *)remoteHost->h_addr_list[0] ) );

			hIPHLPAPI = LoadLibrary( "iphlpapi.dll" );

			if ( hIPHLPAPI != NULL )
			{
				pIcmpCreateFile  = (fnIcmpCreateFile)GetProcAddress( hIPHLPAPI, "IcmpCreateFile" );
				pIcmpSendEcho    = (fnIcmpSendEcho)GetProcAddress( hIPHLPAPI, "IcmpSendEcho" );
				pIcmpCloseHandle = (fnIcmpCloseHandle)GetProcAddress( hIPHLPAPI, "IcmpCloseHandle" );
			}
			else
			{
				hICMP = LoadLibrary( "icmp.dll" );

				if ( hICMP != NULL )
				{
					pIcmpCreateFile  = (fnIcmpCreateFile)GetProcAddress( hICMP, "IcmpCreateFile" );
					pIcmpSendEcho    = (fnIcmpSendEcho)GetProcAddress( hICMP, "IcmpSendEcho" );
					pIcmpCloseHandle = (fnIcmpCloseHandle)GetProcAddress( hICMP, "IcmpCloseHandle" );
				}
			}

			if ( pIcmpCreateFile && pIcmpSendEcho && pIcmpCloseHandle )
			{
				hICMPFile = pIcmpCreateFile();

				if ( hICMPFile != INVALID_HANDLE_VALUE )
				{
					strcpy( szSendData, "ABCDEFGHIJKLMNOPQRSTUVWXYZ" );

					dwReplySize = sizeof( ICMP_ECHO_REPLY ) + strlen( szSendData ) + 1;

					pReplyBuffer = NULL;

					pReplyBuffer = (VOID *)malloc( dwReplySize );

					if ( pReplyBuffer != NULL )
					{
						dwStatus = pIcmpSendEcho( hICMPFile, *(DWORD *)remoteHost->h_addr_list[0], szSendData, strlen( szSendData ) + 1, NULL, pReplyBuffer, dwReplySize, 5000 );

						if ( dwStatus != 0 )
						{
							pEchoReply = (ICMP_ECHO_REPLY *)pReplyBuffer;

							bPingable = TRUE;

							if ( !*bMultipleHosts )
							{
								printf( "Host %s is alive! (%d.%d.%d.%d, Time %dms, TTL %d)\n", szTarget, LOBYTE( LOWORD( pEchoReply->Address ) ), HIBYTE( LOWORD( pEchoReply->Address ) ), LOBYTE( HIWORD( pEchoReply->Address ) ), HIBYTE( HIWORD( pEchoReply->Address ) ), pEchoReply->RoundTripTime, pEchoReply->Options.Ttl );
							}

							WaitForSingleObject( hSemaphore, INFINITE );

							pOutputFile = fopen( "PingableHosts.txt", "r" );

							if ( pOutputFile != NULL )
							{
								fclose( pOutputFile );
							}
							else
							{
								pOutputFile = fopen( "PingableHosts.txt", "w" );

								if ( pOutputFile != NULL )
								{
									fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
									fprintf( pOutputFile, "\n" );
									fprintf( pOutputFile, "Hostname\tIP Address\tIs Pingable?\n" );

									fclose( pOutputFile );
								}
							}

							pOutputFile = fopen( "PingableHosts.txt", "a+" );

							if ( pOutputFile != NULL )
							{
								fprintf( pOutputFile, "%s\t%s\tYes\n", szTarget, szIPAddress );

								fclose( pOutputFile );
							}

							ReleaseSemaphore( hSemaphore, 1, NULL );
						}

						free( pReplyBuffer );
					}
					else
					{
						if ( !*bMultipleHosts )
						{
							fprintf( stderr, "ERROR! Unable to allocate memory.\n" );
						}
					}

					pIcmpCloseHandle( hICMPFile );
				}
				else
				{
					if ( !*bMultipleHosts )
					{
						fprintf( stderr, "ERROR! Unable to open ICMP handle.\n" );
					}
				}
			}
			else
			{
				if ( !*bMultipleHosts )
				{
					fprintf( stderr, "ERROR! Cannot load functions from iphlpapi.dll or icmp.dll.\n" );
				}
			}
		}
		else
		{
			if ( !*bMultipleHosts )
			{
				fprintf( stderr, "ERROR! Cannot resolve host name to IP address.\n" );
			}
		}
	}

	WSACleanup();

	return bPingable;
}

VOID GuessSNMPCommunityStrings( CHAR szTarget[], BOOL *bMultipleHosts )
{
	FILE  *pCommStringFile;
	CHAR szCommunityString[ 128 ];

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

				if ( !*bMultipleHosts )
				{
					printf( "Trying community string... %s\n", szCommunityString );
				}

				SNMPConnect( szTarget, szCommunityString, bMultipleHosts );
			}
		}

		fclose( pCommStringFile );
	}
	else
	{
		if ( !*bMultipleHosts )
		{
			fprintf( stderr, "ERROR! Cannot open CommunityStrings.input file.\n" );
		}
	}
}

VOID SNMPConnect( CHAR szTarget[], CHAR szCommunityString[], BOOL *bMultipleHosts )
{
	BOOL                bLogonSuccess;
	LPSNMP_MGR_SESSION     smsSession;
	AsnObjectIdentifier        aoiOID;
	RFC1157VarBindList         vbInfo;
	DWORD                    dwResult;
	AsnInteger          aiErrorStatus;
	AsnInteger           aiErrorIndex;
	CHAR                   szHostname[ 128 ];
	AsnAny                     *pInfo;
	DWORD                           i;
	CHAR                szDescription[ 256 ];

	bLogonSuccess = FALSE;

	smsSession = SnmpMgrOpen( szTarget, szCommunityString, 1000, 1 );

	if ( smsSession != NULL )
	{
		if ( SnmpMgrStrToOid( ".1.3.6.1.2.1.1.5.0", &aoiOID ) )
		{
			vbInfo.len  = 1;
			vbInfo.list = NULL;

			vbInfo.list = (RFC1157VarBind *)SnmpUtilMemReAlloc( vbInfo.list, sizeof( RFC1157VarBind ) * vbInfo.len );

			vbInfo.list[0].name          = aoiOID;
			vbInfo.list[0].value.asnType = ASN_NULL;

			dwResult = SnmpMgrRequest( smsSession, ASN_RFC1157_GETREQUEST, &vbInfo, &aiErrorStatus, &aiErrorIndex );

			if ( dwResult != 0 )
			{
				strcpy( szHostname, "" );

				if ( aiErrorStatus == SNMP_ERRORSTATUS_NOERROR )
				{
					bLogonSuccess = TRUE;

					pInfo = &vbInfo.list[0].value;

					i = 0;

					while ( i < pInfo->asnValue.string.length )
					{
						szHostname[i] = pInfo->asnValue.string.stream[i];

						i++;
					}

					szHostname[i] = '\0';
				}

				if ( aiErrorStatus == SNMP_ERRORSTATUS_NOSUCHNAME )
				{
					bLogonSuccess = TRUE;
				}
			}

			SnmpUtilVarBindFree( &vbInfo.list[0] );

			SnmpUtilVarBindListFree( &vbInfo );
		}

		if ( SnmpMgrStrToOid( ".1.3.6.1.2.1.1.1.0", &aoiOID ) )
		{
			vbInfo.len  = 1;
			vbInfo.list = NULL;

			vbInfo.list = (RFC1157VarBind *)SnmpUtilMemReAlloc( vbInfo.list, sizeof( RFC1157VarBind ) * vbInfo.len );

			vbInfo.list[0].name          = aoiOID;
			vbInfo.list[0].value.asnType = ASN_NULL;

			dwResult = SnmpMgrRequest( smsSession, ASN_RFC1157_GETREQUEST, &vbInfo, &aiErrorStatus, &aiErrorIndex );

			if ( dwResult != 0 )
			{
				strcpy( szDescription, "" );

				if ( aiErrorStatus == SNMP_ERRORSTATUS_NOERROR )
				{
					bLogonSuccess = TRUE;

					pInfo = &vbInfo.list[0].value;

					i = 0;

					while ( i < pInfo->asnValue.string.length )
					{
						szDescription[i] = pInfo->asnValue.string.stream[i];

						i++;
					}

					szDescription[i] = '\0';

					for ( i = 0; i < strlen( szDescription ); i++ )
					{
						if ( szDescription[i] == '\r' || szDescription[i] == '\n' )
						{
							szDescription[i] = ' ';
						}
					}
				}

				if ( aiErrorStatus == SNMP_ERRORSTATUS_NOSUCHNAME )
				{
					bLogonSuccess = TRUE;
				}
			}

			SnmpUtilVarBindFree( &vbInfo.list[0] );

			SnmpUtilVarBindListFree( &vbInfo );
		}

		SnmpMgrClose( smsSession );
	}

	if ( bLogonSuccess )
	{
		if ( !*bMultipleHosts )
		{
			printf( "\n" );
			printf( "COMMUNITY STRING GUESSED! %s\n", szCommunityString );
		}

		LogGuessedCommunityStrings( szTarget, szCommunityString, szHostname, szDescription );
	}
	else
	{
		if ( !*bMultipleHosts )
		{
			fprintf( stderr, "ERROR! Cannot connect to host %s with community string \"%s\".\n", szTarget, szCommunityString );
		}
	}
}

VOID LogGuessedCommunityStrings( CHAR szTarget[], CHAR szCommunityString[], CHAR szHostname[], CHAR szDescription[] )
{
	FILE *pOutputFile;

	WaitForSingleObject( hSemaphore, INFINITE );

	pOutputFile = fopen( "SNMPCommunityStringInfo.txt", "r" );

	if ( pOutputFile != NULL )
	{
		fclose( pOutputFile );
	}
	else
	{
		pOutputFile = fopen( "SNMPCommunityStringInfo.txt", "w" );

		if ( pOutputFile != NULL )
		{
			fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
			fprintf( pOutputFile, "\n" );
			fprintf( pOutputFile, "Hostname\tSNMP Community String\tSNMP Hostname\tSNMP Description\n" );

			fclose( pOutputFile );
		}
	}

	pOutputFile = fopen( "SNMPCommunityStringInfo.txt", "a+" );

	if ( pOutputFile != NULL )
	{
		fprintf( pOutputFile, "%s\t%s\t%s\t%s\n", szTarget, szCommunityString, szHostname, szDescription );

		fclose( pOutputFile );
	}

	ReleaseSemaphore( hSemaphore, 1, NULL );
}

// Written by Reed Arvin | reed@canaudit.com
