# @(#)36	1.7  src/bldenv/bldtools/bldstatusfunc.pl, bldprocess, bos412, GOLDA411a 9/9/93 12:56:41
#
#   COMPONENT_NAME: BLDPROCESS
#
#   FUNCTIONS: get_cmd_line
#              get_perm
#              get_status_file
#              handler
#              initialize
#              lock
#              process_opt_arg
#              process_option
#              quit
#              unlock
#              usage
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1991
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

$SIG{'INT'} = 'handler';        # Interrupt handler

#
# NAME: handler
#
# FUNCTION: Traps interrupts , unlocks and quits from the program
#
# INPUT: none
#
# OUTPUT: none
#

sub handler{
        local($sig) = @_;
	&unlock;
        die "$0: Caught a SIG$sig -- quitting\n";
}


# NAME: usage
#
# FUNCTION: Prints out the valid options on STDERR if invalid options are used
#
# INPUT: none
#
# OUTPUT: Prints the command line options to the terminal
#
# NOTES: Exits from the program after printing the output.
#

sub usage{
	local($line);
	$line = "$0 [-a <value>] [-b <value>] [-c <value>]";
	$line .= " [-d <value>] [-e <value>] [-f <value>] [-g <value>]";
	$line .= " [-h <value>] [-i <value>] [-j <value>] filename";
	die "$line";
}


#
# NAME: initialize
#
# FUNCTION: initializes the option on the command line to default values
#
# INPUT: avlaue,bvalue,cvalue,dvalue,evalue (global command 
#	 line options)
#
# OUTPUT: avlaue,bvalue,cvalue,dvalue,evalue (global command 
#	 line options)
#

sub initialize{
	local($value) = $_[0]; 
	$a_value = $value;
	$b_value = $value;
	$c_value = $value;
	$d_value = $value;
	$e_value = $value;
	$f_value = $value;
	$g_value = $value;
	$h_value = $value;
	$i_value = $value;
	$j_value = $value;
}


# NAME: get_cmd_line
#
# FUNCTION: gets the command line options and their arguments
#
# INPUT: avalue,bvalue,cvalue,dvalue,evalue,filename
#
# OUTPUT: avalue,bvalue,cvalue,dvalue,evalue,filename
#

sub get_cmd_line{
	while ($ARGV[0] =~ /^-/){
		$_ = shift(@ARGV);
		if (/^-(a)(.*)/){
			&process_opt_arg($a_value);
		}
		elsif (/^-(b)(.*)/){
			&process_opt_arg($b_value);
		}
		elsif (/^-(c)(.*)/){
			&process_opt_arg($c_value);
		}
		elsif (/^-(d)(.*)/){
			&process_opt_arg($d_value);
		}
		elsif (/^-(e)(.*)/){
			&process_opt_arg($e_value);
		}
		elsif (/^-(f)(.*)/){
			&process_opt_arg($f_value);
		}
		elsif (/^-(g)(.*)/){
			&process_opt_arg($g_value);
		}
		elsif (/^-(h)(.*)/){
			&process_opt_arg($h_value);
		}
		elsif (/^-(i)(.*)/){
			&process_opt_arg($i_value);
		}
		elsif (/^-(j)(.*)/){
			&process_opt_arg($j_value);
		}
		elsif (/^-F(.*)/){
			&process_option;
			$F_value = 1;
		}
		elsif (/^-A(.*)/){
			&process_option;
			$A_value = 1;
		}
		else {
			print "Unrecognized option: $_\n";
			&usage;
		}
	}
	$status_file = shift(ARGV);
}


#
# NAME: process_opt_arg
#
# FUNCTION: Processes the command line optional argument
#
# INPUT: ARGV (global special perl variable)
#	 $1 $2 (Special perl local variables)
#	 $_ (special local perl variable)
#
# OUTPUT: ARGV (global special perl variable)
#	  $_ (special local perl variable)
#

sub process_opt_arg{
	local($option) = $1;
	local($temp);
	if (index($0,"QueryStatus") >= 0){
        	$temp = ( $2 ? $2 : shift(@ARGV));
        	if ($temp =~ /^\*$|^\'\*\'$/){
                	$_[0] = "don't care";
        	}
        	else{
                	$_[0] = $temp;
        	}
	}
	else{
		$_[0] = ( $2 ? $2 : shift(@ARGV));
	}	
	die "$0: $option requires a value\n" if $_[0] =~ /^-/;
}

#
# NAME: process_option
#
# FUNCTION: Processes the command line option
#
# INPUT: ARGV (global special perl variable)
#	 $1 (Special perl local variables)
#
# OUTPUT: ARGV (global special perl variable)
#

sub process_option{
	local($rest);
	unshift(ARGV,"-$rest") if(($rest = $1) ne '');
}
sub get_status_file{

	local($bldtmp);

	if ($status_file eq ""){
		if ( $ENV{'STATUS_FILE'} ne ""){
			$status_file = $ENV{'STATUS_FILE'};
		}
		else{
			$bldtmp = $ENV{'BLDTMP'};
			$bldtmp = "/tmp" if ($bldtmp eq "");
			$status_file = "$bldtmp/status.db";
		}	
	}
}

sub get_perm{
	local(*perms) = @_;
	local($host);
	if ( $ENV{'PERM'} ne ""){
	$perms = $ENV{'PERM'};
	}
	else{
		chop(($host = `hostname`));
		$perms = "-p $host.$0.$$";
	}	
}

sub unlock{
	local($permissions);
	&get_perm(*permissions);
	(($rc = system("bldlock -u $STATUS $permissions")/256) == 0) ||
	(($rc == 2) && ( print STDERR "UNLOCK: $status_file locked\n") &&
	exit 1) || exit 1;
}

sub lock{
	local($permissions);
	&get_perm(*permissions);
	(($rc = system("bldlock -l $STATUS $permissions")/256) == 0) ||
	(($rc == 2) && ( print STDERR "LOCK: $status_file locked\n") && exit 1)
	|| exit 1;
}

sub quit{
	local($string) = @_;
	print STDERR $string;
	&unlock;
	exit 1;
}
