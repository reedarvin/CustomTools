//
// gcc BecomeUser.c -o BecomeUser.exe -lmpr -ladvapi32
//

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <winnetwk.h>
#include <process.h>

BOOL               Connect( CHAR *szTarget, CHAR *szUsername, CHAR *szPassword );
VOID         RunBecomeUser( CHAR *szTarget, CHAR *szCommand, CHAR *szPID );
VOID ManagePipeInformation( HANDLE *hPipe );
VOID   ReadPipeInformation( VOID *pParameter );
VOID            Disconnect( CHAR *szTarget );

typedef struct _THREAD_ARGS
{
	HANDLE *Pipe;
} THREAD_ARGS, *PTHREAD_ARGS;

BOOL bReadPipe;

INT main( INT argc, CHAR *argv[] )
{
	CHAR       szTarget[ 128 ];
	CHAR     szUsername[ 128 ];
	CHAR     szPassword[ 128 ];
	CHAR      szCommand[ 128 ];
	CHAR          szPID[ 128 ];
	CHAR szFullUsername[ 128 ];

	if ( argc > 4 && argc < 7 )
	{
		strcpy( szTarget,   argv[1] );
		strcpy( szUsername, argv[2] );
		strcpy( szPassword, argv[3] );
		strcpy( szCommand,  argv[4] );
		strcpy( szPID,      "0" );

		printf( "Running BecomeUser v1.1 with the following arguments:\n" );
		printf( "[+] Target Host: \"%s\"\n", szTarget );
		printf( "[+] Username:    \"%s\"\n", szUsername );
		printf( "[+] Password:    \"%s\"\n", szPassword );
		printf( "[+] Command:     \"%s\"\n", szCommand );

		if ( argc == 6 )
		{
			strcpy( szPID, argv[5] );

			printf( "[+] Process ID:  \"%s\"\n", szPID );
		}

		printf( "\n" );

		if ( strstr( szUsername, "\\" ) == NULL && strcmp( szUsername, "+" ) != 0 && strcmp( szUsername, "" ) != 0 )
		{
			sprintf( szFullUsername, "%s\\%s", szTarget, szUsername );

			if ( Connect( szTarget, szFullUsername, szPassword ) )
			{
				RunBecomeUser( szTarget, szCommand, szPID );

				Disconnect( szTarget );
			}
		}
		else
		{
			if ( strcmp( szUsername, "+" ) == 0 && strcmp( szPassword, "+" ) == 0 )
			{
				RunBecomeUser( szTarget, szCommand, szPID );
			}
			else
			{
				if ( Connect( szTarget, szUsername, szPassword ) )
				{
					RunBecomeUser( szTarget, szCommand, szPID );

					Disconnect( szTarget );
				}
			}
		}
	}
	else
	{
		printf( "BecomeUser v1.1 | https://github.com/reedarvin\n" );
		printf( "\n" );
		printf( "Usage: BecomeUser <hostname | ip address> <username> <password> <command> <process id>\n" );
		printf( "\n" );
		printf( "<hostname | ip address>  -- required argument\n" );
		printf( "<username>               -- required argument\n" );
		printf( "<password>               -- required argument\n" );
		printf( "<command>                -- required argument\n" );
		printf( "<process id>             -- optional argument\n" );
		printf( "\n" );
		printf( "If the <username> and <password> arguments are both plus signs (+), the\n" );
		printf( "existing credentials of the user running this utility will be used.\n" );
		printf( "\n" );
		printf( "Examples:\n" );
		printf( "BecomeUser 10.10.10.10 + + \"whoami > c:\\test.txt\"\n" );
		printf( "BecomeUser 10.10.10.10 administrator password cmd.exe\n" );
		printf( "BecomeUser 10.10.10.10 administrator password cmd.exe 1492\n" );
		printf( "\n" );
		printf( "BecomeUser MyWindowsMachine + + \"whoami > c:\\test.txt\"\n" );
		printf( "BecomeUser MyWindowsMachine administrator password cmd.exe\n" );
		printf( "BecomeUser MyWindowsMachine administrator password cmd.exe 1492\n" );
		printf( "\n" );
		printf( "(Written by Reed Arvin | reedlarvin@gmail.com)\n" );
	}

	return 0;
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

VOID RunBecomeUser( CHAR szTarget[], CHAR szCommand[], CHAR szPID[] )
{
	CHAR                   szRemoteEXEPath[ 256 ];
	SC_HANDLE                 schSCManager;
	SC_HANDLE                   schService;
	CHAR                      szTmpCommand[ 128 ];
	CHAR                   szUseNamedPipes[ 128 ];
	BOOL                    bUseNamedPipes;
	CHAR                            *pArgs[ 3 ];
	DWORD                                i;
	SERVICE_STATUS_PROCESS        ssStatus;
	DWORD                    dwBytesNeeded;
	CHAR                        szPipeName[ 128 ];
	HANDLE                           hPipe;
	CHAR                     szSrcFileName[ 128 ];
	CHAR                     szDstFileName[ 128 ];

	sprintf( szRemoteEXEPath, "\\\\%s\\ADMIN$\\System32\\BecomeUserSvc.exe", szTarget );

	if ( CopyFile( "BecomeUserSvc.exe", szRemoteEXEPath, FALSE ) )
	{
		schSCManager = OpenSCManager( szTarget, NULL, SC_MANAGER_ALL_ACCESS );

		if ( schSCManager != NULL )
		{
			schService = CreateService( schSCManager, "BecomeUserSvc", "BecomeUser Service", SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE, "%SystemRoot%\\System32\\BecomeUserSvc.exe", NULL, NULL, NULL, NULL, NULL );

			if ( schService != NULL )
			{
				strcpy( szTmpCommand, szCommand );

				strupr( szTmpCommand );

				if ( strcmp( szTmpCommand, "CMD" ) == 0 || strcmp( szTmpCommand, "CMD.EXE" ) == 0 )
				{
					strcpy( szUseNamedPipes, "1" );

					bUseNamedPipes = TRUE;
				}
				else
				{
					strcpy( szUseNamedPipes, "0" );

					bUseNamedPipes = FALSE;
				}

				pArgs[0] = szCommand;
				pArgs[1] = szPID;
				pArgs[2] = szUseNamedPipes;

				if ( StartService( schService, 3, (LPCSTR *)pArgs ) )
				{
					printf( "Waiting for BecomeUser service to start on host %s", szTarget );

					i = 0;

					while ( i < 10 )
					{
						if ( QueryServiceStatusEx( schService, SC_STATUS_PROCESS_INFO, (BYTE *)&ssStatus, sizeof( SERVICE_STATUS_PROCESS ), &dwBytesNeeded ) )
						{
							if ( ssStatus.dwCurrentState == SERVICE_RUNNING )
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
							fprintf( stderr, "ERROR! Cannot query BecomeUser service status on host %s.\n", szTarget );

							break;
						}

						Sleep( 1000 );

						i++;
					}

					printf( "\n" );
					printf( "\n" );

					if ( bUseNamedPipes )
					{
						i = 0;

						while ( i < 5 )
						{
							sprintf( szPipeName, "\\\\%s\\pipe\\BecomeUser", szTarget );

							hPipe = CreateFile( szPipeName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
 
							if ( hPipe != INVALID_HANDLE_VALUE )
							{
								ManagePipeInformation( &hPipe );

								CloseHandle( hPipe );

								break;
							}
							else
							{
								if ( GetLastError() != ERROR_PIPE_BUSY )
								{
									if ( i == 4 )
									{
										fprintf( stderr, "ERROR! Cannot connect to pipe %s.\n", szPipeName );
									}
								}
								else
								{
									if ( WaitNamedPipe( szPipeName, 10000 ) )
									{
										ManagePipeInformation( &hPipe );

										CloseHandle( hPipe );

										break;
									}
									else
									{
										if ( i == 4 )
										{
											fprintf( stderr, "ERROR! Wait time expired while connecting to pipe %s.\n", szPipeName );
										}
									}
								}
							}

							Sleep( 1000 );

							i++;
						}
					}
					else
					{
						printf( "Launching the following command on host %s:\n", szCommand );
						printf( "\n" );
						printf( "cmd.exe /c %s\n", szCommand );
					}

					printf( "\n" );
					printf( "Waiting for BecomeUser service to terminate on host %s", szTarget );

					i = 0;

					while ( i < 10 )
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
							fprintf( stderr, "ERROR! Cannot query BecomeUser service status on host %s.\n", szTarget );

							break;
						}

						Sleep( 1000 );

						i++;
					}

					printf( "\n" );
					printf( "\n" );

					sprintf( szSrcFileName, "\\\\%s\\ADMIN$\\System32\\ErrorLog.txt", szTarget );
					sprintf( szDstFileName, "%s-ErrorLog.txt", szTarget );

					if ( CopyFile( szSrcFileName, szDstFileName, FALSE ) )
					{
						printf( "Retrieved file %s-ErrorLog.txt\n", szTarget );

						if ( !DeleteFile( szSrcFileName ) )
						{
							fprintf( stderr, "ERROR! Cannot delete file \\\\%s\\ADMIN$\\System32\\ErrorLog.txt.\n", szTarget );
						}
					}
				}
				else
				{
					fprintf( stderr, "ERROR! Cannot start BecomeUser service on host %s.\n", szTarget );
				}

				if ( !DeleteService( schService ) != 0 )
				{
					fprintf( stderr, "ERROR! Cannot remove BecomeUser service from host %s.\n", szTarget );
				}

				CloseServiceHandle( schService );
			}
			else
			{
				fprintf( stderr, "ERROR! Cannot create BecomeUser service on host %s.\n", szTarget );
			}

			CloseServiceHandle( schSCManager );
		}
		else
		{
			fprintf( stderr, "ERROR! Cannot open service manager on host %s.\n", szTarget );
		}

		if ( !DeleteFile( szRemoteEXEPath ) )
		{
			fprintf( stderr, "ERROR! Cannot delete file \\\\%s\\ADMIN$\\System32\\BecomeUserSvc.exe.\n", szTarget );
		}
	}
	else
	{
		fprintf( stderr, "ERROR! Cannot copy file BecomeUserSvc.exe to \\\\%s\\ADMIN$\\System32\\.\n", szTarget );
	}
}

VOID ManagePipeInformation( HANDLE *hPipe )
{
	CHAR      szTempSend[ 1024 ];
	CHAR          szSend[ 1024 ];
	DWORD dwBytesWritten;

	PTHREAD_ARGS pThreadArgs;

	bReadPipe = TRUE;

	pThreadArgs = (PTHREAD_ARGS)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( THREAD_ARGS ) );

	if ( pThreadArgs != NULL )
	{
		pThreadArgs->Pipe = hPipe;

		_beginthread( ReadPipeInformation, 0, (VOID *)pThreadArgs );
	}

	while ( TRUE )
	{
		gets( szTempSend );

		sprintf( szSend, "%s\n", szTempSend );

		if ( !WriteFile( *hPipe, szSend, strlen( szSend ), &dwBytesWritten, NULL ) )
		{
			fprintf( stderr, "ERROR! Cannot write to pipe.\n" );

			break;
		}

		strupr( szTempSend );

		if ( strcmp( szTempSend, "EXIT" ) == 0 )
		{
			bReadPipe = FALSE;

			break;
		}
	}
}

VOID ReadPipeInformation( VOID *pParameter )
{
	HANDLE           *hPipe;
	DWORD  dwBytesAvailable;
	CHAR             szRecv[ 4096 ];
	DWORD       dwBytesRead;

	PTHREAD_ARGS pThreadArgs;

	pThreadArgs = (PTHREAD_ARGS)pParameter;

	hPipe = pThreadArgs->Pipe;

	HeapFree( GetProcessHeap(), 0, pThreadArgs );

	while ( bReadPipe )
	{
		if ( PeekNamedPipe( *hPipe, NULL, 0, NULL, &dwBytesAvailable, NULL ) )
		{
			if ( dwBytesAvailable > 0 )
			{
				if ( dwBytesAvailable > ( sizeof( szRecv ) - 1 ) )
				{
					if ( ReadFile( *hPipe, szRecv, sizeof( szRecv ) - 1, &dwBytesRead, NULL ) )
					{
						szRecv[dwBytesRead] = '\0';

						printf( "%s", szRecv );
					}
				}
				else
				{
					if ( ReadFile( *hPipe, szRecv, dwBytesAvailable, &dwBytesRead, NULL ) )
					{
						szRecv[dwBytesRead] = '\0';

						printf( "%s", szRecv );
					}
				}
			}
		}

		Sleep( 10 );
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

// Written by Reed Arvin | reedlarvin@gmail.com
