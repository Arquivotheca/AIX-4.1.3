#!/bin/ksh
# @(#)17	1.10  src/bos/usr/lib/nim/methods/c_nimpush.sh, cmdnim, bos411, 9428A410j  4/20/94  16:24:46
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/methods/c_nimpush.sh
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#---------------------------- nimpush.sh        --------------------------------
#
# NAME: nimpush.sh
#
# FUNCTION:
#		a ksh script which is used as a wrapper around commands which are remotely
#			executed by the NIM master
#		this command is a wrapper because the NIM remote execution mechanism
#			requires the return code from the command which nimpush will execute
#			as the first thing on stderr, and, it expects it in a specific format:
#				"rc=<exit code>"
#		nimpush accomplishes this by capturing the command's stderr output & its
#			exit status
#
# EXECUTION ENVIRONMENT:
#		nimpush is a ksh script and is invoked by the rshd, which gets called
#			from the NIM master by the nimexec function
#
# NOTES:
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#			1 						= command to execute
#			2 ->					= args to the command
#		global:
#
# RETURNS: (int)
#
# OUTPUT:
#		writes "rc=<exit code>" as the first thing on stderr; this is followed by
#			any stderr output from the command which nimpush executes
#-------------------------------------------------------------------------------

umask 022

# initialize local variables
rc=0

# include /etc/environment (so that we at least get the default PATH)
if [[ -r /etc/environment ]]
then
	. /etc/environment
	export PATH
fi

# make sure a command was specified
if [[ -z "$1" ]]
then
	print "rc=1 nimpush: no command was specified" 1>&2
	exit 1
else
	cmd=$1
	shift
fi

# make sure command is found & can be executed
exit_status=""
path=$(whence $cmd 2>/tmp/$$)
if [[ -n "$path" ]]
then

	if [[ -x $path ]] && [[ ! -d $path ]]
	then

		# execute the specified command & direct its stderr into a tmp file
		oldIFS=$IFS
		IFS="	"
		$cmd $@ 2>/tmp/$$
		rc=$?
		IFS=$oldIFS
		exit_status="rc=$rc"

		# any stderr output?
		if [[ -s /tmp/$$ ]]
		then

			# is "rc=[0-9]" already part of the stderr output?
			[[ -z "$(/usr/bin/egrep ^rc=[0-9]+ /tmp/$$)" ]] && \
				print ${exit_status} 1>&2

			# write the stderr output
			/usr/bin/cat /tmp/$$ 1>&2
			
		else

			# just write the exit status
			print ${exit_status} 1>&2

		fi

	else

		# command not executable
		print "rc=1 nimpush: $cmd not executable" 1>&2
		rc=1

	fi

else

	# command not found
	print "rc=1 nimpush: $cmd not found" 1>&2
	rc=1

fi

# remove the tmp file
/usr/bin/rm /tmp/$$ >/dev/null 2>&1

# bye-bye
exit $rc

