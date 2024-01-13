//
// gcc source.c -o source.exe
//

#include <windows.h>
#include <string.h>
#include <stdio.h>

VOID GeneratePayload( CHAR *szOverflowChars, DWORD *dwNumRepeats, CHAR *szPayloadData );

INT main( INT argc, CHAR *argv[] )
{
	CHAR       szFileName[ 128 ];
	CHAR        szFuzzOpt[ 128 ];
	CHAR  szOverflowChars[ 128 ];
	DWORD    dwNumRepeats;
	DWORD       dwFileNum;
	DWORD      dwNumBytes;
	DWORD               i;
	FILE      *pInputFile;
	CHAR    szNewFileName[ 128 ];
	FILE     *pOutputFile;
	DWORD               j;
	INT             nByte;
	CHAR    szPayloadData[ 102400 ];
	DWORD               k;

	if ( argc > 4 && argc < 7 && ( strcmp( argv[2], "-i" ) == 0 || strcmp( argv[2], "-a" ) == 0 ) )
	{
		strcpy( szFileName,      argv[1] );
		strcpy( szFuzzOpt,       argv[2] );
		strcpy( szOverflowChars, argv[3] );

		strlwr( szFuzzOpt );

		dwNumRepeats = atoi( argv[4] );

		if ( argc == 6 )
		{
			dwFileNum = atoi( argv[5] );
		}
		else
		{
			dwFileNum = 0;
		}

		printf( "Running FuzzyFiles v1.0 with the following arguments:\n" );
		printf( "[+] File Name:      \"%s\"\n", szFileName );
		printf( "[+] Fuzz Mode:      \"%s\"\n", szFuzzOpt );
		printf( "[+] Overflow Chars: \"%s\"\n", szOverflowChars );
		printf( "[+] Repeat #:       \"%d\"\n", dwNumRepeats );

		if ( dwFileNum != 0 )
		{
			printf( "[+] File #:         \"%d\"\n", dwFileNum );
		}

		printf( "\n" );

		dwNumBytes = -1;

		i = 0;

		while ( i < dwNumBytes || dwNumBytes == -1 )
		{
			pInputFile = fopen( szFileName, "rb" );

			if ( pInputFile != NULL )
			{
				if ( dwFileNum == 0 || dwFileNum == ( i + 1 ) )
				{
					sprintf( szNewFileName, "FileCache\\%d_%s", i + 1, szFileName );

					pOutputFile = fopen( szNewFileName, "w" );

					if ( pOutputFile != NULL )
					{
						j = 0;
						k = 0;

						while ( feof( pInputFile ) == 0 )
						{
							nByte = fgetc( pInputFile );

							if ( nByte != -1 )
							{
								if ( i == j )
								{
									GeneratePayload( szOverflowChars, &dwNumRepeats, szPayloadData );

									if ( strcmp( szFuzzOpt, "-i" ) == 0 )
									{
										fprintf( pOutputFile, "%s", szPayloadData );

										k = strlen( szPayloadData ) - 1;
									}

									if ( strcmp( szFuzzOpt, "-a" ) == 0 )
									{
										fprintf( pOutputFile, "%c", nByte );
										fprintf( pOutputFile, "%s", szPayloadData );
									}
								}
								else
								{
									if ( k == 0 )
									{
										fprintf( pOutputFile, "%c", nByte );
									}
									else
									{
										k--;
									}
								}

								j++;
							}
						}

						dwNumBytes = j;

						fclose( pOutputFile );

						printf( "Created file %d of %d\n", i + 1, dwNumBytes );
					}
				}

				fclose( pInputFile );
			}
			else
			{
				fprintf( stderr, "ERROR! Cannot open input file \"%s\"\n", szFileName );

				break;
			}

			i++;
		}
	}
	else
	{
		printf( "FuzzyFiles v1.0 | http://reedarvin.thearvins.com/\n" );
		printf( "\n" );
		printf( "Usage: FuzzyFiles <file name> [-i|-a] <overflow chars> <repeat #> <file #>\n" );
		printf( "\n" );
		printf( "<file name>       -- required argument\n" );
		printf( "[-i|-a]           -- required argument\n" );
		printf( "<overflow chars>  -- required argument\n" );
		printf( "<repeat #>        -- required argument\n" );
		printf( "<file #>          -- optional argument\n" );
		printf( "\n" );
		printf( "-i  -- Insert overflow characters\n" );
		printf( "-a  -- Append overflow characters\n" );
		printf( "\n" );
		printf( "Examples:\n" );
		printf( "FuzzyFiles test.gif -i A 1024\n" );
		printf( "FuzzyFiles test.gif -a %%x 1024\n" );
		printf( "FuzzyFiles test.gif -i A 1024 100\n" );
		printf( "\n" );
		printf( "(Written by Reed Arvin | reedarvin@gmail.com)\n" );
	}

	return 0;
}

VOID GeneratePayload( CHAR szOverflowChars[], DWORD *dwNumRepeats, CHAR szPayloadData[] )
{
	DWORD i;
	DWORD j;

	strcpy( szPayloadData, "" );

	if ( strlen( szOverflowChars ) == 1 )
	{
		memset( szPayloadData, szOverflowChars[0], *dwNumRepeats );

		szPayloadData[*dwNumRepeats] = '\0';
	}
	else
	{
		i = 0;
		j = 0;

		while ( i < *dwNumRepeats )
		{
			strcat( szPayloadData, szOverflowChars );

			j = j + strlen( szOverflowChars );

			i++;
		}

		szPayloadData[j] = '\0';
	}
}

// Written by Reed Arvin | reedarvin@gmail.com
