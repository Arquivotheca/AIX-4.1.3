#! /usr/bin/perl
# @(#)80	1.7  src/bldenv/bldtools/CheckStatus.pl, bldprocess, bos412, GOLDA411a 1/23/92 14:52:41
#
# COMPONENT_NAME: (BLDPROCESS) BAI Build Process
#
# FUNCTIONS: CheckStatus
#	     process_opt_arg
#	     usage
#	     initialize
#	     get_cmd_line
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
# NAME: CheckStatus
#
# FUNCTION: Checks the status of any operation or command in the  database file
#
# INPUT: avalue,bvalue,cvalue,dvalue,evalue,filename(global command line
#	 (arguments)
#
# EXECUTION ENVIRONMENT: Build process environment
#
# RETURNS: 0 on Success
#	   1 on Failure
#

push(@INC,split(/:/,$ENV{"PATH"}));          # Including path 
do 'bldperlconst';
do 'bldstatusfunc';
&initialize('*');
&get_cmd_line;
&get_status_file;

open (STATUS_FILE,"+<$status_file") || 
		die("Can't open the file $status_file\n");

while (<STATUS_FILE>){

	chop($_);
	($a_db_val,$b_db_val,$c_db_val,$d_db_val,$e_db_val,$f_db_val, $g_db_val,$h_db_val,$i_db_val,$j_db_val) = split(/\|/,$_);

	if((($a_value eq "*") || ($a_value eq $a_db_val))
	&& (($b_value eq "*") || ($b_value eq $b_db_val))
	&& (($c_value eq "*") || ($c_value eq $c_db_val))
	&& (($d_value eq "*") || ($d_value eq $d_db_val))
	&& (($e_value eq "*") || ($e_value eq $e_db_val))
	&& (($f_value eq "*") || ($f_value eq $f_db_val))
	&& (($g_value eq "*") || ($g_value eq $g_db_val))
	&& (($h_value eq "*") || ($h_value eq $h_db_val))
	&& (($i_value eq "*") || ($i_value eq $i_db_val))
	&& (($j_value eq "*") || ($j_value eq $j_db_val))){
		exit $SUCCESS;
	}
}
exit $FAILURE;

