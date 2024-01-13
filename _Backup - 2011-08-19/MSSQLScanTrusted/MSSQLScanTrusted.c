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

VOID            ThreadedSub( VOID *pParameter );
BOOL      MSSQLConnectTCPIP( CHAR *szTarget );
VOID     GetMSSQLODBCDriver( CHAR *szODBCDriver );
BOOL MSSQLConnectNamedPipes( CHAR *szTarget );

typedef struct _THREAD_ARGS
{
	CHAR Target[ 128 ];
} THREAD_ARGS, *PTHREAD_ARGS;

HANDLE hSemaphore;

INT nThreads = 0;

INT main( INT argc, CHAR *argv[] )
{
	CHAR szTargetInput[ 128 ];
	FILE   *pInputFile;
	CHAR      szTarget[ 128 ];

	PTHREAD_ARGS pThreadArgs;

	hSemaphore = CreateSemaphore( NULL, 1, 1, NULL );

	if ( argc == 2 )
	{
		strcpy( szTargetInput, argv[1] );

		printf( "Running MSSQLScanTrusted v1.0 with the following arguments:\n" );
		printf( "[+] Host Input: \"%s\"\n", szTargetInput );
		printf( "\n" );

		pInputFile = fopen( szTargetInput, "r" );

		if ( pInputFile != NULL )
		{
			while ( fscanf( pInputFile, "%s", szTarget ) != EOF )
			{
				while ( nThreads >= MAX_THREADS )
				{
					Sleep( 200 );
				}

				pThreadArgs = (PTHREAD_ARGS)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( THREAD_ARGS ) );

				if ( pThreadArgs != NULL )
				{
					strcpy( pThreadArgs->Target, szTarget );

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
				strcpy( pThreadArgs->Target, szTarget );

				WaitForSingleObject( hSemaphore, INFINITE );

				nThreads++;

				ReleaseSemaphore( hSemaphore, 1, NULL );

				_beginthread( ThreadedSub, 0, (VOID *)pThreadArgs );
			}
		}

		while ( nThreads >= MAX_THREADS )
		{
			Sleep( 200 );
		}
	}
	else
	{
		printf( "MSSQLScanTrusted v1.0 | Canaudit, Inc. http://www.canaudit.com/\n" );
		printf( "\n" );
		printf( "Usage: MSSQLScanTrusted <hostname | ip input file>\n" );
		printf( "\n" );
		printf( "<hostname | ip input file>  -- required argument\n" );
		printf( "\n" );
		printf( "Examples:\n" );
		printf( "MSSQLScanTrusted 10.10.10.10\n" );
		printf( "MSSQLScanTrusted MyMSSQLMachine\n" );
		printf( "MSSQLScanTrusted IPInputFile.txt\n" );
		printf( "\n" );
		printf( "(Written by Reed Arvin | reed@canaudit.com)\n" );
	}

	CloseHandle( hSemaphore );

	return 0;
}

VOID ThreadedSub( VOID *pParameter )
{
	CHAR szTarget[ 128 ];

	PTHREAD_ARGS pThreadArgs;

	pThreadArgs = (PTHREAD_ARGS)pParameter;

	strcpy( szTarget, pThreadArgs->Target );

	HeapFree( GetProcessHeap(), 0, pThreadArgs );

	printf( "Trying to connect to MS SQL Server on host %s...\n", szTarget );

	if ( MSSQLConnectTCPIP( szTarget ) )
	{
		printf( "\n" );
		printf( "SUCCESS! Connected successfully to MS SQL Server on host %s\n", szTarget );
	}
	else
	{
		if ( MSSQLConnectNamedPipes( szTarget ) )
		{
			printf( "\n" );
			printf( "SUCCESS! Connected successfully to MS SQL Server on host %s\n", szTarget );
		}
	}

	WaitForSingleObject( hSemaphore, INFINITE );

	nThreads--;

	ReleaseSemaphore( hSemaphore, 1, NULL );

	_endthread();
}

BOOL MSSQLConnectTCPIP( CHAR szTarget[] )
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

	GetMSSQLODBCDriver( szODBCDriver );

	sprintf( szConnectString, "DRIVER={%s};SERVER=%s;NETWORK=dbmssocn;Trusted_Connection=Yes", szODBCDriver, szTarget );

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

					sqlReturn1 = SQLAllocHandle( SQL_HANDLE_STMT, hDbc, &hStmt );

					if ( sqlReturn1 == SQL_SUCCESS || sqlReturn1 == SQL_SUCCESS_WITH_INFO )
					{
						strcpy( szQuery, "SELECT master..syslogins.name FROM master..syslogins" );

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
										sprintf( szLogFileName, "%s-MSSQLUserInfo.txt", szTarget );

										pOutputFile = fopen( szLogFileName, "w" );

										if ( pOutputFile != NULL )
										{
											fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
											fprintf( pOutputFile, "\n" );
											fprintf( pOutputFile, "Hostname\tMS SQL Username\n" );

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

									sprintf( szLogFileName, "%s-MSSQLUserInfo.txt", szTarget );

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
						strcpy( szQuery, "SELECT master..sysxlogins.name, master..sysxlogins.password FROM master..sysxlogins" );

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
									sprintf( szLogFileName, "%s-MSSQLPasswordHashes.txt", szTarget );

									if ( i == 0 )
									{
										pOutputFile = fopen( szLogFileName, "w" );

										if ( pOutputFile != NULL )
										{
											fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
											fprintf( pOutputFile, "\n" );
											fprintf( pOutputFile, "Hostname\tMS SQL Username\tPassword Hash\n" );

											fclose( pOutputFile );
										}

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
										sprintf( szLogFileName, "%s-MSSQLQuery%d.txt", szTarget, i + 1 );

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

						if ( sErr != 18456 )
						{
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
									fprintf( pOutputFile, "Hostname\tError Number\tError Message\n" );

									fclose( pOutputFile );
								}
							}

							pOutputFile = fopen( "ConnectionErrors.txt", "a+" );

							if ( pOutputFile != NULL )
							{
								fprintf( pOutputFile, "%s\t%d\t%s\n", szTarget, sErr, szErrMsg );

								fclose( pOutputFile );
							}

							ReleaseSemaphore( hSemaphore, 1, NULL );
						}
					}
				}
			}

			SQLFreeHandle( SQL_HANDLE_DBC, hDbc );
		}
	}

	SQLFreeHandle( SQL_HANDLE_ENV, hEnv );

	return bLogonSuccess;
}

VOID GetMSSQLODBCDriver( CHAR szODBCDriver[] )
{
	FILE *pInputFile;
	CHAR      szLine[ 1024 ];

	pInputFile = fopen( "MSSQLScanTrusted.ini", "r" );

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

BOOL MSSQLConnectNamedPipes( CHAR szTarget[] )
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

	GetMSSQLODBCDriver( szODBCDriver );

	sprintf( szConnectString, "DRIVER={%s};SERVER=%s;NETWORK=dbnmpntw;Trusted_Connection=Yes", szODBCDriver, szTarget );

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

					sqlReturn1 = SQLAllocHandle( SQL_HANDLE_STMT, hDbc, &hStmt );

					if ( sqlReturn1 == SQL_SUCCESS || sqlReturn1 == SQL_SUCCESS_WITH_INFO )
					{
						strcpy( szQuery, "SELECT master..syslogins.name FROM master..syslogins" );

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
										sprintf( szLogFileName, "%s-MSSQLUserInfo.txt", szTarget );

										pOutputFile = fopen( szLogFileName, "w" );

										if ( pOutputFile != NULL )
										{
											fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
											fprintf( pOutputFile, "\n" );
											fprintf( pOutputFile, "Hostname\tMS SQL Username\n" );

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

									sprintf( szLogFileName, "%s-MSSQLUserInfo.txt", szTarget );

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
						strcpy( szQuery, "SELECT master..sysxlogins.name, master..sysxlogins.password FROM master..sysxlogins" );

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
									sprintf( szLogFileName, "%s-MSSQLPasswordHashes.txt", szTarget );

									if ( i == 0 )
									{
										pOutputFile = fopen( szLogFileName, "w" );

										if ( pOutputFile != NULL )
										{
											fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
											fprintf( pOutputFile, "\n" );
											fprintf( pOutputFile, "Hostname\tMS SQL Username\tPassword Hash\n" );

											fclose( pOutputFile );
										}

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
										sprintf( szLogFileName, "%s-MSSQLQuery%d.txt", szTarget, i + 1 );

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

						if ( sErr != 18456 )
						{
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
									fprintf( pOutputFile, "Hostname\tError Number\tError Message\n" );

									fclose( pOutputFile );
								}
							}

							pOutputFile = fopen( "ConnectionErrors.txt", "a+" );

							if ( pOutputFile != NULL )
							{
								fprintf( pOutputFile, "%s\t%d\t%s\n", szTarget, sErr, szErrMsg );

								fclose( pOutputFile );
							}

							ReleaseSemaphore( hSemaphore, 1, NULL );
						}
					}
				}
			}

			SQLFreeHandle( SQL_HANDLE_DBC, hDbc );
		}
	}

	SQLFreeHandle( SQL_HANDLE_ENV, hEnv );

	return bLogonSuccess;
}

VOID LogGuessedPasswords( CHAR szTarget[], CHAR szUsername[], CHAR szPassword[] )
{
	FILE *pOutputFile;

	WaitForSingleObject( hSemaphore, INFINITE );

	pOutputFile = fopen( "GuessedMSSQLPasswords.txt", "r" );

	if ( pOutputFile != NULL )
	{
		fclose( pOutputFile );
	}
	else
	{
		pOutputFile = fopen( "GuessedMSSQLPasswords.txt", "w" );

		if ( pOutputFile != NULL )
		{
			fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
			fprintf( pOutputFile, "\n" );
			fprintf( pOutputFile, "Hostname\tMS SQL Username\tPassword\n" );

			fclose( pOutputFile );
		}
	}

	pOutputFile = fopen( "GuessedMSSQLPasswords.txt", "a+" );

	if ( pOutputFile != NULL )
	{
		if ( strcmp( szPassword, "" ) == 0 )
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

VOID LogGuessedDBAPasswords( CHAR szTarget[], CHAR szUsername[], CHAR szPassword[] )
{
	FILE *pOutputFile;

	pOutputFile = fopen( "GuessedMSSQLDBAPasswords.txt", "r" );

	if ( pOutputFile != NULL )
	{
		fclose( pOutputFile );
	}
	else
	{
		pOutputFile = fopen( "GuessedMSSQLDBAPasswords.txt", "w" );

		if ( pOutputFile != NULL )
		{
			fprintf( pOutputFile, "NOTE: This file is tab separated. Open with Excel to view and sort information.\n" );
			fprintf( pOutputFile, "\n" );
			fprintf( pOutputFile, "Hostname\tMS SQL Username\tPassword\tIs Privileged Account?\n" );

			fclose( pOutputFile );
		}
	}

	pOutputFile = fopen( "GuessedMSSQLDBAPasswords.txt", "a+" );

	if ( pOutputFile != NULL )
	{
		if ( strcmp( szPassword, "" ) == 0 )
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
