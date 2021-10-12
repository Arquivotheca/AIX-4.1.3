#!/bin/ksh
# @(#)46        1.19  src/bos/usr/lib/nim/methods/c_installp.sh, cmdnim, bos41J, 9511A_all  3/3/95  15:02:46
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/methods/c_installp.sh
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

#---------------------------- module globals    --------------------------------
REQUIRED_ATTRS=""
OPTIONAL_ATTRS="lpp_source installp_flags filesets installp_bundle boot_env fix_bundle fixes"
typeset -i version=0
typeset -i release=0
lpp_source=""
installp_flags=""
filesets=""
installp_bundle=""
fileset_list=""

#---------------------------- prep_tape         --------------------------------
#
# NAME: prep_tape  
#
# FUNCTION:
#		prepares the specified tape device for use
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
#			lpp_source			= source device name
#			src_access			= local access point for the source device
#
# RETURNS: (int)
#		0							= success
#
# OUTPUT:
#-------------------------------------------------------------------------------
function prep_tape {

	# tape needs a "no-rewind-on-close" extension 
	# was one provided?
	if [[ ${lpp_source} = /dev/rmt[0-9].* ]]
	then

		[[ ${lpp_source} != /dev/rmt[0-9].1 ]] && \
			[[ ${lpp_source} != /dev/rmt[0-9].5 ]] && \
			error ${ERR_SOURCE} ${lpp_source}

		src_access=${lpp_source}

	else

		# append the extension
		src_access="${lpp_source}.5"

	fi

	# rewind the tape
	${TCTL} -f ${src_access} rewind 2>${ERR} || \
		err_from_cmd "${TCTL} -f ${src_access} rewind"

} # end of prep_tape

#---------------------------- prep_cd           --------------------------------
#
# NAME: prep_cd
#
# FUNCTION:
#		prepares a CDROM device as the source for a SPOT
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
#			1						= local mount point
#		global:
#			lpp_source			= source device name
#			src_access			= local access point for the source device
#			access_pnt			= local access point returned from nim_mount
#
# RETURNS: (int)
#		0							= success
#
# OUTPUT:
#-------------------------------------------------------------------------------
function prep_cd {

	typeset local_mntpnt=${1}

	# mount the CDROM at a specific offset
	nim_mount ${lpp_source} ${local_mntpnt}

	# images are actually at another offset
	src_access=${access_pnt}${OFFSET_FOR_CDROM}

} # end of prep_cd

#---------------------------- prep_dir         --------------------------------
#
# NAME: prep_dir       
#
# FUNCTION:
#		prepares environment to use a directory as the source for installp images
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
#			1						= local mount point
#		global:
#			src_access			= local access point for the source device
#			access_pnt			= local access point returned from nim_mount
#
# RETURNS: (int)
#		0							= success
#
# OUTPUT:
#-------------------------------------------------------------------------------
function prep_dir {

	typeset local_mntpnt=${1}

	# mount images at a specific offset
	nim_mount ${lpp_source} ${local_mntpnt}
	src_access=${access_pnt}

} # end of prep_dir

#---------------------------- prep_lpp_source       ----------------------------
#
# NAME: prep_lpp_source
#
# FUNCTION:
#		prepares the specified LPP source device for use
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
#			lpp_source			= source device name
#
# RETURNS: (int)
#		0							= success
#
# OUTPUT:
#-------------------------------------------------------------------------------
function prep_lpp_source {

	# what kind of source?
	case ${lpp_source} in

		/dev/*)	# tape or CDROM?
					if [[ ${lpp_source} = /dev/rmt[0-9]* ]]
					then
						prep_tape
					elif [[ ${lpp_source} = /dev/cd[0-9]* ]]
					then
						prep_cd
					else
						error ${ERR_SOURCE} ${lpp_source}
					fi
					;;

		*/*)		# source is a directory
					prep_dir
					;;

		*)			# unknown type
					error ${ERR_SYS} "unknown lpp_source type - \"${lpp_source}\""
					;;
	esac

} # end of prep_lpp_source

#*---------------------------- c_installp           ----------------------------
#
# NAME: c_installp
#
# FUNCTION:
#		sets up remote access to resources, then calls installp
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
typeset c=""

# set parameters from command line
while getopts :a:qv c
do
	case ${c} in

		a)    # validate the attr ass
				parse_attr_ass "${OPTARG}"

				# include the assignment for use in this environment
				eval ${variable}=\"${value}\"
		;;

		q)    # show attr info
				cmd_what
				exit 0
		;;

		v)    # verbose mode (for debugging)
				set -x
				for i in $(typeset +f)
				do
					typeset -ft $i
				done
		;;

		\?)   # unknown option
				error ${ERR_BAD_OPT} ${OPTARG}
		;;

	esac
done

# check for missing attrs
ck_attrs

# set defaults
installp_flags=${installp_flags:-${DEFAULT_INSTALLP_FLAGS}}
installp_flags=$( ck_installp_flags "${installp_flags}" )

# if boot_env specified...
if [[ -n "${boot_env}" ]]
then

	# make sure "-b" is used
	[[ "${installp_flags}" != *b* ]] && installp_flags="-b ${installp_flags}"

fi

# use an lpp_source?
if [[ -n "${lpp_source}" ]]
then

	# setup local access
	prep_lpp_source

	lpp_source="-d ${src_access}"

fi

# use a bundle file?
if [[ -n "${installp_bundle}" ]]
then

	# setup local access
	prep_bundle

	filesets="-f ${bundle_access}"

# use fix_bundle? 
elif [[ -n "${fix_bundle}" ]]
then

	# setup local access (use installp's bundle prep routine)
	installp_bundle=${fix_bundle}
	prep_bundle

	# convert list of fix keywords to list of filesets
	prep_instfix_lst "client" "bun" "${bundle_access}" "${lpp_source}"

	filesets="-f ${fileset_list}"

# use fixes? 
elif [[ -n "${fixes}" ]]
then

	# "update_all" keyword specified?
	if [ "${fixes}" = "update_all" ]
	then
		# get list of fileset updates needing installation.
		prep_updt_all_lst "client" "${lpp_source}" 

		if [ ! -s "${fileset_list}" ]
		then
			# print sm_inst error for "nothing to update"
			${INUUMSG} 177 > ${ERR} 2>&1
		 	err_from_cmd "c_installp"
		fi
		filesets="-f ${fileset_list}"
	else
		# convert list of fix keywords to list of filesets
	        prep_instfix_lst "client" "fixes" "${fixes}" "${lpp_source}"

		filesets="-f ${fileset_list}"
	fi

elif [[ ${installp_flags} = *u* ]] || [[ ${installp_flags} = *r* ]]
then

	# this is a deinstall operation: filesets must be specified
	[[ -z "${filesets}" ]] && error ${ERR_MISSING_ATTR} filesets

elif [[ ${installp_flags} != *C* ]]
then

	# this is an install operation: use default if filesets not specified
	filesets=${filesets:-${DEFAULT_INSTALLP_OPTIONS}}

fi

if [[ -n "${fixes}" ]] || [[ -n "${fix_bundle}" ]]
then
	# if not doing an update_all, add B flag for safety 
	# (so that base levels are not accidently instld via requisites)
	if [[ "${fixes}" != "update_all" ]] && [[ "${installp_flags}" != *B* ]]
	then
		installp_flags=${installp_flags}B
	fi
fi

# invoke installp
# NOTE that INUSERVERS is an environment variable which effects the behavior
#		of installp.  When set, it causes installp to bypass looking in the
#		/etc/niminfo file for the NIM_USR_SPOT variable, which is used to
#		prevent users from executing installp from the command line on machines
#		whose /usr filesystem has been converted into a SPOT
export INUSERVERS=yes

# if doing an installp preview, capture the output in ERR.  Cat the file
# to stdout if there were no errors, otherwise do normal error processing.
if [[ ${installp_flags} = *p* ]]
then
	${INSTALLP} ${installp_flags} ${lpp_source} ${filesets} >${ERR} 2>&1

 	if [ $? -eq 0 ]
	then
		cat ${ERR} 2>&1   # display the output
		>${ERR}           # wipe out all traces of errors
	else
		err_from_cmd ${INSTALLP}
	fi
else
	${INSTALLP} -e ${INSTALLP_LOG} ${installp_flags} ${lpp_source} \
		${filesets} 2>${ERR} 1>&2 || err_from_cmd ${INSTALLP}
fi

# success
exit 0
