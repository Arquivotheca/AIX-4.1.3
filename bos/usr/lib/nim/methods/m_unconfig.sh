#!/bin/ksh
#
#   @(#)53 1.3  src/bos/usr/lib/nim/methods/m_unconfig.sh, cmdnim, bos411, 9430C411a  7/29/94  11:44:07
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/methods/m_unconfig.sh
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

# ------------------------- module globals    --------------------------------
REQUIRED_ATTRS=""
OPTIONAL_ATTRS=""

# ------------------------- Locals
NIMDB=/etc/NIMdb
NIMINFOPREV=/etc/niminfo.prev

# ------------------------- unconfig
#
# NAME: unconfig
#
# FUNCTION:
#	Unconfigure the master. (To allow installp to remove the 
#	bos.sysmgt.nim.master fileset )
#
#
# ----------------------------------------------------------------------------

function unconfig {
	[[ ${1} != "master" ]] && error ${ERR_VALUE_CONTEXT} ${1} name 
	${ODMGET} nim_object > ${NIMDB}
	${ODMGET} nim_attr >> ${NIMDB}
	${ODMGET} nim_pdattr > ${NIMDB}.pd
	${RM} ${NIMINFO}

        # Copy any saved off, previously existing niminfo file back.
        # (Do NOT mv the file since deinstall of master fileset will have
        # nothing to recover after removing /etc/niminfo).
        [[ -f ${NIMINFOPREV} ]] && ${CP} ${NIMINFOPREV} ${NIMINFO}

}
# ------------------------- m_unconfig
#
# NAME: m_unconfig
#
# FUNCTION:
#	Unconfigure the master. (To allow installp to remove the 
#	bos.sysmgt.nim.master fileset )
#
# NOTES:
#		calls error on failure
#
# DATA STRUCTURES:
#		parameters:
#		global:
#
# RETURNS: (int)
#		0 = Unconfig OK
#		1 = failure
#
# ----------------------------------------------------------------------------

# signal processing
trap cleanup 0
trap err_signal 1 2 11 15

# NIM initialization
nim_init

# initialize local variables
typeset c=""

# set parameters from command line
while getopts :a:vq c
do
	case ${c} in

		a)		# validate the attr ass
				parse_attr_ass "${OPTARG}"

				eval ${variable}=\"${value}\"
				;;

		q)		cmd_what
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

#
unconfig ${1}

# all done
exit 0

