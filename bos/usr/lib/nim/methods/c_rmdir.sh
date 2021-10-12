#!/bin/ksh
#@(#)50	1.13  src/bos/usr/lib/nim/methods/c_rmdir.sh, cmdnim, bos411, 9428A410j  5/6/94  09:41:20 

#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/methods/c_rmdir.sh
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

# include common NIM shell defines/functions
NIMPATH=${0%/*}
NIMPATH=${NIMPATH%/*}
[[ ${NIMPATH} = ${0} ]] && NIMPATH=/usr/lpp/bos.sysmgt/nim
NIM_METHODS="${NIMPATH}/methods"
. ${NIM_METHODS}/c_sh_lib

#---------------------------- local defines     --------------------------------

#---------------------------- module globals    --------------------------------
REQUIRED_ATTRS="location"
OPTIONAL_ATTRS="nfs_cmd"
location=""
nfs_cmd=""

#---------------------------- c_rmdir.sh        --------------------------------
#
# NAME: c_rmdir.sh
#
# FUNCTION:
#		removes the specified directory
#		optionally, executes the specified NFS command
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#		calls error on failure
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#		global:
#
# RETURNS: (int)
#		0							= success
#		1							= failure
#
# OUTPUT:
#-------------------------------------------------------------------------------

# signal processing
trap cleanup 0
trap err_signal 1 2 11 15

# NIM initialization
nim_init

# initialize local variables
typeset c=""

# set parameters from command line
while getopts :a:qv c
do
	case ${c} in

		a)		# validate the attr ass
				parse_attr_ass "${OPTARG}"

				# include the assignment for use in this environment
				eval ${variable}=\"${value}\"
				;;

		q)		# show attr info
				cmd_what
				exit 0
				;;

		v)		# verbose mode (for debugging)
				set -x
				for i in $(typeset +f)
				do
					typeset -ft $i
				done
				;;

		\?)	# unknown option
				error ${ERR_BAD_OPT} ${OPTARG}
				;;
	esac
done

# check for missing attrs
ck_attrs

# make sure it's ok to remove the directory
protected_dir ${location} && \
	error ${ERR_SYS} "\"${location}\" is protected from removal"

# make sure we're in a neutral directory
cd /tmp 2>${ERR} || err_from_cmd cd

# remove the directory
${RM} -r ${location} 2>${ERR} || err_from_cmd "${RM} -r ${location}"

# any NFS command to execute?
if [[ -n "${nfs_cmd}" ]]
then
	${nfs_cmd} 2>${ERR} || warning_from_cmd "${nfs_cmd}"
fi

# all done
exit 0

