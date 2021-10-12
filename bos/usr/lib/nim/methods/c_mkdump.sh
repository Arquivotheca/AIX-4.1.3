#!/bin/ksh
#@(#)50	1.7  src/bos/usr/lib/nim/methods/c_mkdump.sh, cmdnim, bos411, 9428A410j  12/20/93  10:21:46 

#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/methods/c_mkdump.sh
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

	/usr/bin/rm -r /tmp/$$* 2>/dev/null

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

	# change to a neutral directory
	cd /tmp

	# error messages
	if [[ $# > 0 ]]
	then
		print "\nc_mkdump: $*\n" 1>&2
	fi

	if [[ -s /tmp/$$.err ]]
	then
		print "\nc_mkdump: error returned from command; output follows" 1>&2
		/usr/bin/cat /tmp/$$.err 1>&2
	elif [[ $# = 0 ]]
	then
		print "\nc_mkdump: unspecified fatal error encountered\n" 1>&2
	fi

	# remove the dump file
	/usr/bin/rm ${dump}/dump 2>/dev/null

	# undo what we've done
	if [[ -n "${dir_created}" ]]
	then
		# remove the directory we created
		eval ${C_RMDIR} \$${ATTR_LOCATION}=${dump} 2>/dev/null
	fi

	cleanup

	exit 1

} # end of error

#*---------------------------- interrupted        ------------------------------
#
# NAME: interrupted   
#
# FUNCTION:
#		catches interrupt signals
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
function interrupted {

	error "interrupt signal received - aborting processing"

} # end of interrupted

#*---------------------------- mk_dump     -----------------------------------
#
# NAME: mk_dump
#
# FUNCTION:
#		initializes the dump resource for a client
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
function mk_dump {

	# does dump directory already exist?
	if [[ ! -d "${dump}" ]]
	then
		# create it
		${C_MKDIR} -alocation=${dump} -aperms=${DEFAULT_PERMS} \
			-anfsparams="-t rw -c ${hostname} -B" 2>/tmp/$$.err || return 1

		dir_created=true
	fi

	# create an empty file in the dump directory
	# this is required because of the way remote dump works
	>${dump}/dump

	# success
	return 0

} # end of mk_dump

#*---------------------------- c_mkdump           ----------------------------
#
# NAME: c_mkdump
#
# FUNCTION:
#		creates a dump file for the specified client
#		will also create the parent directory if it doesn't already exist
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
trap interrupted 1 2 11 15
umask 022

# local defines
NIM_AWK="/usr/lpp/bos.sysmgt/nim/awk"
NIM_METHODS="/usr/lpp/bos.sysmgt/nim/methods"
C_MKDIR=${NIM_METHODS}/c_mkdir
C_RMDIR=${NIM_METHODS}/c_rmdir
SWAPNFS="swapnfs"
PERMS="777"
DEFAULT_SIZE=32
ATTR_LOCATION=location
ATTR_PERMS=perms
ATTR_NFSPARAMS=nfsparams

REQUIRED="name hostname dump"
OPTIONAL="dbg"

# global variables
PROG=${0}
OLDIFS=${IFS}
dir_created=""

# Assign the attrs passed to us. 
for arg
do
	if [[ ${arg} != ?*=?* ]]
	then
		error "\ninvalid attribute assignment: ${arg}"
	fi

	# separate the attr from the value
	attr=${arg%=*}
	value=${arg#*=}

	# make the assignment
	eval $attr=\$value || error "unable to make assignment for: ${arg}"
done

# Make sure we have ALL REQUIRED attrs. 
for needed in ${REQUIRED}
do
	eval [[ -z \$${needed} ]] &&  error "missing attribute: ${needed}"
done

# debug mode?
if [[ -n "${dbg}" ]]
then
	for i in $(typeset +f)
	do
		typeset -ft $i
	done
	set -x
fi

# append the client's NIM name to the dump location
dump=${dump}/${name}

# initialize dump
mk_dump || error

# success
exit 0

