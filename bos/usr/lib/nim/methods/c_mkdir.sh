#!/bin/ksh
#@(#)24	1.16  src/bos/usr/lib/nim/methods/c_mkdir.sh, cmdnim, bos410  12/20/93  10:48:56 

#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/methods/c_mkdir.sh
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
OPTIONAL_ATTRS="perms nfsparams"
location=""
perms=""
nfsparams=""

#---------------------------- undo              --------------------------------
#
# NAME: undo
#
# FUNCTION:
#		backs out changes due to fatal error
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
#			1				= name of cmd which failed
#		global:
#
# RETURNS: (int)
#
# OUTPUT:
#-------------------------------------------------------------------------------
function undo {

	typeset cmd=${1}

	if [[ -n "${first_dir}" ]]
	then

		# remove the dir we created
		${RMDIR} ${first_dir} 2>/dev/null

	fi

	err_from_cmd ${cmd}

} # end of undo

#---------------------------- mk_file           --------------------------------
#
# NAME: mk_file
#
# FUNCTION:
#		creates an empty file with the specified permissions
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
function mk_file {

	if [[ ! -f ${location} ]]
	then

		>${location} >/dev/null 2>${ERR} || error ${ERR_SYS} "$( ${CAT} ${ERR} )"

	fi

	${CHMOD} ${perms} ${location} 2>${ERR} || err_from_cmd ${CHMOD}

} # end of mk_file

#*---------------------------- mk_dir             ------------------------------
#
# NAME: mk_dir        
#
# FUNCTION:
#		creates the specified directory
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
function mk_dir {

	typeset i=""
	typeset dir=""

	# find the first directory which doesn't exist
	IFS='/'
	set -- ${location}
	IFS=${OLDIFS}
	for i
	do
		[[ "${dir}" != / ]] && dir=${dir}/${i} || dir=${dir}${i}

		# does this dir exist?
		if [[ -d "${dir}" ]]
		then 

			# make sure it's a local filesystem
			${C_STAT} -a location="${dir}" 2>${ERR} || err_from_cmd ${C_STAT}

		else

			first_dir=${dir}
			break

		fi
	done
	
	# need to create any dirs?
	if [[ -n "${first_dir}" ]]
	then
		# create the dirs now
		${MKDIR} -p ${location} 2>${ERR} || err_from_cmd ${MKDIR}
	fi
	
	# set permissions
	perms=${perms:-${DEFAULT_PERMS}}
	${CHMOD} ${perms} ${location} 2>${ERR} || undo ${CHMOD}
	
	# export dir?
	if [[ -n "${nfsparams}" ]]
	then
		${MKNFSEXP} -d ${location} ${nfsparams} 2>${ERR} || undo ${MKNFSEXP}
	fi

	# success
	return 0

} # end of mk_dir

#*---------------------------- c_mkdir           ------------------------------
#
# NAME: c_mkdir
#
# FUNCTION:
#		creates the specified directory and exports it when specified
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

# local variables
func=mk_dir

# set parameters from command line
while getopts :a:fqv c
do
	case ${c} in

		a)		# validate the attr ass
				parse_attr_ass "${OPTARG}"

				# include the assignment for use in this environment
				eval ${variable}=\"${value}\"
		;;

		f)		# request is really for a file, not a directory
				func=mk_file
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

# set defaults
perms=${perms:-755}

# create the directory or file
eval ${func} 

# preserve info about directories NIM has created
[[ -n "${first_dir}" ]] && \
	print "first_dir=${first_dir}\ndir_created=${location}"

# success
exit 0

