//
// gcc source.c libdasm.c -o source.exe -ldasm
//

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "libdasm.h"

VOID GetExceptionInfo( DEBUG_EVENT deInfo, CONTEXT context );

INT main( INT argc, CHAR *argv[] )
{
	DWORD                   dwTimeout;
	CHAR                 szExecutable[ 1024 ];
	CHAR                  szArguments[ 4096 ];
	CHAR                szCommandLine[ 4096 ];
	DWORD                           i;
	DWORD                           j;
	PROCESS_INFORMATION            pi;
	STARTUPINFO                    si;
	BOOL                   bException;
	DWORD                 dwStartTime;
	DEBUG_EVENT                deInfo;
	HANDLE                    hThread;
	CONTEXT                   context;

	if ( argc > 2 )
	{
		dwTimeout = atoi( argv[1] );

		strcpy( szExecutable, argv[2] );
		strcpy( szArguments,  ""      );

		i = 0;

		for ( j = 3; j < argc; j++ )
		{
			if ( i > 0 )
			{
				strcat( szArguments, " " );
			}

			strcat( szArguments, argv[j] );

			i++;
		}

		strcpy( szCommandLine, szExecutable );

		if ( strcmp( szArguments, "" ) != 0 )
		{
			strcat( szCommandLine, " " );
			strcat( szCommandLine, szArguments );
		}

		printf( "Running ExecMon v1.0 with the following arguments:\n" );
		printf( "[+] Timeout:    \"%d seconds\"\n", dwTimeout );
		printf( "[+] Executable: \"%s\"\n", szExecutable );
		printf( "[+] Arguments:  \"%s\"\n", szArguments );
		printf( "\n" );

		ZeroMemory( &si, sizeof( si ) );
		ZeroMemory( &pi, sizeof( pi ) );

		si.cb = sizeof( si );

		if ( CreateProcess( NULL,
				    szCommandLine,
				    NULL,
				    NULL,
				    FALSE,
				    DEBUG_PROCESS,
				    NULL,
				    NULL,
				    &si,
				    &pi ) )
		{
			bException = FALSE;

			dwStartTime = GetTickCount();

			while ( ( GetTickCount() - dwStartTime ) < ( dwTimeout * 1000 ) )
			{
				if ( WaitForDebugEvent( &deInfo, 100 ) )
				{
					if ( deInfo.dwDebugEventCode != EXCEPTION_DEBUG_EVENT )
					{
						ContinueDebugEvent( deInfo.dwProcessId, deInfo.dwThreadId, DBG_CONTINUE );

						continue;
					}

					hThread = OpenThread( THREAD_ALL_ACCESS, FALSE, deInfo.dwThreadId );

					if ( hThread != NULL )
					{
						context.ContextFlags = CONTEXT_FULL;

						if ( GetThreadContext( hThread, &context ) != 0 )
						{
							switch ( deInfo.u.Exception.ExceptionRecord.ExceptionCode )
							{
								case EXCEPTION_ACCESS_VIOLATION:
									printf( "#### Access Violation Exception\n" );

									bException = TRUE;

									break;

								case EXCEPTION_INT_DIVIDE_BY_ZERO:
									printf( "#### Divide by Zero Exception\n" );

									bException = TRUE;

									break;

								case EXCEPTION_STACK_OVERFLOW:
									printf( "#### Stack Overflow Exception\n" );

									bException = TRUE;

									break;

								default:
									ContinueDebugEvent( deInfo.dwProcessId, deInfo.dwThreadId, DBG_CONTINUE );
							}

							if ( bException )
							{
								GetExceptionInfo( deInfo, context );
							}
						}
						else
						{
							fprintf( stderr, "ERROR! Cannot get thread context\n" );

							break;
						}
					}
					else
					{
						fprintf( stderr, "ERROR! Cannot open thread\n" );

						break;
					}
				}
			}

			printf( "Timeout expired. Terminating process...\n" );

			TerminateProcess( pi.hProcess, 0 );

			CloseHandle( pi.hProcess );
			CloseHandle( pi.hThread );
		}
		else
		{
			fprintf( stderr, "ERROR! Cannot create process\n" );
		}
	}
	else
	{
		printf( "ExecMon v1.0 | http://reedarvin.thearvins.com/\n" );
		printf( "\n" );
		printf( "Usage: ExecMon <wait seconds> <path to exe> <arg 1> <arg 2> ... <arg n>\n" );
		printf( "\n" );
		printf( "<wait seconds>  -- required argument\n" );
		printf( "<path to exe>   -- required argument\n" );
		printf( "<arg 1>         -- optional argument\n" );
		printf( "<arg 2>         -- optional argument\n" );
		printf( "<arg n>         -- optional argument\n" );
		printf( "\n" );
		printf( "Examples:\n" );
		printf( "ExecMon 5 test.exe\n" );
		printf( "ExecMon 5 test.exe file.txt\n" );
		printf( "ExecMon 5 test.exe file.txt AAAAAAAAAA\n" );
		printf( "\n" );
		printf( "(Written by Reed Arvin | reedarvin@gmail.com)\n" );
	}

	return 0;
}

VOID GetExceptionInfo( DEBUG_EVENT deInfo, CONTEXT context )
{
	HANDLE         hProcess;
	CHAR          szMemInfo[ 32 ];
	INSTRUCTION instruction;
	CHAR        szInstrInfo[ 256 ];

	hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, deInfo.dwProcessId );

	if ( hProcess != NULL)
	{
		ReadProcessMemory( hProcess, (VOID *)context.Eip, &szMemInfo, 32, NULL );

		get_instruction( &instruction, szMemInfo, MODE_32 );

		get_instruction_string( &instruction, FORMAT_INTEL, 0, szInstrInfo, sizeof( szInstrInfo ) );

		printf( "Exception at EIP:%08x instruction \"%s\"\n", context.Eip, szInstrInfo );
		printf( "EAX:%08x EBX:%08x ECX:%08x EDX:%08x\n", context.Eax, context.Ebx, context.Ecx, context.Edx );
		printf( "ESI:%08x EDI:%08x ESP:%08x EBP:%08x\n", context.Esi, context.Edi, context.Esp, context.Ebp );
		printf( "\n" );
	}
	else
	{
		fprintf (stderr, "ERROR! Cannot open process\n" );
	}
}

// Written by Reed Arvin | reedarvin@gmail.com
