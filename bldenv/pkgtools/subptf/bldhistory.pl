#! /usr/bin/perl
# @(#)45	1.9  src/bldenv/pkgtools/subptf/bldhistory.pl, pkgtools, bos412, GOLDA411a 11/17/92 11:01:06
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools 
#
# FUNCTIONS: 	bldhistory 
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
do 'bldperlfunc';     # define common functions
do 'bldperllog';      # init perl interface to log functions
do 'bldhistorylib';   # bldhistory database functions

#
# NAME: bldhistory
#
# FUNCTION: Provide access to dependency history "database".
#
#       The "op" parameter selects which bldhistory operation should 
#       be performed.  The appropriate function is then called with the
#       remaining parameters.
#
# INPUT: op (parameter) - selects the operation to perform.
#        DEBUG (environment) - flag to generate debug data, when true
#
#        Any remaining parameters are passed on to the underlying function.
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

  if ($op eq "SAVEDEP")
        { $rc=&BLDHISTORYsavedep ($ARGV[1]); }
  elsif ($op eq "GETDEP")
        { $rc=&BLDHISTORYgetdep ($ARGV[1]); }
  elsif ($op eq "REVISE")
        { $rc=&BLDHISTORYrevise (); }
  elsif ($op eq "SAVEPRIMEPTF")
        { $rc=&BLDHISTORYsavePTFdefects ("PRIME",$ARGV[1]); }
  elsif ($op eq "SAVESECONDPTF")
        { $rc=&BLDHISTORYsavePTFdefects ("SECOND",$ARGV[1]); }
  elsif ($op eq "GETPRIMEPTF")
        { $rc=&BLDHISTORYgetdefectPTFs ("PRIME",$ARGV[1]); }
  elsif ($op eq "GETSECONDPTF")
        { $rc=&BLDHISTORYgetdefectPTFs ("SECOND",$ARGV[1]); }
  elsif ($op eq "INITPTFIDS")
        { $rc=&BLDHISTORYinitPTFsession (); }
  elsif ($op eq "COMMITPTFIDS")
        { $rc=&BLDHISTORYcommitPTFsession (); }
  elsif ($op eq "DUMP")
        { $rc=&BLDHISTORYdump (); }
  elsif ($op eq "IMPORT")
        { $rc=&BLDHISTORYimport ($ARGV[1]); }
  else  {
        &log("$lvar -e","illegal syntax");
        print "Usage:\n";
        print "\t$COMMAND SAVEDEP <defectsfile>\n";
        print "\t$COMMAND GETDEP <namesfile>\n";
        print "\t$COMMAND REVISE <changesfile>\n";
        print "\t$COMMAND SAVEPRIMEPTF <ptfdefectfile>\n";
        print "\t$COMMAND SAVESECPTF <ptfdefectfile>\n";
        print "\t$COMMAND GETPRIMEPTF <defectfile>\n";
        print "\t$COMMAND GETSECPTF <defectfile>\n";
        print "\t$COMMAND INITPTFIDS\n";
        print "\t$COMMAND COMMITPTFIDS\n";
        print "\t$COMMAND DUMP\n";
        print "\t$COMMAND IMPORT <dumpfile>\n";
        }

  (defined $ENV{'DEBUG'}) && &log("$lvar -b","exiting");
  exit $rc;

sub Abort {
        exit $FATAL;
}

