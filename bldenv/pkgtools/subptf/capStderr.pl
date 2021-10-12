#! /usr/bin/perl
# @(#)54	1.1  src/bldenv/pkgtools/subptf/capStderr.pl, bldprocess, bos412, GOLDA411a 2/6/92 11:47:15
# COMPONENT_NAME: (BLDPROCESS) BAI Build Process
#
# FUNCTIONS: capStderr
#	     
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

$logfile = shift(ARGV);
open(TERM,">/dev/tty");
select((select(LOG), $| = 1)[0]);
select((select(TERM), $| = 1)[0]);
open(LOG,">>$logfile");
while (<>){
	print TERM;
	print LOG ;
}
