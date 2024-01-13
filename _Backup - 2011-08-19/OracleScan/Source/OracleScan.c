//
// gcc source.c -o source.exe -lodbc32
//

#define _WIN32_WINNT 0x0500
#define WINVER       0x0500

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <process.h>
#include <sql.h>
#include <sqlext.h>

#define MAX_THREADS 4

VOID                Usage( VOID );
BOOL          IsValidFile( CHAR *szDictionaryFile );
BOOL   ParseOracleScanINI( CHAR *szODBCConnectString, CHAR *szUserListQuery );
VOID                 Trim( CHAR *szText );
BOOL                Split( CHAR *szText, CHAR *szFindText, CHAR *szOutputText1, CHAR *szOutputText2 );
VOID  GuessOracleDefaults( CHAR *szTarget, CHAR *szDatabaseInput, CHAR *szDictionaryFile, CHAR *szODBCConnectString, CHAR *szUserListQuery );
VOID GuessOraclePasswords( CHAR *szTarget, CHAR *szDatabaseInput, CHAR *szDictionaryFile, CHAR *szODBCConnectString, CHAR *szUserListQuery );
VOID          ThreadedSub( VOID *pParameter );
BOOL        OracleConnect( CHAR *szTarget, CHAR *szDatabaseName, CHAR *szUsername, CHAR *szPassword, CHAR *szODBCConnectString, CHAR *szUserListQuery );
BOOL              Replace( CHAR *szText, CHAR *szFindText, CHAR *szReplacementText );
VOID  LogGuessedPasswords( CHAR *szTarget, CHAR *szDatabaseName, CHAR *szUsername, CHAR *szPassword );

typedef struct _THREAD_ARGS
{
	CHAR            Target[ 128 ];
	CHAR      DatabaseName[ 128 ];
	CHAR          Username[ 128 ];
	CHAR          Password[ 128 ];
	CHAR ODBCConnectString[ 1024 ];
	CHAR     UserListQuery[ 1024 ];
} THREAD_ARGS, *PTHREAD_ARGS;

HANDLE hSemaphore;

INT nThreads = 0;

INT main( INT argc, CHAR *argv[] )
{
	DWORD             dwError;
	CHAR             szTarget[ 128 ];
	CHAR      szDatabaseInput[ 128 ];
	CHAR             szMethod[ 64 ];
	CHAR     szDictionaryFile[ 128 ];
	CHAR  szODBCConnectString[ 1024 ];
	CHAR      szUserListQuery[ 1024 ];

	hSemaphore = CreateSemaphore( NULL, 1, 1, NULL );

	if ( !CreateDirectory( "UserCache", NULL ) )
	{
		dwError = GetLastError();

		if ( dwError != ERROR_ALREADY_EXISTS )
		{
			fprintf( stderr, "ERROR! Cannot create UserCache directory.\n" );

			return 1;
		}
	}

	if ( argc == 5 )
	{
		strcpy( szTarget,         argv[1] );
		strcpy( szDatabaseInput,  argv[2] );
		strcpy( szMethod,         argv[3] );
		strcpy( szDictionaryFile, argv[4] );

		strlwr( szMethod );

		if ( strcmp( szMethod, "-d" ) == 0 || strcmp( szMethod, "-u" ) == 0 )
		{
			if ( IsValidFile( szDictionaryFile ) )
			{
				if ( strcmp( szMethod, "-d" ) == 0 )
				{
					printf( "Running OracleScan v1.2 with the following arguments:\n" );
					printf( "[+] Host Input:           \"%s\"\n", szTarget );
					printf( "[+] Database Input:       \"%s\"\n", szDatabaseInput );
					printf( "[+] Method:               \"Oracle Defaults List\"\n" );
					printf( "[+] Oracle Defaults List: \"%s\"\n", szDictionaryFile );
					printf( "\n" );

					if ( ParseOracleScanINI( szODBCConnectString, szUserListQuery ) )
					{
						GuessOracleDefaults( szTarget, szDatabaseInput, szDictionaryFile, szODBCConnectString, szUserListQuery );
					}
					else
					{
						fprintf( stderr, "ERROR! Problem parsing file OracleScan.ini.\n" );
					}
				}

				if ( strcmp( szMethod, "-u" ) == 0 )
				{
					printf( "Running OracleScan v1.2 with the following arguments:\n" );
					printf( "[+] Host Input:        \"%s\"\n", szTarget );
					printf( "[+] Database Input:    \"%s\"\n", szDatabaseInput );
					printf( "[+] Method:            \"Oracle Users List\"\n" );
					printf( "[+] Oracle Users List: \"%s\"\n", szDictionaryFile );
					printf( "\n" );

					if ( ParseOracleScanINI( szODBCConnectString, szUserListQuery ) )
					{
						GuessOraclePasswords( szTarget, szDatabaseInput, szDictionaryFile, szODBCConnectString, szUserListQuery );
					}
					else
					{
						fprintf( stderr, "ERROR! Problem parsing file OracleScan.ini.\n" );
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

	CloseHandle( hSemaphore );

	return 0;
}

VOID Usage( VOID )
{
	printf( "OracleScan v1.3 | Canaudit, Inc. http://www.canaudit.com/\n" );
	printf( "\n" );
	printf( "Usage: OracleScan <hostname> <database | database input file> [-d|-u] <dictionary file>\n" );
	printf( "\n" );
	printf( "<hostname>                        -- required argument\n" );
	printf( "<database | database input file>  -- required argument\n" );
	printf( "[-d|-u]                           -- required argument\n" );
	printf( "<dictionary file>                 -- required argument\n" );
	printf( "\n" );
	printf( "Examples:\n" );
	printf( "OracleScan 10.10.10.10 OracleDB -d Defaults.txt\n" );
	printf( "OracleScan 10.10.10.10 OracleDBInput.txt -u OracleUsers.txt\n" );
	printf( "\n" );
	printf( "OracleScan MyOracleMachine OracleDB -d Defaults.txt\n" );
	printf( "OracleScan MyOracleMachine OracleDBInput.txt -u OracleUsers.txt\n" );
	printf( "\n" );
	printf( "(Written by Reed Arvin | reed@canaudit.com)\n" );
}

BOOL IsValidFile( CHAR szDictionaryFile[] )
{
	BOOL bFileExists;
	FILE      *pFile;

	bFileExists = FALSE;

	pFile = fopen( szDictionaryFile, "r" );

	if ( pFile != NULL )
	{
		bFileExists = TRUE;

		fclose( pFile );
	}
	else
	{
		fprintf( stderr, "ERROR! Cannot open input file %s\n", szDictionaryFile );
	}

	return bFileExists;
}

BOOL ParseOracleScanINI( CHAR szODBCConnectString[], CHAR szUserListQuery[] )
{
	BOOL              bResult;
	DWORD                   i;
	FILE  *pOracleScanINIFile;
	CHAR               szLine[ 2048 ];
	CHAR        szOutputText1[ 1024 ];
	CHAR        szOutputText2[ 1024 ];

	bResult = FALSE;

	i = 0;

	pOracleScanINIFile = fopen( "OracleScan.ini", "r" );

	if ( pOracleScanINIFile != NULL )
	{
		while ( fgets( szLine, sizeof( szLine ), pOracleScanINIFile ) != NULL )
		{
			Trim( szLine );

			if ( szLine[0] != '#' && szLine[0] != '\n' )
			{
				if ( szLine[strlen( szLine ) - 1] == '\n' )
				{
					szLine[strlen( szLine ) - 1] = '\0';
				}

				strcpy( szOutputText1, "" );
				strcpy( szOutputText2, "" );

				if ( Split( szLine, ":", szOutputText1, szOutputText2 ) )
				{
					Trim( szOutputText1 );
					Trim( szOutputText2 );

					strupr( szOutputText1 );

					if ( strcmp( szOutputText1, "ODBC_CONNECTION_STRING" ) == 0 )
					{
						strcpy( szODBCConnectString, szOutputText2 );

						i++;
					}

					if ( strcmp( szOutputText1, "USER_LIST_QUERY" ) == 0 )
					{
						strcpy( szUserListQuery, szOutputText2 );

						i++;
					}
				}
			}
		}

		fclose( pOracleScanINIFile );
	}

	if ( i == 2 )
	{
		bResult = TRUE;
	}

	return bResult;
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

BOOL Split( CHAR szText[], CHAR szFindText[], CHAR szOutputText1[], CHAR szOutputText2[] )
{
	BOOL  bFindTextLocated;
	CHAR    *pTextLocation;
	DWORD   dwTextLocation;
	DWORD                i;
	DWORD                j;

	bFindTextLocated = FALSE;

	pTextLocation = strstr( szText, szFindText );

	dwTextLocation = (INT)( pTextLocation - szText );

	if ( pTextLocation != NULL )
	{
		bFindTextLocated = TRUE;

		i = 0;

		while ( i < dwTextLocation )
		{
			szOutputText1[i] = szText[i];

			i++;
		}

		szOutputText1[i] = '\0';

		i = dwTextLocation + strlen( szFindText );

		j = 0;

		while ( i < strlen( szText ) )
		{
			szOutputText2[j] = szText[i];

			i++;
			j++;
		}

		szOutputText2[j] = '\0';
	}

	return bFindTextLocated;
}

VOID GuessOracleDefaults( CHAR szTarget[], CHAR szDatabaseInput[], CHAR szDictionaryFile[], CHAR szODBCConnectString[], CHAR szUserListQuery[] )
{
	DWORD               i;
	FILE  *pDatabaseInput;
	CHAR   szDatabaseName[ 128 ];
	FILE   *pDefaultsFile;
	CHAR       szUserPass[ 256 ];
	CHAR       szUsername[ 128 ];
	CHAR       szPassword[ 128 ];

	PTHREAD_ARGS pThreadArgs;

	i = 0;

	pDatabaseInput = fopen( szDatabaseInput, "r" );

	if ( pDatabaseInput != NULL )
	{
		while ( fscanf( pDatabaseInput, "%s", szDatabaseName ) != EOF )
		{
			pDefaultsFile = fopen( szDictionaryFile, "r" );

			if ( pDefaultsFile != NULL )
			{
				if ( i == 0 )
				{
					printf( "\n" );
					printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
					printf( "+++++         GUESS ORACLE PASSWORDS          +++++\n" );
					printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
					printf( "\n" );

					i++;
				}

				while ( fgets( szUserPass, sizeof( szUserPass ), pDefaultsFile  ) != NULL )
				{
					Trim( szUserPass );

					if ( szUserPass[0] != '#' && szUserPass[0] != '\n' )
					{
						if ( szUserPass[strlen( szUserPass ) - 1] == '\n' )
						{
							szUserPass[strlen( szUserPass ) - 1] = '\0';
						}

						if ( Split( szUserPass, ":", szUsername, szPassword ) )
						{
							Trim( szUsername );
							Trim( szPassword );

							while ( nThreads >= MAX_THREADS )
							{
								Sleep( 200 );
							}

							pThreadArgs = (PTHREAD_ARGS)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( THREAD_ARGS ) );

							if ( pThreadArgs != NULL )
							{
								strcpy( pThreadArgs->Target,            szTarget );
								strcpy( pThreadArgs->DatabaseName,      szDatabaseName );
								strcpy( pThreadArgs->Username,          szUsername );
								strcpy( pThreadArgs->Password,          szPassword );
								strcpy( pThreadArgs->ODBCConnectString, szODBCConnectString );
								strcpy( pThreadArgs->UserListQuery,     szUserListQuery );

								WaitForSingleObject( hSemaphore, INFINITE );

								nThreads++;

								ReleaseSemaphore( hSemaphore, 1, NULL );

								_beginthread( ThreadedSub, 0, (VOID *)pThreadArgs );
							}
						}
					}
				}

				fclose( pDefaultsFile );
			}
		}

		fclose( pDatabaseInput );

		Sleep( 1000 );

		printf( "Waiting for threads to terminate...\n" );
	}
	else
	{
		strcpy( szDatabaseName, szDatabaseInput );

		pDefaultsFile = fopen( szDictionaryFile, "r" );

		if ( pDefaultsFile != NULL )
		{
			if ( i == 0 )
			{
				printf( "\n" );
				printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
				printf( "+++++         GUESS ORACLE PASSWORDS          +++++\n" );
				printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
				printf( "\n" );

				i++;
			}

			while ( fgets( szUserPass, sizeof( szUserPass ), pDefaultsFile  ) != NULL )
			{
				Trim( szUserPass );

				if ( szUserPass[0] != '#' && szUserPass[0] != '\n' )
				{
					if ( szUserPass[strlen( szUserPass ) - 1] == '\n' )
					{
						szUserPass[strlen( szUserPass ) - 1] = '\0';
					}

					if ( Split( szUserPass, ":", szUsername, szPassword ) )
					{
						Trim( szUsername );
						Trim( szPassword );

						while ( nThreads >= MAX_THREADS )
						{
							Sleep( 200 );
						}

						pThreadArgs = (PTHREAD_ARGS)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( THREAD_ARGS ) );

						if ( pThreadArgs != NULL )
						{
							strcpy( pThreadArgs->Target,            szTarget );
							strcpy( pThreadArgs->DatabaseName,      szDatabaseName );
							strcpy( pThreadArgs->Username,          szUsername );
							strcpy( pThreadArgs->Password,          szPassword );
							strcpy( pThreadArgs->ODBCConnectString, szODBCConnectString );
							strcpy( pThreadArgs->UserListQuery,     szUserListQuery );

							WaitForSingleObject( hSemaphore, INFINITE );

							nThreads++;

							ReleaseSemaphore( hSemaphore, 1, NULL );

							_beginthread( ThreadedSub, 0, (VOID *)pThreadArgs );
						}
					}
				}
			}

			fclose( pDefaultsFile );
		}
	}

	while ( nThreads > 0 )
	{
		Sleep( 200 );
	}
}

VOID GuessOraclePasswords( CHAR szTarget[], CHAR szDatabaseInput[], CHAR szDictionaryFile[], CHAR szODBCConnectString[], CHAR szUserListQuery[] )
{
	DWORD                i;
	FILE   *pDatabaseInput;
	CHAR    szDatabaseName[ 128 ];
	CHAR   szUserCacheFile[ 128 ];
	FILE   *pUserCacheFile;
	CHAR        szUsername[ 128 ];
	FILE  *pDictionaryFile;
	CHAR        szPassword[ 128 ];

	PTHREAD_ARGS pThreadArgs;

	i = 0;

	pDatabaseInput = fopen( szDatabaseInput, "r" );

	if ( pDatabaseInput != NULL )
	{
		while ( fscanf( pDatabaseInput, "%s", szDatabaseName ) != EOF )
		{
			sprintf( szUserCacheFile, "UserCache\\%s-%s.users", szTarget, szDatabaseName );

			pUserCacheFile = fopen( szUserCacheFile, "r" );

			if ( pUserCacheFile != NULL )
			{
				while ( fgets( szUsername, sizeof( szUsername ), pUserCacheFile ) != NULL )
				{
					Trim( szUsername );

					if ( szUsername[0] != '#' && szUsername[0] != '\n' )
					{
						if ( szUsername[strlen( szUsername ) - 1] == '\n' )
						{
							szUsername[strlen( szUsername ) - 1] = '\0';
						}

						pDictionaryFile = fopen( szDictionaryFile, "r" );

						if ( pDictionaryFile != NULL )
						{
							if ( i == 0 )
							{
								printf( "\n" );
								printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
								printf( "+++++         GUESS ORACLE PASSWORDS          +++++\n" );
								printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
								printf( "\n" );

								i++;
							}

							while ( fgets( szPassword, sizeof( szPassword ), pDictionaryFile ) != NULL )
							{
								Trim( szPassword );

								if ( szPassword[0] != '#' && szPassword[0] != '\n' )
								{
									if ( szPassword[strlen( szPassword ) - 1] == '\n' )
									{
										szPassword[strlen( szPassword ) - 1] = '\0';
									}

									while ( nThreads >= MAX_THREADS )
									{
										Sleep( 200 );
									}

									pThreadArgs = (PTHREAD_ARGS)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( THREAD_ARGS ) );

									if ( pThreadArgs != NULL )
									{
										strcpy( pThreadArgs->Target,            szTarget );
										strcpy( pThreadArgs->DatabaseName,      szDatabaseName );
										strcpy( pThreadArgs->Username,          szUsername );
										strcpy( pThreadArgs->Password,          szPassword );
										strcpy( pThreadArgs->ODBCConnectString, szODBCConnectString );
										strcpy( pThreadArgs->UserListQuery,     szUserListQuery );

										WaitForSingleObject( hSemaphore, INFINITE );

										nThreads++;

										ReleaseSemaphore( hSemaphore, 1, NULL );

										_beginthread( ThreadedSub, 0, (VOID *)pThreadArgs );
									}
								}
							}
						}

						fclose( pDictionaryFile );
					}
				}

				fclose( pUserCacheFile );
			}
		}

		fclose( pDatabaseInput );

		Sleep( 1000 );

		printf( "Waiting for threads to terminate...\n" );
	}
	else
	{
		strcpy( szDatabaseName, szDatabaseInput );

		sprintf( szUserCacheFile, "UserCache\\%s-%s.users", szTarget, szDatabaseName );

		pUserCacheFile = fopen( szUserCacheFile, "r" );

		if ( pUserCacheFile != NULL )
		{
			while ( fgets( szUsername, sizeof( szUsername ), pUserCacheFile ) != NULL )
			{
				Trim( szUsername );

				if ( szUsername[0] != '#' && szUsername[0] != '\n' )
				{
					if ( szUsername[strlen( szUsername ) - 1] == '\n' )
					{
						szUsername[strlen( szUsername ) - 1] = '\0';
					}

					pDictionaryFile = fopen( szDictionaryFile, "r" );

					if ( pDictionaryFile != NULL )
					{
						if ( i == 0 )
						{
							printf( "\n" );
							printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
							printf( "+++++         GUESS ORACLE PASSWORDS          +++++\n" );
							printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
							printf( "\n" );

							i++;
						}

						while ( fgets( szPassword, sizeof( szPassword ), pDictionaryFile ) != NULL )
						{
							Trim( szPassword );

							if ( szPassword[0] != '#' && szPassword[0] != '\n' )
							{
								if ( szPassword[strlen( szPassword ) - 1] == '\n' )
								{
									szPassword[strlen( szPassword ) - 1] = '\0';
								}

								while ( nThreads >= MAX_THREADS )
								{
									Sleep( 200 );
								}

								pThreadArgs = (PTHREAD_ARGS)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( THREAD_ARGS ) );

								if ( pThreadArgs != NULL )
								{
									strcpy( pThreadArgs->Target,            szTarget );
									strcpy( pThreadArgs->DatabaseName,      szDatabaseName );
									strcpy( pThreadArgs->Username,          szUsername );
									strcpy( pThreadArgs->Password,          szPassword );
									strcpy( pThreadArgs->ODBCConnectString, szODBCConnectString );
									strcpy( pThreadArgs->UserListQuery,     szUserListQuery );

									WaitForSingleObject( hSemaphore, INFINITE );

									nThreads++;

									ReleaseSemaphore( hSemaphore, 1, NULL );

									_beginthread( ThreadedSub, 0, (VOID *)pThreadArgs );
								}
							}
						}
					}

					fclose( pDictionaryFile );
				}
			}

			fclose( pUserCacheFile );
		}
	}

	while ( nThreads > 0 )
	{
		Sleep( 200 );
	}
}

VOID ThreadedSub( VOID *pParameter )
{
	CHAR            szTarget[ 128 ];
	CHAR      szDatabaseName[ 128 ];
	CHAR          szUsername[ 128 ];
	CHAR          szPassword[ 128 ];
	CHAR szODBCConnectString[ 1024 ];
	CHAR     szUserListQuery[ 1024 ];
	CHAR       szTmpPassword[ 128 ];

	PTHREAD_ARGS pThreadArgs;

	pThreadArgs = (PTHREAD_ARGS)pParameter;

	strcpy( szTarget,            pThreadArgs->Target );
	strcpy( szDatabaseName,      pThreadArgs->DatabaseName );
	strcpy( szUsername,          pThreadArgs->Username );
	strcpy( szPassword,          pThreadArgs->Password );
	strcpy( szODBCConnectString, pThreadArgs->ODBCConnectString );
	strcpy( szUserListQuery,     pThreadArgs->UserListQuery );

	HeapFree( GetProcessHeap(), 0, pThreadArgs );

	strcpy( szTmpPassword, szPassword );

	strupr( szTmpPassword );

	if ( strcmp( szTmpPassword, "<BLANK>" ) == 0 )
	{
		strcpy( szPassword, "" );

		printf( "Trying %s:<blank> on database %s...\n", szUsername, szDatabaseName );
	}
	else if ( strcmp( szTmpPassword, "<USERNAME>" ) == 0 )
	{
		strcpy( szPassword, szUsername );

		printf( "Trying %s:%s on database %s...\n", szUsername, szPassword, szDatabaseName );
	}
	else
	{
		printf( "Trying %s:%s on database %s...\n", szUsername, szPassword, szDatabaseName );
	}

	if ( OracleConnect( szTarget, szDatabaseName, szUsername, szPassword, szODBCConnectString, szUserListQuery ) )
	{
		if ( strcmp( szTmpPassword, "<BLANK>" ) == 0 )
		{
			printf( "\n" );
			printf( "PASSWORD GUESSED! %s@%s, password is <blank>\n", szUsername, szDatabaseName );
		}
		else
		{
			printf( "\n" );
			printf( "PASSWORD GUESSED! %s@%s, password is %s\n", szUsername, szDatabaseName, szPassword );
		}
	}

	WaitForSingleObject( hSemaphore, INFINITE );

	nThreads--;

	ReleaseSemaphore( hSemaphore, 1, NULL );

	_endthread();
}

BOOL OracleConnect( CHAR szTarget[], CHAR szDatabaseName[], CHAR szUsername[], CHAR szPassword[], CHAR szODBCConnectString[], CHAR szUserListQuery[] )
{
	BOOL                   bResult;
	SQLRETURN           sqlReturn1;
	SQLHENV                   hEnv;
	SQLHDBC                   hDbc;
	SQLCHAR              szOutConn[ 2048 ];
	SQLSMALLINT               sLen;
	SQLHSTMT                 hStmt;
	DWORD                        i;
	SQLCHAR                szData1[ 2048 ];
	SQLRETURN           sqlReturn2;
	SQLINTEGER              sData1;
	CHAR             szLogFileName[ 128 ];
	FILE              *pOutputFile;
	FILE        *pQueryStringsFile;
	CHAR                   szQuery[ 2048 ];
	DWORD                        j;
	SQLSMALLINT           sColumns;
	DWORD                dwColumns;
	DWORD                        k;
	SQLCHAR        szTmpColumnName[ 2048 ];
	SQLSMALLINT     sColumnNameLen;
	SQLSMALLINT    sColumnDataType;
	SQLUINTEGER        sColumnSize;
	SQLSMALLINT     sDecimalDigits;
	SQLSMALLINT          sNullable;
	CHAR              szColumnName[ 2048 ];
	SQLCHAR             szSQLState[ 8 ];
	SQLINTEGER                sErr;
	SQLCHAR               szErrMsg[ 1024 ];
	SQLSMALLINT            sErrLen;
	CHAR                 *pNewLine;

	bResult = FALSE;

	Replace( szODBCConnectString, "%DATABASE%", szDatabaseName );
	Replace( szODBCConnectString, "%USERID%",   szUsername );
	Replace( szODBCConnectString, "%PASSWORD%", szPassword );

	sqlReturn1 = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv );

	if ( sqlReturn1 == SQL_SUCCESS || sqlReturn1 == SQL_SUCCESS_WITH_INFO )
	{
		sqlReturn1 = SQLSetEnvAttr( hEnv, SQL_ATTR_ODBC_VERSION, (VOID *)SQL_OV_ODBC3, 0 );

		if ( sqlReturn1 == SQL_SUCCESS || sqlReturn1 == SQL_SUCCESS_WITH_INFO )
		{
			sqlReturn1 = SQLAllocHandle( SQL_HANDLE_DBC, hEnv, &hDbc );

			if ( sqlReturn1 == SQL_SUCCESS || sqlReturn1 == SQL_SUCCESS_WITH_INFO )
			{
				SQLSetConnectAttr( hDbc, SQL_LOGIN_TIMEOUT, (VOID *)2, 0 );

				sqlReturn1 = SQLDriverConnect( hDbc, NULL, (SQLCHAR *)szODBCConnectString, (SQLSMALLINT)strlen( szODBCConnectString ), szOutConn, sizeof( szOutConn ), &sLen, SQL_DRIVER_NOPROMPT );

				if ( sqlReturn1 == SQL_SUCCESS || sqlReturn1 == SQL_SUCCESS_WITH_INFO )
				{
					bResult = TRUE;

					LogGuessedPasswords( szTarget, szDatabaseName, szUsername, szPassword );

					sqlReturn1 = SQLAllocHandle( SQL_HANDLE_STMT, hDbc, &hStmt );

					if ( sqlReturn1 == SQL_SUCCESS || sqlReturn1 == SQL_SUCCESS_WITH_INFO )
					{
						sqlReturn1 = SQLExecDirect( hStmt, (SQLCHAR *)szUserListQuery, SQL_NTS );

						WaitForSingleObject( hSemaphore, INFINITE );

						i = 0;

						while ( sqlReturn1 == SQL_SUCCESS || sqlReturn1 == SQL_SUCCESS_WITH_INFO )
						{
							sqlReturn1 = SQLFetch( hStmt );

							if ( sqlReturn1 == SQL_SUCCESS || sqlReturn1 == SQL_SUCCESS_WITH_INFO )
							{
								strcpy( szData1, "" );

								sqlReturn2 = SQLGetData( hStmt, 1, SQL_C_CHAR, szData1, sizeof( szData1 ), &sData1 );

								if ( ( sqlReturn2 == SQL_SUCCESS || sqlReturn2 == SQL_SUCCESS_WITH_INFO ) )
								{
									if ( i == 0 )
									{
										sprintf( szLogFileName, "%s-%s-OracleUserInfo.txt", szTarget, szDatabaseName );

										pOutputFile = fopen( szLogFileName, "w" );

										if ( pOutputFile != NULL )
										{
											fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
											fprintf( pOutputFile, "\n" );
											fprintf( pOutputFile, "%s\n", szUserListQuery );
											fprintf( pOutputFile, "\n" );
											fprintf( pOutputFile, "Hostname\tOracle Database Name\tOracle Username\n" );

											fclose( pOutputFile );
										}

										sprintf( szLogFileName, "UserCache\\%s-%s.users", szTarget, szDatabaseName );

										pOutputFile = fopen( szLogFileName, "w" );

										if ( pOutputFile != NULL )
										{
											fclose( pOutputFile );
										}

										i++;
									}

									sprintf( szLogFileName, "%s-%s-OracleUserInfo.txt", szTarget, szDatabaseName );

									pOutputFile = fopen( szLogFileName, "a+" );

									if ( pOutputFile != NULL )
									{
										fprintf( pOutputFile, "%s\t%s\t%*s\n", szTarget, szDatabaseName, strlen( szData1 ) - 1, szData1 );

										fclose( pOutputFile );
									}

									sprintf( szLogFileName, "UserCache\\%s-%s.users", szTarget, szDatabaseName );

									pOutputFile = fopen( szLogFileName, "a+" );

									if ( pOutputFile != NULL )
									{
										fprintf( pOutputFile, "%*s\n", strlen( szData1 ) - 1, szData1 );

										fclose( pOutputFile );
									}
								}
							}
						}

						ReleaseSemaphore( hSemaphore, 1, NULL );
					}

					SQLFreeHandle( SQL_HANDLE_STMT, hStmt );

					pQueryStringsFile = fopen( "querystrings.txt", "r" );

					if ( pQueryStringsFile != NULL )
					{
						i = 0;

						while ( fgets( szQuery, sizeof( szQuery ), pQueryStringsFile ) != NULL )
						{
							Trim( szQuery );

							if ( szQuery[0] != '#' && szQuery[0] != '\n' )
							{
								if ( szQuery[strlen( szQuery ) - 1] == '\n' )
								{
									szQuery[strlen( szQuery ) - 1] = '\0';
								}

								sqlReturn1 = SQLAllocHandle( SQL_HANDLE_STMT, hDbc, &hStmt );

								if ( sqlReturn1 == SQL_SUCCESS || sqlReturn1 == SQL_SUCCESS_WITH_INFO )
								{
									sqlReturn1 = SQLExecDirect( hStmt, (SQLCHAR *)szQuery, SQL_NTS );

									WaitForSingleObject( hSemaphore, INFINITE );

									j = 0;

									if ( sqlReturn1 == SQL_SUCCESS || sqlReturn1 == SQL_SUCCESS_WITH_INFO )
									{
										sprintf( szLogFileName, "%s-%s-OracleSystemQuery%d.txt", szTarget, szDatabaseName, i + 1 );

										sqlReturn1 = SQLNumResultCols( hStmt, &sColumns );

										if ( sqlReturn1 == SQL_SUCCESS || sqlReturn1 == SQL_SUCCESS_WITH_INFO )
										{
											dwColumns = sColumns;

											for ( k = 1; k <= dwColumns; k++ )
											{
												sqlReturn1 = SQLDescribeCol( hStmt, (SQLSMALLINT)k, szTmpColumnName, sizeof( szTmpColumnName ), &sColumnNameLen, &sColumnDataType, &sColumnSize, &sDecimalDigits, &sNullable );

												if ( sqlReturn1 == SQL_SUCCESS || sqlReturn1 == SQL_SUCCESS_WITH_INFO )
												{
													sprintf( szColumnName, "%*s", strlen( szTmpColumnName ) - 1, szTmpColumnName );

													strupr( szColumnName );

													if ( j == 0 )
													{
														pOutputFile = fopen( szLogFileName, "w" );

														if ( pOutputFile != NULL )
														{
															fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
															fprintf( pOutputFile, "\n" );
															fprintf( pOutputFile, "%s\n", szQuery );
															fprintf( pOutputFile, "\n" );

															fclose( pOutputFile );
														}

														j++;
													}

													pOutputFile = fopen( szLogFileName, "a+" );

													if ( pOutputFile != NULL )
													{
														if ( k == 1 )
														{
															fprintf( pOutputFile, "Hostname\tOracle Database Name\t" );
														}

														if ( ( dwColumns - k ) > 0 )
														{
															fprintf( pOutputFile, "%s\t", szColumnName );
														}
														else
														{
															fprintf( pOutputFile, "%s\n", szColumnName );
														}

														fclose( pOutputFile );
													}
												}
											}
										}

										sqlReturn1 = SQLFetch( hStmt );

										while ( sqlReturn1 == SQL_SUCCESS || sqlReturn1 == SQL_SUCCESS_WITH_INFO )
										{
											for ( k = 1; k <= dwColumns; k++ )
											{
												strcpy( szData1, "" );

												sqlReturn2 = SQLGetData( hStmt, (SQLSMALLINT)k, SQL_C_CHAR, szData1, sizeof( szData1 ), &sData1 );

												if ( sqlReturn2 == SQL_SUCCESS || sqlReturn2 == SQL_SUCCESS_WITH_INFO )
												{
													if ( j == 0 )
													{
														pOutputFile = fopen( szLogFileName, "w" );

														if ( pOutputFile != NULL )
														{
															fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
															fprintf( pOutputFile, "\n" );
															fprintf( pOutputFile, "%s\n", szQuery );
															fprintf( pOutputFile, "\n" );

															fclose( pOutputFile );
														}

														j++;
													}

													pOutputFile = fopen( szLogFileName, "a+" );

													if ( pOutputFile != NULL )
													{
														if ( k == 1 )
														{
															fprintf( pOutputFile, "%s\t%s\t", szTarget, szDatabaseName );
														}

														if ( ( dwColumns - k ) > 0 )
														{
															fprintf( pOutputFile, "%*s\t", strlen( szData1 ) - 1, szData1 );
														}
														else
														{
															fprintf( pOutputFile, "%*s\n", strlen( szData1 ) - 1, szData1 );
														}

														fclose( pOutputFile );
													}
												}
											}

											sqlReturn1 = SQLFetch( hStmt );
										}
									}

									ReleaseSemaphore( hSemaphore, 1, NULL );
								}

								SQLFreeHandle( SQL_HANDLE_STMT, hStmt );

								i++;
							}
						}

						fclose( pQueryStringsFile );
					}

					SQLDisconnect( hDbc );
				}
				else
				{
					sqlReturn1 = SQLGetDiagRec( SQL_HANDLE_DBC, hDbc, 1, szSQLState, &sErr, szErrMsg, sizeof( szErrMsg ), &sErrLen );

					if ( sqlReturn1 == SQL_SUCCESS || sqlReturn1 == SQL_SUCCESS_WITH_INFO )
					{
						pNewLine = strchr( szErrMsg, '\r' );

						if ( pNewLine != NULL )
						{
							*pNewLine = '\0';
						}

						pNewLine = strchr( szErrMsg, '\n' );

						if ( pNewLine != NULL )
						{
							*pNewLine = '\0';
						}

						WaitForSingleObject( hSemaphore, INFINITE );

						pOutputFile = fopen( "ConnectionErrors.txt", "r" );

						if ( pOutputFile != NULL )
						{
							fclose( pOutputFile );
						}
						else
						{
							pOutputFile = fopen( "ConnectionErrors.txt", "w" );

							if ( pOutputFile != NULL )
							{
								fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
								fprintf( pOutputFile, "\n" );
								fprintf( pOutputFile, "Hostname\tOracle Database Name\tOracle Username\tPassword\tError Number\tError Message\n" );

								fclose( pOutputFile );
							}
						}

						pOutputFile = fopen( "ConnectionErrors.txt", "a+" );

						if ( pOutputFile != NULL )
						{
							if ( strcmp( szPassword, "" ) == 0 )
							{
								fprintf( pOutputFile, "%s\t%s\t%s\t<blank>\t%d\t%s\n", szTarget, szDatabaseName, szUsername, sErr, szErrMsg );
							}
							else
							{
								fprintf( pOutputFile, "%s\t%s\t%s\t%s\t%d\t%s\n", szTarget, szDatabaseName, szUsername, szPassword, sErr, szErrMsg );
							}

							fclose( pOutputFile );
						}

						ReleaseSemaphore( hSemaphore, 1, NULL );
					}
				}
			}

			SQLFreeHandle( SQL_HANDLE_DBC, hDbc );
		}
	}

	SQLFreeHandle( SQL_HANDLE_ENV, hEnv );

	return bResult;
}

BOOL Replace( CHAR szText[], CHAR szFindText[], CHAR szReplacementText[] )
{
	BOOL  bFindTextLocated;
	CHAR    *pTextLocation;
	DWORD   dwTextLocation;
	DWORD    dwTempTextLen;
	DWORD                i;
	CHAR        szTempText[ 10240 ];
	DWORD                j;

	bFindTextLocated = FALSE;

	pTextLocation = strstr( szText, szFindText );

	dwTextLocation = (INT)( pTextLocation - szText );

	dwTempTextLen = 0;

	while ( pTextLocation != NULL )
	{
		bFindTextLocated = TRUE;

		i = 0;

		while ( i < dwTextLocation )
		{
			szTempText[i] = szText[i];

			i++;
		}

		szTempText[i] = '\0';

		strcat( szTempText, szReplacementText );

		dwTempTextLen = strlen( szTempText );

		i = dwTextLocation + strlen( szFindText );

		j = strlen( szTempText );

		while ( i < strlen( szText ) )
		{
			szTempText[j] = szText[i];

			i++;
			j++;
		}

		szTempText[j] = '\0';

		strcpy( szText, szTempText );

		pTextLocation = strstr( szText, szFindText );

		dwTextLocation = (INT)( pTextLocation - szText );
	}

	return bFindTextLocated;
}

VOID LogGuessedPasswords( CHAR szTarget[], CHAR szDatabaseName[], CHAR szUsername[], CHAR szPassword[] )
{
	FILE *pOutputFile;

	WaitForSingleObject( hSemaphore, INFINITE );

	pOutputFile = fopen( "GuessedOraclePasswords.txt", "r" );

	if ( pOutputFile != NULL )
	{
		fclose( pOutputFile );
	}
	else
	{
		pOutputFile = fopen( "GuessedOraclePasswords.txt", "w" );

		if ( pOutputFile != NULL )
		{
			fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
			fprintf( pOutputFile, "\n" );
			fprintf( pOutputFile, "Hostname\tOracle Database Name\tOracle Username\tPassword\n" );

			fclose( pOutputFile );
		}
	}

	pOutputFile = fopen( "GuessedOraclePasswords.txt", "a+" );

	if ( pOutputFile != NULL )
	{
		if ( strcmp( szPassword, "" ) == 0 )
		{
			fprintf( pOutputFile, "%s\t%s\t%s\t<blank>\n", szTarget, szDatabaseName, szUsername );
		}
		else
		{
			fprintf( pOutputFile, "%s\t%s\t%s\t%s\n", szTarget, szDatabaseName, szUsername, szPassword );
		}

		fclose( pOutputFile );
	}

	ReleaseSemaphore( hSemaphore, 1, NULL );
}

// Written by Reed Arvin | reed@canaudit.com
