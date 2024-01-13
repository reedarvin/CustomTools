//
// gcc source.c -o source.exe
//

#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "lmhash.h"

VOID GenerateLMHash( CHAR *szPassword, INT a, INT b, INT c, INT d, INT e, INT f, INT g );

INT main( INT argc, CHAR *argv[] )
{
	CHAR  szCharset[ 37 ];
	INT           a;
	INT           b;
	INT           c;
	INT           d;
	INT           e;
	INT           f;
	INT           g;
	INT          a1;
	INT          b1;
	INT          c1;
	INT          d1;
	INT          e1;
	INT          f1;
	INT          g1;
	CHAR szPassword[ 8 ];

	strcpy( szCharset, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789" );

	a = 0;
	b = 0;
	c = 0;
	d = 0;
	e = 0;
	f = 0;
	g = 0;

	a1 = -1;
	b1 = -1;
	c1 = -1;
	d1 = -1;
	e1 = -1;
	f1 = -1;
	g1 = -1;

	if ( argc > 1 )
	{
		a1 = atoi( argv[1] );
	}

	if ( argc > 2 )
	{
		b1 = atoi( argv[2] );
	}

	if ( argc > 3 )
	{
		c1 = atoi( argv[3] );
	}

	if ( argc > 4 )
	{
		d1 = atoi( argv[4] );
	}

	if ( argc > 5 )
	{
		e1 = atoi( argv[5] );
	}

	if ( argc > 6 )
	{
		f1 = atoi( argv[6] );
	}

	if ( argc > 7 )
	{
		g1 = atoi( argv[7] );
	}

	if ( a1 == -1 )
	{
		a1 = 0;
	}

	if ( b1 == -1 )
	{
		for ( a = a1; a < 36; a++ )
		{
			sprintf( szPassword, "%c", szCharset[a] );

			GenerateLMHash( szPassword, a, -1, -1, -1, -1, -1, -1 );
		}

		a1 = 0;
		b1 = 0;
	}

	if ( c1 == -1 )
	{
		for ( a = a1; a < 36; a++ )
		{
			for ( b = b1; b < 36; b++ )
			{
				sprintf( szPassword, "%c%c", szCharset[a], szCharset[b] );

				GenerateLMHash( szPassword, a, b, -1, -1, -1, -1, -1 );
			}

			b1 = 0;
		}

		a1 = 0;
		c1 = 0;
	}

	if ( d1 == -1 )
	{
		for ( a = a1; a < 36; a++ )
		{
			for ( b = b1; b < 36; b++ )
			{
				for ( c = c1; c < 36; c++ )
				{
					sprintf( szPassword, "%c%c%c", szCharset[a], szCharset[b], szCharset[c] );

					GenerateLMHash( szPassword, a, b, c, -1, -1, -1, -1 );
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
		for ( a = a1; a < 36; a++ )
		{
			for ( b = b1; b < 36; b++ )
			{
				for ( c = c1; c < 36; c++ )
				{
					for ( d = d1; d < 36; d++ )
					{
						sprintf( szPassword, "%c%c%c%c", szCharset[a], szCharset[b], szCharset[c], szCharset[d] );

						GenerateLMHash( szPassword, a, b, c, d, -1, -1, -1 );
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
		for ( a = a1; a < 36; a++ )
		{
			for ( b = b1; b < 36; b++ )
			{
				for ( c = c1; c < 36; c++ )
				{
					for ( d = d1; d < 36; d++ )
					{
						for ( e = e1; e < 36; e++ )
						{
							sprintf( szPassword, "%c%c%c%c%c", szCharset[a], szCharset[b], szCharset[c], szCharset[d], szCharset[e] );

							GenerateLMHash( szPassword, a, b, c, d, e, -1, -1 );
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
		for ( a = a1; a < 36; a++ )
		{
			for ( b = b1; b < 36; b++ )
			{
				for ( c = c1; c < 36; c++ )
				{
					for ( d = d1; d < 36; d++ )
					{
						for ( e = e1; e < 36; e++ )
						{
							for ( f = f1; f < 36; f++ )
							{
								sprintf( szPassword, "%c%c%c%c%c%c", szCharset[a], szCharset[b], szCharset[c], szCharset[d], szCharset[e], szCharset[f] );

								GenerateLMHash( szPassword, a, b, c, d, e, f, -1 );
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

	for ( a = a1; a < 36; a++ )
	{
		for ( b = b1; b < 36; b++ )
		{
			for ( c = c1; c < 36; c++ )
			{
				for ( d = d1; d < 36; d++ )
				{
					for ( e = e1; e < 36; e++ )
					{
						for ( f = f1; f < 36; f++ )
						{
							for ( g = g1; g < 36; g++ )
							{
								sprintf( szPassword, "%c%c%c%c%c%c%c", szCharset[a], szCharset[b], szCharset[c], szCharset[d], szCharset[e], szCharset[f], szCharset[g] );

								GenerateLMHash( szPassword, a, b, c, d, e, f, g );
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

	return 0;
}

VOID GenerateLMHash( CHAR szPassword[], INT a, INT b, INT c, INT d, INT e, INT f, INT g )
{
	CHAR          szLMPassword[ 15 ];
	unsigned char          p16[ 16 ];
	CHAR              szLMHash[ 17 ];
	INT                      i;
	FILE          *pOutputFile;

	for ( i = 0; i < 14; i++ )
	{
		memset( szLMPassword, '\x00', i );
	}

	memset( szLMPassword, '\0', sizeof( szLMPassword ) );

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

	strcpy( szLMHash, "" );

	for ( i = 0; i < 8; i++ )
	{
		sprintf( szLMHash, "%s%.2x", szLMHash, p16[i] );
	}

	strupr( szLMHash );

	pOutputFile = fopen( "john.pot", "r" );

	if ( pOutputFile != NULL )
	{
		fclose( pOutputFile );
	}
	else
	{
		pOutputFile = fopen( "john.pot", "w" );

		if ( pOutputFile != NULL )
		{
			fclose( pOutputFile );
		}
	}

	pOutputFile = fopen( "john.pot", "a+" );

	if ( pOutputFile != NULL )
	{
		fprintf( pOutputFile, "$LM$%s:%s\n", szLMHash, szPassword );

		fclose( pOutputFile );
	}

	printf( "%s %7s Sequence #: %2d %2d %2d %2d %2d %2d %2d\n", szLMHash, szPassword, a, b, c, d, e, f, g );
}

// Written by Reed Arvin | reed@canaudit.com
