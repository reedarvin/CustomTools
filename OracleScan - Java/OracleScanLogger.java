// javac OracleScanLogger.java
//
// java -classpath .;ojdbc6.jar OracleScan

import java.io.*;

public class OracleScanLogger
{
	public static synchronized void logSQLConnectionErrors( String sTarget, int nTCPPort, String sDatabaseName, String sUsername, String sPassword, String sErrorMsg )
	{
		File               oFile;
		FileWriter   oFileWriter;
		PrintWriter oPrintWriter;

		sErrorMsg = OracleScanUtils.chomp( sErrorMsg );

		oFile = new File( "SQLConnectionErrors.txt" );

		if ( !oFile.exists() )
		{
			try
			{
				oFile.createNewFile();

				if ( oFile.exists() && oFile.canWrite() )
				{
					try
					{
						oFileWriter = new FileWriter( oFile, true );

						oPrintWriter = new PrintWriter( oFileWriter );

						oPrintWriter.println( "NOTE: This file is tab separated. Open with Excel to view and sort information." );
						oPrintWriter.println();
						oPrintWriter.println( "Hostname\tTCP Port\tDatabase Name\tUsername\tPasswordError Message" );

						oFileWriter.close();
					}
					catch ( IOException e2 )
					{
						System.err.println( "ERROR! " + e2.getMessage() );
					}
				}
			}
			catch ( IOException e1 )
			{
				System.err.println( "ERROR! " + e1.getMessage() );
			}
		}

		if ( oFile.exists() && oFile.canWrite() )
		{
			try
			{
				oFileWriter = new FileWriter( oFile, true );

				oPrintWriter = new PrintWriter( oFileWriter );

				oPrintWriter.println( sTarget + "\t" + Integer.toString( nTCPPort ) + "\t" + sDatabaseName + "\t" + sUsername + "\t" + sPassword + "\t" + sErrorMsg );

				oFileWriter.close();
			}
			catch ( IOException e1 )
			{
				System.err.println( "ERROR! " + e1.getMessage() );
			}
		}
	}

	public static void logSQLQueryErrors( String sTarget, int nTCPPort, String sDatabaseName, String sUsername, String sPassword, String sQuery, String sErrorMsg )
	{
		File               oFile;
		FileWriter   oFileWriter;
		PrintWriter oPrintWriter;

		sErrorMsg = OracleScanUtils.chomp( sErrorMsg );

		oFile = new File( "SQLQueryErrors.txt" );

		if ( !oFile.exists() )
		{
			try
			{
				oFile.createNewFile();

				if ( oFile.exists() && oFile.canWrite() )
				{
					try
					{
						oFileWriter = new FileWriter( oFile, true );

						oPrintWriter = new PrintWriter( oFileWriter );

						oPrintWriter.println( "NOTE: This file is tab separated. Open with Excel to view and sort information." );
						oPrintWriter.println();
						oPrintWriter.println( "Hostname\tTCP Port\tDatabase Name\tUsername\tPassword\tSQL Query\tError Message" );

						oFileWriter.close();
					}
					catch ( IOException e2 )
					{
						System.err.println( "ERROR! " + e2.getMessage() );
					}
				}
			}
			catch ( IOException e1 )
			{
				System.err.println( "ERROR! " + e1.getMessage() );
			}
		}

		if ( oFile.exists() && oFile.canWrite() )
		{
			try
			{
				oFileWriter = new FileWriter( oFile, true );

				oPrintWriter = new PrintWriter( oFileWriter );

				oPrintWriter.println( sTarget + "\t" + Integer.toString( nTCPPort ) + "\t" + sDatabaseName + "\t" + sUsername + "\t" + sPassword + "\t" + sQuery + "\t" + sErrorMsg );

				oFileWriter.close();
			}
			catch ( IOException e1 )
			{
				System.err.println( "ERROR! " + e1.getMessage() );
			}
		}
	}

	public static void logGuessedPassword( String sTarget, int nTCPPort, String sDatabaseName, String sUsername, String sPassword, boolean bIsPrivilegedAccount )
	{
		File               oFile;
		FileWriter   oFileWriter;
		PrintWriter oPrintWriter;

		oFile = new File( "GuessedOracleDatabaseAccountPasswords.txt" );

		if ( !oFile.exists() )
		{
			try
			{
				oFile.createNewFile();

				if ( oFile.exists() && oFile.canWrite() )
				{
					try
					{
						oFileWriter = new FileWriter( oFile, true );

						oPrintWriter = new PrintWriter( oFileWriter );

						oPrintWriter.println( "NOTE: This file is tab separated. Open with Excel to view and sort information." );
						oPrintWriter.println();
						oPrintWriter.println( "Hostname\tTCP Port\tDatabase Name\tUsername\tPassword\tIs Privileged Account?" );

						oFileWriter.close();
					}
					catch ( IOException e2 )
					{
						System.err.println( "ERROR! " + e2.getMessage() );
					}
				}
			}
			catch ( IOException e1 )
			{
				System.err.println( "ERROR! " + e1.getMessage() );
			}
		}

		if ( oFile.exists() && oFile.canWrite() )
		{
			try
			{
				oFileWriter = new FileWriter( oFile, true );

				oPrintWriter = new PrintWriter( oFileWriter );

				if ( bIsPrivilegedAccount )
				{
					oPrintWriter.println( sTarget + "\t" + Integer.toString( nTCPPort ) + "\t" + sDatabaseName + "\t" + sUsername + "\t" + sPassword + "\tYes" );
				}
				else
				{
					oPrintWriter.println( sTarget + "\t" + Integer.toString( nTCPPort ) + "\t" + sDatabaseName + "\t" + sUsername + "\t" + sPassword + "\tNo" );
				}

				oFileWriter.close();
			}
			catch ( IOException e1 )
			{
				System.err.println( "ERROR! " + e1.getMessage() );
			}
		}
	}

	public static void deleteFile( String sFileName )
	{
		File oFile;

		oFile = new File( sFileName );

		if ( oFile.exists() )
		{
			try
			{
				oFile.delete();
			}
			catch ( SecurityException e )
			{
				System.err.println( "ERROR! " + e.getMessage() );
			}
		}
	}

	public static void logQueryResults( String sFileName, String sQuery, String sQueryResults )
	{
		File               oFile;
		FileWriter   oFileWriter;
		PrintWriter oPrintWriter;

		oFile = new File( sFileName );

		if ( !oFile.exists() )
		{
			try
			{
				oFile.createNewFile();

				if ( oFile.exists() && oFile.canWrite() )
				{
					try
					{
						oFileWriter = new FileWriter( oFile, true );

						oPrintWriter = new PrintWriter( oFileWriter );

						oPrintWriter.println( "NOTE: This file is tab separated. Open with Excel to view and sort information." );
						oPrintWriter.println();
						oPrintWriter.println( sQuery );
						oPrintWriter.println();

						oFileWriter.close();
					}
					catch ( IOException e2 )
					{
						System.err.println( "ERROR! " + e2.getMessage() );
					}
				}
			}
			catch ( IOException e1 )
			{
				System.err.println( "ERROR! " + e1.getMessage() );
			}
		}

		if ( oFile.exists() && oFile.canWrite() )
		{
			try
			{
				oFileWriter = new FileWriter( oFile, true );

				oPrintWriter = new PrintWriter( oFileWriter );

				oPrintWriter.print( sQueryResults );

				oFileWriter.close();
			}
			catch ( IOException e1 )
			{
				System.err.println( "ERROR! " + e1.getMessage() );
			}
		}
	}
}

// Written by Reed Arvin | reedarvin@gmail.com
