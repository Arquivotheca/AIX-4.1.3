#! /usr/bin/perl
# @(#)31	1.7  src/bldenv/bldtools/bldlock.pl, bldprocess, bos412, GOLDA411a 5/13/92 08:36:35
#
# COMPONENT_NAME: (BLDPROCESS) BAI Build Process
#
# FUNCTIONS: bldlock
#	     getlock
#	     getindex
#	     read_to_ary
#	     write_to_lock
#	     write_ary_tolock
#	     lock
#	     unlock
#	     process_opt_arg
#	     usage
#	     check_opt
#	     get_cmd_line
#	     get_lockfile
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
# NAME: bldlock
#
# FUNCTION: The bldlock command is a general purpose locking function which can
# 	    be used to prevent multiple processes from accessing the same 
#	    resource simultaneously.
#
# INPUT: command (locking and unlocking string), PERM, time, lockfile
#
# OUTPUT: Standard output
#
# EXECUTION ENVIRONMENT: Build process environment
#

$LOCK_EX=2;
$LOCK_UN=8;
$sleep_time = 2;

&get_cmd_line;
$time = 10
	unless defined $time;
&get_lockfile;
if ($locked){
	&lock;
}
else{
	&unlock if ($unlocked);
	exit 0;
}

###############################################################################

sub getlock{
	flock(LOCKFILE,$LOCK_EX);
}

###############################################################################


sub getindex{
	local($cmd) = @_;
	$index = '';
	for (0..$#lock_ary){
		local($lck_str,$perm) = split(/\|/,$lock_ary[$_]);
		if ($lck_str eq $cmd){
			next if (($unlocked) && ($perm ne $PERM));
			$index = $_;
			last;
		}
	}
}

###############################################################################


sub lock{
	open(LOCKFILE,"+>>$lockfile");
	chmod 0666, $lockfile;
	&getlock;
	&read_to_ary;
	&getindex($command);
	seek(LOCKFILE,0,2);
	if ($index =~ /^$/){
		&write_to_lock("$command|$PERM\n");
		exit 0;
	}
	else{
		exit 2 if($time <= 1);
		close(LOCKFILE);
		sleep $sleep_time;
		$time -= $sleep_time;
		&lock;
	}
}

###############################################################################


sub read_to_ary{
	local($buf);
	local($len);
	local($BUFFER) = '';
	while ($len = sysread(LOCKFILE,$buf,16384)){
		$BUFFER .= $buf;
		if (!defined $len){
			next if $! =~ /^Interrupted/;
			die "$0: System read error: $!\n";
		}
	}
	@lock_ary = split(/\n/,$BUFFER);
}

###############################################################################


sub write_to_lock{
	local($buf) = @_;
	$len = length($buf);
	$offset = 0;
	while($len){ 
	        $written = syswrite(LOCKFILE,$buf,$len,$offset);
	        die "$0: System write error: $!\n"
	                unless defined $written;
	        $len -= $written;
	        $offset += $written;
	}
}

###############################################################################

sub write_ary_tolock{
	truncate(LOCKFILE,0);
	seek(LOCKFILE,0,0);
	for (0..$#lock_ary){
		&write_to_lock("$lock_ary[$_]\n");
	}
}

###############################################################################

sub unlock{
	open (LOCKFILE,"+>>$lockfile") ||
		die "$0: Can't find the lock file ";
	die "$0: The lock file is empty" if (-z "$lockfile");
	&getlock;
	&read_to_ary;
	&getindex($command);
	exit 0 if($index =~ /^$/);
	splice(@lock_ary,$index,1);
	&write_ary_tolock;
	flock(LOCKFILE,$LOCK_UN);
}

###############################################################################


# NAME: get_cmd_line
#
# FUNCTION: gets the command line options and their arguments
#
# INPUT: avalue,bvalue,cvalue,dvalue,evalue,filename
#
# OUTPUT: avalue,bvalue,cvalue,dvalue,evalue,filename
#

###############################################################################


sub get_cmd_line{
	while ($ARGV[0] =~ /^-/){
		$_ = shift(@ARGV);
		if (/^-(l)(.*)/){
			&checkopt;
			&process_opt_arg($command);
			$locked = 1;
		}
		elsif (/^-(u)(.*)/){
			&checkopt;
			&process_opt_arg($command);
			$unlocked = 1;
		}
		elsif (/^-(p)(.*)/){
			&process_opt_arg($PERM);
		}
		elsif (/^-(t)(.*)/){
			&process_opt_arg($time);
		}
		elsif (/^-(f)(.*)/){
			&process_opt_arg($lockfile);
		}
		else {
			print "Unrecognized option: $_\n";
			&usage;
		}
	}
}

###############################################################################


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
	$_[0] = ( $2 ? $2 : shift(@ARGV));
	die "$0: $option requires a value\n" if $_[0] =~ /^-|^$/;
}

###############################################################################


sub checkopt{
	die "$0: locking and unlocking are mutually exclusive" 
		if (($locked) || ($unlocked));
}


###############################################################################

sub usage{
	local($line);
	$line = "$0 [-l <lock string> | -u <unlock string>] [-p <string>]";
	$line .= " [-t <seconds>]\n	  [-f <lockfile>]";
	die "$0: $line\n";
}

###############################################################################


sub get_lockfile{

	if ($lockfile eq ""){
		if ( $ENV{'LOCKFILE'} ne ""){
			$lockfile = $ENV{'LOCKFILE'};
		}
		else{
			$lockfile = "/tmp/lockfile";
		}	
	}
}
