#! /usr/bin/perl
# @(#)38	1.4  src/bldenv/pkgtools/subptf/blddepunique.pl, pkgtools, bos412, GOLDA411a 5/26/92 09:17:34
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: blddepunique
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
# FUNCTION: This command uniques make list data (i.e. target dependent [NEW]).
#	    Originally Unix sort and unique commands were used, but the
#	    large data files caused sort to core dump or get the wrong
#	    results.
#
#	    Because we are using this dedicated command to uniq data, an
#	    optimization can be made; with the follows lines
#			target1	dependent1
#			target1	dependent1	NEW
#			target1	dependent1
#	    Only keep
#			target1	dependent1	NEW
#
#	    (Duplicate data is created in the make list when multiple
#	    v3bld commands are run on the same release - usually because of
#	    build errors.)
#
# INPUT: file (parameter) - the file to unique
#
# OUTPUT: stdout - the uniqued data 
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: n/a
#
# RETURNS: 0 (success) or 1 (failure)
#
$rc=$SUCCESS;
$COMMAND=$0; $COMMAND =~ s#/.*/##;  # save this command's basename
$lvar="-c $COMMAND +l";   # log parameters

open (DATA,"<$ARGV[0]") || &log("$lvar -x","file ($ARGV[0]) open error");

## the data is implicitly uniqued by reading it into an associative array
while (<DATA>) {  ## for each input line
        chop;
        ($target,$dependency,$built) = split;
	if ($line{"$target $dependency"} ne "NEW") {
		$line{"$target $dependency"} = $built;
	}
}

## now output the uniqued data
while (($key,$value) = each %line) { 
	$built=$line{$key};
	if ($built eq undef) {
		print "$key\n"; 
	}
	else {
		print "$key $built\n"; 
	}
}

exit $rc;
