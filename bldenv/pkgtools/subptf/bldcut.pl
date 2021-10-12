#! /usr/bin/perl
# @(#)42	1.4  src/bldenv/pkgtools/subptf/bldcut.pl, pkgtools, bos412, GOLDA411a 5/14/92 10:23:28
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: bldcut
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

push(@INC,split(/:/,$ENV{"PATH"})); # define search path
do 'bldperlconst';    # define constants
do 'bldperllog';      # init perl interface to log functions

#
# FUNCTION: This command provides a basic replacement for the Unix
#	    cut command.  It was necessary because the Unix command (3.1.5)
#	    cannot handle "unlimited" line lengths -- this command can.
#	    (The Unix cut command was core dumping on long (>1K) lines.)
#
# INPUT: file (parameter) - the file whose contents are to be cut
#	 delimitertype (parameter) - the delimiter/separator to cut by;
#	     the two delimiter values are "BAR" and "SPACE" to represent
#	     "|" and " " (this simplified the code)
#	 fields (parameters) - the remaining parameters (one or more) 
#	     specify the fields to be cut
#
# OUTPUT: stdout - the cut fields are written to stdout; each field is
#	           separated by the delimitertype
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: n/a
#
# RETURNS: 0 
#
$rc=$SUCCESS;
$COMMAND=$0; $COMMAND =~ s#/.*/##;  # save this command's basename
$lvar="-c $COMMAND +l";  # log parameters
($ENV{'DEBUG'} == 2) && &log("$lvar -b","entering (@ARGV)");
$rc=$SUCCESS;

$file=shift; $delimitertype=shift; @fields=@ARGV;
open (DATA,"<$file") || &log("$lvar -x","file ($ARGV[0]) open error");

grep($_--,@fields);  ## decrement field number for array indexing
while (<DATA>) {
	chop; $newline='';
	if ($delimitertype eq "BAR") {
		@values=split(/\|/);
		$delimiter='|';
	}
	elsif ($delimitertype eq "SPACE") {
		@values=split;
		$delimiter=' ';
	}
	foreach $position (@fields) {
		$newline .= $values[$position] . $delimiter;
	}
	chop $newline;
	print "$newline\n";
}

($ENV{'DEBUG'} == 2) && &log("$lvar -b","exiting");
exit $rc;
