#!/bin/ksh

# @(#)35 1.4  src/bos/usr/lib/nim/methods/c_ch_rhost.sh, cmdnim, bos411, 9428A410j  5/16/94  09:55:08

#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/methods/c_ch_rhost.sh
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

# ----
RHOSTS="/.rhosts" 

#---------------------------- module globals    --------------------------------
REQUIRED_ATTRS="perms hostname"
OPTIONAL_ATTRS=
perms=""
hostname=""

#---------------------------- ch_rhost
#
# NAME: ch_rhost
#
# FUNCTION:
#		actually does the work of changing .rhost file 
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
#		0 = success; 
#
#-------------------------------------------------------------------------------
function ch_rhost {

	if [[ ! -s ${RHOSTS} ]] 
	then
		if [[ ${perms} = define ]]
	  	then
 	   	print $hostname root > ${RHOSTS}.nim || error ${ERR_FILE_MOD} ${RHOSTS}.nim
		fi
	  return 0
	fi

	${AWK} -v hostname=${hostname} -v perms=${perms} \
		' 	BEGIN {	\
				gotcha = 0 \
		  	} \
			$1 == hostname	{ if (perms == "define")  \
						gotcha++;  \
					  else
						next \
			   		} \
			{print} \
			END {	\
				if (perms == "define")\
					if ( gotcha == 0 )\
						print hostname " root" \
			}
		 ' ${RHOSTS}  > ${RHOSTS}.nim
	return $?
} # end of ch_rhost

#---------------------------- c_ch_rhost
#
# NAME: c_ch_rhost
#
# FUNCTION:
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
#		0	= success
#		1	= failure
#
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

# change 
 if ch_rhost
 then
	${CP} ${RHOSTS}.nim ${RHOSTS} 
 fi

# all done
exit 0

