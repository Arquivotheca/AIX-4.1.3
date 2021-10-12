#!/bin/ksh
#@(#)48	1.4  src/bos/usr/lib/nim/methods/c_bosinst_env.sh, cmdnim, bos411, 9428A410j  3/24/94  18:21:50 

#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/methods/c_bosinst_env.sh
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
REQUIRED_ATTRS=""
OPTIONAL_ATTRS="hostname"
hostname=""
allocate=""
deallocate=""

#---------------------------- setup_env         --------------------------------
#
# NAME: setup_env 
#
# FUNCTION:
#		initializes the BOS install environment for NIM
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
function setup_env {

	typeset i=""
	typeset stanza=""

	# copy config files from RAM filesystem into new, local filesystem
	${CP} /SPOT/niminfo /etc/niminfo
	${CP} /../sbin/helpers/nfsmnthelp /sbin/helpers/nfsmnthelp
	[[ ! -s ${HOSTS} ]] && ${CP} /../etc/hosts ${HOSTS}

	# for each NIM_HOSTS
	for i in ${NIM_HOSTS}
	do

		# separate IP from hostname
		IFS=':'
		set ${i} --
		IFS=${OLD_IFS}

		# is the hostname already in the /etc/hosts file?
		stanza=$( ${AWK} -v host="${2}" '$2==host || $3==host {print}' ${HOSTS} )
		[[ -z "${stanza}" ]] && print "${1}\t${2}" >>${HOSTS}

	done
	
	# set client's hostname
	${HOSTNAME} ${hostname}
	
	# make sure we've got our dir
	[[ ! -d /var/adm/nim ]] && ${MKDIR} -p /var/adm/nim
	
	# re-mount the LOG?
	if [[ -n "${NIM_LOG}" ]]
	then
	
			nim_mount ${NIM_LOG} /var/adm/ras
	
			# reset the mount list so it doesn't get unmounted upon exit
			mounts=""
	fi

} # end of setup_env

#---------------------------- alloc_bos         --------------------------------
#
# NAME: alloc_bos 
#
# FUNCTION:
#		allocates the specified BOS image resource and updates the niminfo file
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
#			res					= resource to alloc in form of <type>=<name>
#		global:
#
# RETURNS: (int)
#		0							= success
#		1							= failure
#
# OUTPUT:
#-------------------------------------------------------------------------------
function alloc_bos {

	typeset res_type=${1%%=*}
	typeset res_name=${1##*=}

	# make sure the /NIM_BOS_IMAGE directory exists
	if [[ ! -d /NIM_BOS_IMAGE ]]
	then

		${MKDIR} /NIM_BOS_IMAGE 2>${ERR} || err_from_cmd ${MKDIR}

	fi

	# use nimclient to allocate the resource
	${NIMCLIENT} -o allocate -a ${res_type}=${res_name} -a force=yes \
			-a ignore_state=yes -a mount_ctrl=/NIM_BOS_IMAGE 2>${ERR} || \
		err_from_cmd ${NIMCLIENT}

	# update the niminfo file
	${C_NIMINFO} -a location=/SPOT/niminfo NIM_BOS_IMAGE=/NIM_BOS_IMAGE \
		NIM_BOS_FORMAT=${res_type} 2>${ERR} || err_from_cmd ${C_NIMINFO}

} # end of alloc_bos

#---------------------------- dealloc_bos       --------------------------------
#
# NAME: dealloc_bos 
#
# FUNCTION:
#		deallocates the specified BOS image resource and updates the niminfo file
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
#			res					= resource to dealloc in form of <type>=<name>
#		global:
#
# RETURNS: (int)
#		0							= success
#		1							= failure
#
# OUTPUT:
#-------------------------------------------------------------------------------
function dealloc_bos {

	typeset res_type=${1%%=*}
	typeset res_name=${1##*=}

	# use nimclient to deallocate the resource
	${NIMCLIENT} -o deallocate -a ${res_type}=${res_name} -a force=yes \
			-a ignore_state=yes -a mount_ctrl=/NIM_BOS_IMAGE 2>${ERR} || \
		err_from_cmd ${NIMCLIENT}

	# update the niminfo file
	${C_NIMINFO} -a location=/SPOT/niminfo NIM_BOS_IMAGE= \
		NIM_BOS_FORMAT= 2>${ERR} || err_from_cmd ${C_NIMINFO}

} # end of dealloc_bos

#---------------------------- c_bosinst_env     --------------------------------
#
# NAME: c_bosinst_env
#
# FUNCTION:
#		during a BOS install, everything NIM needs is present in the RAM
#			filesystem
#		however, once BOS install has installed the client's local filesystem, it
#			will umount the temporary mount points for those filesystems, then
#			mount the local filesystem at their normal mount points (eg, /, /usr)
#		when that happens, NIM looses access to the files it needs that are
#			still in the RAM filesystem
#		so, this method is called by BOS install so that NIM can setup the
#			local filesystem to support further NIM processing
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
#		0							= success
#
# OUTPUT:
#-------------------------------------------------------------------------------

# signal processing
trap cleanup 0
trap err_signal 1 2 11 15

# NIM initialization
nim_init

# initialize local variables

# set parameters from command line
while getopts :a:A:D:qv c
do
	case ${c} in

		a)		# validate the attr ass
				parse_attr_ass "${OPTARG}"

				# include the assignment for use in this environment
				eval ${variable}=\"${value}\"
		;;

		A)		# allocate resource
				allocate=${OPTARG}
		;;

		D)		# deallocate resource
				deallocate=${OPTARG}
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

# setup environment or perform allocation/deallocation?
if [[ -n "${hostname}" ]]
then

	# setup environment to support NIM processing
	setup_env

elif [[ -n "${allocate}" ]]
then

	alloc_bos "${allocate}"

elif [[ -n "${deallocate}" ]]
then

	dealloc_bos "${deallocate}"

fi

# success
exit 0


