PWDumpX v1.1 | http://reedarvin.thearvins.com/


ATTENTION:

THIS TOOL IS PROVIDED FOR EDUCATIONAL PURPOSES ONLY. WHICH MEANS IF YOU
USE IT, YOU USE IT AT YOUR OWN RISK. I AM NOT RESPONSIBLE FOR WHAT YOU
DO WITH THIS TOOL.

IT IS RECOMMENDED THAT YOU READ THIS README FILE ENTIRELY BEFORE RUNNING
PWDUMPX.


========================================================================

1. SUPPORTED OPERATING SYSTEMS
2. PWDUMPX FEATURES
3. HOW TO USE PWDUMPX
4. CREDITS
5. QUESTIONS, BUGS, ETC.

========================================================================



1. SUPPORTED OPERATING SYSTEMS

This tool was tested against and works with the following operating
systems:

Windows NT 4.0
Windows 2000
Windows XP
Windows Server 2003



2. PWDUMPX FEATURES

PWDumpX version 1.1 allows a user with administrative privileges to
retrieve the domain password cache, password hashes and LSA secrets from
a Windows system. This tool can be used on the local system or on one or
more remote systems.

If an input list of remote systems is supplied, PWDumpX will attempt to
obtain the domain password cache, the password hashes and the LSA
secrets from each remote Windows system in a multi-threaded fashion (up
to 64 systems simultaneously).

The domain password cache, password hashes and LSA secrets from remote
Windows systems are encrypted as they are transfered over the network.
No data is sent over the network in clear text.

This tool is a completely re-written version of CacheDump, PWDump3e and
LSADump2 which integrates suggestions/bug fixes for PWDump3e and
LSADump2 found on various web sites, etc.



3. HOW TO USE PWDUMPX

Usage: PWDumpX <hostname | ip input file> <username> <password>

<hostname | ip input file>  -- required argument
<username>                  -- required argument
<password>                  -- required argument

If the <username> and <password> arguments are both plus signs (+), the
existing credentials of the user running this utility will be used.

Examples:
PWDumpX 10.10.10.10 + +
PWDumpX 10.10.10.10 administrator password

PWDumpX MyWindowsMachine + +
PWDumpX MyWindowsMachine administrator password

PWDumpX IPInputFile.txt + +
PWDumpX IPInputFile.txt administrator password



4. CREDITS

My intent with including the source code along with this tool is to
give something back to the I.T. security community. I learned a lot
while creating PWDumpX but I could not have done it without the
original source code for PWDump2, PWDump3e, and LSADump2. So...thanks
to the creators of these tools for being generous enough to include the
source code with these tools so that hungry minds can learn new things.



5. QUESTIONS, BUGS, ETC.

Please send all comments, suggestions, bugs, etc. to me. I will do my
best to find the time to make corrections if need be. Thank you.

Reed Arvin <reedarvin@gmail.com>
