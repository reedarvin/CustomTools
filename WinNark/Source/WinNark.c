//
// gcc source.c -o source.exe
//

#include <windows.h>
#include <ntsecapi.h>
#include <string.h>
#include <stdio.h>

BOOL    EnableLogonAuditing( VOID );
BOOL GetNewestSecurityEvent( DWORD *dwNewestEvent );
VOID     ReadSecurityEvents( DWORD *dwStartEvent );

INT main( INT argc, CHAR *argv[] )
{
	BOOL      bContinue;
	DWORD  dwStartEvent;
	DWORD dwNewestEvent;

	if ( EnableLogonAuditing() )
	{
		bContinue = TRUE;

		dwStartEvent = 0;

		while ( bContinue )
		{
			if ( GetNewestSecurityEvent( &dwNewestEvent ) )
			{
				if ( dwStartEvent == 0 || ( dwStartEvent > ( dwNewestEvent + 1 ) ) )
				{
					dwStartEvent = dwNewestEvent + 1;
				}

				ReadSecurityEvents( &dwStartEvent );
			}
			else
			{
				printf( "ERROR!\n" );

				bContinue = FALSE;
			}

			Sleep( 60000 );
		}
	}

	return 0;
}

BOOL EnableLogonAuditing( VOID )
{
	BOOL                              bReturn;
	LSA_OBJECT_ATTRIBUTES       loaAttributes;
	NTSTATUS                         ntStatus;
	LSA_HANDLE               lsahPolicyHandle;
	POLICY_AUDIT_EVENTS_INFO           *pInfo;

	bReturn = FALSE;

	ZeroMemory( &loaAttributes, sizeof( loaAttributes ) );

	ntStatus = LsaOpenPolicy( NULL, &loaAttributes, POLICY_ALL_ACCESS, &lsahPolicyHandle );

	if ( ntStatus == 0 )
	{
		ntStatus = LsaQueryInformationPolicy( lsahPolicyHandle, PolicyAuditEventsInformation, (PVOID *)&pInfo);

		if ( ntStatus == 0 )
		{
			pInfo->AuditingMode = TRUE;

			switch ( pInfo->EventAuditingOptions[(POLICY_AUDIT_EVENT_TYPE)AuditCategoryLogon] )
			{
				case POLICY_AUDIT_EVENT_SUCCESS:
					pInfo->EventAuditingOptions[(POLICY_AUDIT_EVENT_TYPE)AuditCategoryLogon] = (POLICY_AUDIT_EVENT_SUCCESS | POLICY_AUDIT_EVENT_FAILURE);

					break;

				case POLICY_AUDIT_EVENT_FAILURE:
					break;

				case (POLICY_AUDIT_EVENT_SUCCESS | POLICY_AUDIT_EVENT_FAILURE):
					break;

				default:
					pInfo->EventAuditingOptions[(POLICY_AUDIT_EVENT_TYPE)AuditCategoryLogon] = POLICY_AUDIT_EVENT_FAILURE;

					break;
			}

			ntStatus = LsaSetInformationPolicy( lsahPolicyHandle, PolicyAuditEventsInformation, (VOID *)pInfo);

			if ( ntStatus == 0 )
			{
				bReturn = TRUE;
			}

			LsaFreeMemory( pInfo );
		}

		LsaClose( lsahPolicyHandle );
	}

	return bReturn;
}

BOOL GetNewestSecurityEvent( DWORD *dwNewestEvent )
{
	BOOL         bReturn;
	HANDLE     hEventLog;
	DWORD  dwOldestEvent;
	DWORD    dwNumEvents;

	bReturn = FALSE;

	hEventLog = OpenEventLog( NULL, "Security" );

	if ( hEventLog != NULL )
	{
		if ( GetOldestEventLogRecord( hEventLog, &dwOldestEvent ) )
		{
			if ( GetNumberOfEventLogRecords( hEventLog, &dwNumEvents) )
			{
				if ( dwOldestEvent > 1 )
				{
					*dwNewestEvent = dwNumEvents + dwOldestEvent - 1;
				}
				else
				{
					*dwNewestEvent = dwNumEvents;
				}

				bReturn = TRUE;
			}
		}

		CloseEventLog( hEventLog );
	}

	return bReturn;
}

VOID ReadSecurityEvents( DWORD *dwStartEvent )
{
	HANDLE                hEventLog;
	EVENTLOGRECORD *pEventLogRecord; 
	BYTE                    bBuffer[ 65536 ];
	DWORD                    dwRead;
	DWORD                  dwNeeded;
	DWORD                 dwEventID;
	DWORD              dwNumStrings;
	unsigned char         *pStrings;
	CHAR                     szUser[ 128 ];
	CHAR              szWorkstation[ 128 ];
	DWORD                         i;
	CHAR                szCacheFile[ 128 ];
	FILE                *pCacheFile;
 
	hEventLog = OpenEventLog( NULL, "Security" );

	if ( hEventLog != NULL )
	{
		pEventLogRecord = (EVENTLOGRECORD *)&bBuffer;

		if ( ReadEventLog( hEventLog, EVENTLOG_FORWARDS_READ | EVENTLOG_SEEK_READ, *dwStartEvent, pEventLogRecord, sizeof( bBuffer ), &dwRead, &dwNeeded ) )
		{
			while ( dwRead > 0 )
			{
				dwEventID = pEventLogRecord->EventID;

				if ( dwEventID == 529 )
				{
					dwNumStrings = pEventLogRecord->NumStrings;

					pStrings = ( (BYTE *)pEventLogRecord + pEventLogRecord->StringOffset );

					strcpy( szUser,        "" );
					strcpy( szWorkstation, "" );

					for ( i = 0; i < dwNumStrings; i++ )
					{
						if ( i == 0 )
						{
							strcpy( szUser, pStrings );
						}

						if ( i == 5 )
						{
							strcpy( szWorkstation, pStrings );
						}

						pStrings = pStrings + strlen( pStrings ) + 1;
					}

					if ( strcmp( szUser, "" ) != 0 && strcmp( szWorkstation, "" ) != 0 )
					{
						printf( "Bad password attempt for user \"%s\" from workstation \"%s\".\n", szUser, szWorkstation );

						sprintf( szCacheFile, "WkstaCache\\%s.wksta", szWorkstation );

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
						}

						pCacheFile = fopen( szCacheFile, "a+" );

						if ( pCacheFile != NULL )
						{
							fprintf( pCacheFile, "%s\n", szUser );

							fclose( pCacheFile );
						}
					}
				}

				dwRead = dwRead - pEventLogRecord->Length;

				pEventLogRecord = (EVENTLOGRECORD *)( (BYTE *)pEventLogRecord + pEventLogRecord->Length );

				*dwStartEvent = *dwStartEvent + 1;
			}
		}

		CloseEventLog( hEventLog );
	}
}

// Written by Reed Arvin | reedarvin@gmail.com
