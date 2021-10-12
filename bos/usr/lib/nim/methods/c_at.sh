#!/bin/ksh
#@(#)14	1.6  src/bos/usr/lib/nim/methods/c_at.sh, cmdnim, bos411, 9428A410j  9/27/93  13:24:37 

#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/methods/c_at.sh
#		
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

#*---------------------------- cleanup           ------------------------------
#
# NAME: cleanup
#
# FUNCTION:
#		exit processing
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#		global:
#
# RETURNS: (int)
#
# OUTPUT:
#-----------------------------------------------------------------------------*/
function cleanup {

	/usr/bin/rm  /tmp/$$ >/dev/null 2>&1

} # end of cleanup

#*---------------------------- error             ------------------------------
#
# NAME: error  
#
# FUNCTION:
#		prints out error info & exits
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#		global:
#
# RETURNS: (int)
#
# OUTPUT:
#-----------------------------------------------------------------------------*/
function error {

	if [[ $# > 0 ]]
	then
		print "\n$*\n" 1>&2
	elif [[ -s /tmp/$$ ]]
	then
		cat /tmp/$$ 1>&2
	fi

	cleanup

	exit 1

} # end of error

#*---------------------------- c_at              ------------------------------
#
# NAME: c_at   
#
# FUNCTION:
#		schedules a cron job
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#		global:
#
# RETURNS: (int)
#		0							= no errors
#		>0							= failure
#
# OUTPUT:
#-----------------------------------------------------------------------------*/

trap cleanup 0
trap error 1 2 11 15
umask 022

PROG=${0}
REQUIRED="at command name"
AT="/usr/bin/at"

#
# initialize the environment
#
	#
	# Assign the values passed to us. 
	#
	for arg
	do
		if [[ ${arg} != ?*=?* ]]
		then
			error "\ninvalid attribute assignment: ${arg}"
		fi

		#
		# separate the attr from the value
		#
		attr=${arg%=*}
		value=${arg#*=}

		#
		# make the assignment
		#
		eval $attr=\$value || error "unable to make assignment for: ${arg}"
	done

	#
	# Make sure we have ALL REQUIRED Parms. 
	#
	for needed in ${REQUIRED}
	do
		eval [[ -z \$${needed} ]] &&  error "missing attribute: ${needed}"
	done

#
# schedule the specified command
#
/usr/bin/echo ${command} ${name} | ${AT} -k -t ${at} 2>/tmp/$$ || error "unable to AT: ${command}"

#
# print out job number (second field from AT stdout)
#
/usr/bin/awk '{ print $2 }' /tmp/$$ 2>/dev/null

#
# success
#
exit 0

