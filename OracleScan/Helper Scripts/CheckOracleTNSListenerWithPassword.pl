# Usage: CheckOracleTNSListenerWithPassword.pl <input file> <tcp port> <password>

use strict;

my($strInputFile) = "";
my($intTCPPort)   = "";
my($strPassword)  = "";
my($strTarget)    = "";
my(@strTargets)   = ();
my($strReadLine)  = "";

$strInputFile = $ARGV[0];
$intTCPPort   = $ARGV[1];
$strPassword  = $ARGV[2];

if ($strInputFile eq "" || $intTCPPort eq "" || $strPassword eq "")
{
	print "Usage: CheckOracleListenerWithPassword.pl <input file> <tcp port> <password>\n";

	exit;
}

if (open(INPUT, "< $strInputFile"))
{
	while (<INPUT>)
	{
		$strTarget = $_;

		$strTarget =~ s/^\s+//;
		$strTarget =~ s/\s+$//;

		chomp($strTarget);

		if ($strTarget ne "")
		{
			push(@strTargets, $strTarget);
		}
	}

	close(INPUT);
}
else
{
	print "ERROR: Cannot open input file $strInputFile.\n";

	exit;
}

foreach $strTarget (@strTargets)
{
	if (open(LOG, "> $strTarget.commands"))
	{
		print LOG "set current_listener $strTarget:$intTCPPort\n";
		print LOG "set password $strPassword\n";
		print LOG "status\n";
		print LOG "services\n";
		print LOG "version\n";

		close(LOG);
	}

	if (open(LSNRCTL, "lsnrctl \@$strTarget.commands |"))
	{
		if (open(LOG, ">> $strTarget.txt"))
		{
			print LOG "\n";
			print LOG "\#\#\#\# Using password \"$strPassword\"...\n";
			print LOG "\n";

			close(LOG);
		}

		while (<LSNRCTL>)
		{
			$strReadLine = $_;

			chomp($strReadLine);

			print "$strReadLine\n";

			if (open(LOG, ">> $strTarget.txt"))
			{
				print LOG "$strReadLine\n";

				close(LOG);
			}
		}

		close(LSNRCTL);
	}

	if (-e "$strTarget.commands")
	{
		system("del $strTarget.commands");
	}
}
