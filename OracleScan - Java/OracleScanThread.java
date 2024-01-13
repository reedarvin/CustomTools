// javac OracleScanThread.java
//
// java -classpath .;ojdbc6.jar OracleScan

import java.sql.*;

public class OracleScanThread extends Thread
{
	private String       m_sTarget;
	private int         m_nTCPPort;
	private String m_sDatabaseName;
	private String     m_sUsername;
	private String     m_sPassword;

	public OracleScanThread()
	{

	}

	public void setTarget( String sTarget )
	{
		m_sTarget = sTarget;
	}

	public void setTCPPort( int nTCPPort )
	{
		m_nTCPPort = nTCPPort;
	}

	public void setDatabase( String sDatabaseName )
	{
		m_sDatabaseName = sDatabaseName;
	}

	public void setUsername( String sUsername )
	{
		m_sUsername = sUsername;
	}

	public void setPassword( String sPassword )
	{
		m_sPassword = sPassword;
	}

	public void run()
	{
		oracleConnect( m_sTarget, m_nTCPPort, m_sDatabaseName, m_sUsername, m_sPassword );

		OracleScanThreadCounter.decrementCounter();
	}

	private static void oracleConnect( String sTarget, int nTCPPort, String sDatabaseName, String sUsername, String sPassword )
	{
		String      sURL;
		Connection oConn;

		sURL = "jdbc:oracle:thin:@" + sTarget + ":" + Integer.toString( nTCPPort ) + ":" + sDatabaseName;

		System.out.println( "Trying " + sUsername + "/" + sPassword + " on database " + sDatabaseName + " at " + sTarget + ":" + Integer.toString( nTCPPort ) + "..." );

		try
		{
			oConn = DriverManager.getConnection( sURL, sUsername, sPassword );

			if ( !oConn.isClosed() )
			{
				System.out.println();
				System.out.println( "PASSWORD GUESSED! " + sUsername + "/" + sPassword + " on database " + sDatabaseName + " at " + sTarget + ":" + Integer.toString( nTCPPort ) );

				if ( runPrivilegedQuery( oConn, OracleScanINI.getPrivilegedQuery(), sTarget, nTCPPort, sDatabaseName, sUsername, sPassword ) )
				{
					OracleScanLogger.logGuessedPassword( sTarget, nTCPPort, sDatabaseName, sUsername, sPassword, true );
				}
				else
				{
					OracleScanLogger.logGuessedPassword( sTarget, nTCPPort, sDatabaseName, sUsername, sPassword, false );
				}

//				runUserListQuery( oConn, OracleScanINI.getUserListQuery(), sTarget, nTCPPort, sDatabaseName, sUsername, sPassword );

				runUserInfoQuery( oConn, OracleScanINI.getUserInfoQuery(), sTarget, nTCPPort, sDatabaseName, sUsername, sPassword );

				oConn.close();
			}
		}
		catch ( SQLException e1 )
		{
			OracleScanLogger.logSQLConnectionErrors( sTarget, nTCPPort, sDatabaseName, sUsername, sPassword, e1.getMessage() );
		}
	}

	private static synchronized boolean runPrivilegedQuery( Connection oConn, String sQuery, String sTarget, int nTCPPort, String sDatabaseName, String sUsername, String sPassword )
	{
		boolean           bQuerySuccess;
		String             sLogFileName;
		Statement                 oStmt;
		ResultSet                   oRS;
		ResultSetMetaData         oRSMD;
		int                nColumnCount;
		int                           i;

		bQuerySuccess = false;

		sLogFileName = sTarget + "-" + Integer.toString( nTCPPort ) + "-" + sDatabaseName + "-OraclePrivilegedQuery.txt";

		try
		{
			oStmt = oConn.createStatement();
			oRS   = oStmt.executeQuery( sQuery );
			oRSMD = oRS.getMetaData();

			nColumnCount = oRSMD.getColumnCount();

			for ( i = 1; i <= nColumnCount; i++ )
			{
				if ( i < nColumnCount )
				{
					OracleScanLogger.logQueryResults( sLogFileName, sQuery, oRSMD.getColumnName(i) + "\t" );
				}
				else
				{
					OracleScanLogger.logQueryResults( sLogFileName, sQuery, oRSMD.getColumnName(i) + "\r\n" );
				}
			}

			while ( oRS.next() )
			{
				bQuerySuccess = true;

				for ( i = 1; i <= nColumnCount; i++ )
				{
					if ( i < nColumnCount )
					{
						OracleScanLogger.logQueryResults( sLogFileName, sQuery, oRS.getString(i) + "\t" );
					}
					else
					{
						OracleScanLogger.logQueryResults( sLogFileName, sQuery, oRS.getString(i) + "\r\n" );
					}
				}
			}

			oRS.close();
			oStmt.close();
		}
		catch( SQLException e )
		{
			OracleScanLogger.logSQLQueryErrors( sTarget, nTCPPort, sDatabaseName, sUsername, sPassword, sQuery, e.getMessage() );
		}

		return bQuerySuccess;
	}

	private static synchronized void runUserInfoQuery( Connection oConn, String sQuery, String sTarget, int nTCPPort, String sDatabaseName, String sUsername, String sPassword )
	{
		String            sLogFileName;
		Statement                oStmt;
		ResultSet                  oRS;
		ResultSetMetaData        oRSMD;
		int               nColumnCount;
		int                          i;

		sLogFileName = sTarget + "-" + Integer.toString( nTCPPort ) + "-" + sDatabaseName + "-OracleUserInfoQuery.txt";

		try
		{
			oStmt = oConn.createStatement();
			oRS   = oStmt.executeQuery( sQuery );
			oRSMD = oRS.getMetaData();

			nColumnCount = oRSMD.getColumnCount();

			for ( i = 1; i <= nColumnCount; i++ )
			{
				if ( i < nColumnCount )
				{
					OracleScanLogger.logQueryResults( sLogFileName, sQuery, oRSMD.getColumnName(i) + "\t" );
				}
				else
				{
					OracleScanLogger.logQueryResults( sLogFileName, sQuery, oRSMD.getColumnName(i) + "\r\n" );
				}
			}

			while ( oRS.next() )
			{
				for ( i = 1; i <= nColumnCount; i++ )
				{
					if ( i < nColumnCount )
					{
						OracleScanLogger.logQueryResults( sLogFileName, sQuery, oRS.getString(i) + "\t" );
					}
					else
					{
						OracleScanLogger.logQueryResults( sLogFileName, sQuery, oRS.getString(i) + "\r\n" );
					}
				}
			}

			oRS.close();
			oStmt.close();
		}
		catch( SQLException e )
		{
			OracleScanLogger.logSQLQueryErrors( sTarget, nTCPPort, sDatabaseName, sUsername, sPassword, sQuery, e.getMessage() );
		}
	}
}

// Written by Reed Arvin | reedarvin@gmail.com
