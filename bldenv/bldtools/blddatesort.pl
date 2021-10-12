#! /usr/bin/perl
# @(#)94	1.1  src/bldenv/bldtools/blddatesort.pl, bldtools, bos412, GOLDA411a 1/14/93 11:31:38
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: blddatesort
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1991
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# NAME: blddatesort
#
# FUNCTION: sorts status file entries by the value contained in the date field
#
# INPUT: status file entries with all fields given whether they are 
#	 empty or not; input may come from standard input or an input file
#
# OUTPUT: status file entries sorted by date sent to standard output
#
# EXECUTION ENVIRONMENT: Build process environment
#

push(@INC,split(/:/,$ENV{"PATH"}));          # Including path 
do 'bldperlconst';
do 'bldperlfunc';

sub	bydate {
	local(@fielda) = split(/\|/,$a);
	local(@fieldb) = split(/\|/,$b);
	local($rc) = &datecmp($fielda[5],$fieldb[5]);
	if ($reverse) {
		$rc = -$rc;
	}
	return($rc);
}

while (&getopts(":r",*opt)) {
	if ($opt eq "r") {
		$reverse = $TRUE;
	} elsif ($opt eq "?") {
		$usage_err = $TRUE;
	}
	shift;
}

$infile = $ARGV[$#ARGV];
if ($infile ne "") {
	open(INFILE,"< $infile");
	$IN="INFILE";
} else {
	$IN="STDIN";
}

(@lines) = <$IN>;

(@sorted) = sort bydate @lines;
print @sorted;
