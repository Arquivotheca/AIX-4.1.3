#! /usr/bin/perl
# @(#)57	1.26  src/bldenv/pkgtools/subptf/bldnormalize.pl, pkgtools, bos412, GOLDA411a 8/19/94 16:57:12
#
# COMPONENT_NAME: PKGTOOLS
#
# FUNCTIONS:    bldnormalize
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991, 1994
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
# NAME: bldnormalize
#
# FUNCTION: Normalize path names by resolving relative path references and
#           cutting leading subpaths off.
#
# INPUT: file (parameter) - contains list of paths (zero or more per line)
#	 NONORMERR (environment) - if set, do not generate an error if
#	                           a path could not be normalized because it
#				   did not meet any of the expected patterns
#
# OUTPUT: the normalize path names are written to stdout (same line format as
#         the input)
#
# RETURNS: 0 (success) or non-zero (failure)
#
$rc=$SUCCESS;
$COMMAND=$0; $COMMAND =~ s#/.*/##;  # save this command's basename
$lvar="-c $COMMAND +l";  # log parameters
%SIG=('HUP','Abort','INT','Abort','QUIT','Abort',
      'TERM','Abort','ABRT','Abort');
open (DATA,"<$ARGV[0]") || &log("$lvar -x","file ($ARGV[0]) open error");

while (<DATA>) {	# for each input line
	# if relative path (./ at beginning), assume the path
	# is a ship file so convert it to a ship path
	s#^\./#/ship/power/#;

	while (m#[^.]\./#) {	 # replace instances of "./" with "/"
		s#[^.]\./#/#;	 # example: /a/b/./c/d becomes a/b/c/d
	}
	while (m#//#) { 	 # replace instances of "//" with "/"
		s#//#/#;
	}
	while (m#/\.\./#) { 	 # resolve instances of "../"
		s#[^/]*/\.\./##; # example: /a/b/../c/d becomes /a/c/d
	}

	undef $line;
	foreach (split) {	 # for each token in line
	   $normalizeFlag=$FALSE;
	   ## Substitute for all platform names.
           ## Currently the only platforms are power and power_mp
	   $s1=s#/ship/power/|/ship/power_mp/#!!/ship/#;
	   if ( $s1 != 0 ) {
		$normalizeFlag=$TRUE;
	   }

	   # This one is for liblpp.a only updates which only affect the
	   # package.
	   if ( $normalizeFlag == $FALSE) {
		$s7=s#.*/packages/#!!/#;
	        if ( $s7 != 0 ) {
		   $normalizeFlag=$TRUE;
	        }
	    }

	   ## This handles makelist format.
	   if ( $normalizeFlag == $FALSE) {
		## substitute for power or power_mp
		$s2 = s#/obj/power/|/obj/power_mp/#/!!/#; 
		if ( $s2 != 0 ) {
		    $normalizeFlag=$TRUE;
		}
	   }
	   if ( $normalizeFlag == $FALSE) {
		## substitute for export
		$s3=s#.*/export/power/|.*/export/power_mp/#/export/@@/#;
	        if ( $s3 != 0 ) {
		   $normalizeFlag=$TRUE;
	        }
	   }
	   if ( $normalizeFlag == $FALSE) {
		## substitute for src
		$s4=s#/src/|^src/#!!src/#;
	        if ( $s4 != 0 ) {
		   $normalizeFlag=$TRUE;
	        }
	    }
	   if ( $normalizeFlag == $FALSE) {
		## substitute for UPDATE first
		$s5=s#.*/UPDATE/#!!/#;
	        if ( $s5 != 0 ) {
		   $normalizeFlag=$TRUE;
	        }
	    }
	   if ( $normalizeFlag == $FALSE) {
		## substitute for selfix
		$s6=s#.*/selfix/#!!/#;
	        if ( $s6 != 0 ) {
		   $normalizeFlag=$TRUE;
	        }
	    }

	   s#.*!!#!!#;			## cut paths up to !!
	   $line.="$_ ";		## build new output line
	   if ($normalizeFlag == $FALSE && ! /NEW/ && $ENV{'NONORMERR'} eq undef) {  
		## no normalize subtitutions made (but should have been)
	     &log("$lvar -w","unexpected path (Line $.: $_)");
	   }
	}
	chop $line;			   # remove extra blank
	print "$line\n" unless /^\s*$/;	   # print new line unless white space
}

exit $rc;

sub Abort {
        exit $FATAL;
}

