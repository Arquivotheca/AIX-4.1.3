#! /usr/bin/perl
#
# @(#)90	1.9  src/bldenv/bldtools/bldperllog.pl, bldprocess, bos412, GOLDA411a 12/3/92 18:02:41
# COMPONENT_NAME: (BLDPROCESS) BAI Build Process
#
# FUNCTIONS: log
#	     logset
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

# Get the real terminal name, BLDTTY will be some form of error message
# if no terminal is connected to this session.
# Make sure ENV is not set when invoking ksh, if the file named by ENV
# calls bldloginit we will enter a infinite loop.
$old_ENV = "$ENV{'ENV'}";
$ENV{'ENV'} = "";
chop($ENV{'BLDTTY'} = `ksh -c 'tty < /dev/tty' 2>&1`);
$ENV{'ENV'} = "${old_ENV}";

# NAME: log
#
# FUNCTION: This is a interface to perl functions which calls
#	    the command 'bldlog' with all the parameters passed to it (log)
#
# INPUT: Since this function is primarily an perl interface to 'bldlog', All
#	 the options and optional arguments intended to be passed to 'bldlog' 
#	 command will be the input. See man pages of 'bldlog' for more 
#	 details on the input options.
#
# OUTPUT: none
#
# EXECUTION ENVIRONMENT: Build process environment.
#
# RETURNS: Error codes from 'bldlog' command.
#

sub log{
	local($arg1,$arg2) = @_;
	local($logrc);
	$arg2 = "\'$arg2\'";       # string message being single quoted so
				   # that any meta characters will not be 
				   # interpreted as such by bldlog
	$logrc = system("ksh bldlog $arg1 $arg2")/256;
	$rc = $logrc if($logrc >=1);
	if ($logrc == 2){
		kill 'INT',$$;
		exit 2;
	}
	else{
		return $logrc;
	}
}

#
# NAME: logset
#
# FUNCTION: This is a interface to perl functions, which calls
#	    the command bldlogset with all the parameters passed to it (logset)
#	    This also redirects STDERR in the calling function to the specified
#	    log file.
#
# INPUT: Since this function is primarily an perl interface to 'bldlogset', All
#	 the options and optional arguments intended to be passed to bldlogset 
#	 command will be the input. See man pages of bldlogset for more 
#	 details on the input options.
#
# OUTPUT: none
#
# EXECUTION ENVIRONMENT: Build process environment.
#
# RETURNS: Error codes from 'bldlogset' command.
#
sub logset{
	local($arg1,$arg2) = @_;
	local($logrc,$logfile);
	chop($logfile = `ksh bldlogset $arg1$arg2`);
	$logrc = $?;
	$rc = $logrc if($logrc >=1);
	if ($logrc == 2){
		exit 2;
	}
	else{			# Capture STDERR to the log file and to the
				# screen by piping STDERR to capStderr

		open(STDERR,"|capStderr $logfile") || 
					warn "can't pipe STDERR to capStderr";
		return $logrc;
	} 
}
