// javac OracleScan.java
//
// java -classpath .;ojdbc6.jar OracleScan

import java.io.*;

public class OracleScan
{
	private static int         MAX_THREADS;
	private static String JDBC_DRIVER_NAME;

	public static void main( String args[] )
	{
		File             oFile;
		String         sTarget;
		int           nTCPPort;
		String   sDatabaseName;
		String         sMethod;
		String sDictionaryFile;

		OracleScanINI.parseINI();

		if ( !OracleScanINI.isParsed() )
		{
			return;
		}

		MAX_THREADS      = OracleScanINI.getMaxThreads();
		JDBC_DRIVER_NAME = OracleScanINI.getJDBCDriverName();

		if ( !OracleScanUtils.isJavaVersion6() )
		{
			return;
		}

		oFile = new File( "UserCache" );

		if ( !oFile.exists() )
		{
			oFile.mkdir();
		}

		if ( args.length == 5 )
		{
			sTarget         = args[0];
			sDatabaseName   = args[2];
			sMethod         = args[3];
			sDictionaryFile = args[4];

			try
			{
				nTCPPort = Integer.parseInt( args[1] );
			}
			catch ( NumberFormatException e )
			{
				nTCPPort = 0;
			}

			sTarget       = sTarget.toUpperCase();
			sDatabaseName = sDatabaseName.toUpperCase();
			sMethod       = sMethod.toLowerCase();

			if ( sMethod.compareTo( "-d" ) == 0 || sMethod.compareTo( "-u" ) == 0 )
			{
				if ( OracleScanUtils.isValidTCPPort( nTCPPort ) )
				{
					if ( OracleScanUtils.isValidFile( sDictionaryFile ) )
					{
						if ( OracleScanUtils.isValidJDBCDriver( JDBC_DRIVER_NAME ) )
						{
							if ( sMethod.compareTo( "-d" ) == 0 )
							{
								System.out.println( "Running OracleScan v1.0 with the following arguments:" );
								System.out.println( "[+] Host Input:           \"" + sTarget + "\"" );
								System.out.println( "[+] TCP Port:             \"" + Integer.toString( nTCPPort ) + "\"" );
								System.out.println( "[+] Database Name:        \"" + sDatabaseName + "\"" );
								System.out.println( "[+] Method:               \"Oracle Defaults List\"" );
								System.out.println( "[+] Oracle Defaults List: \"" + sDictionaryFile + "\"" );
								System.out.println();

								guessOracleDefaults( sTarget, nTCPPort, sDatabaseName, sDictionaryFile );
							}

							if ( sMethod.compareTo( "-u" ) == 0 )
							{
								System.out.println( "Running OracleScan v1.0 with the following arguments:\n" );
								System.out.println( "[+] Host Input:        \"" + sTarget + "\"" );
								System.out.println( "[+] TCP Port:          \"" + Integer.toString( nTCPPort ) + "\"" );
								System.out.println( "[+] Database Name:     \"" + sDatabaseName + "\"" );
								System.out.println( "[+] Method:            \"Oracle Users List\"" );
								System.out.println( "[+] Oracle Users List: \"" + sDictionaryFile + "\"" );
								System.out.println();
							}
						}
					}
				}
			}
			else
			{
				showUsage();
			}
		}
		else
		{
			showUsage();
		}
	}

	private static void showUsage()
	{
		System.out.println( "OracleScan-Java v1.0 | http://reedarvin.thearvins.com/" );
		System.out.println();
		System.out.println( "Usage: OracleScan <hostname> <tcp port> <database name> [-d|-u] <dictionary file>" );
		System.out.println();
		System.out.println( "<hostname>         -- required argument" );
		System.out.println( "<tcp port>         -- required argument" );
		System.out.println( "<database name>    -- required argument" );
		System.out.println( "[-d|-u]            -- required argument" );
		System.out.println( "<dictionary file>  -- required argument" );
		System.out.println();
		System.out.println( "Examples:" );
		System.out.println( "OracleScan 10.10.10.10 1521 oracle8idb -d defaults.txt" );
		System.out.println( "OracleScan 10.10.10.10 1521 oracle10gdb -u dict.txt" );
		System.out.println();
		System.out.println( "OracleScan MyOracleMachine 1521 oracle8idb -d defaults.txt" );
		System.out.println( "OracleScan MyOracleMachine 1521 oracle10gdb -u dict.txt" );
		System.out.println();
		System.out.println( "(Written by Reed Arvin | reedarvin@gmail.com)" );
	}

	private static void guessOracleDefaults( String sTarget, int nTCPPort, String sDatabaseName, String sDictionaryFile )
	{
		File                  oFile;
		FileReader      oFileReader;
		BufferedReader  oBuffReader;
		String                sLine;
		String[]         sLineSplit;
		String            sUsername;
		String            sPassword;
		OracleScanThread    oThread;

		oFile = new File( sDictionaryFile );

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

								sUsername = sLineSplit[0].trim();
								sPassword = sLineSplit[1].trim();

								if ( sUsername.length() > 0 )
								{
									while ( OracleScanThreadCounter.getCount() >= MAX_THREADS )
									{
										try
										{
											Thread.sleep( 200 );
										}
										catch ( InterruptedException e3 )
										{
											System.err.println( "ERROR! " + e3.getMessage() );
										}
									}

									oThread = new OracleScanThread();

									oThread.setTarget( sTarget );
									oThread.setTCPPort( nTCPPort );
									oThread.setDatabase( sDatabaseName );
									oThread.setUsername( sUsername );
									oThread.setPassword( sPassword );

									OracleScanThreadCounter.incrementCounter();

									oThread.start();
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

		if ( OracleScanThreadCounter.getCount() > 0 )
		{
			System.out.println( "Waiting for threads to terminate..." );
		}

		while ( OracleScanThreadCounter.getCount() >= MAX_THREADS )
		{
			try
			{
				Thread.sleep( 200 );
			}
			catch ( InterruptedException e1 )
			{
				System.err.println( "ERROR! " + e1.getMessage() );
			}
		}
	}
}

// Written by Reed Arvin | reedarvin@gmail.com
