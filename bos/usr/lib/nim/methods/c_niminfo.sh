#!/bin/ksh
#@(#)44	1.13  src/bos/usr/lib/nim/methods/c_niminfo.sh, cmdnim, bos411, 9428A410j  5/6/94  09:40:22 

#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/methods/c_niminfo.sh
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
REQUIRED_ATTRS=""
OPTIONAL_ATTRS="location"
location=""

#---------------------------- undo              --------------------------------
#
# NAME: undo
#
# FUNCTION:
#		puts back original version of the niminfo file
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
#
# OUTPUT:
#-------------------------------------------------------------------------------
function undo {

	trap "" 1 2 11 15

	# does backup exist?
	if [[ -s ${TMPDIR}/backup ]]
	then

		${CAT} ${TMPDIR}/backup >${location} 2>${ERR} || warning_from_cmd ${CAT}

	else

		${RM} ${location} 2>/dev/null

	fi

	[[ -n "${1}" ]] && err_from_cmd ${1}

} # end of undo

#*---------------------------- c_niminfo           -----------------------------
#
# NAME: c_niminfo
#
# FUNCTION:
#		adds or changes the specified variable in the specified file
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
while getopts :a:qv c
do

	case ${c} in

		a)		# validate the attr ass
				parse_attr_ass "${OPTARG}"

				# make the assignment
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
shift OPTIND-1

# set defaults
location=${location:-${NIMINFO:-/etc/niminfo}}

# backup current version (if it exists)
if [[ -s ${location} ]]
then

	${CP} ${location} ${TMPDIR}/backup 2>${ERR} || err_from_cmd ${CP}

else

	>${location}

fi

# prepare for interrupts
undo_on_interrupt=undo

# for each operand specified on the command line
for i
do

	# must be in the form of <name=value>
	[[ ${i} != ?*=* ]] && undo "invalid assignment: \"${i}\""

	# separate name from value
	name=${i%%=*}
	value=${i##*=}

	# change the value
	${AWK} -v name="${name}" -v value="${value}" '\
		END{if((f==0)&&(value!="")) print "export " name "=\"" value "\""};\
		/^export .*=.*/{	split($2,a,"=");\
								if(a[1]==name)\
								{	if(value!="")\
										{print "export " name "=\"" value "\"";f=1;};\
									next;\
								};\
							};\
		{print;}' ${location} >${TMPDIR}/niminfo 2>${ERR} || undo ${AWK}

	# replace old file with new
	${CAT} ${TMPDIR}/niminfo >${location} 2>${ERR} || undo ${CAT}

done

# success
undo_on_interrupt=""
exit 0

