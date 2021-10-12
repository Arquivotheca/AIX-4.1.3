#! /usr/bin/perl
# @(#)84	1.7  src/bldenv/bldtools/DeleteStatus.pl, bldprocess, bos412, GOLDA411a 1/23/92 14:51:25
#
# COMPONENT_NAME: (BLDPROCESS) BAI Build Process
#
# FUNCTIONS: DeleteStatus
#	     process_opt_arg
#	     usage
#	     initialize
#	     get_cmd_line
#	     read_status_file
#	     replace_status_file
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
# NAME: DeleteStatus
#
# FUNCTION: Queries the status of any operation or command in the  database file
#	    and deletes the first matched entry in the db file
#
# INPUT: avalue,bvalue,cvalue,dvalue,evalue,filename(global command line
#	 (arguments)
#
# OUTPUT: filename( default being /tmp/status.db)
#
# EXECUTION ENVIRONMENT: Build process environment
#
# RETURNS: 0 on success
#	   1 on Failure
#
exit $SUCCESS if ($#ARGV == -1);
push(@INC,split(/:/,$ENV{"PATH"}));          # Including path 
do 'bldperlconst';
do 'bldstatusfunc';
&initialize('*');
&get_cmd_line;
&get_status_file;
exit $SUCCESS if (!(-e $status_file) && (-z $status_file));
&read_status_file;
for (0..$#status_ary){
	local($line) = $status_ary[$_];
	chop($line);
	($a_db_val,$b_db_val,$c_db_val,$d_db_val,$e_db_val,$f_db_val, $g_db_val,$h_db_val,$i_db_val,$j_db_val) = split(/\|/,$line);

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
		if ($F_value){
			push(D_ARY,$_);
		}
		else{
			$index = $_;
			last;
		}
	}
}
if (($index =~ /^$/) && (! $F_value)){
	&unlock;
	exit $SUCCESS;
}
if ($F_value){
	for(0..$#D_ARY){
	splice(@status_ary,pop(D_ARY),1);
	system("cp $status_file $status_file.old");
	}
}
else{
	splice(@status_ary,$index,1);
}
&replace_status_file;
&unlock;
exit $SUCCESS;

# NAME: read_status_file
#
# FUNCTION: Reads the db file into a global array
#
# INPUT: status_file (Global variable)
#	 status_ary (Global array)
#
# OUTPUT: status_ary (Global array)
#

sub read_status_file{
	&lock;
	open (STATUS_FILE,"+<$status_file") ||
		&quit("Can't find the file $status_file\n");
	if (-z $status_file){
		&unlock;
		exit 0;
	}
	@status_ary = <STATUS_FILE>;
}

# NAME: replace_status_file
#
# FUNCTION: Replaces the contents of the db file with the updated 
# 	    array elements,i.e after the matched record has been deleted
#
# INPUT: status_file ( global variable for the db file)
# 	 status_ary (Global array)
#
# OUTPUT: status_file (Global variable)
#
 
sub replace_status_file{
	local($file) = $status_file;
	close(STATUS_FILE);
	system("rm $file");
	open (STATUS_FILE,">$status_file") || 
		&quit("can't open file $status_file\n");
	print STATUS_FILE join("",@status_ary);
}
