# Usage: AutoFuzzyFilesExecMon.pl

use Win32::ProcFarm::Pool;
use strict;

my($strFuzzyFileName)           = "";
my($strFuzzOpt)                 = "";
my($strOverflowChars)           = "";
my($intNumRepeats)              = "";
my($intExecMonTimeout)          = "";
my($strPathToExecMonExecutable) = "";
my($strExecMonArguments)        = "";
my($intTotalFileVariations)     = "";
my($objPool)                    = "";
my(@strScriptArgs)              = ();
my($i)                          = "";
my($strTempExecMonArguments)    = "";

print "AutoFuzzyFilesExecMon v1.0\n";
print "\n";
print "Enter Fuzzy File Name: ";

$strFuzzyFileName = <STDIN>;

print "Enter Fuzz Option: ";

$strFuzzOpt = <STDIN>;

print "Enter Overflow Chars: ";

$strOverflowChars = <STDIN>;

print "Enter Repeat \#: ";

$intNumRepeats = <STDIN>;

print "Enter ExecMon Timeout: ";

$intExecMonTimeout = <STDIN>;

print "Enter Path to ExecMon Executable: ";

$strPathToExecMonExecutable = <STDIN>;

print "Enter ExecMon Arguments: ";

$strExecMonArguments = <STDIN>;

chomp($strFuzzyFileName);
chomp($strFuzzOpt);
chomp($strOverflowChars);
chomp($intNumRepeats);
chomp($intExecMonTimeout);
chomp($strPathToExecMonExecutable);
chomp($strExecMonArguments);

system("cls");

print "Running AutoFuzzyFilesExecMon v1.0 with the following arguments:\n";
print "[+] Fuzzy File Name:            \"$strFuzzyFileName\"\n";
print "[+] Fuzz Option:                \"$strFuzzOpt\"\n";
print "[+] Overflow Chars:             \"$strOverflowChars\"\n";
print "[+] Repeat \#:                   \"$intNumRepeats\"\n";
print "[+] ExecMon Timeout:            \"$intExecMonTimeout\"\n";
print "[+] Path to ExecMon Executable: \"$strPathToExecMonExecutable\"\n";
print "[+] ExecMon Arguments:          \"$strExecMonArguments\"\n";
print "\n";

if (open(FUZZYFILES, "FuzzyFiles.exe $strFuzzyFileName $strFuzzOpt $strOverflowChars $intNumRepeats 1 |"))
{
	while (<FUZZYFILES>)
	{
		if (/^Created file \S+ of (\S+)$/)
		{
			$intTotalFileVariations = $1;
		}
	}

	close(FUZZYFILES);
}

$objPool = Win32::ProcFarm::Pool->new(10, 9000, "Child.pl", ".");

push(@strScriptArgs, $strFuzzyFileName);
push(@strScriptArgs, $strFuzzOpt);
push(@strScriptArgs, $strOverflowChars);
push(@strScriptArgs, $intNumRepeats);
push(@strScriptArgs, $intExecMonTimeout);
push(@strScriptArgs, $strPathToExecMonExecutable);
push(@strScriptArgs, "($strOverflowChars x $intNumRepeats)");
push(@strScriptArgs, "0");

$objPool->add_waiting_job($i, "ThreadedSub", @strScriptArgs);

pop(@strScriptArgs);
pop(@strScriptArgs);

for($i = 1; $i < ($intTotalFileVariations + 1); $i++)
{
	$strTempExecMonArguments = $strExecMonArguments;

	$strTempExecMonArguments =~ s/\%EXE\%/\"$i\_$strFuzzyFileName\"/g;

	push(@strScriptArgs, $strTempExecMonArguments);
	push(@strScriptArgs, $i);

	$objPool->add_waiting_job($i, "ThreadedSub", @strScriptArgs);

	pop(@strScriptArgs);
	pop(@strScriptArgs);
}

$objPool->do_all_jobs(0.1);
