#! /usr/bin/perl
# @(#)73	1.9  src/bldenv/pkgtools/subptf/bldtd.pl, pkgtools, bos412, GOLDA411a 11/17/92 10:59:52
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools 
#
# FUNCTIONS:	bldtd 
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

# This file contains code for the "selective-fix" target-dependency database.
# Entry points are provided to build the database and access the data.
#
# Note that returns codes are oriented to the shell (i.e. success is a 0 which
# is false in perl).

# Declare common perl code/constants.
push(@INC,split(/:/,$ENV{"PATH"})); # define search path
do 'bldperlconst';    	# define constants
do 'bldperlfunc';    	# define general functions
do 'bldperllog';	# init perl interface to log functions
do 'bldoptlib';         # optimization functions for library dependencies
do 'bldtdlib';	        # bldtd database functions

#
# NAME: bldtd
#
# FUNCTION: Provide access (entry points) to target-dependency "database".
#
#       The "command" parameter selects which bldtd operation should be
#       performed.  The appropriate function is then called with the
#       remaining parameters.
#
# INPUT: op (parameter) - selects the operation to perform.
#
#        Any remaining parameters are passed on to the underlying function.
#
# OUTPUT: none
#
# RETURNS: 0 (successful) or 1 (failure) or 2 (fatal)
#
  $COMMAND=$0; $COMMAND =~ s#.*/##;  # save this command's basename
  $lvar="-c $COMMAND +l";  # log parameters
  %SIG=('HUP','Abort','INT','Abort','QUIT','Abort',
        'TERM','Abort','ABRT','Abort');
  (defined $ENV{'DEBUG'}) && &log("$lvar -b","entering (args: @ARGV)");
  $op = $ARGV[0]; 

  if ($op eq "BUILD") 
        { $rc=&BLDTDbuild 
		($ARGV[1],$ARGV[2],$ARGV[3],$ARGV[4],$ARGV[5],$ARGV[6]); 
	}
  elsif ($op eq "GETTARGETDEFECTS") 
	{ $rc=&BLDTDgettargetdefects ($ARGV[1],$ARGV[2]); }
  elsif ($op eq "GETSRCDEFECTS") 
	{ $rc=&BLDTDgetsrcdefects ($ARGV[1]); }
  elsif ($op eq "GETNEWDEPENDS") 
	{ $rc=&BLDTDgetdependents ($ARGV[1],$ARGV[2],"NEW"); }
  elsif ($op eq "GETALLDEPENDS") 
	{ $rc=&BLDTDgetdependents ($ARGV[1],$ARGV[2],"ALL"); }
  else  { 
	&log("$lvar -e","illegal syntax"); 
	&log("+l -b -c'NULL'","Usage:");
	$parms="<rel> <makelist> <defectlist> <updatelist> <bldenvlist>";
	&log("+l -b -c'NULL'","  $COMMAND BUILD $parms");
	&log("+l -b -c'NULL'","  $COMMAND GETTARGETDEFECTS <release> <file>");
	&log("+l -b -c'NULL'","  $COMMAND GETSRCDEFECTS <release>");
	&log("+l -b -c'NULL'","  $COMMAND GETNEWDEPENDS <release> <file>");
	&log("+l -b -c'NULL'","  $COMMAND GETALLDEPENDS <release> <file>");
	}

  (defined $ENV{'DEBUG'}) && &log("$lvar -b","exiting");
  exit $rc;

sub Abort {
        exit $FATAL;
}

