require 'Win32/ProcFarm/Child.pl';

&init;

while (1)
{
	&main_loop;
}

use strict;

sub ThreadedSub()
{
	my($strFuzzyFileName, $strFuzzOpt, $strOverflowChars, $intNumRepeats, $intExecMonTimeout, $strExecutable, $strArguments, $intFileNum) = @_;
	my($strActualArguments)                                                                                                               = "";
	my($strException)                                                                                                                     = "";
	my($strExceptionData)                                                                                                                 = "";

	if (open(FUZZYFILES, "FuzzyFiles.exe $strFuzzyFileName $strFuzzOpt $strOverflowChars $intNumRepeats $intFileNum |"))
	{
		while (<FUZZYFILES>)
		{
			# Do nothing
		}

		close(FUZZYFILES);
	}

	if ($strArguments =~ /(.*)\((\S+)\s+x\s+(\S+)\)(.*)/)
	{
		$strActualArguments = "$1" . "$2" x $3 . "$4";
	}
	else
	{
		$strActualArguments = $strArguments;
	}

	print "Trying arguments $strArguments\n";

	chdir "FileCache";

	if (open(EXECMON, "..\\ExecMon.exe $intExecMonTimeout \"$strExecutable\" $strActualArguments |"))
	{
		while (<EXECMON>)
		{
			if (/EIP/)
			{
				$strException = "YES";

				print "#### Exception detected using arguments $strArguments\n";

				$strExceptionData = $strExceptionData . "#### Exception detected ####\n";
				$strExceptionData = $strExceptionData . "\"$strExecutable\" $strActualArguments\n";
				$strExceptionData = $strExceptionData . $_;

			}

			if (/EAX/)
			{
				$strExceptionData = $strExceptionData . $_;
			}

			if (/ESI/)
			{
				$strExceptionData = $strExceptionData . $_;
				$strExceptionData = $strExceptionData . "\n";
			}
		}

		close(EXECMON);
	}

	if ($intFileNum ne "0")
	{
		if (open(DELETE, "del $intFileNum\_$strFuzzyFileName |"))
		{
			while (<DELETE>)
			{
				# Do nothing
			}

			close(DELETE);
		}
	}

	chdir "..";


	if ($strException eq "YES")
	{
		if (open(LOG, ">> Exceptions.txt"))
		{
			print LOG $strExceptionData;

			close(LOG);
		}
	}
}
