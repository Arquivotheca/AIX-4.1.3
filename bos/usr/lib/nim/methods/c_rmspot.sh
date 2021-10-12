#!/bin/ksh
# @(#)53	1.8  src/bos/usr/lib/nim/methods/c_rmspot.sh, cmdnim, bos411, 9428A410j  5/6/94  09:38:33
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/methods/c_rmspot.sh
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

# include common NIM shell defines/functions
NIMPATH=${0%/*}
NIMPATH=${NIMPATH%/*}
[[ ${NIMPATH} = ${0} ]] && NIMPATH=/usr/lpp/bos.sysmgt/nim
NIM_METHODS="${NIMPATH}/methods"
. ${NIM_METHODS}/c_sh_lib

#---------------------------- local defines     --------------------------------

#---------------------------- module globals    --------------------------------
REQUIRED_ATTRS="location name"
OPTIONAL_ATTRS="exported"
location=""
name=""
exported=""

#---------------------------- rm_spot     --------------------------------------
#
# NAME: rm_spot
#
# FUNCTION:
#		removes SPOT specific files
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
#
# OUTPUT:
#-------------------------------------------------------------------------------
function rm_spot {

	typeset inst_root=${location}/${INST_ROOT}

	# first, make sure we're in a neutral directory
	cd /tmp 2>${ERR} || err_from_cmd cd

	# unexport the SPOT?
	if [[ -n "${exported}" ]]
	then

		${RMNFSEXP} -d ${location} 2>${ERR} || warning_from_cmd ${RMNFSEXP}

	fi

	# any boot images to remove?
	if [[ -n "$(${LS} ${TFTPBOOT}/${name}.* 2>/dev/null)" ]]
	then

		# remove all the boot images
		# NOTE - important to continue if any error encountered here
		${RM} ${TFTPBOOT}/${name}.* 2>${ERR} || warning_from_cmd ${RM}

	fi

	# is this a /usr SPOT?
	if [[ ${location} = /usr ]]
	then

		# remove the INST_ROOT files
		protected_dir ${inst_root} || ${RM} -r ${inst_root} 2>/dev/null

		# remove variable from NIMINFO file
		${C_NIMINFO} -alocation=${NIMINFO} NIM_USR_SPOT= 2>${ERR} || \
			err_from_cmd ${C_NIMINFO}

	else

		# SPOT is NOT this machine's /usr filesystem - ok to remove it
		# strip "/usr" off of path if it was given
		[[ ${location} = ?*/usr ]] && location=${location%/*}

		# remove the SPOT
		# let's make SURE it is ok
		if protected_dir ${location}
		then

			# do NOT remove this dir
			:

		else

			# remove the SPOT
			${RM} -r ${location} 2>${ERR} || err_from_cmd ${RM}

		fi

	fi

} # end of rm_spot

#---------------------------- c_rmspot          --------------------------------
#
# NAME: c_rmspot
#
# FUNCTION:
#		removes the specified SPOT
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
#		0							= SPOT removed
#		1							= error encountered - message on stderr
#
# OUTPUT:
#-------------------------------------------------------------------------------

# signal processing
trap cleanup 0
trap err_signal 1 2 11 15

# NIM initialization
nim_init

# initialize local variables
location=
source=

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

# remove the SPOT
rm_spot

# all done
exit 0

