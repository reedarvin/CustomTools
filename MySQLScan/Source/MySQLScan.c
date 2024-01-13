//
// gcc source.c -o source.exe -lodbc32
//

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <process.h>
#include <sql.h>
#include <sqlext.h>

#define MAX_THREADS 16

BOOL                          IsValidFile( CHAR *szDictionary );
VOID                                Usage( VOID );
VOID                   GuessMySQLDefaults( CHAR *szTargetInput, CHAR *szDictionary );
VOID                  GuessMySQLPasswords( CHAR *szTargetInput, CHAR *szDictionary );
BOOL                                Split( CHAR *szText, CHAR *szSplitText, CHAR *szStartText, CHAR *szEndText );
VOID                          ThreadedSub( VOID *pParameter );
BOOL                         MySQLConnect( CHAR *szTarget, CHAR *szUsername, CHAR *szPassword );
VOID                   GetMySQLODBCDriver( CHAR *szODBCDriver );
VOID                  LogGuessedPasswords( CHAR *szTarget, CHAR *szUsername, CHAR *szPassword );
VOID LogGuessedPrivilegedAccountPasswords( CHAR *szTarget, CHAR *szUsername, CHAR *szPassword );

typedef struct _THREAD_ARGS
{
	CHAR   Target[ 128 ];
	CHAR Username[ 128 ];
	CHAR Password[ 128 ];
} THREAD_ARGS, *PTHREAD_ARGS;

HANDLE hSemaphore;

INT nThreads = 0;

INT main( INT argc, CHAR *argv[] )
{
	CHAR szTargetInput[ 128 ];
	CHAR      szMethod[ 64 ];
	CHAR  szDictionary[ 128 ];

	hSemaphore = CreateSemaphore( NULL, 1, 1, NULL );

	if ( argc == 4 )
	{
		strcpy( szTargetInput, argv[1] );
		strcpy( szMethod,      argv[2] );
		strcpy( szDictionary,  argv[3] );

		if ( strcmp( szMethod, "-d" ) == 0 || strcmp( szMethod, "-u" ) == 0 )
		{
			if ( IsValidFile( szDictionary ) )
			{
				if ( strcmp( szMethod, "-d" ) == 0 )
				{
					printf( "Running MySQLScan v1.0 with the following arguments:\n" );
					printf( "[+] Host Input:          \"%s\"\n", szTargetInput );
					printf( "[+] Method:              \"MySQL Defaults List\"\n" );
					printf( "[+] MySQL Defaults List: \"%s\"\n", szDictionary );
					printf( "\n" );

					GuessMySQLDefaults( szTargetInput, szDictionary );
				}

				if ( strcmp( szMethod, "-u" ) == 0 )
				{
					printf( "Running MySQLScan v1.0 with the following arguments:\n" );
					printf( "[+] Host Input:       \"%s\"\n", szTargetInput );
					printf( "[+] Method:           \"MySQL Users List\"\n" );
					printf( "[+] MySQL Users List: \"%s\"\n", szDictionary );
					printf( "\n" );

					GuessMySQLPasswords( szTargetInput, szDictionary );
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

BOOL IsValidFile( CHAR szDictionary[] )
{
	BOOL bValidFile;
	FILE     *pFile;

	bValidFile = TRUE;

	pFile = fopen( szDictionary, "r" );

	if ( pFile != NULL )
	{
		fclose( pFile );
	}
	else
	{
		fprintf( stderr, "ERROR! Cannot open input file %s\n", szDictionary );

		bValidFile = FALSE;
	}

	if ( bValidFile )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

VOID Usage( VOID )
{
	printf( "MySQLScan v1.0 | Canaudit, Inc. http://www.canaudit.com/\n" );
	printf( "\n" );
	printf( "Usage: MySQLScan <hostname | ip input file> [-d|-u] <dictionary file>\n" );
	printf( "\n" );
	printf( "<hostname | ip input file>  -- required argument\n" );
	printf( "[-d|-u]                     -- required argument\n" );
	printf( "<dictionary file>           -- required argument\n" );
	printf( "\n" );
	printf( "Examples:\n" );
	printf( "MySQLScan 10.10.10.10 -d Defaults.txt\n" );
	printf( "MySQLScan 10.10.10.10 MySQLDBInput.txt -u MySQLUsers.txt\n" );
	printf( "\n" );
	printf( "MySQLScan MyMySQLMachine -d Defaults.txt\n" );
	printf( "MySQLScan MyMySQLMachine -u MySQLUsers.txt\n" );
	printf( "\n" );
	printf( "MySQLScan IPInputFile.txt -d Defaults.txt\n" );
	printf( "MySQLScan IPInputFile.txt -u MySQLUsers.txt\n" );
	printf( "\n" );
	printf( "(Written by Reed Arvin | reed@canaudit.com)\n" );
}

VOID GuessMySQLDefaults( CHAR szTargetInput[], CHAR szDictionary[] )
{
	DWORD              i;
	FILE     *pInputFile;
	CHAR        szTarget[ 128 ];
	FILE  *pDefaultsFile;
	CHAR      szUserPass[ 256 ];
	CHAR      szUsername[ 128 ];
	CHAR      szPassword[ 128 ];

	PTHREAD_ARGS pThreadArgs;

	i = 0;

	pInputFile = fopen( szTargetInput, "r" );

	if ( pInputFile != NULL )
	{
		while ( fscanf( pInputFile, "%s", szTarget ) != EOF )
		{
			pDefaultsFile = fopen( szDictionary, "r" );

			if ( pDefaultsFile != NULL )
			{
				if ( i == 0 )
				{
					printf( "\n" );
					printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
					printf( "+++++          GUESS MySQL PASSWORDS          +++++\n" );
					printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
					printf( "\n" );

					i++;
				}

				while ( fgets( szUserPass, sizeof( szUserPass ), pDefaultsFile  ) != NULL )
				{
					if ( szUserPass[0] != '#' && szUserPass[0] != '\n' )
					{
						if ( szUserPass[strlen( szUserPass ) - 1] == '\n' )
						{
							szUserPass[strlen( szUserPass ) - 1] = '\0';
						}

						if ( Split( szUserPass, ":", szUsername, szPassword ) )
						{
							while ( nThreads >= MAX_THREADS )
							{
								Sleep( 200 );
							}

							pThreadArgs = (PTHREAD_ARGS)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( THREAD_ARGS ) );

							if ( pThreadArgs != NULL )
							{
								strcpy( pThreadArgs->Target,   szTarget );
								strcpy( pThreadArgs->Username, szUsername );
								strcpy( pThreadArgs->Password, szPassword );

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

		fclose( pInputFile );

		Sleep( 1000 );

		printf( "Waiting for threads to terminate...\n" );
	}
	else
	{
		strcpy( szTarget, szTargetInput );

		pDefaultsFile = fopen( szDictionary, "r" );

		if ( pDefaultsFile != NULL )
		{
			if ( i == 0 )
			{
				printf( "\n" );
				printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
				printf( "+++++          GUESS MySQL PASSWORDS          +++++\n" );
				printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
				printf( "\n" );

				i++;
			}

			while ( fgets( szUserPass, sizeof( szUserPass ), pDefaultsFile  ) != NULL )
			{
				if ( szUserPass[0] != '#' && szUserPass[0] != '\n' )
				{
					if ( szUserPass[strlen( szUserPass ) - 1] == '\n' )
					{
						szUserPass[strlen( szUserPass ) - 1] = '\0';
					}

					if ( Split( szUserPass, ":", szUsername, szPassword ) )
					{
						while ( nThreads >= MAX_THREADS )
						{
							Sleep( 200 );
						}

						pThreadArgs = (PTHREAD_ARGS)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( THREAD_ARGS ) );

						if ( pThreadArgs != NULL )
						{
							strcpy( pThreadArgs->Target,   szTarget );
							strcpy( pThreadArgs->Username, szUsername );
							strcpy( pThreadArgs->Password, szPassword );

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

VOID GuessMySQLPasswords( CHAR szTargetInput[], CHAR szDictionary[] )
{
	DWORD             i;
	FILE    *pInputFile;
	CHAR       szTarget[ 128 ];
	CHAR    szUsersFile[ 128 ];
	FILE    *pUsersFile;
	CHAR     szUsername[ 128 ];
	FILE   *pPasswdFile;
	CHAR     szPassword[ 128 ];
	CHAR  szTmpPassword[ 128 ];

	PTHREAD_ARGS pThreadArgs;

	i = 0;

	pInputFile = fopen( szTargetInput, "r" );

	if ( pInputFile != NULL )
	{
		while ( fscanf( pInputFile, "%s", szTarget ) != EOF )
		{
			sprintf( szUsersFile, "UserCache\\%s.users", szTarget );

			pUsersFile = fopen( szUsersFile, "r" );

			if ( pUsersFile != NULL )
			{
				while ( fgets( szUsername, sizeof( szUsername ), pUsersFile ) != NULL )
				{
					if ( szUsername[0] != '#' && szUsername[0] != '\n' )
					{
						if ( szUsername[strlen( szUsername ) - 1] == '\n' )
						{
							szUsername[strlen( szUsername ) - 1] = '\0';
						}

						pPasswdFile = fopen( szDictionary, "r" );

						if ( pPasswdFile != NULL )
						{
							if ( i == 0 )
							{
								printf( "\n" );
								printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
								printf( "+++++          GUESS MySQL PASSWORDS          +++++\n" );
								printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
								printf( "\n" );

								i++;
							}

							while ( fgets( szPassword, sizeof( szPassword ), pPasswdFile ) != NULL )
							{
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
										strcpy( pThreadArgs->Target,   szTarget );
										strcpy( pThreadArgs->Username, szUsername );
										strcpy( pThreadArgs->Password, szPassword );

										WaitForSingleObject( hSemaphore, INFINITE );

										nThreads++;

										ReleaseSemaphore( hSemaphore, 1, NULL );

										_beginthread( ThreadedSub, 0, (VOID *)pThreadArgs );
									}
								}
							}
						}

						fclose( pPasswdFile );
					}
				}

				fclose( pUsersFile );
			}
		}

		fclose( pInputFile );

		Sleep( 1000 );

		printf( "Waiting for threads to terminate...\n" );
	}
	else
	{
		strcpy( szTarget, szTargetInput );

		sprintf( szUsersFile, "UserCache\\%s.users", szTarget );

		pUsersFile = fopen( szUsersFile, "r" );

		if ( pUsersFile != NULL )
		{
			while ( fgets( szUsername, sizeof( szUsername ), pUsersFile ) != NULL )
			{
				if ( szUsername[0] != '#' && szUsername[0] != '\n' )
				{
					if ( szUsername[strlen( szUsername ) - 1] == '\n' )
					{
						szUsername[strlen( szUsername ) - 1] = '\0';
					}

					pPasswdFile = fopen( szDictionary, "r" );

					if ( pPasswdFile != NULL )
					{
						if ( i == 0 )
						{
							printf( "\n" );
							printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
							printf( "+++++          GUESS MySQL PASSWORDS          +++++\n" );
							printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
							printf( "\n" );

							i++;
						}

						while ( fgets( szPassword, sizeof( szPassword ), pPasswdFile ) != NULL )
						{
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
									strcpy( pThreadArgs->Target,   szTarget );
									strcpy( pThreadArgs->Username, szUsername );
									strcpy( pThreadArgs->Password, szPassword );

									WaitForSingleObject( hSemaphore, INFINITE );

									nThreads++;

									ReleaseSemaphore( hSemaphore, 1, NULL );

									_beginthread( ThreadedSub, 0, (VOID *)pThreadArgs );
								}
							}
						}
					}

					fclose( pPasswdFile );
				}
			}

			fclose( pUsersFile );
		}
	}

	while ( nThreads > 0 )
	{
		Sleep( 200 );
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

VOID ThreadedSub( VOID *pParameter )
{
	CHAR       szTarget[ 128 ];
	CHAR     szUsername[ 128 ];
	CHAR     szPassword[ 128 ];
	CHAR  szTmpUsername[ 128 ];
	CHAR  szTmpPassword[ 128 ];

	PTHREAD_ARGS pThreadArgs;

	pThreadArgs = (PTHREAD_ARGS)pParameter;

	strcpy( szTarget,   pThreadArgs->Target );
	strcpy( szUsername, pThreadArgs->Username );
	strcpy( szPassword, pThreadArgs->Password );

	HeapFree( GetProcessHeap(), 0, pThreadArgs );

	strcpy( szTmpUsername, szUsername );
	strcpy( szTmpPassword, szPassword );

	strupr( szTmpUsername );
	strupr( szTmpPassword );

	if ( strcmp( szTmpUsername, "<BLANK>" ) == 0 && strcmp( szTmpPassword, "<BLANK>" ) == 0 )
	{
		strcpy( szUsername, "" );
		strcpy( szPassword, "" );

		printf( "Trying <blank>:<blank> on target %s...\n", szTarget );
	}
	else if ( strcmp( szTmpUsername, "<BLANK>" ) == 0 && strcmp( szTmpPassword, "<USERNAME>" ) == 0 )
	{
		strcpy( szUsername, "" );
		strcpy( szPassword, szUsername );

		printf( "Trying <blank>:<blank> on target %s...\n", szTarget );
	}
	else if ( strcmp( szTmpUsername, "<BLANK>" ) == 0 )
	{
		strcpy( szUsername, "" );

		printf( "Trying <blank>:%s on target %s...\n", szPassword, szTarget );
	}
	else if ( strcmp( szTmpPassword, "<USERNAME>" ) == 0 )
	{
		strcpy( szPassword, szUsername );

		printf( "Trying %s:%s on target %s...\n", szUsername, szPassword, szTarget );
	}
	else if ( strcmp( szTmpPassword, "<BLANK>" ) == 0 )
	{
		strcpy( szPassword, "" );

		printf( "Trying %s:<blank> on target %s...\n", szUsername, szTarget );
	}
	else
	{
		printf( "Trying %s:%s on target %s...\n", szUsername, szPassword, szTarget );
	}

	if ( MySQLConnect( szTarget, szUsername, szPassword ) )
	{
		if ( strcmp( szTmpUsername, "<BLANK>" ) == 0 && strcmp( szTmpPassword, "<BLANK>" ) == 0 )
		{
			printf( "\n" );
			printf( "PASSWORD GUESSED! <blank>@%s, password is <blank>\n", szTarget );
		}
		else if ( strcmp( szTmpUsername, "<BLANK>" ) == 0 && strcmp( szTmpPassword, "<USERNAME>" ) == 0 )
		{
			printf( "\n" );
			printf( "PASSWORD GUESSED! <blank>@%s, password is <blank>\n", szTarget );
		}
		else if ( strcmp( szTmpPassword, "<BLANK>" ) == 0 )
		{
			printf( "\n" );
			printf( "PASSWORD GUESSED! %s@%s, password is <blank>\n", szUsername, szTarget );
		}
		else
		{
			printf( "\n" );
			printf( "PASSWORD GUESSED! %s@%s, password is %s\n", szUsername, szTarget, szPassword );
		}
	}

	WaitForSingleObject( hSemaphore, INFINITE );

	nThreads--;

	ReleaseSemaphore( hSemaphore, 1, NULL );

	_endthread();
}

BOOL MySQLConnect( CHAR szTarget[], CHAR szUsername[], CHAR szPassword[] )
{
	BOOL          bLogonSuccess;
	CHAR           szODBCDriver[ 1024 ];
	CHAR        szConnectString[ 1024 ];
	SQLRETURN        sqlReturn1;
	SQLHENV                hEnv;
	SQLHDBC                hDbc;
	SQLCHAR           szOutConn[ 2048 ];
	SQLSMALLINT            sLen;
	SQLHSTMT              hStmt;
	CHAR                szQuery[ 2048 ];
	DWORD                     i;
	SQLCHAR             szData1[ 2048 ];
	SQLRETURN        sqlReturn2;
	SQLINTEGER           sData1;
	CHAR          szLogFileName[ 128 ];
	FILE           *pOutputFile;
	SQLCHAR             szData2[ 2048 ];
	SQLRETURN        sqlReturn3;
	SQLINTEGER           sData2;
	FILE            *pInputFile;
	DWORD                     j;
	SQLSMALLINT        sColumns;
	DWORD             dwColumns;
	DWORD                     k;
	SQLCHAR     szTmpColumnName[ 2048 ];
	SQLSMALLINT  sColumnNameLen;
	SQLSMALLINT sColumnDataType;
	SQLUINTEGER     sColumnSize;
	SQLSMALLINT  sDecimalDigits;
	SQLSMALLINT       sNullable;
	CHAR           szColumnName[ 2048 ];
	SQLCHAR          szSQLState[ 8 ];
	SQLINTEGER             sErr;
	SQLCHAR            szErrMsg[ 1024 ];
	SQLSMALLINT         sErrLen;
	CHAR              *pNewLine;

	bLogonSuccess = FALSE;

	GetMySQLODBCDriver( szODBCDriver );

	sprintf( szConnectString, "DRIVER={%s};SERVER=%s;DATABASE=mysql;USER=%s;PASSWORD=%s;OPTION=3", szODBCDriver, szTarget, szUsername, szPassword );

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

				sqlReturn1 = SQLDriverConnect( hDbc, NULL, (SQLCHAR *)szConnectString, strlen( szConnectString ), szOutConn, 2048, &sLen, SQL_DRIVER_NOPROMPT );

				if ( sqlReturn1 == SQL_SUCCESS || sqlReturn1 == SQL_SUCCESS_WITH_INFO )
				{
					bLogonSuccess = TRUE;

					LogGuessedPasswords( szTarget, szUsername, szPassword );

					sqlReturn1 = SQLAllocHandle( SQL_HANDLE_STMT, hDbc, &hStmt );

					if ( sqlReturn1 == SQL_SUCCESS || sqlReturn1 == SQL_SUCCESS_WITH_INFO )
					{
						strcpy( szQuery, "SELECT DISTINCT mysql.user.user FROM mysql.user WHERE mysql.user.user <> ''" );

						sqlReturn1 = SQLExecDirect( hStmt, (SQLCHAR *)szQuery, SQL_NTS );

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
										sprintf( szLogFileName, "%s-MySQLUserInfo.txt", szTarget );

										pOutputFile = fopen( szLogFileName, "w" );

										if ( pOutputFile != NULL )
										{
											fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
											fprintf( pOutputFile, "\n" );
											fprintf( pOutputFile, "Hostname\tMySQL Username\n" );

											fclose( pOutputFile );
										}

										sprintf( szLogFileName, "UserCache\\%s.users", szTarget );

										pOutputFile = fopen( szLogFileName, "w" );

										if ( pOutputFile != NULL )
										{
											fclose( pOutputFile );
										}

										i++;
									}

									sprintf( szLogFileName, "%s-MySQLUserInfo.txt", szTarget );

									pOutputFile = fopen( szLogFileName, "a+" );

									if ( pOutputFile != NULL )
									{
										fprintf( pOutputFile, "%s\t%*s\n", szTarget, strlen( szData1 ) - 1, szData1 );

										fclose( pOutputFile );
									}

									sprintf( szLogFileName, "UserCache\\%s.users", szTarget );

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

					sqlReturn1 = SQLAllocHandle( SQL_HANDLE_STMT, hDbc, &hStmt );

					if ( sqlReturn1 == SQL_SUCCESS || sqlReturn1 == SQL_SUCCESS_WITH_INFO )
					{
						strcpy( szQuery, "SELECT mysql.user.user, mysql.user.password FROM mysql.user" );

						sqlReturn1 = SQLExecDirect( hStmt, (SQLCHAR *)szQuery, SQL_NTS );

						WaitForSingleObject( hSemaphore, INFINITE );

						i = 0;

						while ( sqlReturn1 == SQL_SUCCESS || sqlReturn1 == SQL_SUCCESS_WITH_INFO )
						{
							sqlReturn1 = SQLFetch( hStmt );

							if ( sqlReturn1 == SQL_SUCCESS || sqlReturn1 == SQL_SUCCESS_WITH_INFO )
							{
								strcpy( szData1, "" );
								strcpy( szData2, "" );

								sqlReturn2 = SQLGetData( hStmt, 1, SQL_C_CHAR, szData1, sizeof( szData1 ), &sData1 );
								sqlReturn3 = SQLGetData( hStmt, 2, SQL_C_CHAR, szData2, sizeof( szData2 ), &sData2 );

								if ( ( sqlReturn2 == SQL_SUCCESS || sqlReturn2 == SQL_SUCCESS_WITH_INFO ) && ( sqlReturn3 == SQL_SUCCESS || sqlReturn3 == SQL_SUCCESS_WITH_INFO ) )
								{
									sprintf( szLogFileName, "%s-MySQLPasswordHashes.txt", szTarget );

									if ( i == 0 )
									{
										pOutputFile = fopen( szLogFileName, "w" );

										if ( pOutputFile != NULL )
										{
											fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
											fprintf( pOutputFile, "\n" );
											fprintf( pOutputFile, "Hostname\tMySQL Username\tPassword Hash\n" );

											fclose( pOutputFile );
										}

										LogGuessedPrivilegedAccountPasswords( szTarget, szUsername, szPassword );

										i++;
									}

									pOutputFile = fopen( szLogFileName, "a+" );

									if ( pOutputFile != NULL )
									{
										fprintf( pOutputFile, "%s\t%*s\t%*s\n", szTarget, strlen( szData1 ) - 1, szData1, strlen( szData2 ) - 1, szData2 );

										fclose( pOutputFile );
									}
								}
							}
						}

						ReleaseSemaphore( hSemaphore, 1, NULL );
					}

					SQLFreeHandle( SQL_HANDLE_STMT, hStmt );

					pInputFile = fopen( "querystrings.txt", "r" );

					if ( pInputFile != NULL )
					{
						i = 0;

						while ( fgets( szQuery, sizeof( szQuery ), pInputFile ) != NULL )
						{
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
										sprintf( szLogFileName, "%s-MySQLQuery%d.txt", szTarget, i + 1 );

										sqlReturn1 = SQLNumResultCols( hStmt, &sColumns );

										if ( sqlReturn1 == SQL_SUCCESS || sqlReturn1 == SQL_SUCCESS_WITH_INFO )
										{
											dwColumns = sColumns;

											for ( k = 0; k < dwColumns; k++ )
											{
												sqlReturn1 = SQLDescribeCol( hStmt, k + 1, szTmpColumnName, sizeof( szTmpColumnName ), &sColumnNameLen, &sColumnDataType, &sColumnSize, &sDecimalDigits, &sNullable );

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
														if ( k == 0 )
														{
															fprintf( pOutputFile, "Hostname\t" );
														}

														if ( ( dwColumns - k ) > 1 )
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
											for ( k = 0; k < dwColumns; k++ )
											{
												strcpy( szData1, "" );

												sqlReturn2 = SQLGetData( hStmt, k + 1, SQL_C_CHAR, szData1, sizeof( szData1 ), &sData1 );

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
														if ( k == 0 )
														{
															fprintf( pOutputFile, "%s\t", szTarget );
														}

														if ( ( dwColumns - k ) > 1 )
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

						fclose( pInputFile );
					}

					SQLDisconnect( hDbc );
				}
				else
				{
					sqlReturn1 = SQLGetDiagRec( SQL_HANDLE_DBC, hDbc, 1, szSQLState, &sErr, szErrMsg, sizeof( szErrMsg ), &sErrLen );

					if ( sqlReturn1 == SQL_SUCCESS || sqlReturn1 == SQL_SUCCESS_WITH_INFO )
					{
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
								fprintf( pOutputFile, "Hostname\tMySQL Username\tPassword\tError Number\tError Message\n" );

								fclose( pOutputFile );
							}
						}

						pOutputFile = fopen( "ConnectionErrors.txt", "a+" );

						if ( pOutputFile != NULL )
						{
							if ( strcmp( szPassword, "" ) == 0 )
							{
								fprintf( pOutputFile, "%s\t%s\t<blank>\t%d\t%s\n", szTarget, szUsername, sErr, szErrMsg );
							}
							else
							{
								fprintf( pOutputFile, "%s\t%s\t%s\t%d\t%s\n", szTarget, szUsername, szPassword, sErr, szErrMsg );
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

	return bLogonSuccess;
}

VOID GetMySQLODBCDriver( CHAR szODBCDriver[] )
{
	FILE *pInputFile;
	CHAR      szLine[ 1024 ];

	pInputFile = fopen( "MySQLScan.ini", "r" );

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

				strcpy( szODBCDriver, szLine );

				break;
			}
		}

		fclose( pInputFile );
	}
}

VOID LogGuessedPasswords( CHAR szTarget[], CHAR szUsername[], CHAR szPassword[] )
{
	FILE *pOutputFile;

	WaitForSingleObject( hSemaphore, INFINITE );

	pOutputFile = fopen( "GuessedMySQLPasswords.txt", "r" );

	if ( pOutputFile != NULL )
	{
		fclose( pOutputFile );
	}
	else
	{
		pOutputFile = fopen( "GuessedMySQLPasswords.txt", "w" );

		if ( pOutputFile != NULL )
		{
			fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
			fprintf( pOutputFile, "\n" );
			fprintf( pOutputFile, "Hostname\tMySQL Username\tPassword\n" );

			fclose( pOutputFile );
		}
	}

	pOutputFile = fopen( "GuessedMySQLPasswords.txt", "a+" );

	if ( pOutputFile != NULL )
	{
		if ( strcmp( szUsername, "" ) == 0 && strcmp( szPassword, "" ) == 0 )
		{
			fprintf( pOutputFile, "%s\t<blank>\t<blank>\n", szTarget );
		}
		else if ( strcmp( szPassword, "" ) == 0 )
		{
			fprintf( pOutputFile, "%s\t%s\t<blank>\n", szTarget, szUsername );
		}
		else
		{
			fprintf( pOutputFile, "%s\t%s\t%s\n", szTarget, szUsername, szPassword );
		}

		fclose( pOutputFile );
	}

	ReleaseSemaphore( hSemaphore, 1, NULL );
}

VOID LogGuessedPrivilegedAccountPasswords( CHAR szTarget[], CHAR szUsername[], CHAR szPassword[] )
{
	FILE *pOutputFile;

	pOutputFile = fopen( "GuessedMySQLPrivilegedAccountPasswords.txt", "r" );

	if ( pOutputFile != NULL )
	{
		fclose( pOutputFile );
	}
	else
	{
		pOutputFile = fopen( "GuessedMySQLPrivilegedAccountPasswords.txt", "w" );

		if ( pOutputFile != NULL )
		{
			fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
			fprintf( pOutputFile, "\n" );
			fprintf( pOutputFile, "Hostname\tMySQL Username\tPassword\tIs Privileged Account?\n" );

			fclose( pOutputFile );
		}
	}

	pOutputFile = fopen( "GuessedMySQLPrivilegedAccountPasswords.txt", "a+" );

	if ( pOutputFile != NULL )
	{
		if ( strcmp( szUsername, "" ) == 0 && strcmp( szPassword, "" ) == 0 )
		{
			fprintf( pOutputFile, "%s\t<blank>\t<blank>\tYes\n", szTarget );
		}
		else if ( strcmp( szPassword, "" ) == 0 )
		{
			fprintf( pOutputFile, "%s\t%s\t<blank>\tYes\n", szTarget, szUsername );
		}
		else
		{
			fprintf( pOutputFile, "%s\t%s\t%s\tYes\n", szTarget, szUsername, szPassword );
		}

		fclose( pOutputFile );
	}
}

// Written by Reed Arvin | reed@canaudit.com
