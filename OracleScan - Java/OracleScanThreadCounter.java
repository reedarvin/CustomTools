// javac OracleScanThreadCounter.java
//
// java -classpath .;ojdbc6.jar OracleScan

public class OracleScanThreadCounter
{
	private static int nThreads = 0;

	public static synchronized void incrementCounter()
	{
		nThreads = nThreads + 1;
	}

	public static synchronized void decrementCounter()
	{
		nThreads = nThreads - 1;
	}

	public static synchronized int getCount()
	{
		return nThreads;
	}
}

// Written by Reed Arvin | reedarvin@gmail.com
