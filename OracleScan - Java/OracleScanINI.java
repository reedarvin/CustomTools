// javac OracleScanINI.java
//
// java -classpath .;ojdbc6.jar OracleScan

import java.io.*;

public class OracleScanINI
{
	private static boolean        m_bIsParsed;
	private static int             m_nThreads;
	private static String   m_sJDBCDriverName;
	private static String    m_sUserListQuery;
	private static String    m_sUserInfoQuery;
	private static String  m_sPrivilegedQuery;

	public static void parseINI()
	{
		int                nParsed;
		File                 oFile;
		FileReader     oFileReader;
		BufferedReader oBuffReader;
		String               sLine;
		String[]        sLineSplit;

		nParsed = 0;

		oFile = new File( "OracleScan.ini" );

		try
		{
			oFileReader = new FileReader( oFile );

			try
			{
				oBuffReader = new BufferedReader( oFileReader );

				sLine = oBuffReader.readLine();

				while ( sLine != null )
				{
					sLine = sLine.trim();

					if ( sLine.length() > 0 )
					{
						if ( sLine.charAt( 0 ) != '#' )
						{
							if ( sLine.indexOf( ":" ) != -1 )
							{
								sLineSplit = sLine.split( ":" );

								sLineSplit[0] = sLineSplit[0].trim();
								sLineSplit[1] = sLineSplit[1].trim();

								if ( sLineSplit[0].length() > 0 && sLineSplit[1].length() > 0 )
								{
									sLineSplit[0] = sLineSplit[0].toUpperCase();

									if ( sLineSplit[0].compareTo( "MAX_THREADS" ) == 0 )
									{
										try
										{
											m_nThreads = Integer.parseInt( sLineSplit[1] );

											nParsed++;
										}
										catch ( NumberFormatException e3 )
										{
											System.err.println( "ERROR! OracleScan.ini value MAX_THREADS is not a valid number." );
										}
									}

									if ( sLineSplit[0].compareTo( "JDBC_DRIVER_NAME" ) == 0 )
									{
										m_sJDBCDriverName = sLineSplit[1];

										nParsed++;
									}

									if ( sLineSplit[0].compareTo( "USER_LIST_QUERY" ) == 0 )
									{
										m_sUserListQuery = sLineSplit[1];

										nParsed++;
									}

									if ( sLineSplit[0].compareTo( "USER_INFO_QUERY" ) == 0 )
									{
										m_sUserInfoQuery = sLineSplit[1];

										nParsed++;
									}

									if ( sLineSplit[0].compareTo( "PRIVILEGED_QUERY" ) == 0 )
									{
										m_sPrivilegedQuery = sLineSplit[1];

										nParsed++;
									}
								}
							}
						}
					}

					sLine = oBuffReader.readLine();
				}

				oFileReader.close();
			}
			catch ( IOException e2 )
			{
				System.err.println( "ERROR! " + e2.getMessage() );
			}
		}
		catch ( FileNotFoundException e1 )
		{
			System.err.println( "ERROR! " + e1.getMessage() );
		}

		if ( nParsed == 5 )
		{
			m_bIsParsed = true;
		}
		else
		{
			System.err.println( "ERROR! Problems parsing OracleScan.ini file." );
		}
	}

	public static boolean isParsed()
	{
		return m_bIsParsed;
	}

	public static int getMaxThreads()
	{
		return m_nThreads;
	}

	public static String getJDBCDriverName()
	{
		return m_sJDBCDriverName;
	}

	public static String getUserListQuery()
	{
		return m_sUserListQuery;
	}

	public static String getUserInfoQuery()
	{
		return m_sUserInfoQuery;
	}

	public static String getPrivilegedQuery()
	{
		return m_sPrivilegedQuery;
	}
}

// Written by Reed Arvin | reedarvin@gmail.com
