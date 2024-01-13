//
// gcc -c BecomeUserSvc.c -o BecomeUserSvc.exe -luserenv
//

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <userenv.h>

typedef BOOL  (WINAPI      *EnumProcesses)( DWORD *, DWORD, DWORD * );
typedef BOOL  (WINAPI *EnumProcessModules)( HANDLE, HMODULE *, DWORD, DWORD * );
typedef DWORD (WINAPI  *GetModuleBaseName)( HANDLE, HMODULE, CHAR *, DWORD );

INT WINAPI                 ServiceMain( INT argc, CHAR *argv[] );
VOID WINAPI       MyServiceCtrlHandler( DWORD dwOption );
BOOL                    GetExplorerPID( DWORD *szExplorerPID );
VOID                   WriteToErrorLog( CHAR *szErrorMsg );
VOID        CreateProcessUsingPIDToken( CHAR *szCommand, DWORD *dwPID, DWORD *dwUseNamedPipes );
VOID           GetTokenUserInformation( HANDLE *hToken, CHAR *szUsername, DWORD *dwPID );

SERVICE_STATUS        MyServiceStatus;
SERVICE_STATUS_HANDLE MyServiceStatusHandle;

INT main( INT argc, CHAR *argv[] )
{
	SERVICE_TABLE_ENTRY DispatchTable[] = { { "BecomeUserSvc", (LPSERVICE_MAIN_FUNCTION)ServiceMain }, { NULL, NULL } };

	StartServiceCtrlDispatcher( DispatchTable );

	return 0;
}

INT WINAPI ServiceMain( INT argc, CHAR *argv[] )
{
	CHAR        szCommand[ 128 ];
	CHAR            szPID[ 128 ];
	CHAR  szUseNamedPipes[ 128 ];
	DWORD dwUseNamedPipes;
	DWORD   dwExplorerPID;
	DWORD           dwPID;

	strcpy( szCommand,       argv[1] );
	strcpy( szPID,           argv[2] );
	strcpy( szUseNamedPipes, argv[3] );

	dwUseNamedPipes = atoi( szUseNamedPipes );

	MyServiceStatus.dwServiceType             = SERVICE_WIN32;
	MyServiceStatus.dwCurrentState            = SERVICE_STOP;
	MyServiceStatus.dwControlsAccepted        = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;
	MyServiceStatus.dwWin32ExitCode           = 0;
	MyServiceStatus.dwServiceSpecificExitCode = 0;
	MyServiceStatus.dwCheckPoint              = 0;
	MyServiceStatus.dwWaitHint                = 0;

	MyServiceStatusHandle = RegisterServiceCtrlHandler( "BecomeUserSvc", MyServiceCtrlHandler );

	if ( MyServiceStatusHandle != 0 )
	{
		MyServiceStatus.dwCurrentState = SERVICE_START_PENDING;

		if ( SetServiceStatus( MyServiceStatusHandle, &MyServiceStatus ) )
		{
			MyServiceStatus.dwCurrentState = SERVICE_RUNNING;
 
			if ( SetServiceStatus( MyServiceStatusHandle, &MyServiceStatus ) )
			{
				if ( strcmp( szPID, "0" ) == 0 )
				{
					if ( GetExplorerPID( &dwExplorerPID ) )
					{
						CreateProcessUsingPIDToken( szCommand, &dwExplorerPID, &dwUseNamedPipes );
					}
					else
					{
						WriteToErrorLog( "ERROR! Cannot get process ID for explorer.exe.\n" );
					}
				}
				else
				{
					dwPID = atoi( szPID );

					CreateProcessUsingPIDToken( szCommand, &dwPID, &dwUseNamedPipes );
				}
			}
		}
	}

	MyServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;

	if ( SetServiceStatus( MyServiceStatusHandle, &MyServiceStatus ) )
	{
		MyServiceStatus.dwCurrentState = SERVICE_ACCEPT_STOP;

		SetServiceStatus( MyServiceStatusHandle, &MyServiceStatus );
	}

	return 0;
}

VOID WINAPI MyServiceCtrlHandler( DWORD dwOption )
{ 
	switch ( dwOption )
	{
		case SERVICE_CONTROL_PAUSE:
			MyServiceStatus.dwCurrentState = SERVICE_PAUSED;

			SetServiceStatus( MyServiceStatusHandle, &MyServiceStatus );

			break;

		case SERVICE_CONTROL_CONTINUE:
			MyServiceStatus.dwCurrentState = SERVICE_RUNNING;

			SetServiceStatus( MyServiceStatusHandle, &MyServiceStatus );

			break;
 
		case SERVICE_CONTROL_STOP:
			break;

		case SERVICE_CONTROL_INTERROGATE:
			break;

		default:
			break;
	}
}

BOOL GetExplorerPID( DWORD *dwExplorerPID )
{
	BOOL                           bReturn;
	HANDLE                          hPSAPI;
	EnumProcesses           pEnumProcesses;
	EnumProcessModules pEnumProcessModules;
	GetModuleBaseName   pGetModuleBaseName;
	DWORD                     dwProcessIDs[ 2048 ];
	DWORD                         dwNeeded;
	DWORD                      dwProcesses;
	unsigned int                         i;
	HANDLE                        hProcess;
	HMODULE                        hModule;
	CHAR                     szProcessName[ 256 ];

	bReturn = FALSE;

	hPSAPI = LoadLibrary( "psapi.dll" );

	pEnumProcesses      =      (EnumProcesses)GetProcAddress( hPSAPI, "EnumProcesses" );
	pEnumProcessModules = (EnumProcessModules)GetProcAddress( hPSAPI, "EnumProcessModules" );
	pGetModuleBaseName  =  (GetModuleBaseName)GetProcAddress( hPSAPI, "GetModuleBaseNameA" );

	if ( pEnumProcesses && pEnumProcessModules && pGetModuleBaseName )
	{
		if ( pEnumProcesses( dwProcessIDs, sizeof( dwProcessIDs ), &dwNeeded ) )
		{
			dwProcesses = dwNeeded / sizeof( DWORD );

			for ( i = 0; i < dwProcesses; i++ )
			{
				if ( dwProcessIDs[i] != 0 )
				{
					hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessIDs[i] );

					if ( hProcess != NULL )
					{
						if ( pEnumProcessModules( hProcess, &hModule, sizeof( hModule ), &dwNeeded ) )
						{
							pGetModuleBaseName( hProcess, hModule, szProcessName, sizeof( szProcessName ) / sizeof( CHAR ) );

							strupr( szProcessName );

							if ( strcmp( szProcessName, "EXPLORER.EXE" ) == 0 )
							{
								bReturn = TRUE;

								*dwExplorerPID = dwProcessIDs[i];

								CloseHandle( hProcess );

								break;
							}
						}

						CloseHandle( hProcess );
					}
				}
			}
		}

		FreeLibrary( hPSAPI );
	}
	else
	{
		WriteToErrorLog( "ERROR! Cannot load Psapi.dll functions on remote host.\n" );
	}

	return bReturn;
}

VOID WriteToErrorLog( CHAR szErrorMsg[] )
{
	FILE *pOutputFile;

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
			fclose( pOutputFile );
		}
	}

	pOutputFile = fopen( "ErrorLog.txt", "a+" );

	if ( pOutputFile != NULL )
	{
		fprintf( pOutputFile, "%s", szErrorMsg );

		fclose( pOutputFile );
	}
}

VOID CreateProcessUsingPIDToken( CHAR szCommand[], DWORD *dwPID, DWORD *dwUseNamedPipes )
{
	HANDLE                    hProcess;
	HANDLE                     hToken1;
	CHAR                    szUsername[ 256 ];
	HANDLE                     hToken2;
	CHAR                   szWinSysDir[ 256 ];
	VOID                 *pEnvironment;
	CHAR                    szPipeName[ 128 ];
	SECURITY_ATTRIBUTES             sa;
	BOOL                bPipeConnected;
	HANDLE                       hPipe;
	CHAR                    szErrorMsg[ 128 ];
	CHAR                     szCmdLine[ 256 ];
	WCHAR                   wszCmdLine[ 512 ];
	STARTUPINFOW                    si;
	PROCESS_INFORMATION             pi;
	CHAR                     szAppPath[ 128 ];
	WCHAR                   wszAppPath[ 256 ];

	hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, *dwPID );

	if ( hProcess != NULL )
	{
		if ( OpenProcessToken( hProcess, TOKEN_ALL_ACCESS, &hToken1 ) )
		{
			strcpy( szUsername, "" );

			GetTokenUserInformation( &hToken1, szUsername, dwPID );

			if ( DuplicateTokenEx( hToken1, MAXIMUM_ALLOWED, NULL, SecurityImpersonation, TokenPrimary, &hToken2 ) )
			{
				if ( GetSystemDirectory( szWinSysDir, 256 ) > 0 )
				{
					if ( CreateEnvironmentBlock( &pEnvironment, hToken2, FALSE ) )
					{
						if ( *dwUseNamedPipes == 1 )
						{
							strcpy( szPipeName, "\\\\.\\pipe\\BecomeUser" );

							sa.nLength              = sizeof( SECURITY_ATTRIBUTES );
							sa.lpSecurityDescriptor = NULL;
							sa.bInheritHandle       = TRUE;

							bPipeConnected = FALSE;

							hPipe = CreateNamedPipe( szPipeName, PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 1, 4096, 4096, 0, &sa );

							if ( hPipe != INVALID_HANDLE_VALUE )
							{
								if ( ConnectNamedPipe( hPipe, NULL ) )
								{
									bPipeConnected = TRUE;
								}
								else
								{
									if ( GetLastError() == ERROR_PIPE_CONNECTED )
									{
										bPipeConnected = TRUE;
									}
								}
							}
							else
							{
								sprintf( szErrorMsg, "ERROR! Cannot create named pipe %s.\n", szPipeName );

								WriteToErrorLog( szErrorMsg );
							}

							if ( bPipeConnected )
							{
								sprintf( szCmdLine, "\"%s\\cmd.exe\" /K ECHO OFF & ECHO Running as user %s... & ECHO ON", szWinSysDir, szUsername );

								MultiByteToWideChar( CP_ACP, 0, szCmdLine, strlen( szCmdLine ) + 1, wszCmdLine, sizeof( wszCmdLine ) / sizeof( wszCmdLine[0] ) );

								ZeroMemory( &si, sizeof( si ) );

								si.cb          = sizeof( si );
								si.hStdError   = hPipe;
								si.hStdOutput  = hPipe;
								si.hStdInput   = hPipe;
								si.dwFlags     = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
								si.wShowWindow = SW_HIDE;

								ZeroMemory( &pi, sizeof( pi ) );

								if ( CreateProcessAsUserW( hToken2, NULL, wszCmdLine, NULL, NULL, TRUE, CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT, pEnvironment, NULL, &si, &pi ) )
								{
									WaitForSingleObject( pi.hProcess, INFINITE );

									CloseHandle( pi.hProcess );
									CloseHandle( pi.hThread );
								}
								else
								{
									sprintf( szErrorMsg, "ERROR! Error %d while creating process as user %s.\n", GetLastError(), szUsername );

									WriteToErrorLog( szErrorMsg );
								}

								CloseHandle( hPipe );
							}
						}
						else
						{
							sprintf( szCmdLine, "cmd.exe /c %s", szCommand );
							sprintf( szAppPath, "%s\\cmd.exe", szWinSysDir );

							MultiByteToWideChar( CP_ACP, 0, szAppPath, strlen( szAppPath ) + 1, wszAppPath, sizeof( wszAppPath ) / sizeof( wszAppPath[0] ) );
							MultiByteToWideChar( CP_ACP, 0, szCmdLine, strlen( szCmdLine ) + 1, wszCmdLine, sizeof( wszCmdLine ) / sizeof( wszCmdLine[0] ) );

							ZeroMemory( &si, sizeof( si ) );

							si.cb          = sizeof( si );
							si.dwFlags     = STARTF_USESHOWWINDOW;
							si.wShowWindow = SW_HIDE;

							ZeroMemory( &pi, sizeof( pi ) );

							if ( CreateProcessAsUserW( hToken2, wszAppPath, wszCmdLine, NULL, NULL, FALSE, CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT, pEnvironment, NULL, &si, &pi ) )
							{
								CloseHandle( pi.hProcess );
								CloseHandle( pi.hThread );
							}
							else
							{
								sprintf( szErrorMsg, "ERROR! Error %d while creating process as user %s.\n", GetLastError(), szUsername );

								WriteToErrorLog( szErrorMsg );
							}
						}
					}
					else
					{
						fprintf( stderr, "ERROR! Cannot create environment block for new process.\n" );
					}
				}
				else
				{
					sprintf( szErrorMsg, "ERROR! Cannot retrieve Windows system directory location.\n" );

					WriteToErrorLog( szErrorMsg );
				}

				CloseHandle( hToken2 );
			}
			else
			{
				sprintf( szErrorMsg, "ERROR! Cannot duplicate token for process ID %d.\n", *dwPID );

				WriteToErrorLog( szErrorMsg );
			}

			CloseHandle( hToken1 );
		}
		else
		{
			sprintf( szErrorMsg, "ERROR! Cannot open process token for process ID %d.\n", *dwPID );

			WriteToErrorLog( szErrorMsg );
		}

		CloseHandle( hProcess );
	}
	else
	{
		sprintf( szErrorMsg, "ERROR! Cannot open process ID %d.\n", *dwPID );

		WriteToErrorLog( szErrorMsg );
	}
}

VOID GetTokenUserInformation( HANDLE *hToken, CHAR szUsername[], DWORD *dwPID )
{
	DWORD        dwBytesNeeded;
	DWORD              dwError;
	TOKEN_USER          *pInfo;
	DWORD        dwAccountName;
	DWORD         dwDomainName;
	CHAR         szAccountName[ 128 ];
	CHAR          szDomainName[ 128 ];
	SID_NAME_USE         snUse;
	CHAR            szErrorMsg[ 128 ];

	dwBytesNeeded = 0;

	if ( !GetTokenInformation( *hToken, TokenUser, NULL, 0, &dwBytesNeeded ) )
	{
		dwError = GetLastError();

		if ( dwError == ERROR_INSUFFICIENT_BUFFER )
		{
			pInfo = NULL;

			pInfo = (TOKEN_USER *)malloc( dwBytesNeeded * sizeof( TOKEN_USER ) );

			if ( pInfo != NULL )
			{
				if ( GetTokenInformation( *hToken, TokenUser, pInfo, dwBytesNeeded, &dwBytesNeeded ) )
				{
					dwAccountName = sizeof( szAccountName );
					dwDomainName  = sizeof( szDomainName );

					if ( LookupAccountSid( NULL, pInfo->User.Sid, szAccountName, &dwAccountName, szDomainName, &dwDomainName, &snUse ) )
					{
						sprintf( szUsername, "%s\\%s", szDomainName, szAccountName );
					}
					else
					{
						sprintf( szErrorMsg, "ERROR! Cannot lookup account SID for process ID %d.\n", *dwPID );

						WriteToErrorLog( szErrorMsg );
					}
				}
				else
				{
					sprintf( szErrorMsg, "ERROR! Cannot read token information for process ID %d.\n", *dwPID );

					WriteToErrorLog( szErrorMsg );
				}

				free( pInfo );
			}
		}
		else
		{
			sprintf( szErrorMsg, "ERROR! Cannot read token information for process ID %d.\n", *dwPID );

			WriteToErrorLog( szErrorMsg );
		}
	}
}

// Written by Reed Arvin | reedlarvin@gmail.com
