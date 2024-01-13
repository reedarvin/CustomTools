# Usage: ParseOracleTNSListenerChecks.pl

use strict;

my($strFile)      = "";
my(@strFiles)     = ();
my(@strFileSplit) = ();
my($strTarget)    = "";
my($x)            = "";
my($strService)   = "";
my($strInstance)  = "";
my($strHost)      = "";
my($intPort)      = "";

if (open(DIR, "DIR /B /O:N |"))
{
	while(<DIR>)
	{
		$strFile = $_;

		chomp($strFile);

		if ($strFile ne "")
		{
			push(@strFiles, $strFile);
		}
	}

	close(DIR);
}

foreach $strFile (@strFiles)
{
	if ($strFile =~ /\.txt$/)
	{
		@strFileSplit = split(/\.txt/, $strFile);

		$strTarget = $strFileSplit[0];

		if (open(FILE, "< $strFile"))
		{
			print "Parsing file $strFile...\n";

			$x = 0;

			$strService  = "";
			$strInstance = "";
			$strHost     = "";
			$intPort     = "";

			while(<FILE>)
			{
				if (/Services Summary\.\.\./)
				{
					$x = $x + 1;
				}

				if ($x eq "2")
				{
					if (/^Service \"(\S+)\" has/)
					{
						if ($strService ne "")
						{
							if ($strInstance ne "")
							{
								if ($strHost ne "" && $intPort ne "")
								{
									&AddToLogFiles($strTarget, $strService, $strInstance, $strHost, $intPort);
								}
								else
								{
									&AddToLogFiles($strTarget, $strService, $strInstance, $strTarget, "1521");
								}
							}

							$strService  = "";
							$strInstance = "";
							$strHost     = "";
							$intPort     = "";
						}

						$strService = $1;
					}

					if (/^  Instance \"(\S+)\", status READY/)
					{
						$strInstance = $1;
					}

					if (/\(HOST=(\S+)\)\(PORT=(\d+)\)/)
					{
						$strHost = $1;
						$intPort = $2;
					}

					if (/The command completed successfully/)
					{
						$x = 0;

						if ($strService ne "")
						{
							if ($strInstance ne "")
							{
								if ($strHost ne "" && $intPort ne "")
								{
									&AddToLogFiles($strTarget, $strService, $strInstance, $strHost, $intPort);
								}
								else
								{
									&AddToLogFiles($strTarget, $strService, $strInstance, $strTarget, "1521");
								}
							}

							$strService  = "";
							$strInstance = "";
							$strHost     = "";
							$intPort     = "";
						}
					}
				}
			}

			close(FILE);
		}
	}
}

if (open(DIR, "DIR /B /O:N |"))
{
	while(<DIR>)
	{
		$strFile = $_;

		chomp($strFile);

		if ($strFile ne "")
		{
			push(@strFiles, $strFile);
		}
	}

	close(DIR);
}

foreach $strFile (@strFiles)
{
	if ($strFile =~ /\.databases$/)
	{
		@strFileSplit = split(/\.databases/, $strFile);

		$strTarget  = $strFileSplit[0];
		$strService = $strFileSplit[1];

		if (! -e "OracleScan-Defaults.bat")
		{
			if (open(LOG, "> OracleScan-Defaults.bat"))
			{
				close(LOG);
			}
		}

		if (open(LOG, ">> OracleScan-Defaults.bat"))
		{
			print LOG "OracleScan.exe $strTarget $strTarget.databases -d defaults.txt\n";

			close(LOG);
		}

		if (! -e "OracleScan-Users.bat")
		{
			if (open(LOG, "> OracleScan-Users.bat"))
			{
				close(LOG);
			}
		}

		if (open(LOG, ">> OracleScan-Users.bat"))
		{
			print LOG "OracleScan.exe $strTarget $strTarget.databases -u dict.txt\n";

			close(LOG);
		}
	}
}

sub AddToLogFiles()
{
	my($strTarget, $strService, $strInstance, $strHost, $intPort) = @_;

	if (! -e "tnsnames.ora")
	{
		if (open(LOG, "> tnsnames.ora"))
		{
			close(LOG);
		}
	}

	if (open(LOG, ">> tnsnames.ora"))
	{
		print LOG "$strService-$strTarget =\n";
		print LOG "  (DESCRIPTION =\n";
		print LOG "    (ADDRESS = (PROTOCOL = TCP)(HOST = $strHost)(PORT = $intPort))\n";
		print LOG "    (CONNECT_DATA =\n";
		print LOG "      (SERVICE_NAME = $strService)\n";
		print LOG "      (INSTANCE_NAME = $strInstance)\n";
		print LOG "    )\n";
		print LOG "  )\n";
		print LOG "\n";

		close(LOG);
	}

	if (! -e "$strTarget.databases")
	{
		if (open(LOG, "> $strTarget.databases"))
		{
			close(LOG);
		}
	}

	if (open(LOG, ">> $strTarget.databases"))
	{
		print LOG "$strService-$strTarget\n";

		close(LOG);
	}
}
