//
// gcc source.c -o source.exe -lsnmpapi -lmgmtapi
//

#define _WIN32_WINNT 0x0500
#define WINVER       0x0500

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <process.h>
#include <mgmtapi.h>
#include <snmp.h>
#include <stdlib.h>
#include <time.h>

#define MAX_THREADS 64

VOID         ThreadedSub( VOID *pParameter );
VOID DownloadCiscoConfig( CHAR *szTarget, CHAR *szCommunityString, CHAR *szTFTPServerIP, BOOL *bMultipleHosts );

typedef struct _THREAD_ARGS
{
	CHAR          Target[ 128 ];
	CHAR CommunityString[ 128 ];
	CHAR    TFTPServerIP[ 128 ];
	BOOL   MultipleHosts;
} THREAD_ARGS, *PTHREAD_ARGS;

HANDLE hSemaphore;

INT nThreads = 0;

INT main( INT argc, CHAR *argv[] )
{
	CHAR     szTargetInput[ 128 ];
	CHAR szCommunityString[ 128 ];
	CHAR    szTFTPServerIP[ 128 ];
	FILE       *pInputFile;
	CHAR        szReadLine[ 128 ];
	CHAR          szTarget[ 128 ];

	PTHREAD_ARGS pThreadArgs;

	hSemaphore = CreateSemaphore( NULL, 1, 1, NULL );

	if ( argc == 4 )
	{
		strcpy( szTargetInput,     argv[1] );
		strcpy( szCommunityString, argv[2] );
		strcpy( szTFTPServerIP,    argv[3] );

		printf( "Running DownloadCiscoConfigViaSNMP v1.0 with the following arguments:\n" );
		printf( "[+] Host Input:       \"%s\"\n", szTargetInput );
		printf( "[+] Community String: \"%s\"\n", szCommunityString );
		printf( "[+] Destination IP:   \"%s\"\n", szTFTPServerIP );
		printf( "[+] # of Threads:     \"64\"\n" );
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
					strcpy( pThreadArgs->Target,          szTarget );
					strcpy( pThreadArgs->CommunityString, szCommunityString );
					strcpy( pThreadArgs->TFTPServerIP,    szTFTPServerIP );

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
		else
		{
			strcpy( szTarget, szTargetInput );

			pThreadArgs = (PTHREAD_ARGS)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( THREAD_ARGS ) );

			if ( pThreadArgs != NULL )
			{
				strcpy( pThreadArgs->Target,          szTarget );
				strcpy( pThreadArgs->CommunityString, szCommunityString );
				strcpy( pThreadArgs->TFTPServerIP,    szTFTPServerIP );

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
		printf( "DownloadCiscoConfigViaSNMP v1.0 | Canaudit, Inc. http://www.canaudit.com/\n" );
		printf( "\n" );
		printf( "Usage: DownloadCiscoConfigViaSNMP <hostname | ip input file> <community string> <tftp server ip>\n" );
		printf( "\n" );
		printf( "<hostname | ip input file>  -- required argument\n" );
		printf( "<community string>          -- required argument\n" );
		printf( "<tftp server ip>            -- required argument\n" );
		printf( "\n" );
		printf( "Examples:\n" );
		printf( "DownloadCiscoConfigViaSNMP 10.10.10.10 private 10.10.10.1\n" );
		printf( "DownloadCiscoConfigViaSNMP MyWindowsMachine private 10.10.10.1\n" );
		printf( "DownloadCiscoConfigViaSNMP IPInputFile.txt private 10.10.10.1\n" );
		printf( "\n" );
		printf( "(Written by Reed Arvin | reed@canaudit.com)\n" );
	}

	CloseHandle( hSemaphore );

	return 0;
}

VOID ThreadedSub( VOID *pParameter )
{
	CHAR          szTarget[ 128 ];
	CHAR szCommunityString[ 128 ];
	CHAR    szTFTPServerIP[ 128 ];
	BOOL    bMultipleHosts;

	PTHREAD_ARGS pThreadArgs;

	pThreadArgs = (PTHREAD_ARGS)pParameter;

	strcpy( szTarget,          pThreadArgs->Target );
	strcpy( szCommunityString, pThreadArgs->CommunityString );
	strcpy( szTFTPServerIP,    pThreadArgs->TFTPServerIP );

	bMultipleHosts = pThreadArgs->MultipleHosts;

	HeapFree( GetProcessHeap(), 0, pThreadArgs );

	if ( bMultipleHosts )
	{
		printf( "Spawning thread for host %s...\n", szTarget );
	}

	DownloadCiscoConfig( szTarget, szCommunityString, szTFTPServerIP, &bMultipleHosts );

	WaitForSingleObject( hSemaphore, INFINITE );

	nThreads--;

	ReleaseSemaphore( hSemaphore, 1, NULL );

	_endthread();
}

VOID DownloadCiscoConfig( CHAR szTarget[], CHAR szCommunityString[], CHAR szTFTPServerIP[], BOOL *bMultipleHosts )
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
	DWORD                    dwRandom;
	CHAR                        szOID[ 128 ];
	BOOL                    bContinue;
	DWORD                           j;
	DWORD                           k;
	CHAR                      szOctet[ 128 ];
	CHAR                 szConfigName[ 128 ];

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

		if ( bLogonSuccess )
		{
			if ( !*bMultipleHosts )
			{
				printf( "Logon successful using community string \"%s\" on host %s.\n", szCommunityString, szTarget );
				printf( "\n" );
				printf( "Issuing SNMP SET request 1 of 6...\n" );
			}

			srand( time( NULL ) );

			dwRandom = rand();

			sprintf( szOID, ".1.3.6.1.4.1.9.9.96.1.1.1.1.2.%d", dwRandom );

			if ( SnmpMgrStrToOid( szOID, &aoiOID ) )
			{
				vbInfo.len  = 1;
				vbInfo.list = NULL;

				vbInfo.list = (RFC1157VarBind *)SnmpUtilMemReAlloc( vbInfo.list, sizeof( RFC1157VarBind ) * vbInfo.len );

				vbInfo.list[0].name                  = aoiOID;
				vbInfo.list[0].value.asnType         = ASN_INTEGER;
				vbInfo.list[0].value.asnValue.number = 1;

				dwResult = SnmpMgrRequest( smsSession, ASN_RFC1157_SETREQUEST, &vbInfo, &aiErrorStatus, &aiErrorIndex );

				if ( dwResult != 0 )
				{
					if ( aiErrorStatus == SNMP_ERRORSTATUS_NOERROR )
					{
						bContinue = TRUE;
					}
					else
					{
						bContinue = FALSE;

						if ( !*bMultipleHosts )
						{
							fprintf( stderr, "ERROR! Write failed for community string \"%s\" on host %s.\n", szCommunityString, szTarget );
						}
					}
				}
				else
				{
					bContinue = FALSE;
				}

				SnmpUtilVarBindFree( &vbInfo.list[0] );

				SnmpUtilVarBindListFree( &vbInfo );
			}

			if ( bContinue )
			{
				if ( !*bMultipleHosts )
				{
					printf( "Issuing SNMP SET request 2 of 6...\n" );
				}

				sprintf( szOID, ".1.3.6.1.4.1.9.9.96.1.1.1.1.3.%d", dwRandom );

				if ( SnmpMgrStrToOid( szOID, &aoiOID ) )
				{
					vbInfo.len  = 1;
					vbInfo.list = NULL;

					vbInfo.list = (RFC1157VarBind *)SnmpUtilMemReAlloc( vbInfo.list, sizeof( RFC1157VarBind ) * vbInfo.len );

					vbInfo.list[0].name                  = aoiOID;
					vbInfo.list[0].value.asnType         = ASN_INTEGER;
					vbInfo.list[0].value.asnValue.number = 4;

					dwResult = SnmpMgrRequest( smsSession, ASN_RFC1157_SETREQUEST, &vbInfo, &aiErrorStatus, &aiErrorIndex );

					if ( dwResult != 0 )
					{
						if ( aiErrorStatus == SNMP_ERRORSTATUS_NOERROR )
						{
							bContinue = TRUE;
						}
						else
						{
							bContinue = FALSE;

							if ( !*bMultipleHosts )
							{
								fprintf( stderr, "ERROR! Write failed for community string \"%s\" on host %s.\n", szCommunityString, szTarget );
							}
						}
					}
					else
					{
						bContinue = FALSE;
					}

					SnmpUtilVarBindFree( &vbInfo.list[0] );

					SnmpUtilVarBindListFree( &vbInfo );
				}
			}

			if ( bContinue )
			{
				if ( !*bMultipleHosts )
				{
					printf( "Issuing SNMP SET request 3 of 6...\n" );
				}

				sprintf( szOID, ".1.3.6.1.4.1.9.9.96.1.1.1.1.4.%d", dwRandom );

				if ( SnmpMgrStrToOid( szOID, &aoiOID ) )
				{
					vbInfo.len  = 1;
					vbInfo.list = NULL;

					vbInfo.list = (RFC1157VarBind *)SnmpUtilMemReAlloc( vbInfo.list, sizeof( RFC1157VarBind ) * vbInfo.len );

					vbInfo.list[0].name                  = aoiOID;
					vbInfo.list[0].value.asnType         = ASN_INTEGER;
					vbInfo.list[0].value.asnValue.number = 1;

					dwResult = SnmpMgrRequest( smsSession, ASN_RFC1157_SETREQUEST, &vbInfo, &aiErrorStatus, &aiErrorIndex );

					if ( dwResult != 0 )
					{
						if ( aiErrorStatus == SNMP_ERRORSTATUS_NOERROR )
						{
							bContinue = TRUE;
						}
						else
						{
							bContinue = FALSE;

							if ( !*bMultipleHosts )
							{
								fprintf( stderr, "ERROR! Write failed for community string \"%s\" on host %s.\n", szCommunityString, szTarget );
							}
						}
					}
					else
					{
						bContinue = FALSE;
					}

					SnmpUtilVarBindFree( &vbInfo.list[0] );

					SnmpUtilVarBindListFree( &vbInfo );
				}
			}

			if ( bContinue )
			{
				if ( !*bMultipleHosts )
				{
					printf( "Issuing SNMP SET request 4 of 6...\n" );
				}

				sprintf( szOID, ".1.3.6.1.4.1.9.9.96.1.1.1.1.5.%d", dwRandom );

				if ( SnmpMgrStrToOid( szOID, &aoiOID ) )
				{
					vbInfo.len  = 1;
					vbInfo.list = NULL;

					vbInfo.list = (RFC1157VarBind *)SnmpUtilMemReAlloc( vbInfo.list, sizeof( RFC1157VarBind ) * vbInfo.len );

					vbInfo.list[0].name                          = aoiOID;
					vbInfo.list[0].value.asnType                 = ASN_IPADDRESS;
					vbInfo.list[0].value.asnValue.address.length = 4;
					vbInfo.list[0].value.asnValue.address.stream = (CHAR *)SnmpUtilMemAlloc( vbInfo.list[0].value.asnValue.address.length * sizeof( CHAR ) );

					i = 0;
					j = 0;

					for ( k = 0; k < strlen( szTFTPServerIP ); k++ )
					{
						if ( szTFTPServerIP[k] != '.' )
						{
							szOctet[j] = szTFTPServerIP[k];

							j++;
						}
						else
						{
							szOctet[j] = '\0';

							j = 0;

							vbInfo.list[0].value.asnValue.address.stream[i] = atoi( szOctet );

							i++;
						}
					}

					szOctet[j] = '\0';

					vbInfo.list[0].value.asnValue.address.stream[i] = atoi( szOctet );

					dwResult = SnmpMgrRequest( smsSession, ASN_RFC1157_SETREQUEST, &vbInfo, &aiErrorStatus, &aiErrorIndex );

					if ( dwResult != 0 )
					{
						if ( aiErrorStatus == SNMP_ERRORSTATUS_NOERROR )
						{
							bContinue = TRUE;
						}
						else
						{
							bContinue = FALSE;

							if ( !*bMultipleHosts )
							{
								fprintf( stderr, "ERROR! Write failed for community string \"%s\" on host %s.\n", szCommunityString, szTarget );
							}
						}
					}
					else
					{
						bContinue = FALSE;
					}

					SnmpUtilVarBindFree( &vbInfo.list[0] );

					SnmpUtilVarBindListFree( &vbInfo );
				}
			}

			if ( bContinue )
			{
				if ( !*bMultipleHosts )
				{
					printf( "Issuing SNMP SET request 5 of 6...\n" );
				}

				sprintf( szOID, ".1.3.6.1.4.1.9.9.96.1.1.1.1.6.%d", dwRandom );

				if ( SnmpMgrStrToOid( szOID, &aoiOID ) )
				{

					sprintf( szConfigName, "%s-%s.CiscoConfig", szTarget, szHostname );

					vbInfo.len  = 1;
					vbInfo.list = NULL;

					vbInfo.list = (RFC1157VarBind *)SnmpUtilMemReAlloc( vbInfo.list, sizeof( RFC1157VarBind ) * vbInfo.len );

					vbInfo.list[0].name                          = aoiOID;
					vbInfo.list[0].value.asnType                 = ASN_OCTETSTRING;
					vbInfo.list[0].value.asnValue.string.dynamic = TRUE;
					vbInfo.list[0].value.asnValue.string.length  = strlen( szConfigName );
					vbInfo.list[0].value.asnValue.string.stream  = (CHAR *)SnmpUtilMemAlloc( vbInfo.list[0].value.asnValue.string.length * sizeof( CHAR ) );

					memcpy( vbInfo.list[0].value.asnValue.string.stream, szConfigName, vbInfo.list[0].value.asnValue.string.length );

					dwResult = SnmpMgrRequest( smsSession, ASN_RFC1157_SETREQUEST, &vbInfo, &aiErrorStatus, &aiErrorIndex );

					if ( dwResult != 0 )
					{
						if ( aiErrorStatus == SNMP_ERRORSTATUS_NOERROR )
						{
							bContinue = TRUE;
						}
						else
						{
							bContinue = FALSE;

							if ( !*bMultipleHosts )
							{
								fprintf( stderr, "ERROR! Write failed for community string \"%s\" on host %s.\n", szCommunityString, szTarget );
							}
						}
					}
					else
					{
						bContinue = FALSE;
					}

					SnmpUtilVarBindFree( &vbInfo.list[0] );

					SnmpUtilVarBindListFree( &vbInfo );
				}
			}

			if ( bContinue )
			{
				if ( !*bMultipleHosts )
				{
					printf( "Issuing SNMP SET request 6 of 6...\n" );
				}

				sprintf( szOID, ".1.3.6.1.4.1.9.9.96.1.1.1.1.14.%d", dwRandom );

				if ( SnmpMgrStrToOid( szOID, &aoiOID ) )
				{
					vbInfo.len  = 1;
					vbInfo.list = NULL;

					vbInfo.list = (RFC1157VarBind *)SnmpUtilMemReAlloc( vbInfo.list, sizeof( RFC1157VarBind ) * vbInfo.len );

					vbInfo.list[0].name                  = aoiOID;
					vbInfo.list[0].value.asnType         = ASN_INTEGER;
					vbInfo.list[0].value.asnValue.number = 1;

					dwResult = SnmpMgrRequest( smsSession, ASN_RFC1157_SETREQUEST, &vbInfo, &aiErrorStatus, &aiErrorIndex );

					if ( dwResult != 0 )
					{
						if ( aiErrorStatus == SNMP_ERRORSTATUS_NOERROR )
						{
							bContinue = TRUE;

							if ( !*bMultipleHosts )
							{
								printf( "\n" );
								printf( "Successfully initiated TFTP backup for Cisco config on host %s.\n", szTarget );
							}
						}
						else
						{
							bContinue = FALSE;

							if ( !*bMultipleHosts )
							{
								fprintf( stderr, "ERROR! Write failed for community string \"%s\" on host %s.\n", szCommunityString, szTarget );
							}
						}
					}
					else
					{
						bContinue = FALSE;
					}

					SnmpUtilVarBindFree( &vbInfo.list[0] );

					SnmpUtilVarBindListFree( &vbInfo );
				}
			}
		}

		SnmpMgrClose( smsSession );
	}

	if ( bLogonSuccess == FALSE )
	{
		if ( !*bMultipleHosts )
		{
			fprintf( stderr, "ERROR! Cannot connect to host %s with community string \"%s\".\n", szTarget, szCommunityString );
		}
	}
}

// Written by Reed Arvin | reed@canaudit.com
