#! /usr/bin/perl
# @(#)53	1.10  src/bldenv/pkgtools/subptf/bldprevdef.pl, pkgtools, bos412, GOLDA411a 11/17/92 11:04:48
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS:    bldprevdef
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

# Declare common perl code/constants.
push(@INC,split(/:/,$ENV{"PATH"})); # define search path
do 'bldperlconst';    	# general bld constants
do 'bldperllog';	# log functions
do 'bldtdlib';		# bldtd database functions
do 'bldhistorylib';	# bldhistory database functions

#
# NAME: bldprevdef
#
# FUNCTION: Gather, for each input file, the defect numbers for which
#	    previous build dependencies exist.
#
#	For each file, get a list of all dependent files (all files, old 
#	or updated, which have gone into building this file).  For each
#	of the dependent files, get from the file history those defects
#	which have most recently caused the file to be rebuilt.  Add these 
#       defects to list of previous dependencies.
#
# INPUT: release (parameter) - CMVC level release name
#        filelist (parameter) - file name containing list of files to gather
#	 			dependency data for.
#	 DEBUG (environment) - flag to generate debug data, when true
#			       List each file if DEBUG = 2.
#
# OUTPUT: Dependency data for each file is written to stdout.  The format
#		is "filename|p,p,p ...
#	  where 'p' is a defect number from a previous dependency.
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or 1 (failure) or 2 (fatal)
#
  $rc=$UNSET;
  $COMMAND=$0; $COMMAND =~ s#/.*/##;  # save this command's basename
  $lvar="-c $COMMAND +l";  # log parmeters
  %SIG=('HUP','Abort','INT','Abort','QUIT','Abort',
        'TERM','Abort','ABRT','Abort');

  ($release,$listfile) = (@ARGV);
  (defined $ENV{'DEBUG'}) && 
	&log("$lvar -b","getting previous defects for $release");

  chop($workpath=`ksh bldreleasepath $release`);
  open(LIST,"<$listfile") || &log("$lvar -x","$listfile open error ($!)");
  (&BLDTDinterfaceinit($release) == $SUCCESS) ||
        &log("$lvar -x","bldtd interface initialization error");
  (&BLDHISTORYinterfaceinit() == $SUCCESS) || 
	&log("$lvar -x","bldhistory interface initialization error");

  while ($file=<LIST>) {
	chop $file;	
        ($ENV{'DEBUG'} == 2) && 
		&log("$lvar -b","getting previous defects for $file");
	undef @directdefects; undef @dependlist; 
	undef @alldepends; undef %defects;
	(&BLDTDgetdependentslist($workpath,$file,"ALL",*alldepends) ne undef) ||
        	&log("$lvar -e","getting ALL dependents for $file in $release");
	&BLDHISTORY2getdep (*alldepends,*defects);
	$defectstr=join(',',(sort keys %defects));
 	if ($defectstr ne undef) {
		print "$file|$defectstr\n";
		if ( $! != 0 ) {
	    	    &log("$lvar -x","write error ($!)");
		}
	}
  }

  (defined $ENV{'DEBUG'}) && 
	&log("$lvar -b","finished getting previous defects for $release");
  if ($rc == $UNSET) { exit $SUCCESS; } else { exit $FAILURE; }

sub Abort {
        exit $FATAL;
}

