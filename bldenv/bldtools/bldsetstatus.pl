#! /usr/bin/perl
# @(#)37	1.2.1.2  src/bldenv/bldtools/bldsetstatus.pl, bldprocess, bos412, GOLDA411a 11/15/93 14:26:56
#
# COMPONENT_NAME: (BLDPROCESS) BAI Build Process
#
# FUNCTIONS: bldsetstatus
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
# NAME: bldsetstatus
#
# FUNCTION: Calls SetStatus with any arguments passed to it in addition to the
#	    the date and Builder( If it is set in Env) fields. 
#
#
# EXECUTION ENVIRONMENT: Build process environment
#
# RETURNS: 0 on Success
#	   1 on Failure
#

push(@INC,split(/:/,$ENV{"PATH"}));          # Including path
if ( $ENV{'_BUILDER'} ne ""){
	$BUILDER = $ENV{'_BUILDER'};
}
else{
        $BUILDER = "-e";
}
if ( $ENV{'_DATE'} ne ""){
	$DATE = $ENV{'_DATE'};
}
else{
        $DATE = "-f";
}
if ( $ENV{'_HOSTNAME'} ne ""){
	$HOSTNAME = $ENV{'_HOSTNAME'};
}
else{
        $HOSTNAME = "-h";
}
chop($date = `date +"%h %d %H:%M"`);
chop($host = `hostname | cut -d'.' -f1`);
if ( $ENV{'BUILDER'} ne ""){
	$builder = $ENV{'BUILDER'};	     # Builder name

	$rc = system("SetStatus $BUILDER '$builder' $DATE '$date' $HOSTNAME $host @ARGV");
	}
else{
	$rc = system("SetStatus $DATE '$date' $HOSTNAME $host @ARGV");
     }
exit $rc;

