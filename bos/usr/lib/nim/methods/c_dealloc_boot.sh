#!/bin/ksh
#@(#)87	1.9  src/bos/usr/lib/nim/methods/c_dealloc_boot.sh, cmdnim, bos411, 9428A410j  3/16/94  16:21:27 

#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/methods/c_dealloc_boot.sh
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
REQUIRED_ATTRS="hostname"
hostname=""

#---------------------------- c_dealloc_boot    --------------------------------
#
# NAME: c_dealloc_boot
#
# FUNCTION:
#		removes the boot resources for the specified target
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

# remove target files from TFTPBOOT dir
${RM} "${TFTPBOOT}/${hostname}" 1>/dev/null 2>&1
${RM} "${TFTPBOOT}/${hostname}.info" 1>/dev/null 2>&1

# remove the BOOTPTAB stanza
${AWK} -F ":" -vhostname=${hostname} '$1!=hostname{print}' ${BOOTPTAB} \
	>${TMPDIR}/bootptab 2>/dev/null && ${CAT} ${TMPDIR}/bootptab >${BOOTPTAB}

exit 0
