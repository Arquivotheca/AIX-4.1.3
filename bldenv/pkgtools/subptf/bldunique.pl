#! /usr/bin/perl
# @(#)75	1.7  src/bldenv/pkgtools/subptf/bldunique.pl, pkgtools, bos412, GOLDA411a 11/17/92 11:04:31
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS:    bldunique
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991, 1992
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
# NAME: bldunique
#
# FUNCTION: 
#
# INPUT: file (parameter) - 
#
# OUTPUT: 
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: n/a
#
# RETURNS: 0 (successful) or 1 (failure) or 2 (fatal)
#
$rc=$SUCCESS;
$COMMAND=$0; $COMMAND =~ s#/.*/##;  # save this command's basename
$lvar="-c $COMMAND +l";   # log parameters
%SIG=('HUP','Abort','INT','Abort','QUIT','Abort',
      'TERM','Abort','ABRT','Abort');
(defined $ENV{'DEBUG'}) && &log("$lvar -b","entering (@ARGV)");

# checking command syntax
if (($#ARGV > 0) || (($#ARGV == 0) && ($ARGV[0] eq "-?"))) {
	print STDERR "$COMMAND [<filename>]\n";
	exit 1;
}

# use STDIN if no file was given, otherwise use file name
if (defined $ARGV[0]) {
	open($FD=DATA,"< $ARGV[0]") || 
		&log("$lvar -x","file ($ARGV[0]) open error");
} else {
	$FD=STDIN;
}

while (<$FD>) {	# for each input line
	$lines{$_} = $DEFINED;
}
while (($key,$value) = each %lines) { 
	print $key; 
	if ( $! != 0 ) {
	    &log("$lvar -x","write error ($!)");
	}
}

(defined $ENV{'DEBUG'}) && &log("$lvar -b","exiting");
exit $rc;

sub Abort {
        exit $FATAL;
}

