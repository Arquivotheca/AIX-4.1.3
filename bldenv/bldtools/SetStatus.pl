#! /usr/bin/perl
# @(#)81	1.6  src/bldenv/bldtools/SetStatus.pl, bldprocess, bos412, GOLDA411a 1/23/92 14:49:45
#
# COMPONENT_NAME: (BLDPROCESS) BAI Build Process
#
# FUNCTIONS: SetStatus
#	     process_opt_arg
#	     get_cmd_line
#	     initialize
#	     print_status
#	     usage
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
# NAME: SetStatus
#
# FUNCTION: Sets the status of any operation or command in a database file
#
# INPUT: avalue,bvalue,cvalue,dvalue,evalue,filename(global command line
#	 (arguments)
#
# OUTPUT: status.db (The Status database (db) file
#
# EXECUTION ENVIRONMENT: Build process environment
#
#

push(@INC,split(/:/,$ENV{"PATH"}));          # Including path 
do 'bldperlconst';
do 'bldstatusfunc';

&initialize('');
for(0..$#ARGV){
	if(index($ARGV[$_],' ') >= 0){
		$temp[$_] = "\"$ARGV[$_]\"";
	}
	else{
		$temp[$_] = $ARGV[$_];
	}
}
&get_cmd_line;
&get_status_file;
&print_status;

#
# NAME: print_status
#
# FUNCTION: Prints the values of options on the command line to the
#	    to the db file
#
# INPUT: avalue,bvalue,cvalue,dvalue,evalue,filename
#
# OUTPUT: filename (default being status.db)
#
# SIDE EFFECTS: exits with a value of 0 on a successfull write to the db file
#

sub print_status{
	&lock;
	open (FILENAME,"+>>$status_file") ||
		&quit("can't open file $status_file\n");
	chmod(0666,"/tmp/status.db")
		if (index($FILENAME,"/tmp/status.db") >= 0);
	print FILENAME "$a_value|$b_value|$c_value|$d_value|$e_value|";
	print FILENAME "$f_value|$g_value|$h_value|$i_value|$j_value\n";
	&unlock;
	exit $SUCCESS;
}

