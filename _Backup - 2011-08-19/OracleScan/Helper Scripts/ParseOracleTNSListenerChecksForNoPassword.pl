# Usage: ParseOracleTNSListenerChecksForNoPassword.pl

use strict;

my($strFile)      = "";
my(@strFiles)     = ();
my(@strFileSplit) = ();
my($strTarget)    = "";

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
			while(<FILE>)
			{
				if (/^Security\s+OFF$/)
				{
					print "Adding target $strTarget...\n";

					if (! -e "NoOracleTNSListenerPassword.txt")
					{
						if (open(LOG, "> NoOracleTNSListenerPassword.txt"))
						{
							close(LOG);
						}
					}

					if (open(LOG, ">> NoOracleTNSListenerPassword.txt"))
					{
						print LOG "$strTarget\n";

						close(LOG);
					}
				}
			}

			close(FILE);
		}
	}
}
