#!/bin/ksh
#@(#)47	1.10  src/bos/usr/lib/nim/methods/c_script.sh, cmdnim, bos411, 9428A410j  6/6/94  18:20:09 

#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/methods/c_script.sh
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
export NIMPATH NIM_METHODS
. ${NIM_METHODS}/c_sh_lib

#---------------------------- local defines     --------------------------------

#---------------------------- module globals    --------------------------------
REQUIRED_ATTRS="location"
OPTIONAL_ATTRS=""
location=""

#*---------------------------- c_script           ----------------------------
#
# NAME: c_script
#
# FUNCTION:
#		executes the specified shell script
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

# signal processing
trap cleanup 0
trap err_signal 1 2 11 15

# NIM initialization
nim_init

# initialize local variables
source=
DBG=
rc=

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
				DBG=TRUE
		;;

		\?)	# unknown option
				error ${ERR_BAD_OPT} ${OPTARG}
		;;
esac
done

# check for missing attrs
ck_attrs

# ensure local access to the script (local path returned via access_pnt)
nim_mount ${location}

# attempt to copy it
if ${CP} ${access_pnt} ${TMPDIR}/script >/dev/null 2>&1
then

	# make sure the script is executable
	if ${CHMOD} +x ${TMPDIR}/script >/dev/null 2>&1
	then

		# use local copy
		# unmount the source
		nim_unmount ${access_pnt}

		# set new access point
		access_pnt="${TMPDIR}/script"

	fi

fi

# execute the script
if ${access_pnt} 2>${ERR}
then

	if [[ -n "${DBG}" ]]
	then

		${CAT} ${ERR} 1>&2

	fi

else

	# one or more failures encountered inside of the script
	err_from_cmd ${location}

fi

# success
exit 0

