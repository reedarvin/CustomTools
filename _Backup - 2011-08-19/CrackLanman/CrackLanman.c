//
// gcc source.c -o source.exe
//

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <process.h>
#include "lmhash.h"

VOID              KeyboardEventMonitor( VOID *pParameter );
struct Dictionary  *LoadPasswordHashes( FILE *pInputFile, struct Dictionary *pDict );
struct Dictionary   *AddDictionaryItem( struct Dictionary *pDict, CHAR *szUsername, CHAR *szRID, CHAR *szLanman1, CHAR *szLanman2 );
struct Dictionary   *ReverseDictionary( struct Dictionary *pDict );
VOID                    GenerateLMHash( CHAR *szPassword, DWORD *a, DWORD *b, DWORD *c, DWORD *d, DWORD *e, DWORD *f, DWORD *g, struct Dictionary *pDictStart, struct Dictionary *pDict );
VOID                        BruteForce( CHAR *szCharset, DWORD a1, DWORD b1, DWORD c1, DWORD d1, DWORD e1, DWORD f1, DWORD g1, struct Dictionary *pDictStart, struct Dictionary *pDict );
VOID               FreeDictionaryItems( struct Dictionary *pDict );

struct Dictionary
{
	CHAR   Username[ 128 ];
	DWORD       RID;
	CHAR    Lanman1[ 64 ];
	BOOL   Cracked1;
	CHAR  Password1[ 64 ];
	CHAR    Lanman2[ 64 ];
	BOOL   Cracked2;
	CHAR  Password2[ 64 ];
	BOOL      Shown;
	VOID      *Next;
};

BOOL bShowStatus = FALSE;
BOOL bVerbose    = FALSE;

INT main( INT argc, CHAR *argv[] )
{
	CHAR    szCharsetOpt[ 128 ];
	CHAR     szInputFile[ 128 ];
	CHAR       szCharset[ 128 ];
	FILE     *pInputFile;
	DWORD         dwNull;
	CHAR      szUsername[ 128 ];
	DWORD              i;
	DWORD              j;
	CHAR  szTempUsername[ 128 ];
	FILE      *pDictFile;
	CHAR      szPassword[ 128 ];
	CHAR  szTempPassword[ 128 ];

	struct Dictionary      *pDict;
	struct Dictionary *pDictStart;
	struct Dictionary  *pTempDict;

	if ( argc == 3 )
	{
		strcpy( szCharsetOpt, argv[1] );
		strcpy( szInputFile,  argv[2] );

		if ( strcmp( szCharsetOpt, "-1" ) == 0 || strcmp( szCharsetOpt, "-2" ) == 0 || strcmp( szCharsetOpt, "-3" ) == 0 )
		{
			if ( strcmp( szCharsetOpt, "-1" ) == 0 )
			{
				strcpy( szCharset, "ABCDEFGHIJKLMNOPQRSTUVWXYZ" );
			}

			if ( strcmp( szCharsetOpt, "-2" ) == 0 )
			{
				strcpy( szCharset, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789" );
			}

			if ( strcmp( szCharsetOpt, "-3" ) == 0 )
			{
				strcpy( szCharset, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()" );
			}

			pInputFile = fopen( szInputFile, "r" );

			if ( pInputFile != NULL )
			{
				_beginthread( KeyboardEventMonitor, 0, NULL );

				pDict = NULL;

				pDict = LoadPasswordHashes( pInputFile, pDict );

				fclose( pInputFile );

				pDict = ReverseDictionary( pDict );

				pDictStart = pDict;
				pTempDict  = pDict;

				dwNull = -1;

				while ( pTempDict != NULL )
				{
					strcpy( szUsername, pTempDict->Username );

					strupr( szUsername );

					if ( strlen( szUsername ) > 7 )
					{
						i = 0;

						for ( j = 0; j < 7; j++ )
						{
							szTempUsername[i] = szUsername[j];

							i++;
						}

						szTempUsername[i] = '\0';

						GenerateLMHash( szTempUsername, &dwNull, &dwNull, &dwNull, &dwNull, &dwNull, &dwNull, &dwNull, pDictStart, pDict );

						i = 0;

						for ( j = 7; j < strlen( szUsername ); j++ )
						{
							szTempUsername[i] = szUsername[j];

							i++;
						}

						szTempUsername[i] = '\0';

						GenerateLMHash( szTempUsername, &dwNull, &dwNull, &dwNull, &dwNull, &dwNull, &dwNull, &dwNull, pDictStart, pDict );
					}
					else
					{
						GenerateLMHash( szUsername, &dwNull, &dwNull, &dwNull, &dwNull, &dwNull, &dwNull, &dwNull, pDictStart, pDict );
					}

					pTempDict = pTempDict->Next;
				}

				pTempDict = pDict;

				pDictFile = fopen( "dict.txt", "r" );

				if ( pDictFile != NULL )
				{
					while ( fscanf( pInputFile, "%s", szPassword ) != EOF )
					{
						strupr( szPassword );

						if ( strlen( szPassword ) > 7 )
						{
							i = 0;

							for ( j = 0; j < 7; j++ )
							{
								szTempPassword[i] = szPassword[j];

								i++;
							}

							szTempPassword[i] = '\0';

							GenerateLMHash( szTempPassword, &dwNull, &dwNull, &dwNull, &dwNull, &dwNull, &dwNull, &dwNull, pDictStart, pDict );

							i = 0;

							for ( j = 7; j < strlen( szPassword ); j++ )
							{
								szTempPassword[i] = szPassword[j];

								i++;
							}

							szTempPassword[i] = '\0';

							GenerateLMHash( szTempPassword, &dwNull, &dwNull, &dwNull, &dwNull, &dwNull, &dwNull, &dwNull, pDictStart, pDict );
						}
						else
						{
							GenerateLMHash( szPassword, &dwNull, &dwNull, &dwNull, &dwNull, &dwNull, &dwNull, &dwNull, pDictStart, pDict );
						}
					}

					fclose( pDictFile );
				}

				BruteForce( szCharset, -1, -1, -1, -1, -1, -1, -1, pDictStart, pDict );

				FreeDictionaryItems( pDict );
			}
			else
			{
				fprintf( stderr, "ERROR! Could not open file \"%s\".\n", szInputFile );
			}
		}
		else
		{
			printf( "CrackLanman v1.0 | Canaudit, Inc. http://www.canaudit.com/\n" );
			printf( "\n" );
			printf( "Usage: CrackLanman <-1 | -2> <hash file>\n" );
			printf( "\n" );
			printf( "<-1 | -2>    -- required argument\n" );
			printf( "<hash file>  -- required argument\n" );
			printf( "\n" );
			printf( "-1  -- ABCDEFGHIJKLMNOPQRSTUVWXYZ\n" );
			printf( "-2  -- ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\n" );
			printf( "-3  -- ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()\n" );
			printf( "\n" );
			printf( "Examples:\n" );
			printf( "CrackLanman -1 passwordhashes.txt\n" );
			printf( "CrackLanman -2 passwordhashes.txt\n" );
			printf( "CrackLanman -3 passwordhashes.txt\n" );
			printf( "\n" );
			printf( "(Written by Reed Arvin | reed@canaudit.com)\n" );
		}
	}
	else
	{
		printf( "CrackLanman v1.0 | Canaudit, Inc. http://www.canaudit.com/\n" );
		printf( "\n" );
		printf( "Usage: CrackLanman <-1 | -2> <hash file>\n" );
		printf( "\n" );
		printf( "<-1 | -2>    -- required argument\n" );
		printf( "<hash file>  -- required argument\n" );
		printf( "\n" );
		printf( "-1  -- ABCDEFGHIJKLMNOPQRSTUVWXYZ\n" );
		printf( "-2  -- ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\n" );
		printf( "-3  -- ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()\n" );
		printf( "\n" );
		printf( "Examples:\n" );
		printf( "CrackLanman -1 passwordhashes.txt\n" );
		printf( "CrackLanman -2 passwordhashes.txt\n" );
		printf( "CrackLanman -3 passwordhashes.txt\n" );
		printf( "\n" );
		printf( "(Written by Reed Arvin | reed@canaudit.com)\n" );
	}

	return 0;
}

VOID KeyboardEventMonitor( VOID *pParameter )
{
	HANDLE              hStdin;
	INPUT_RECORD      irBuffer[ 128 ];
	DWORD            dwNumRead;
	DWORD                    i;
	KEY_EVENT_RECORD       ker;

	hStdin = GetStdHandle( STD_INPUT_HANDLE );

	if ( hStdin != INVALID_HANDLE_VALUE )
	{
		while ( 1 )
		{
			if ( ReadConsoleInput( hStdin, irBuffer, 128, &dwNumRead ) )
			{
				for ( i = 0; i < dwNumRead; i++ )
				{
					if ( irBuffer[i].EventType == KEY_EVENT )
					{
						ker = irBuffer[i].Event.KeyEvent;

						if ( ker.bKeyDown )
						{
							if ( ker.wVirtualKeyCode == 86 )
							{
								if ( bVerbose == FALSE )
								{
									printf( "\nVerbose mode ON\n\n" );

									bVerbose = TRUE;
								}
								else
								{
									printf( "\nVerbose mode OFF\n\n" );

									bVerbose = FALSE;
								}
							}

							if ( ker.wVirtualKeyCode == VK_SPACE )
							{
								bShowStatus = TRUE;
							}
						}
					}
				}
			}

			Sleep( 10 );
		}
	}
}

struct Dictionary *LoadPasswordHashes( FILE *pInputFile, struct Dictionary *pDict )
{
	CHAR    szReadLine[ 512 ];
	CHAR    szUsername[ 128 ];
	CHAR         szRID[ 64 ];
	CHAR  szFullLanman[ 128 ];
	CHAR     szLanman1[ 64 ];
	CHAR     szLanman2[ 64 ];
	DWORD            i;
	DWORD            j;
	DWORD            k;

	while ( fgets( szReadLine, sizeof( szReadLine ), pInputFile ) != NULL )
	{
		if ( strstr( szReadLine, ":::" ) != NULL )
		{
			strcpy( szUsername,   "" );
			strcpy( szRID,        "" );
			strcpy( szFullLanman, "" );
			strcpy( szLanman1,    "" );
			strcpy( szLanman2,    "" );
						
			i = 0;
			j = 0;

			for ( k = 0; k < strlen( szReadLine ); k++ )
			{
				if ( szReadLine[k] != ':' )
				{
					if ( i == 0 )
					{
						szUsername[j] = szReadLine[k];

						j++;
					}

					if ( i == 1 )
					{
						szRID[j] = szReadLine[k];

						j++;
					}

					if ( i == 2 )
					{
						szFullLanman[j] = szReadLine[k];

						j++;
					}
				}
				else
				{
					if ( i == 0 )
					{
						szUsername[j] = '\0';
					}

					if ( i == 1 )
					{
						szRID[j] = '\0';
					}

					if ( i == 2 )
					{
						szFullLanman[j] = '\0';
					}

					j = 0;

					i++;
				}
			}

			if ( strlen( szFullLanman ) == 32 )
			{
				i = 0;

				for ( j = 0; j < 16; j++ )
				{
					szLanman1[i] = szFullLanman[j];

					i++;
				}

				szLanman1[16] = '\0';

				i = 0;

				for ( j = 16; j < 32; j++ )
				{
					szLanman2[i] = szFullLanman[j];

					i++;
				}

				szLanman2[16] = '\0';
			}

			if ( strcmp( szUsername, "" ) != 0 && strcmp( szRID, "" ) != 0 && strcmp( szLanman1, "" ) != 0 && strcmp( szLanman1, "NO PASSWORD*****" ) != 0 && strcmp( szLanman2, "" ) != 0 )
			{
				pDict = AddDictionaryItem( pDict, szUsername, szRID, szLanman1, szLanman2 );
			}
		}
	}

	return pDict;
}

struct Dictionary *AddDictionaryItem( struct Dictionary *pDict, CHAR szUsername[], CHAR szRID[], CHAR szLanman1[], CHAR szLanman2[] )
{
	DWORD dwRID;

	struct Dictionary *pNewItem;

	pNewItem = ( struct Dictionary * )malloc( sizeof( struct Dictionary ) );

	dwRID = atoi( szRID );

	strcpy( pNewItem->Username,  szUsername );
	strcpy( pNewItem->Lanman1,   szLanman1 );
	strcpy( pNewItem->Password1, "" );
	strcpy( pNewItem->Lanman2,   szLanman2 );
	strcpy( pNewItem->Password2, "" );

	pNewItem->RID      = dwRID;
	pNewItem->Cracked1 = FALSE;
	pNewItem->Cracked2 = FALSE;
	pNewItem->Shown    = FALSE;

	pNewItem->Next = pDict;

	pDict = pNewItem;

	return pDict;
}

struct Dictionary *ReverseDictionary( struct Dictionary *pDict )
{
	struct Dictionary  *pCurrent;
	struct Dictionary *pPrevious;
	struct Dictionary     *pNext;

	pCurrent = pDict;

	pPrevious = NULL;

	while ( pCurrent != NULL )
	{
		pNext = (struct Dictionary *)pCurrent->Next;

		pCurrent->Next = pPrevious;

		pPrevious = pCurrent;
		pCurrent  = pNext;
	}

	pDict = pPrevious;

	return pDict;
}

VOID GenerateLMHash( CHAR szPassword[], DWORD *a, DWORD *b, DWORD *c, DWORD *d, DWORD *e, DWORD *f, DWORD *g, struct Dictionary *pDictStart, struct Dictionary *pDict )
{
	CHAR          szLMPassword[15];
	DWORD                    i;
	unsigned char          p16[16];
	CHAR              szLMHash[17];

	szLMPassword[0]  = '\x00';
	szLMPassword[1]  = '\x00';
	szLMPassword[2]  = '\x00';
	szLMPassword[3]  = '\x00';
	szLMPassword[4]  = '\x00';
	szLMPassword[5]  = '\x00';
	szLMPassword[6]  = '\x00';
	szLMPassword[7]  = '\x00';
	szLMPassword[8]  = '\x00';
	szLMPassword[9]  = '\x00';
	szLMPassword[10] = '\x00';
	szLMPassword[11] = '\x00';
	szLMPassword[12] = '\x00';
	szLMPassword[13] = '\x00';
	szLMPassword[14] = '\x00';
	szLMPassword[15] = '\x00';

	i = 0;

	while ( szPassword[i] != '\0' )
	{
		szLMPassword[i] = szPassword[i];

		i++;

		if ( szPassword[i] == '\0' )
		{
			break;
		}
	}

	E_P16( (const unsigned char *)szLMPassword, p16 );

	sprintf( szLMHash, "%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X", p16[0], p16[1], p16[2], p16[3], p16[4], p16[5], p16[6], p16[7] );

	while ( pDict != NULL )
	{
		if ( strcmp( szLMHash, pDict->Lanman1 ) == 0 )
		{
			pDict->Cracked1 = TRUE;

			strcpy( pDict->Password1, szPassword );

			if ( bVerbose )
			{
				printf( "%-25s (1): %s\n", pDict->Username, pDict->Password1 );
			}
		}

		if ( strcmp( szLMHash, pDict->Lanman2 ) == 0 )
		{
			pDict->Cracked2 = TRUE;

			strcpy( pDict->Password2, szPassword );

			if ( bVerbose )
			{
				printf( "%-25s (2): %s\n", pDict->Username, pDict->Password2 );
			}
		}

		if ( pDict->Cracked1 && pDict->Cracked2 && !pDict->Shown )
		{
			pDict->Shown = TRUE;

			printf( "%-25s (RID %6d), LANMAN password is %s%s\n", pDict->Username, pDict->RID, pDict->Password1, pDict->Password2 );
		}

		pDict = pDict->Next;
	}

	pDict = pDictStart;

	if ( bShowStatus )
	{
		printf( "%s %7s Sequence #: %2d %2d %2d %2d %2d %2d %2d\n", szLMHash, szPassword, *a, *b, *c, *d, *e, *f, *g );

		bShowStatus = FALSE;
	}
}

VOID BruteForce( CHAR szCharset[], DWORD a1, DWORD b1, DWORD c1, DWORD d1, DWORD e1, DWORD f1, DWORD g1, struct Dictionary *pDictStart, struct Dictionary *pDict )
{
	DWORD dwCharsetLen;
	DWORD            a;
	DWORD            b;
	DWORD            c;
	DWORD            d;
	DWORD            e;
	DWORD            f;
	DWORD            g;
	CHAR    szPassword[ 8 ];
	DWORD       dwNull;

	dwCharsetLen = strlen( szCharset );

	a = 0;
	b = 0;
	c = 0;
	d = 0;
	e = 0;
	f = 0;
	g = 0;

	dwNull = -1;

	if ( a1 == -1 )
	{
		a1 = 0;
	}

	if ( b1 == -1 )
	{
		for ( a = a1; a < dwCharsetLen; a++ )
		{
			sprintf( szPassword, "%c", szCharset[a] );

			GenerateLMHash( szPassword, &a, &dwNull, &dwNull, &dwNull, &dwNull, &dwNull, &dwNull, pDictStart, pDict );
		}

		a1 = 0;
		b1 = 0;
	}

	if ( c1 == -1 )
	{
		for ( a = a1; a < dwCharsetLen; a++ )
		{
			for ( b = b1; b < dwCharsetLen; b++ )
			{
				sprintf( szPassword, "%c%c", szCharset[a], szCharset[b] );

				GenerateLMHash( szPassword, &a, &b, &dwNull, &dwNull, &dwNull, &dwNull, &dwNull, pDictStart, pDict );
			}

			b1 = 0;
		}

		a1 = 0;
		c1 = 0;
	}

	if ( d1 == -1 )
	{
		for ( a = a1; a < dwCharsetLen; a++ )
		{
			for ( b = b1; b < dwCharsetLen; b++ )
			{
				for ( c = c1; c < dwCharsetLen; c++ )
				{
					sprintf( szPassword, "%c%c%c", szCharset[a], szCharset[b], szCharset[c] );

					GenerateLMHash( szPassword, &a, &b, &c, &dwNull, &dwNull, &dwNull, &dwNull, pDictStart, pDict );
				}

				c1 = 0;
			}

			b1 = 0;
		}

		a1 = 0;
		d1 = 0;
	}

	if ( e1 == -1 )
	{
		for ( a = a1; a < dwCharsetLen; a++ )
		{
			for ( b = b1; b < dwCharsetLen; b++ )
			{
				for ( c = c1; c < dwCharsetLen; c++ )
				{
					for ( d = d1; d < dwCharsetLen; d++ )
					{
						sprintf( szPassword, "%c%c%c%c", szCharset[a], szCharset[b], szCharset[c], szCharset[d] );

						GenerateLMHash( szPassword, &a, &b, &c, &d, &dwNull, &dwNull, &dwNull, pDictStart, pDict );
					}

					d1 = 0;
				}

				c1 = 0;
			}

			b1 = 0;
		}

		a1 = 0;
		e1 = 0;
	}

	if ( f1 == -1 )
	{
		for ( a = a1; a < dwCharsetLen; a++ )
		{
			for ( b = b1; b < dwCharsetLen; b++ )
			{
				for ( c = c1; c < dwCharsetLen; c++ )
				{
					for ( d = d1; d < dwCharsetLen; d++ )
					{
						for ( e = e1; e < dwCharsetLen; e++ )
						{
							sprintf( szPassword, "%c%c%c%c%c", szCharset[a], szCharset[b], szCharset[c], szCharset[d], szCharset[e] );

							GenerateLMHash( szPassword, &a, &b, &c, &d, &e, &dwNull, &dwNull, pDictStart, pDict );
						}

						e1 = 0;
					}

					d1 = 0;
				}

				c1 = 0;
			}

			b1 = 0;
		}

		a1 = 0;
		f1 = 0;
	}

	if ( g1 == -1 )
	{
		for ( a = a1; a < dwCharsetLen; a++ )
		{
			for ( b = b1; b < dwCharsetLen; b++ )
			{
				for ( c = c1; c < dwCharsetLen; c++ )
				{
					for ( d = d1; d < dwCharsetLen; d++ )
					{
						for ( e = e1; e < dwCharsetLen; e++ )
						{
							for ( f = f1; f < dwCharsetLen; f++ )
							{
								sprintf( szPassword, "%c%c%c%c%c%c", szCharset[a], szCharset[b], szCharset[c], szCharset[d], szCharset[e], szCharset[f] );

								GenerateLMHash( szPassword, &a, &b, &c, &d, &e, &f, &dwNull, pDictStart, pDict );
							}

							f1 = 0;
						}

						e1 = 0;
					}

					d1 = 0;
				}

				c1 = 0;
			}

			b1 = 0;
		}

		a1 = 0;
		g1 = 0;
	}

	for ( a = a1; a < dwCharsetLen; a++ )
	{
		for ( b = b1; b < dwCharsetLen; b++ )
		{
			for ( c = c1; c < dwCharsetLen; c++ )
			{
				for ( d = d1; d < dwCharsetLen; d++ )
				{
					for ( e = e1; e < dwCharsetLen; e++ )
					{
						for ( f = f1; f < dwCharsetLen; f++ )
						{
							for ( g = g1; g < dwCharsetLen; g++ )
							{
								sprintf( szPassword, "%c%c%c%c%c%c%c", szCharset[a], szCharset[b], szCharset[c], szCharset[d], szCharset[e], szCharset[f], szCharset[g] );

								GenerateLMHash( szPassword, &a, &b, &c, &d, &e, &f, &g, pDictStart, pDict );
							}

							g1 = 0;
						}

						f1 = 0;
					}

					e1 = 0;
				}

				d1 = 0;
			}

			c1 = 0;
		}

		b1 = 0;
	}
}

VOID FreeDictionaryItems( struct Dictionary *pDict )
{
	struct Dictionary *pRemove;
	struct Dictionary *pCurrent;

	pCurrent = pDict;

	while ( pCurrent )
	{
		pRemove = pCurrent;

		free( pRemove );

		pCurrent = pCurrent->Next;
	}
}

// Written by Reed Arvin | reed@canaudit.com
