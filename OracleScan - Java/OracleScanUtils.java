// javac OracleScanUtils.java
//
// java -classpath .;ojdbc6.jar OracleScan

import java.io.*;

public class OracleScanUtils
{
	public static boolean isJavaVersion6()
	{
		boolean bIsJavaVersion6;
		String     sJavaVersion;
		double     nJavaVersion;

		bIsJavaVersion6 = false;

		sJavaVersion = System.getProperty( "java.version" );

		sJavaVersion = sJavaVersion.substring( 0, 3 );

		nJavaVersion = Double.parseDouble( sJavaVersion );

		if ( nJavaVersion == 1.6 )
		{
			bIsJavaVersion6 = true;
		}
		else
		{
			System.err.println( "ERROR! OracleScan requires Java version 6 (1.6)." );
			System.err.println();
			System.err.println( "Current Java version: " + System.getProperty( "java.version" ) );
		}

		return bIsJavaVersion6;
	}

	public static boolean isValidTCPPort( int nTCPPort )
	{
		boolean bIsValidTCPPort;

		bIsValidTCPPort = false;

		if ( nTCPPort >= 1 && nTCPPort <= 65535 )
		{
			bIsValidTCPPort = true;
		}
		else
		{
			System.err.println( "ERROR! Invalid TCP port specified." );
		}

		return bIsValidTCPPort;
	}

	public static boolean isValidFile( String sFileName )
	{
		boolean bIsValidFile;
		File           oFile;

		bIsValidFile = false;

		oFile = new File( sFileName );

		if ( oFile.exists() )
		{
			bIsValidFile = true;
		}
		else
		{
			System.err.println( "ERROR! Cannot find input file " + sFileName + "." );
		}

		return bIsValidFile;
	}

	public static boolean isValidJDBCDriver( String sJDBCDriverName )
	{
		boolean bIsValidJDBCDriver;

		bIsValidJDBCDriver = false;

		try
		{
			Class.forName( sJDBCDriverName );

			bIsValidJDBCDriver = true;
		}
		catch ( ClassNotFoundException e )
		{
			System.err.println( "ERROR! Cannot find JDBC driver " + sJDBCDriverName + "." );
		}

		return bIsValidJDBCDriver;
	}

	public static String chomp( String sString )
	{
		int nIndex;

		nIndex = sString.indexOf( '\r' );

		if ( nIndex != -1 )
		{
			sString = sString.substring( 0, nIndex );
		}

		nIndex = sString.indexOf( '\n' );

		if ( nIndex != -1 )
		{
			sString = sString.substring( 0, nIndex );
		}

		return sString;
	}
}

// Written by Reed Arvin | reedarvin@gmail.com
