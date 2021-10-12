#! /usr/bin/perl
# @(#)21	1.5  src/bldenv/pkgtools/subptf/subgroup.pl, pkgtools, bos412, GOLDA411a 4/1/94 14:25:41
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS:	subgroup
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# Declare common perl code/constants.
push(@INC,split(/:/,$ENV{"PATH"})); # define search path
do 'bldperlconst';    # define constants
do 'bldperllog';      # init perl interface to log functions

#
# NAME: subgroup -- Replacement for bldgroup in subsystem model.
#       Splitting because of "special files" is handled by forcing the
#       special files into their own subsystem.
#       This routine will simply assign all files to group 1.
#       subgroup supports liblpp.a file processing as follows:     
#       If a liblpp.a file appears in the input 
#       the following should be applicable. 
#       If $spath is defined in $newinsdefects, the liblpp.a file is output
#       with those defects.  ($spath is the liblpp.a filename prefixed by
#       by the subsystem name.)                                       
#
# INPUT: DEBUG (environment) - flag to generate debug data, when true
#
# OUTPUT: group appended to input and written to stdout.
#
#
# EXAMPLE: command input:
#		!!/sysx/disp/sab/geadefine.c|bos320.4957
#		!!/sysx/msla/msladecls.h|bos320.5927
#		!!/cmd/diag/da/eth/denet.msg|bos320.8247
#		!!/cmd/diag/Makefile|bos320.8209
#		!!/cmd/diag/tu/gio/Makefile|bos320.8224,bos320.5927
#		!!/cmd/diag/tu/tca/c327diag.c|bos320.8240,bos320.5927
#		!!/cmd/diag/tu/tok/hxihtx.h|bos320.8247
#          command output:
#		1|!!/sysx/disp/sab/geadefine.c|bos320.4957
#		1|!!/sysx/msla/msladecls.h|bos320.5927
#		1|!!/cmd/diag/tu/gio/Makefile|bos320.8224,bos320.5927
#		1|!!/cmd/diag/tu/tca/c327diag.c|bos320.8240,bos320.5927
#		1|!!/cmd/diag/da/eth/denet.msg|bos320.8247
#		1|!!/cmd/diag/tu/tok/hxihtx.h|bos320.8247
#		1|!!/cmd/diag/Makefile|bos320.8209
#
# RETURNS: 0 (successful) or non-zero (failure)
#
#
  $COMMAND=$0; $COMMAND =~ s#.*/##;  # save this command's basename
  &logset("-c $COMMAND +l");  # init log command
  %SIG=('HUP','Abort','INT','Abort','QUIT','Abort',
        'TERM','Abort','ABRT','Abort');
  (defined $ENV{'DEBUG'}) && &log("-b","entering (@ARGV)");
  $subsys = $ARGV[0]; $inputfile = $ARGV[1];
  $group=1;

  chop ($globalpath=`ksh bldglobalpath`);
  dbmopen(newinsdefects,"$globalpath/newinsdefects",0666) ||
	&log("-x","unable to dbmopen $globalpath/newinsdefects ($!)");
	
  chop ($LPPSDIR=`ksh bldupdatepath`);
  @fields = split('\.', $subsys);
  $lpp = $fields[0];

  open (INPUT,"<$inputfile") || &log("-x","unable to open $inputfile ($!)");
  
  while ($inputline = <INPUT>) {
      chop $inputline;
      ($file,$directdefects,$indirectdefects) = split(/\|/,$inputline);
      if ($file =~ m#/liblpp.a$#) {
	  $spath = $subsys . $file;
          $directdefects = $newinsdefects{$spath};
          if ($directdefects ne undef) {
                print "$group|$spath|$directdefects|$indirectdefects\n";
                if ( $! != 0 ) {
                    &log("-x","write error ($!)");
                }
          }
      }
      else {

          if ($directdefects ne undef || $indirectdefects ne undef) {
              print "$group|$file|$directdefects|$indirectdefects\n";
              if ( $! != 0 ) {
                  &log("-x","write error ($!)");
              }
	  }
      }   
  }
  
  close (INPUT);

  (defined $ENV{'DEBUG'}) && &log("-b","exiting");
  &logset("-r");
  exit $SUCCESS;

sub Abort {
	&logset("-r");
        exit $FATAL;
}

