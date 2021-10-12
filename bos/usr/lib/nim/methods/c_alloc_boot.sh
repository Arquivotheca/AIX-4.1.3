#!/bin/ksh

# @(#)99	1.13  src/bos/usr/lib/nim/methods/c_alloc_boot.sh, cmdnim, bos41J, 9516B_all  4/21/95  12:55:52

#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/methods/c_alloc_boot.sh
#		
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
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
BOOTPTAB_TOK_STR="token-ring"
BOOTPTAB_ETH_STR="ethernet"
BOOTPTAB_FDDI_STR="fddi"

# NOTE - order is important here, as "ht" MUST come before "ha" or the BOOTP
#		daemon will barf
TAB_ENTRIES="bf ip ht ha sa gw sm"

#---------------------------- module globals    --------------------------------
REQUIRED_ATTRS="hostname bf ip ht sa"
OPTIONAL_ATTRS="ha gw sm"
hostname=""
bf=""
ip=""
ha=""
ht=""
sa=""
gw=""
sm=""

#---------------------------- alloc_boot        --------------------------------
#
# NAME: alloc_boot
#
# FUNCTION:
#		allocates a NIM boot image by:
#			1) creating sym link to target's boot image
#			2) adding an entry in the BOOTPTAB for the client
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
function alloc_boot {

	typeset boot_image=${TFTPBOOT}/${hostname}
	typeset tags=""

	# verify that SPOT has specified boot image
	[[ ! -s ${bf} ]] && error ${ERR_FILE_ACCESS} ${bf}

	# if client already has BOOTPTAB stanza, cut it out
	if [[ -s ${BOOTPTAB} ]]
	then

		${AWK} -vhostname=${hostname} 'BEGIN{FS=":"};$1!=hostname{print}' \
			${BOOTPTAB} >${TMPDIR}/bootptab 2>${ERR} || err_from_cmd ${AWK}

	fi

	# if client already has boot image, remove sym link
	${RM} ${boot_image} 2>/dev/null

	# create sym link to the boot image
	${LN} -s ${bf} ${boot_image} 2>${ERR} || err_from_cmd ${LN}
	bf=${boot_image}

	# convert the ht value to a valid bootpd ht value
	case ${ht} in 
		tok)	ht=${BOOTPTAB_TOK_STR} ;;
		ent)	ht=${BOOTPTAB_ETH_STR} ;;
		fddi)	ht=${BOOTPTAB_FDDI_STR} ;;
		*)		error ${ERR_SYS} "unknown network adapter type \"${ht}\"";;
	esac

	# add the entry to the bootptab file.
	stanza="${hostname}:"
	for tags in ${TAB_ENTRIES}
	do
		eval [[ -z \$${tags} ]] || eval stanza="\${stanza}${tags}=\$${tags}:"
	done
	if print ${stanza} >>${TMPDIR}/bootptab 2>${ERR}
	then
		${CAT} ${TMPDIR}/bootptab >${BOOTPTAB} 2>${ERR} || err_from_cmd ${CAT}
	else
		error ${ERR_FILE_MOD} ${BOOTPTAB}
	fi

} # end of alloc_boot

# ---------------------------- c_alloc_boot    -------------------------------- 
#
# NAME: c_alloc_boot
#
# FUNCTION:
#	
#
# DATA STRUCTURES:
#  parameters:
#	All parameters MUST be passed as valid environment varible 
#	assignment syntax (ie. name=value.. )
#	
#  REQUIRED: 
#	hostname	-- The clients NIM name
#	spot 	-- Name of the SPOT
#	ip   	-- Clients IP address
#	ht   	-- Hardware type
#	sa   	-- server IP address put in bootp reply packet
#
#  Optional: 
#	ha   	-- Hardware address
#	gw   	-- Gateways
#	sm   	-- Netmask
#					
# RETURNS: (int)
#	0	= no errors
#	>0	= failure
#
# --------------------------------------------------------------------------- 

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

# alloc boot resource
alloc_boot

exit 0
