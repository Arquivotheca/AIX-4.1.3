#!/bin/ksh

# @(#)45	1.22.1.1  src/bos/usr/lib/nim/methods/c_mkbooti.sh, cmdnim, bos411, 9430C411a  7/22/94  11:40:55

#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/methods/c_mkbooti.sh
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
IF_SUPPORTED="if_supported"
NETDIR="lib/boot/network"
KERNELS='lib/boot'
PROTO='lib/boot/network/*.proto'

#---------------------------- module globals    --------------------------------
REQUIRED_ATTRS="location name platform"
OPTIONAL_ATTRS="debug"
location=""
name=""
debug=""
preview=""
cmd="${BOSBOOT} -a"
typeset -i tmp_needed=0
typeset -i tftpboot_needed=0
typeset -i num_booti=0
enter_dbg_mp=""
enter_dbg_up=""

#---------------------------- turn_debug_on     --------------------------------
#
# NAME: turn_debug_on
#
# FUNCTION:
#		makes the required changed to enable the kernel debugger in the network
#			boot images
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
#			cmd
#
# RETURNS: (int)
#		0							= success
#
# OUTPUT:
#		writes enter_dbg symbol addresses to stdout
#-------------------------------------------------------------------------------
function turn_debug_on {

	typeset kernel=""
	typeset kernel_type=""
	typeset enter_dbg=""

	# we want a network boot image which has the kernel debugger enabled
	cmd="${cmd} -I"

	# turn on debugging in the rc.boot file
	if ${AWK} '$0 !~ /export NIM_DEBUG=/{print};\
				/[ 	]*unset NIM_DEBUG/{print "export NIM_DEBUG=\"set -x\""}'\
		${RCBOOT} >${TMPDIR}/rc.boot 2>${ERR}
	then

		${CAT} ${TMPDIR}/rc.boot >${RCBOOT} 2>${ERR} || err_from_cmd ${CAT}

	fi

	# return the enter_dbg info
	for kernel in ${location}/${KERNELS}/unix_[um]p
	do

		# what type of kernel?
		kernel_type=${kernel##*_}

		enter_dbg=""
		enter_dbg=$( ${NM} -x ${kernel} | ${GREP} enter_dbg | \
						 ${AWK} 'NR==1{print $3}' 2>/dev/null )

		# remember the enter_dbg address
		eval enter_dbg_${kernel_type}=${enter_dbg}

	done

} # end of turn_debug_on

#---------------------------- turn_debug_off    --------------------------------
#
# NAME: turn_debug_off
#
# FUNCTION:
#		changes the appropriate files to disable the kernel debugger for the
#			network boot images
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
#			cmd
#
# RETURNS: (int)
#		0							= success
#
# OUTPUT:
#-------------------------------------------------------------------------------
function turn_debug_off {

	# we don't want the kernel debugger enabled
	cmd="${cmd}"

	# turn off debugging in the rc.boot file
	if ${AWK} '$0 !~ /export NIM_DEBUG=/{print}' ${RCBOOT} \
		>${TMPDIR}/rc.boot 2>${ERR}
	then

		${CAT} ${TMPDIR}/rc.boot >${RCBOOT} 2>${ERR} || err_from_cmd ${CAT}

	fi

} # end of turn_debug_off

#---------------------------- preview_data      --------------------------------
#
# NAME: preview_data
#
# FUNCTION:
#		reads the "bosboot -q" information and writes the information to stdout
#			when parameter #1 is non-NULL
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
#			1						= when non-NULL, write preview data
#		global:
#
# RETURNS: (int)
#		0							= success
#
# OUTPUT:
#		writes preview data to stdout
#-------------------------------------------------------------------------------
function preview_data {

	typeset write_data=${1}
	typeset line=""
	typeset -i new_tmp=0

	# write the preview data or read new info?
	if [[ -n "${write_data}" ]]
	then

		# write the data
		print "tmp_needed=${tmp_needed}"
		print "tftpboot_needed=${tftpboot_needed}"
		print "num_booti=${num_booti}"

	else

		# parse the "bosboot -q" output
		${CAT} ${ERR} | \
		while read line
		do
			set -- ${line}
			case ${1} in

				/tmp)
					(( ${2} > tmp_needed )) && let tmp_needed=${2}
				;;

				/|/tftpboot)
					(( ${2} > tftpboot_needed )) && let tftpboot_needed=${2}
				;;

			esac
		done

		let num_booti+=1

	fi

} # end of preview_data

#---------------------------- mkbooti           --------------------------------
#
# NAME: mkbooti
#
# FUNCTION:
#		actually does the work of creating network boot images
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
#			cmd
#
# RETURNS: (int)
#		0							= success; boot images created
#
# OUTPUT:
#		writes IF_SUPPORTED attr assignments to stdout
#-------------------------------------------------------------------------------
function mkbooti {

	typeset proto_files=""
	typeset pf=""
	typeset if_type=""
	typeset platform_type=""
	typeset kernel=""
	typeset bi_name=""
	typeset if_supported=""
	typeset enter_dbg=""
	typeset prefix_choices="tok ent fddi --"
	typeset prefix=""
	typeset types=""
	typeset supported_platform=""
	typeset boot_image_created=""

	# generate list of proto files which reside in SPOTname/usr/lib/boot/network
	proto_files="${location}/${PROTO}"

	if [[ -z "${preview}" ]]
	then

		#
		# We're committed to re-building the boot images, so 
		# delete all existing boot images for this SPOT. 
		#
		${RM} ${TFTPBOOT}/${name}.* 2>${ERR} || err_from_cmd ${RM}

	fi

	if [[ -z "${preview}" ]]
	then

		# was debug specified?
		if [[ -n "${debug}" ]]
		then

			# we want a network boot image which has the kernel debugger enabled
			turn_debug_on

		else

			# we don't want the kernel debugger enabled
			turn_debug_off

		fi

	fi

	#
	# For each proto file in the SPOT...
	#
	for pf in ${proto_files}
	do
		#
		# Skip any bad proto files
		#
		[[ ! -s ${pf} ]] && continue


		# expecting proto files to be named like:
		#		<platform>.<interface type>.proto
		# ignore the parent directoy pathname & separate the proto filename 
		#		into these parts
		IFS='.'
		set -- ${pf##*/}
		IFS=${OLD_IFS}

		# how many fields in the proto filename?
		if [[ $# -eq 2 ]]
		then

			# assume the platform is rs6k
			platform_type=rs6k
			if_type=${1}

		elif [[ $# -eq 3 ]]
		then

			platform_type=${1}
			if_type=${2}

		else

			warning ${ERR_VALUE} ${pf} ".proto filename"
			continue

		fi

		# interface type must exist in predefined device database or bosboot
		#		will barf
		[[ -z "$( ${ODMGET} -qprefix=${if_type} PdDv 2>/dev/null )" ]] && continue

		# platform must match one of the types specified in the platform attr
		#		which was passed by the caller
		for types in ${platform}
		do

			supported_platform=${types%%=*}
			kernel=${location}/${KERNELS}/${types##*=}

			# does this proto file match a supported platform type?
			[[ ${platform_type} != ${supported_platform} ]] && continue

			# skip bad kernels
			[[ ! -s ${kernel} ]] && continue

			# construct a boot image pathname like this:
			#		/tftpboot/<SPOTname>.<platform type>.<network interface type>
			bi_name="${TFTPBOOT}/${name}.${platform_type}.${if_type}"

			# create a network boot image
			if ${cmd}	-b ${bi_name} \
							-d /dev/${if_type} \
							-T ${platform_type} \
							-p ${pf} \
							-k ${kernel} 2>${ERR} 1>&2
			then

				# just previewing info?
				if [[ -n "${preview}" ]]
				then

					preview_data
					continue

				fi

				# print out the attr assignment so that m_instspot can add it to
				#		the SPOTs definition so that NIM knows what kind of interfaces
				#		this SPOT supports
				print "${IF_SUPPORTED}=${platform_type} ${if_type}"

				# flag that we've created a boot image for this platform type
				[[ "${boot_image_created}" != *\ ${platform_type}=+([! ])\ * ]] &&
					eval boot_image_created=\"${boot_image_created} \
${platform_type}=\$enter_dbg_${kernel##*_} \"

			else

				# cache the error messages from BOSBOOT
				${CAT} ${ERR} >>${TMPDIR}/bosboot.errors 2>/dev/null

			fi

		done # for each platform type

	done # for each proto file

	# replace ERR file with cached error messages
	${CAT} ${TMPDIR}/bosboot.errors >${ERR} 2>/dev/null

	# preview or errors?
	if [[ -n "${preview}" ]]
	then

		# write the preview info
		preview_data "TRUE"

	elif [[ -z "${boot_image_created}" ]]
	then

		# no network boot images could be created
		# print the "missing" attr so that it gets added so the SPOT's definition
		print "missing=\"${MSG_MISSING_BOOTI}\""

		# display error message
		[[ -s ${ERR} ]] && err_from_cmd ${BOSBOOT} || error ${ERR_NO_BOOTI}

	elif [[ -s ${TMPDIR}/bosboot.errors ]]
	then

		warning_from_cmd ${BOSBOOT}

	fi

	if [[ -n "${debug}" ]] && [[ -n "${boot_image_created}" ]]
	then

		# print out the enter_dbg info
		for types in ${boot_image_created}
		do

			print "enter_dbg=\"${types%%=*} ${types##*=}\""

		done

	fi

} # end of mkbooti

#---------------------------- c_mkbooti         --------------------------------
#
# NAME: c_mkbooti
#
# FUNCTION:
#		creates a network boot image for each interface which has a proto file
#			in the SPOTname/usr/lib/boot/network directory
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
#		0							= boot images created; attr assignments on stdout
#		1							= failure
#
# OUTPUT:
#		writes IF_SUPPORTED attr assignments on stdout
#-------------------------------------------------------------------------------

# signal processing
trap cleanup 0
trap err_signal 1 2 11 15

# NIM initialization
nim_init

# initialize local variables
typeset c=""

# set parameters from command line
while getopts :a:pqv c
do
	case ${c} in

		a)		# validate the attr ass
				parse_attr_ass "${OPTARG}"

				# include the assignment for use in this environment
				eval ${variable}=\"${value}\"
		;;

		p)		# preview bosboot size requirements
				preview=TRUE
				cmd="${cmd} -q"
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

# create the boot images
mkbooti

# all done
exit 0

