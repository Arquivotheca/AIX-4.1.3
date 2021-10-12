#!/bin/ksh
#@(#)49	1.11  src/bos/usr/lib/nim/methods/c_mkpaging.sh, cmdnim, bos411, 9428A410j  5/6/94  09:39:54 

#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/methods/c_mkpaging.sh
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
DEFAULT_SIZE=32

#---------------------------- module globals    --------------------------------
REQUIRED_ATTRS="paging"
OPTIONAL_ATTRS="size"
paging=""
size=""
swapfile=""
only_one=""

#---------------------------- undo              --------------------------------
#
# NAME: undo
#
# FUNCTION:
#		removes a the swapfile on error
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
#			swapfile
#
# RETURNS: (int)
#		0							= success
#		1							= failure
#
# OUTPUT:
#-------------------------------------------------------------------------------
function undo {

	trap "" 1 2 11 15

	${RM} ${swapfile} 2>/dev/null

	[[ -n "${1}" ]] && err_from_cmd ${1}

} # end of undo

#*---------------------------- mk_paging     -----------------------------------
#
# NAME: mk_paging
#
# FUNCTION:
#		initializes the paging resource for a client
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
#			paging
#
# RETURNS: (int)
#		0							= no errors
#
# OUTPUT:
#		writes paging filename to stdout
#-----------------------------------------------------------------------------*/
function mk_paging {

	typeset -i seqno=0

	# if paging directory doesn't exist, then allocation error has occured!
	[[ ! -d ${paging} ]] && error ${ERR_FILE_ACCESS} ${paging}

	# cd to the paging directory
	cd ${paging} 2>${ERR} || err_from_cmd "cd ${paging}"

	# only one file wanted or gen another one?
	if [[ -n "${only_one}" ]]
	then

		# remove any which already exist
		${RM} ${SWAPNFS}* 2>/dev/null

	else

		# what's the lowest, unused seqno?
		while [[ -f "${SWAPNFS}${seqno}" ]]
		do
			let seqno=seqno+1
		done

	fi
	swapfile=${paging}/${SWAPNFS}${seqno}

	# was size specified?
	size=${size:-${DEFAULT_SIZE}}

	# prepare for interrupts
	undo_on_interrupt=undo

	# create the paging file
	if ${C_MKPFILE} -a location=${swapfile} -a size=${size} 2>${ERR}
	then
		:
	else
		undo "${C_MKPFILE}"
	fi

	# change perms
	${CHMOD} ${DEFAULT_PERMS} ${swapfile} 2>${ERR} || undo ${CHMOD}

	# write attr assignment in case anyone's interested
	print "paging=${swapfile##*/}"

	# success
	undo_on_interrupt=""
	return 0

} # end of mk_paging

#*---------------------------- c_mkpaging           ----------------------------
#
# NAME: c_mkpaging
#
# FUNCTION:
#		creates a paging file for the specified client
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

# signal processing
trap cleanup 0
trap err_signal 1 2 11 15

# NIM initialization
nim_init

# set parameters from command line
while getopts :a:oqv c
do
	case ${c} in

		a)		# validate the attr ass
				parse_attr_ass "${OPTARG}"

				# if host:dir format given, separate them
				if [[ ${value} = ?*:/* ]]
				then
					eval ${variable}=\"${value#*:}\"
					eval ${variable}"_host"=${value%:*}
				else
					eval ${variable}=\"${value}\"
				fi
		;;

		o)		# flag to indicate that only one paging file should be allowed
				only_one=TRUE;
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

# initialize paging for the specified client
mk_paging

# success
exit 0

