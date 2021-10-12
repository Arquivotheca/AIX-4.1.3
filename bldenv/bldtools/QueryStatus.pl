#! /usr/bin/perl
# @(#)88	1.9  src/bldenv/bldtools/QueryStatus.pl, bldprocess, bos412, GOLDA411a 1/23/92 14:53:13
#
# COMPONENT_NAME: (BLDPROCESS) BAI Build Process
#
# FUNCTIONS: QueryStatus
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
# NAME: QueryStatus
#
# FUNCTION: Queries the status of any operation or command in the  database file
#	    and prints the record that matches to the standard output
#
# INPUT: avalue,bvalue,cvalue,dvalue,evalue,filename(global command line
#	 (arguments)
#
# OUTPUT: Standard output
#
# EXECUTION ENVIRONMENT: Build process environment
#

push(@INC,split(/:/,$ENV{"PATH"}));          # Including path 
do 'bldperlconst';
do 'bldstatusfunc';
&initialize('*');
&get_cmd_line;
&get_status_file;
open status_file || die("Can't find the file $status_file\n");
while (<status_file>){

	chop($_);
	($a_db_val,$b_db_val,$c_db_val,$d_db_val,$e_db_val,$f_db_val, $g_db_val,$h_db_val,$i_db_val,$j_db_val) = split(/\|/,$_);

	if((($a_value =~ /^\*$|^don\'t care$/) || ($a_value eq $a_db_val))
	&& (($b_value =~ /^\*$|^don\'t care$/) || ($b_value eq $b_db_val))
	&& (($c_value =~ /^\*$|^don\'t care$/) || ($c_value eq $c_db_val))
        && (($d_value =~ /^\*$|^don\'t care$/) || ($d_value eq $d_db_val))
        && (($e_value =~ /^\*$|^don\'t care$/) || ($e_value eq $e_db_val))
        && (($f_value =~ /^\*$|^don\'t care$/) || ($f_value eq $f_db_val))
        && (($g_value =~ /^\*$|^don\'t care$/) || ($g_value eq $g_db_val))
        && (($h_value =~ /^\*$|^don\'t care$/) || ($h_value eq $h_db_val))
        && (($i_value =~ /^\*$|^don\'t care$/) || ($i_value eq $i_db_val))
        && (($j_value =~ /^\*$|^don\'t care$/) || ($j_value eq $j_db_val))){
		
		local($line) = "";
		if ($A_value){
			$line .= "$a_db_val\|$b_db_val\|$c_db_val\|$d_db_val";
			$line .= "\|$e_db_val\|$f_db_val\|$g_db_val";
			$line .= "\|$h_db_val\|$i_db_val\|$j_db_val\n";
			print "$line";
			next;
		}


		$line .= "$a_db_val" 
			if (($a_db_val ne "") && ($a_value eq "*"));
		$line .= "\|$b_db_val" 
			if (($b_db_val ne "") && ($b_value eq "*"));
		$line .= "\|$c_db_val" 
			if (($c_db_val ne "") && ($c_value eq "*"));
		$line .= "\|$d_db_val" 
			if (($d_db_val ne "") && ($d_value eq "*"));
		$line .= "\|$e_db_val" 
			if (($e_db_val ne "") && ($e_value eq "*"));
		$line .= "\|$f_db_val" 
			if (($f_db_val ne "") && ($f_value eq "*"));
		$line .= "\|$g_db_val" 
			if (($g_db_val ne "") && ($g_value eq "*"));
		$line .= "\|$h_db_val" 
			if (($h_db_val ne "") && ($h_value eq "*"));
		$line .= "\|$i_db_val" 
			if (($i_db_val ne "") && ($i_value eq "*"));
		$line .= "\|$j_db_val" 
			if (($j_db_val ne "") && ($j_value eq "*"));
		$line =~ s/^\|//;
		print "$line\n" if ($line ne "");
	}
}
exit $SUCCESS;
