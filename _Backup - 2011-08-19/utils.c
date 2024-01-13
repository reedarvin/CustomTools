//
// gcc source.c -o source.exe
//

#define _WIN32_WINNT 0x0500
#define WINVER       0x0500

#include <windows.h>
#include <string.h>
#include <stdio.h>

BOOL   Split( CHAR *szText, CHAR *szFindText, CHAR *szOutputText1, CHAR *szOutputText2 );
BOOL Replace( CHAR *szText, CHAR *szFindText, CHAR *szReplacementText );
VOID    Trim( CHAR *szText );

INT main( INT argc, CHAR *argv[] )
{
	CHAR            szText[ 128 ];
	CHAR        szFindText[ 128 ];
	CHAR szReplacementText[ 128 ];
	CHAR     szOutputText1[ 128 ];
	CHAR     szOutputText2[ 128 ];

	if ( argc == 4 )
	{
		strcpy( szText,            argv[1] );
		strcpy( szFindText,        argv[2] );
		strcpy( szReplacementText, argv[3] );

/*		if ( Split( szText, szFindText, szOutputText1, szOutputText2 ) )
		{
			printf( "%s\n", szText );
			printf( "%s\n", szFindText );
			printf( "%s\n", szOutputText1 );
			printf( "%s\n", szOutputText2 );
		}
		else
		{
			printf( "String not found.\n" );
		}
*/
/*		if ( Replace( szText, szFindText, szReplacementText ) )
		{
			printf( "%s\n", szText );
			printf( "%s\n", szFindText );
			printf( "%s\n", szReplacementText );
		}
		else
		{
			printf( "String not found.\n" );
		}
*/
/*		Trim( szText );

		printf( "|%s|\n", szText );
*/
	}

	return 0;
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
