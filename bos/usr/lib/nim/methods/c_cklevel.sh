#!/bin/ksh
# @(#)82	1.7  src/bos/usr/lib/nim/methods/c_cklevel.sh, cmdnim, bos411, 9428A410j  6/21/94  08:22:54
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/methods/c_cklevel.sh
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
typeset -i FUDGE_FACTOR=5

TAR_CREATE="${TAR} -cdpf - -C"
TAR_EXTRACT="${TAR} -xpf -"

#---------------------------- module globals    --------------------------------
REQUIRED_ATTRS="source type"
OPTIONAL_ATTRS=""
typeset -i version=0
typeset -i release=0
source=""
src_access=""
type=""

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
#
# RETURNS: (int)
#		0							= success
#
# OUTPUT:
#-------------------------------------------------------------------------------
function prep_tape {

	# tape needs a "no-rewind-on-close" extension 
	# was one provided?
	if [[ ${source} = /dev/rmt[0-9].* ]]
	then

		[[ ${source} != /dev/rmt[0-9].1 ]] && \
			[[ ${source} != /dev/rmt[0-9].5 ]] && \
			error ${ERR_SOURCE} ${source}

		src_access=${source}

	else

		# append the extension
		src_access="${source}.5"

	fi

	# rewind the tape
	${TCTL} -f ${src_access} rewind 2>${ERR} || err_from_cmd tctl

	# position to 4th record
	${TCTL} -f ${src_access} fsf 3 2>${ERR} || err_from_cmd tctl

	# need to validate the AIX release level of this image, so we need the
	# 		the LPP_NAME file - restore it now
	cd ${TMPDIR} 2>${ERR} || err_from_cmd cd
	${RESTORE} -xqf ${src_access} ${LPP_NAME} >/dev/NULL 2>${ERR} || \
		err_from_cmd "${RESTORE} -xqf ${src_access} ${LPP_NAME}"

	# validate the release level
	ck_rel_level ${TMPDIR}/${LPP_NAME}

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
#		global:
#
# RETURNS: (int)
#		0							= success
#
# OUTPUT:
#-------------------------------------------------------------------------------
function prep_cd {

	# mount the CDROM
	nim_mount ${source}
	src_access=${access_pnt}/${BOS_PATH_ON_CDROM}

	# need to validate the AIX release level of this image, so we need the
	# 		the LPP_NAME file - restore it now
	cd ${TMPDIR} 2>${ERR} || err_from_cmd cd
	${RESTORE} -xqf ${src_access} ${LPP_NAME} >/dev/null 2>${ERR} || \
		err_from_cmd ${RESTORE}

	# validate the release level
	ck_rel_level ${TMPDIR}/${LPP_NAME}

} # end of prep_cd

#---------------------------- prep_SPOT         --------------------------------
#
# NAME: prep_SPOT
#
# FUNCTION:
#		prepares an existing SPOT to be used as a source for the creation of the
#			new SPOT
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
function prep_SPOT {

	# make sure source pathname ends with "/usr"
	if [[ "${source##*/}" != usr ]]
	then
		source=source/usr
	fi

	# setup local access
	nim_mount ${source}
	src_access=${access_pnt}

	# validate the release level
	ck_rel_level ${src_access}/${LPP_NAME_IN_SPOT}

} # end of prep_SPOT

#---------------------------- prep_rte         --------------------------------
#
# NAME: prep_rte       
#
# FUNCTION:
#		prepares environment to use a file as the source
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
function prep_rte {

	typeset dir=${source%/*}
	typeset filename=${source##*/}

	# directory where rte exists must be local to the server
	${C_STAT} -a location=${dir} 2>${ERR} || err_from_cmd ${C_STAT}

	# need to validate the AIX release level of this image, so we need the
	# 		the LPP_NAME file - restore it now
	cd ${TMPDIR} 2>${ERR} || err_from_cmd cd
	${RESTORE} -xqf ${source} ${LPP_NAME} >/dev/null 2>${ERR} || \
		err_from_cmd ${RESTORE}

	# validate the release level
	ck_rel_level ${TMPDIR}/${LPP_NAME}

} # end of prep_rte

#---------------------------- ck_mksysb         ------------------------------
#
# NAME: ck_mksysb       
#
# FUNCTION:
#		verifies that the specified mksysb has an acceptable version/release
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
function ck_mksysb {

	typeset -i ok_version=${1:-${DEFAULT_REQUIRED_VERSION}}
	typeset -i ok_release=${2:-${DEFAULT_REQUIRED_RELEASE}}
	typeset dir=${source%/*}
	typeset filename=${source##*/}
	typeset datafile=${TMPDIR}/${IMAGE_DATA}
	typeset uname_info=""

	# directory where mksysb exists must be local to the server
	dir=${dir:-/}
	${C_STAT} -a location=${dir} 2>${ERR} || err_from_cmd ${C_STAT}

	# for mksysb images, the version/release info is in the image.data file
	# restore that file now
	cd ${TMPDIR} 2>${ERR} || err_from_cmd cd
	${RESTORE} -xqf ${source} ${IMAGE_DATA} >/dev/null 2>${ERR} || \
		err_from_cmd ${RESTORE}

	# IMAGE_DATA readable?
	[[ ! -r "${datafile}" ]] && error ${ERR_FILE_ACCESS} ${datafile}

	# UNAME_INFO specified in IMAGE_DATA?
	uname_info=$( ${AWK} '$1=="UNAME_INFO="{print}' ${datafile} 2>/dev/null )
	[[ -z "${uname_info}" ]] && \
		error ${ERR_INCOMPLETE} \
			"UNAME_INFO stanza missing from ${IMAGE_DATA} in ${source}"
#	error ${ERR_OBJ_MISSING_ATTR} UNAME_INFO ${IMAGE_DATA}

	# what's the version/release?
	# expecting a string like "UNAME_INFO= AIX <hostname> rel ver cpuid"
	set -- ${uname_info}
	[[ $# -lt 5 ]] && error ${ERR_VALUE} "${uname_info}" \
			"UNAME_INFO stanza from the ${IMAGE_DATA} file in ${source}"
	version=${5}
	release=${4}

	# make sure they're numeric
	[[ "${version}" != +([0-9]) ]] && \
		error ${ERR_RELEASE_LEVEL} "${version}" "${release}" "${source}"
	[[ "${release}" != +([0-9]) ]] && \
		error ${ERR_RELEASE_LEVEL} "${version}" "${release}" "${source}"

	# now check the values
	if (( ${version} < ${ok_version} ))
	then
		error ${ERR_RELEASE_LEVEL} ${version} ${release} "${source}"
	elif (( ${ok_release} > 0 ))
	then
		if (( ${release} < ${ok_release} ))
		then
			error ${ERR_RELEASE_LEVEL} ${version} ${release} "${source}"
		fi
	fi

} # end of ck_mksysb

#---------------------------- ck_source       --------------------------------
#
# NAME: ck_source
#
# FUNCTION:
#		prepares the specified source device for use
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
function ck_source {

	# what kind of source?
	case ${type} in

		device)	# tape or CDROM?
					if [[ ${source} = /dev/rmt[0-9]* ]]
					then
						prep_tape
					elif [[ ${source} = /dev/cd[0-9]* ]]
					then
						prep_cd
					else
						error ${ERR_SOURCE} ${source}
					fi
					;;

		spot)		# source is another SPOT
					prep_SPOT
					;;

		rte)		# source is a BOS rte (runtime) image
					prep_rte
					;;

		mksysb)	# source is a mksysb (system backup) image
					ck_mksysb
					;;

		*)			# unknown type
					error ${ERR_SYS} "unknown source type - \"${type}\""
					;;
	esac

} # end of ck_source

#---------------------------- c_cklevel         --------------------------------
#
# NAME: c_cklevel
#
# FUNCTION:
#		examines the AIX version/release level for the specified source
#		if it is at an acceptable level to NIM, this program will exit with 0;
#			otherwise, it prints an error message and exits non-zero
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
#		0							= version/release ok
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

# check the specified source
ck_source

# if we get this far, level is ok
# return the version & release on stdout as attr assignments so the calling
#		method will pick them up
print "version=${version}"
print "release=${release}"

# all done
exit 0

