#! /usr/bin/perl
# @(#)49	1.9  src/bldenv/pkgtools/subptf/bldinfo.pl, pkgtools, bos412, GOLDA411a 11/17/92 11:00:32
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools 
#
# FUNCTIONS:	bldinfo
#

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

push(@INC,split(/:/,$ENV{"PATH"}));
do 'bldperlconst';    # define constants
do 'bldperllog';      # init perl interface to log functions
do 'bldinfolib';      # bldinfo database functions

#
# NAME: bldinfo
#
# FUNCTION: Provide access to bldinfo dependencies database.
#
#       The "op" parameter selects which bldinfo operation should be
#       performed.  The appropriate function is then called with the
#       remaining parameters.
#
# INPUT: op (parameter) - selects the operation to perform.
#	 DEBUG (environment) - flag to generate debug data, when true
#
#        All remaining parameters are passed on to the underlying function.
#
# OUTPUT: none
#
# RETURNS: 0 (successful) or 1 (failure) or 2 (fatal)
#
  $COMMAND=$0; $COMMAND =~ s#/.*/##;  # save this command's basename
  $lvar="-c $COMMAND +l";  # log parameters
  %SIG=('HUP','Abort','INT','Abort','QUIT','Abort',
        'TERM','Abort','ABRT','Abort');
  (defined $ENV{'DEBUG'}) && &log("$lvar -b","entering (@ARGV)");
  $op = $ARGV[0];

  if ($op eq "GETENV")
        { $rc=&BLDINFOgetenvdefects ($ARGV[1]); }
  elsif ($op eq "GETUP")
        { $rc=&BLDINFOgetupdata ($ARGV[1]); }
  elsif ($op eq "GETFILENAMES")
        { $rc=&BLDINFOgetfilenames (); }
  elsif ($op eq "SAVEENV")
        { $rc=&BLDINFOsaveenvdata ($ARGV[1]); }
  elsif ($op eq "SAVEREQ")
        { $rc=&BLDINFOsavereqdata ($ARGV[1]); }
  elsif ($op eq "SAVEPREV")
        { $rc=&BLDINFOsaveprevdata ($ARGV[1]); }
  elsif ($op eq "SAVEUP")
        { $rc=&BLDINFOsaveupdata ($ARGV[1]); }
  elsif ($op eq "SAVELPPPTFS")
        { $rc=&BLDINFOsavelppgroups ($ARGV[1],$ARGV[2]); }
  elsif ($op eq "GETLPPPTFS")
        { $rc=&BLDINFOgetlppgroups ($ARGV[1]); }
  elsif ($op eq "GETALLPTFS")
        { $rc=&BLDINFOgetlppgroups ("alllppptfs"); }
  elsif ($op eq "GETALLPREVDEPS")
        { $rc=&BLDINFOgetallprevdata (); }
  else  {
	&log("$lvar -e","illegal syntax");
        print "Usage:\n";
        &log("+l -b -c'NULL'","    $COMMAND GETENV <path>");
        &log("+l -b -c'NULL'","    $COMMAND GETUP <namesfile>");
        &log("+l -b -c'NULL'","    $COMMAND SAVEENV <envdatafile>");
        &log("+l -b -c'NULL'","    $COMMAND SAVEREQ <reqdatafile>");
        &log("+l -b -c'NULL'","    $COMMAND SAVEPREV <prevdatafile>");
        &log("+l -b -c'NULL'","    $COMMAND SAVEUP <updatefile>");
        &log("+l -b -c'NULL'",
		"    $COMMAND SAVELPPPTFS <lpp> <ptfgroupsfile>");
        &log("+l -b -c'NULL'","    $COMMAND GETFILENAMES");
        &log("+l -b -c'NULL'","    $COMMAND GETALLPTFS");
        &log("+l -b -c'NULL'","    $COMMAND GETALLPREVDEPS");
	}

  (defined $ENV{'DEBUG'}) && &log("$lvar -b","exiting");
  exit $rc;

sub Abort {
        exit $FATAL;
}

