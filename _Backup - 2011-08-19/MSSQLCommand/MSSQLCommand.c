//
// gcc source.c -o source.exe -lodbc32
//

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <sql.h>
#include <sqlext.h>

INT main( INT argc, CHAR *argv[] )
{
	CHAR               szTarget[ 128 ];
	CHAR             szUsername[ 128 ];
	CHAR             szPassword[ 128 ];
	CHAR         szCommandInput[ 128 ];
	CHAR        szConnectString[ 1024 ];
	SQLRETURN         sqlReturn;
	SQLHENV                hEnv;
	SQLHDBC                hDbc;
	SQLCHAR           szOutConn[ 2048 ];
	SQLSMALLINT            sLen;
	FILE          *pCommandFile;
	SQLHSTMT              hStmt;
	CHAR              szCommand[ 2048 ];
	CHAR                szQuery[ 2048 ];
	SQLCHAR              szData[ 2048 ];
	SQLINTEGER            sData;
	SQLCHAR          szSQLState[ 8 ];
	SQLINTEGER             sErr;
	SQLCHAR            szErrMsg[ 1024 ];
	SQLSMALLINT         sErrLen;

	if ( argc == 5 )
	{
		strcpy( szTarget,       argv[1] );
		strcpy( szUsername,     argv[2] );
		strcpy( szPassword,     argv[3] );
		strcpy( szCommandInput, argv[4] );

		sprintf( szConnectString, "DRIVER={SQL Server};SERVER=%s;NETWORK=dbmssocn;UID=%s;PWD=%s", szTarget, szUsername, szPassword );

		printf( "Running MSSQLCommand v1.0 with the following arguments:\n" );
		printf( "[+] Host Input:    \"%s\"\n", szTarget );
		printf( "[+] Username:      \"%s\"\n", szUsername );
		printf( "[+] Password:      \"%s\"\n", szPassword );
		printf( "[+] Command Input: \"%s\"\n", szCommandInput );
		printf( "\n" );

		sqlReturn = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv );

		if ( sqlReturn == SQL_SUCCESS || sqlReturn == SQL_SUCCESS_WITH_INFO )
		{
			sqlReturn = SQLSetEnvAttr( hEnv, SQL_ATTR_ODBC_VERSION, (VOID *)SQL_OV_ODBC3, 0 );

			if ( sqlReturn == SQL_SUCCESS || sqlReturn == SQL_SUCCESS_WITH_INFO )
			{
				sqlReturn = SQLAllocHandle( SQL_HANDLE_DBC, hEnv, &hDbc );

				if ( sqlReturn == SQL_SUCCESS || sqlReturn == SQL_SUCCESS_WITH_INFO )
				{
					SQLSetConnectAttr( hDbc, SQL_LOGIN_TIMEOUT, (VOID *)2, 0 );

					sqlReturn = SQLDriverConnect( hDbc, NULL, (SQLCHAR *)szConnectString, strlen( szConnectString ), szOutConn, 2048, &sLen, SQL_DRIVER_NOPROMPT );

					if ( sqlReturn == SQL_SUCCESS || sqlReturn == SQL_SUCCESS_WITH_INFO )
					{
						pCommandFile = fopen( szCommandInput, "r" );

						if ( pCommandFile != NULL )
						{
							while ( fgets( szCommand, sizeof( szCommand ), pCommandFile ) != NULL )
							{
								if ( szCommand[0] != '#' && szCommand[0] != '\n' )
								{
									sqlReturn = SQLAllocHandle( SQL_HANDLE_STMT, hDbc, &hStmt );

									if ( sqlReturn == SQL_SUCCESS || sqlReturn == SQL_SUCCESS_WITH_INFO )
									{
										if ( szCommand[strlen( szCommand ) - 1] == '\n' )
										{
											szCommand[strlen( szCommand ) - 1] = '\0';
										}

										printf( "Running command \"%s\"...\n", szCommand );
										printf( "\n" );

										sprintf( szQuery, "EXEC master..xp_cmdshell \"%s\"", szCommand );

										sqlReturn = SQLExecDirect( hStmt, (SQLCHAR *)szQuery, SQL_NTS );

										while ( sqlReturn == SQL_SUCCESS || sqlReturn == SQL_SUCCESS_WITH_INFO )
										{
											sqlReturn = SQLFetch( hStmt );

											if ( sqlReturn == SQL_SUCCESS || sqlReturn == SQL_SUCCESS_WITH_INFO )
											{
												strcpy( szData, "" );

												sqlReturn = SQLGetData( hStmt, 1, SQL_C_CHAR, szData, sizeof( szData ), &sData );

												if ( sqlReturn == SQL_SUCCESS || sqlReturn == SQL_SUCCESS_WITH_INFO )
												{
													printf( "%*s\n", strlen( szData ) - 1, szData );
												}
											}
										}
									}

									SQLFreeHandle( SQL_HANDLE_STMT, hStmt );
								}
							}

							fclose( pCommandFile );
						}
						else
						{
							sqlReturn = SQLAllocHandle( SQL_HANDLE_STMT, hDbc, &hStmt );

							if ( sqlReturn == SQL_SUCCESS || sqlReturn == SQL_SUCCESS_WITH_INFO )
							{
								strcpy( szCommand, szCommandInput );

								printf( "Running command \"%s\"...\n", szCommand );
								printf( "\n" );

								sprintf( szQuery, "EXEC master.dbo.xp_cmdshell \"%s\"", szCommand );

								sqlReturn = SQLExecDirect( hStmt, (SQLCHAR *)szQuery, SQL_NTS);

								while ( sqlReturn == SQL_SUCCESS || sqlReturn == SQL_SUCCESS_WITH_INFO )
								{
									sqlReturn = SQLFetch( hStmt );

									if ( sqlReturn == SQL_SUCCESS || sqlReturn == SQL_SUCCESS_WITH_INFO )
									{
										strcpy( szData, "" );

										sqlReturn = SQLGetData( hStmt, 1, SQL_C_CHAR, szData, sizeof( szData ), &sData );

										if ( sqlReturn == SQL_SUCCESS || sqlReturn == SQL_SUCCESS_WITH_INFO )
										{
											printf( "%*s\n", strlen( szData ) - 1, szData );
										}
									}
								}

								SQLFreeHandle( SQL_HANDLE_STMT, hStmt );
							}
						}

						SQLDisconnect( hDbc );
					}
					else
					{
						sqlReturn = SQLGetDiagRec( SQL_HANDLE_DBC, hDbc, 1, szSQLState, &sErr, szErrMsg, sizeof( szErrMsg ), &sErrLen );

						if ( sqlReturn == SQL_SUCCESS || sqlReturn == SQL_SUCCESS_WITH_INFO )
						{
							if ( sErr == 18452 )
							{
								fprintf( stderr, "ERROR! Trusted connection required.\n" );
							}
							else if ( sErr == 18456 )
							{
								fprintf( stderr, "ERROR! Bad username/password combination.\n" );
							}
							else
							{
								fprintf( stderr, "ERROR! ODBC driver error.\n" );
							}
						}
					}
				}

				SQLFreeHandle( SQL_HANDLE_DBC, hDbc );
			}
		}

		SQLFreeHandle( SQL_HANDLE_ENV, hEnv );
	}
	else
	{
		printf( "MSSQLCommand v1.0 | Canaudit, Inc. http://www.canaudit.com/\n" );
		printf( "\n" );
		printf( "Usage: MSSQLCommand <hostname | ip> <username> <password> <command | command file>\n" );
		printf( "\n" );
		printf( "<hostname | ip>           -- required argument\n" );
		printf( "<username>                -- required argument\n" );
		printf( "<password>                -- required argument\n" );
		printf( "<command | command file>  -- required argument\n" );
		printf( "\n" );
		printf( "Examples:\n" );
		printf( "MSSQLCommand 10.10.10.10 sa \"\" \"net view\"\n" );
		printf( "MSSQLCommand 10.10.10.10 sa sa \"net user\"\n" );
		printf( "MSSQLCommand 10.10.10.10 sa sa commands.txt\n" );
		printf( "\n" );
		printf( "MSSQLCommand MyWindowsMachine sa \"\" \"net view\"\n" );
		printf( "MSSQLCommand MyWindowsMachine sa sa \"net user\"\n" );
		printf( "MSSQLCommand MyWindowsMachine sa sa commands.txt\n" );
		printf( "\n" );
		printf( "(Written by Reed Arvin | reed@canaudit.com)\n" );
	}

	return 0;
}
